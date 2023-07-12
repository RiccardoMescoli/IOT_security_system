#include<Arduino.h>
#include "UltrasoundUnitConfig.h"

/* ---------- MQTT connectivity ----------- */
char local_ip[IP_STRING_SIZE] = NULL_IP;

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t mqttAdvertiseTimer;

char mqtt_broker_ip[IP_STRING_SIZE] = NULL_IP;
uint16_t mqtt_broker_port = 1883;

/* ---------- COAP connectivity ----------- */
WiFiUDP udp;
Coap coap(udp);
char controller_ip[IP_STRING_SIZE] = NULL_IP;

/* ------------- Preferences -------------- */
Preferences preferences;
uint8_t tier = DEFAULT_TIER;
char group = DEFAULT_GROUP;

/* ----------- Sensor handling ------------ */
TimerHandle_t stopSensorTimer;
bool sensorToActivate = false;
bool sensorActive = false;
bool sensorTriggered = false;

/* ------- Controller & Alarm flags ------- */
bool controllerOnline = false;
bool alarmActive = false;

/* ---------- Sensor new ping obj --------- */
NewPing sonar(SENS_TRIGGER_PIN, US_ECHO_PIN, US_PING_MAX_DISTANCE);
uint16_t recorded_distance = US_PING_MAX_DISTANCE;
uint16_t max_distance_recorded = US_PING_MAX_DISTANCE;
std::map<String, sens_inf> sensor_dict;

/* --------- Function Prototypes ---------- */
// Arduino
void setup(void);
void loop(void);
void StopSensor(void);
void TriggerResponse(void);

void setup(void){
    Serial.begin(115200);
    WiFi.mode(WIFI_AP_STA);
    WiFi.onEvent(WiFiEvent);

    // Dyn. config. setup
    dynConfigSetUp(); 

    // MQTT config
    mqttReconnectTimer = xTimerCreate("mqttRecTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(MQTTConnect));
    mqttAdvertiseTimer = xTimerCreate("mqttAdvTimer", 
                                      pdMS_TO_TICKS(MQTT_ADV_FREQUENCY_MS), 
                                      pdFALSE, (void*)0, 
                                      reinterpret_cast<TimerCallbackFunction_t>(MQTTPubConnectMsg)
                                      );
    stopSensorTimer = xTimerCreate("stopSensorTimer", 
                                    pdMS_TO_TICKS(SENS_ACT_PERIOD_DURATION_MS), 
                                    pdFALSE, (void*)0, 
                                    reinterpret_cast<TimerCallbackFunction_t>(StopSensor)
                                    );
    mqttClient.onConnect(onMQTTConnect);
    mqttClient.onDisconnect(onMQTTDisconnect);
    mqttClient.onSubscribe(onMQTTSubscribe);
    mqttClient.onUnsubscribe(onMQTTUnsubscribe);
    mqttClient.onMessage(onMQTTMessage);
    mqttClient.onPublish(onMQTTPublish);

    // COAP config
    coap.server(COAPSetGroup, "setgroup");
    coap.server(COAPSetTier, "settier");
    coap.server(COAPActivateSensor, "activate");
    coap.response(COAPResponse);
    coap.start(5683);

    // WiFi config
    WiFiConnect();

    // MQTT setting up the server
    mqttClient.setServer(mqtt_broker_ip, mqtt_broker_port);

    pinMode(SENS_ACT_PIN, OUTPUT);
    pinMode(US_ECHO_PIN, INPUT);

    digitalWrite(SENS_ACT_PIN, HIGH);
    delay(MAX_LEN_REC_DELAY);
    max_distance_recorded = sonar.ping_cm();
    Serial.print("Max distance recorded: ");
    Serial.print(max_distance_recorded);
    Serial.println("cm");

    if(tier == 0){
        sensorActive = true;
    } else {
        digitalWrite(SENS_ACT_PIN, LOW);
    }
}

void loop(void){
    // Empty. Things are done in Tasks. 
    coap.loop();

    if(!sensorActive && sensorToActivate){
        sensorToActivate = false;
        digitalWrite(SENS_ACT_PIN, HIGH);
        sensorActive = true;
        xTimerStart(stopSensorTimer, 0);
        Serial.println(" - Sensor ACTIVATED!");
    } else if (sensorActive) {
        recorded_distance = sonar.ping_cm();
        int16_t recorded_height = (max_distance_recorded - recorded_distance);
        if(recorded_height > MIN_HEIGHT_TO_TRIGGER){
            Serial.println("Sensor triggered!!");
            Serial.print("Height recorded:");
            Serial.print(recorded_height);
            Serial.println("cm");
            TriggerResponse();
        }
    }

    vTaskDelay(1000/portTICK_PERIOD_MS);
}

void StopSensor(void){
    sensorActive = false;
    sensorToActivate = false;
    digitalWrite(SENS_ACT_PIN, LOW);
    Serial.println(" - Sensor DEACTIVATED!");
}

void TriggerResponse(void){

    uint8_t activated = 0;
    for(std::map<String, sens_inf>::iterator it = sensor_dict.begin(); it != sensor_dict.end(); it++) {
        if(it->second.group == group && it->second.tier == tier+1){
            IPAddress ip_dest;
            String ip_dest_str = it->first;
            ip_dest.fromString(ip_dest_str);
            int msgid = coap.get(ip_dest, 5683, "activate");
            if(msgid != 0){
                activated++;
                Serial.print("Sensor activated - msg ID:");
                Serial.println(msgid);
            } else {
                Serial.print("Sensor unreachable:");
                Serial.println(ip_dest_str);
            }
        }
    }
    Serial.print(" --- AMOUNT OF ACTIVATED SENSORS: ");
    Serial.println(activated);

    if(activated == 0 && controllerOnline && alarmActive){
        Serial.println(" --- TRIGGERING THE ALARM !!!");
        IPAddress ip_dest;
        ip_dest.fromString(controller_ip);
        int msgid = coap.get(ip_dest, 5683, "trigger");

        if(msgid != 0){
            Serial.print("Alarm trigger request succeded - msg ID:");
            Serial.println(msgid);
        } else {
            Serial.print("Alarm trigger failed:");
            Serial.println(controller_ip);
        }
    }
}