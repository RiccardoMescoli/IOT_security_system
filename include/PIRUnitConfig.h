#ifndef PIR_UNIT_CONFIG_H /*== INCLUDE ==*/
#define PIR_UNIT_CONFIG_H /*=== GUARD ===*/

#include "IoTConnectivityImports.h"
#include "PIR/PreferencesUtils/PreferencesUtils.h"
#include "WiFiUtils/WiFiUtils.h"
#include "PIR/MQTTUtils/MQTTUtils.h"
#include "PIR/COAPUtils/COAPUtils.h"

// TODO: VERIFICA NON VENGANO CARICATE LE LIB DEL CONTROLLER
// LA PIR UNIT HA LE SUE

#if CONFIG_FREERTOS_UNICORE
    #define ARDUINO_RUNNING_CORE 0
#else
    #define ARDUINO_RUNNING_CORE 1
#endif

#define DEFAULT_TIER 0
#define DEFAULT_GROUP 'H'

#define SENS_ACT_PERIOD_DURATION_MS 30000

#define SENS_ACT_PIN 27 // SETTED TO A TEMPORARY RANDOM PIN
#define PIR_OUTPUT_PIN 39

#endif