#ifndef WIFI_UTILS_H /*== INCLUDE ==*/
#define WIFI_UTILS_H /*=== GUARD ===*/
#include <Arduino.h>
#include "WiFiConnectivityImports.h"

#ifdef IS_CONTROLLER
    #include "controller/MQTTUtils/MQTTUtils.h"
#elif defined(IS_PIR)
    #include "PIR/MQTTUtils/MQTTUtils.h"
#endif

#define WIFI_CONNECT_MAX_TRIES 10
#define WIFI_CONNECTION_SUCCESS_CHECKS_DELAY_MS 1000
#define WIFI_SMARTCONFIG_SUCCESS_CHECKS_DELAY_MS 1000

bool WiFiVerifyConnectionSuccess(void);
void WiFiSmartConfig(void);
void WiFiConnect(void);
void WiFiEvent( WiFiEvent_t event  );

// MDNS
void MDNSSetup(void);
void MDNSQueryForMQTTBroker(void);

#endif