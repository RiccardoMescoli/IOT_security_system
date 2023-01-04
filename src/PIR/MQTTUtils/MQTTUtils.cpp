#include "MQTTUtils.h"

extern AsyncMqttClient mqttClient;
extern TimerHandle_t mqttReconnectTimer;
extern char local_ip[IP_STRING_SIZE];
extern uint8_t tier;
extern char group;

void MQTTConnect(void){
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
}

uint16_t MQTTPubConnectMsg(void){
    char json_payload[256];

    StaticJsonDocument<CONNECT_JSON_CAPACITY> connect_json;
    connect_json["name"] = "PIR";
    connect_json["status"] = "ONLINE";
    connect_json["ip"] = local_ip;
    connect_json["group"] = group;
    connect_json["tier"] = tier;


    serializeJson(connect_json, json_payload);
    uint16_t packetIdPub = mqttClient.publish(SENSORS_STATUS_TOPIC, 
                                               1, 
                                               true, 
                                               json_payload
                                               );
    return packetIdPub;
}

void onMQTTConnect(bool sessionPresent) {
    Serial.println("Connected to MQTT.");
    Serial.print("Session present: ");
    Serial.println(sessionPresent);

    uint16_t packetIdPub3 = MQTTPubConnectMsg();
    
    Serial.print("Publishing at QoS 1, packetId: ");
    Serial.println(packetIdPub3);
}

void onMQTTDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.println("Disconnected from MQTT.");

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
    Serial.print("  dup: ");
    Serial.println(properties.dup);
    Serial.print("  retain: ");
    Serial.println(properties.retain);
    Serial.print("  len: ");
    Serial.println(len);
    Serial.print("  index: ");
    Serial.println(index);
    Serial.print("  total: ");
    Serial.println(total);
}

void onMQTTPublish(uint16_t packetId) {
    Serial.println("Publish acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

void MQTTSetLastWill(void){
    StaticJsonDocument<LAST_WILL_JSON_CAPACITY> last_will_json;
    last_will_json["name"] = "PIR";
    last_will_json["status"] = "OFFLINE";
    last_will_json["ip"] = local_ip;
    
    char json_payload[256];
    serializeJson(last_will_json, json_payload);
    mqttClient.setWill(SENSORS_STATUS_TOPIC, 1, true, json_payload);
}