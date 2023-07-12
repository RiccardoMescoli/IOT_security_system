#include "MQTTUtils.h"

extern AsyncMqttClient mqttClient;
extern TimerHandle_t mqttReconnectTimer;
extern TimerHandle_t mqttAdvertiseTimer;
extern char local_ip[IP_STRING_SIZE];
extern char controller_ip[IP_STRING_SIZE];
extern uint8_t tier;
extern char group;
extern bool controllerOnline;
extern bool alarmActive;

extern std::map<String, sens_inf> sensor_dict;

void MQTTConnect(void){
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
}

uint16_t MQTTPubConnectMsg(void){
    char json_payload[256];
    //uint8_t test = group;

    StaticJsonDocument<CONNECT_JSON_CAPACITY> connect_json;
    connect_json["name"] = SENSOR_NAME;
    connect_json["status"] = "ONLINE";
    connect_json["ip"] = local_ip;
    connect_json["group"] = (uint8_t) group;
    connect_json["tier"] = tier;


    Serial.println("-*Advertising status*-");
    serializeJson(connect_json, json_payload);
    uint16_t packetIdPub = mqttClient.publish(SENSORS_STATUS_TOPIC, 
                                               1, 
                                               true, 
                                               json_payload
                                               );

    xTimerStart(mqttAdvertiseTimer, 0);

    return packetIdPub;
}

void onMQTTConnect(bool sessionPresent) {
    Serial.println("Connected to MQTT.");
    Serial.print("Session present: ");
    Serial.println(sessionPresent);

    uint16_t packetIdPub = MQTTPubConnectMsg();
    
    Serial.print("Publishing at QoS 1, packetId: ");
    Serial.println(packetIdPub);

    uint16_t packetIdSub1 = mqttClient.subscribe(SENSORS_STATUS_TOPIC, 1);
    Serial.print("Subscribing at QoS 1, packetId: ");
    Serial.println(packetIdSub1);

    uint16_t packetIdSub2 = mqttClient.subscribe(CONTROLLER_STATUS_TOPIC, 1);
    Serial.print("Subscribing at QoS 1, packetId: ");
    Serial.println(packetIdSub2);

    uint16_t packetIdSub3 = mqttClient.subscribe(ALARM_STATUS_TOPIC, 1);
    Serial.print("Subscribing at QoS 1, packetId: ");
    Serial.println(packetIdSub3);
}

void onMQTTDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.println("Disconnected from MQTT.");

    xTimerStop(mqttAdvertiseTimer, 0);
    if (WiFi.isConnected()) {
        xTimerStart(mqttReconnectTimer, 0);
    }
}

void onMQTTSubscribe(uint16_t packetId, uint8_t qos) {
    Serial.println("Subscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
    Serial.print("  qos: ");
    Serial.println(qos);
}

void onMQTTUnsubscribe(uint16_t packetId) {
    Serial.println("Unsubscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

void onMQTTMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    Serial.println("Publish received.");
    Serial.print("  topic: ");
    Serial.println(topic);
    Serial.print("  qos: ");
    Serial.println(properties.qos);
    
    if(strcmp(topic, SENSORS_STATUS_TOPIC) == 0){
        StaticJsonDocument<CONNECT_JSON_CAPACITY> doc;
        DeserializationError err = deserializeJson(doc, payload);
        if(err){
            Serial.print("DESERIALIZATION FAILED: ");
            Serial.println(err.c_str());
        } else if (strcmp(doc["ip"], local_ip) != 0) {
            int test = doc["group"];
            sensor_dict[doc["ip"]].group = (char) test;
            sensor_dict[doc["ip"]].tier = doc["tier"];

            
            Serial.println((const char*) doc["ip"]);
            Serial.println(sensor_dict[doc["ip"]].group);
            Serial.println(sensor_dict[doc["ip"]].tier);
        }
    } else if (strcmp(topic, CONTROLLER_STATUS_TOPIC) == 0){
        StaticJsonDocument<CONNECT_JSON_CAPACITY> doc;
        DeserializationError err = deserializeJson(doc, payload);
        if(err){
            Serial.print("DESERIALIZATION FAILED: ");
            Serial.println(err.c_str());
        } else {
            if(strcmp(doc["status"], "ONLINE") == 0){
                strcpy(controller_ip, doc["ip"]);
                controllerOnline = true;
                Serial.println("## DETECTED: CONTROLLER ONLINE ##");
            } else {
                controllerOnline = false;
                Serial.println("## DETECTED: CONTROLLER OFFLINE ##");
            }
        }
    } else if (strcmp(topic, ALARM_STATUS_TOPIC) == 0){
        StaticJsonDocument<CONNECT_JSON_CAPACITY> doc;
        DeserializationError err = deserializeJson(doc, payload);
        if(err){
            Serial.print("DESERIALIZATION FAILED: ");
            Serial.println(err.c_str());
        } else {
            if(strcmp(doc["alarm_status"], "ON") == 0){
                alarmActive = true;
                Serial.println("## DETECTED: ALARM ON ##");
            } else if (strcmp(doc["alarm_status"], "OFF") == 0) {
                alarmActive = false;
                Serial.println("## DETECTED: ALARM OFF ##");
            } else {
                Serial.println("## DETECTED: ALARM ACTIVATING ##");
            }
        }
    }
}

void onMQTTPublish(uint16_t packetId) {
    Serial.println("Publish acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

void MQTTSetLastWill(void){
    StaticJsonDocument<LAST_WILL_JSON_CAPACITY> last_will_json;
    last_will_json["name"] = SENSOR_NAME;
    last_will_json["status"] = "OFFLINE";
    last_will_json["ip"] = local_ip;
    
    char json_payload[256];
    serializeJson(last_will_json, json_payload);
    mqttClient.setWill(SENSORS_STATUS_TOPIC, 1, true, json_payload);
}
