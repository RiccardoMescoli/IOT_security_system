#ifndef CONTROLLER_UNIT_CONFIG_H /*== INCLUDE ==*/
#define CONTROLLER_UNIT_CONFIG_H /*=== GUARD ===*/

#include "IoTConnectivityImports.h"
#include "WiFiUtils/WiFiUtils.h"
#include "controller/MQTTUtils/MQTTUtils.h"
#include "controller/COAPUtils/COAPUtils.h"

#if CONFIG_FREERTOS_UNICORE
    #define ARDUINO_RUNNING_CORE 0
#else
    #define ARDUINO_RUNNING_CORE 1
#endif

/* ---------- Pin Definitions ------------- */
#define ALARM_LED_PIN 12
#define ALARM_BUZZER_PIN 13

#define SEM_RED_PIN 27
#define SEM_YEL_PIN 26
#define SEM_GRE_PIN 25

#define DEBUG_BUTTON_PIN 36 // Necessary only for debug purposes

/* ------------ Alarm config -------------- */
#define ACTIVATION_DELAY_MS 30000

/* --------- Period Definitions ----------- */
#define CoordinatorPeriod 1000

/* --------- Delay Definitions ------------ */
#define AlarmLEDDelay 2000
#define AlarmBuzzerDelay 3000

/* ------- Priorities Definitions --------- */
#define COORDINATOR_PRIORITY 2
#define ALARM_LED_PRIORITY 3
#define ALARM_BUZZER_PRIORITY 3
#define COAP_LOOP_PRIORITY 1

#define MQTT_ADV_FREQUENCY_MS 60000

#endif
