#include <Arduino.h>
#include "ControllerUnitConfig.h"

/* ----------- Task Handles --------------- */
TaskHandle_t task_handle_Coordinator = NULL;
static TaskHandle_t task_handle_Alarm_LED = NULL;
static TaskHandle_t task_handle_Alarm_Buzzer = NULL;
static TaskHandle_t task_handle_COAP_loop = NULL;

/* -------- Alarm Suspension Flags -------- */
bool alarm_active = false;
bool alarm_to_startup = true;

/* ------ Actuator Suspension Flags ------- */
bool alarm_triggered = false;
bool alarm_LED_ON = false;
bool alarm_buzzer_ON = false;

/* ---------- Alarm status Falgs ---------- */
char alarm_status[11] = "OFF";
bool alarm_act_init_success = false;

/* ---------- MQTT connectivity ----------- */
char local_ip[IP_STRING_SIZE] = NULL_IP;

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;

char mqtt_broker_ip[IP_STRING_SIZE] = NULL_IP;
uint16_t mqtt_broker_port = 1883;

/* ------- Task Function Prototypes ------- */
void TaskCoordinator( void *pvParameters );
void TaskAlarmLED( void *pvParameters );
void TaskAlarmBuzzer( void *pvParameters );
void TaskCoapLoop( void *pvParameters );

/* --------- Function Prototypes ---------- */
// Arduino
void setup(void);
void loop(void);

// Utils
void StartActuators(void);

/* ---------- COAP connectivity ----------- */
WiFiUDP udp;
Coap coap(udp);

void setup(void) {
    Serial.begin(115200);
    WiFi.mode(WIFI_AP_STA);
    WiFi.onEvent(WiFiEvent);
    
    // MQTT config
    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(MQTTConnect));
    mqttClient.onConnect(onMQTTConnect);
    mqttClient.onDisconnect(onMQTTDisconnect);
    mqttClient.onSubscribe(onMQTTSubscribe);
    mqttClient.onUnsubscribe(onMQTTUnsubscribe);
    mqttClient.onMessage(onMQTTMessage);
    mqttClient.onPublish(onMQTTPublish);

    // COAP config
    coap.server(COAPSwitchAlarmRequest, "switch");
    coap.server(COAPAlarmTrigger, "trigger");
    coap.response(COAPResponse);
    coap.start();

    // WiFi config
    WiFiConnect();

    // MQTT setting up the server
    mqttClient.setServer(mqtt_broker_ip, mqtt_broker_port);

    // Pin setup
    pinMode(ALARM_LED_PIN, OUTPUT);
    pinMode(ALARM_BUZZER_PIN, OUTPUT);

    pinMode(SEM_RED_PIN, OUTPUT);
    pinMode(SEM_YEL_PIN, OUTPUT);
    pinMode(SEM_GRE_PIN, OUTPUT);

    pinMode(DEBUG_BUTTON_PIN, INPUT); // Necessary only for debug purposes

    xTaskCreatePinnedToCore(
    TaskCoordinator
    ,  "TaskCoordinator"   // A name just for humans
    ,  ( 2048 + 128 )  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  COORDINATOR_PRIORITY  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &task_handle_Coordinator 
    ,  ARDUINO_RUNNING_CORE);

    xTaskCreatePinnedToCore(
    TaskCoapLoop
    ,  "TaskCoapLoop"   // A name just for humans
    ,  ( 2048 + 128 )  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  COAP_LOOP_PRIORITY  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &task_handle_COAP_loop 
    ,  ARDUINO_RUNNING_CORE);

    xTaskCreatePinnedToCore(
    TaskAlarmLED
    ,  "TaskAlarmLED"   // A name just for humans
    ,  ( 512 + 128 )  
    ,  NULL
    ,  ALARM_LED_PRIORITY  
    ,  &task_handle_Alarm_LED 
    ,  ARDUINO_RUNNING_CORE);

    xTaskCreatePinnedToCore(
    TaskAlarmBuzzer
    ,  "TaskAlarmBuzzer"   // A name just for humans
    ,  ( 512 + 128 ) 
    ,  NULL
    ,  ALARM_BUZZER_PRIORITY
    ,  &task_handle_Alarm_Buzzer
    ,  ARDUINO_RUNNING_CORE);
}
void loop(void) {
    // Empty. Things are done in Tasks.
}

