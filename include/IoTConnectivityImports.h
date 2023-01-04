#ifndef IOT_CONNECTIVITY_IMPORTS_H /*== INCLUDE ==*/
#define IOT_CONNECTIVITY_IMPORTS_H /*=== GUARD ===*/

#include <WiFi.h>
#include <ESPmDNS.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include <coap-simple.h>

#ifndef IP_STRING_SIZE
    #define IP_STRING_SIZE 16
#endif
#ifndef NULL_IP
    #define NULL_IP "000.000.000.000"
#endif

#endif
