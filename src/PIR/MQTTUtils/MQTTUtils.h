#ifndef MQTT_UTILS_H /*== INCLUDE ==*/
#define MQTT_UTILS_H /*=== GUARD ===*/

#include <map>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "SensDataStructures.h"
#include "MQTTConnectivityImports.h"
#include "MQTTTopics.h"

#define CONNECT_JSON_CAPACITY JSON_OBJECT_SIZE(5) + JSON_STRING_SIZE(IP_STRING_SIZE)
#define LAST_WILL_JSON_CAPACITY JSON_OBJECT_SIZE(3) + JSON_STRING_SIZE(IP_STRING_SIZE)
#define ALARM_STATUS_JSON_CAPACITY JSON_OBJECT_SIZE(1) + JSON_STRING_SIZE(11)
#define ALARM_TRIGGERED_JSON_CAPACITY JSON_OBJECT_SIZE(1)
#define SENSOR_NAME "PIR"

void MQTTConnect(void);

uint16_t MQTTPubConnectMsg(void);

void onMQTTConnect(bool sessionPresent);

void onMQTTDisconnect(AsyncMqttClientDisconnectReason reason);

void onMQTTSubscribe(uint16_t packetId, uint8_t qos);

void onMQTTUnsubscribe(uint16_t packetId);

void onMQTTMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);

void onMQTTPublish(uint16_t packetId);

void MQTTSetLastWill(void);

#endif