// **** Utils ****
void StartActuators(void){
    if(!alarm_LED_ON && !alarm_buzzer_ON){
        vTaskResume(task_handle_Alarm_LED);
        vTaskResume(task_handle_Alarm_Buzzer);
    } else if(!alarm_LED_ON){
        vTaskResume(task_handle_Alarm_LED);
    }  else if(!alarm_buzzer_ON){
        vTaskResume(task_handle_Alarm_Buzzer);
    } 
}

// **** Tasks ****

void TaskCoordinator(void *pvParameters)
{
    (void) pvParameters;

    /*
        Task Coordinator
    */

    TickType_t xLastActivationTime = xTaskGetTickCount();
    digitalWrite(SEM_GRE_PIN, HIGH);

    for(;;){
        vTaskDelayUntil(&xLastActivationTime, CoordinatorPeriod / portTICK_PERIOD_MS);

        if(alarm_active && !alarm_to_startup && alarm_triggered
           && (!alarm_LED_ON || !alarm_buzzer_ON)) // Trigger the alarm
        {  
            publishAlarmTriggerStatus();
            StartActuators();
            
        } else if (!alarm_active){ // Turn off the alarm
            digitalWrite(SEM_RED_PIN, LOW);
            digitalWrite(SEM_GRE_PIN, HIGH);
            strcpy(alarm_status, "OFF");
            publishAlarmStatus();
            publishAlarmTriggerStatus();
            vTaskSuspend( NULL );
            alarm_act_init_success = true;
        }

        if(alarm_to_startup){
            strcpy(alarm_status, "ACTIVATING");
            publishAlarmStatus();
            digitalWrite(SEM_GRE_PIN, LOW);
            digitalWrite(SEM_YEL_PIN, HIGH);
            vTaskDelay(ACTIVATION_DELAY_MS / portTICK_PERIOD_MS); // Il delay si potrebbe rendere configurabile da node-red
            strcpy(alarm_status, "ON");
            publishAlarmStatus();
            digitalWrite(SEM_YEL_PIN, LOW);
            digitalWrite(SEM_RED_PIN, HIGH);
            alarm_to_startup = false;
        }

    }    
 }

void TaskAlarmLED(void *pvParameters){
    (void) pvParameters;

    /*
        Task LED
    */

    // When first created, this task is suspended until it's resumed by the Coordinator
    vTaskSuspend( NULL );
    alarm_LED_ON = true;

    for(;;){
        digitalWrite(ALARM_LED_PIN, HIGH);
        vTaskDelay(AlarmLEDDelay / portTICK_PERIOD_MS);
        digitalWrite(ALARM_LED_PIN, LOW);
        vTaskDelay(AlarmLEDDelay / portTICK_PERIOD_MS);

        if(!alarm_triggered)
        {
            alarm_LED_ON = false;
            vTaskSuspend( NULL );
            alarm_LED_ON = true;
        }
    } 
}

void TaskAlarmBuzzer(void *pvParameters){
    (void) pvParameters;

    /*
        Task Buzzer
    */

    // When first created, this task is suspended until it's resumed by the Coordinator
    vTaskSuspend( NULL );
    alarm_buzzer_ON = true;

    for(;;){
        digitalWrite(ALARM_BUZZER_PIN, HIGH);
        vTaskDelay(AlarmBuzzerDelay / portTICK_PERIOD_MS);
        digitalWrite(ALARM_BUZZER_PIN, LOW);
        vTaskDelay(AlarmBuzzerDelay / portTICK_PERIOD_MS);

        if(!alarm_triggered)
        {
            alarm_buzzer_ON = false;
            vTaskSuspend( NULL );
            alarm_buzzer_ON = true;
        }
    } 
}

void TaskCoapLoop( void *pvParameters ){
    (void) pvParameters;

    for(;;){
        vTaskDelay(COAP_LOOP_DELAY);
        coap.loop();
    }
}
