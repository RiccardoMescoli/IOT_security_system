#include<Arduino.h>
#include "PIRUnitConfig.h"

/* ---------- MQTT connectivity ----------- */
char local_ip[IP_STRING_SIZE] = NULL_IP;

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;

char mqtt_broker_ip[IP_STRING_SIZE] = NULL_IP;
uint16_t mqtt_broker_port = 1883;

/* ---------- COAP connectivity ----------- */
WiFiUDP udp;
Coap coap(udp);

/* ------------- Preferences -------------- */
Preferences preferences;
uint8_t tier = DEFAULT_TIER;
char group = DEFAULT_GROUP;

/* ----------- Sensor handling ------------ */
TimerHandle_t stopSensorTimer;
bool sensorToActivate = false;
bool sensorActive = false;
bool sensorTriggered = false;

/* --------- Function Prototypes ---------- */
// Arduino
void setup(void);
void loop(void);
void StopSensor(void);

void setup(void){
    Serial.begin(115200);
    WiFi.mode(WIFI_AP_STA);
    WiFi.onEvent(WiFiEvent);

    // Dyn. config. setup
    dynConfigSetUp(); 

    // MQTT config
    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(MQTTConnect));
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
    coap.start();

    // WiFi config
    WiFiConnect();

    // MQTT setting up the server
    mqttClient.setServer(mqtt_broker_ip, mqtt_broker_port);

    pinMode(SENS_ACT_PIN, OUTPUT);
    if(tier == 0){
        digitalWrite(SENS_ACT_PIN, HIGH);
        sensorActive = true;
    }
    pinMode(PIR_OUTPUT_PIN, INPUT);
}

void loop(void){
    // Empty. Things are done in Tasks. 
    coap.loop();

    if(!sensorActive && tier > 0 && sensorToActivate){
        sensorToActivate = false;
        digitalWrite(SENS_ACT_PIN, HIGH);
        sensorActive = true;
        xTimerStart(stopSensorTimer, 0);
        Serial.println(" - Sensor ACTIVATED!");
    } else if (sensorActive) {
        sensorTriggered = digitalRead(PIR_OUTPUT_PIN);
        if(sensorTriggered)
            Serial.println("Sensor triggered!!");
    }

    vTaskDelay(1000/portTICK_PERIOD_MS);
}

void StopSensor(void){
    sensorActive = false;
    digitalWrite(SENS_ACT_PIN, LOW);
    Serial.println(" - Sensor DEACTIVATED!");
}