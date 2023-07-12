#ifndef ULTRASOUND_UNIT_CONFIG_H /*== INCLUDE ==*/
#define ULTRASOUND_UNIT_CONFIG_H /*=== GUARD ===*/

#include <map>
#include <NewPing.h> 
#include "IoTConnectivityImports.h"
#include "SensDataStructures.h"
#include "ultrasound/PreferencesUtils/PreferencesUtils.h"
#include "WiFiUtils/WiFiUtils.h"
#include "ultrasound/MQTTUtils/MQTTUtils.h"
#include "ultrasound/COAPUtils/COAPUtils.h"

// TODO: VERIFICA NON VENGANO CARICATE LE LIB DEL CONTROLLER
// LA ULTRASOUND UNIT HA LE SUE

#if CONFIG_FREERTOS_UNICORE
    #define ARDUINO_RUNNING_CORE 0
#else
    #define ARDUINO_RUNNING_CORE 1
#endif

#define DEFAULT_TIER 1
#define DEFAULT_GROUP 'H'

#define SENS_ACT_PERIOD_DURATION_MS 60000
#define MQTT_ADV_FREQUENCY_MS 60000
#define US_PING_MAX_DISTANCE 400
#define MAX_LEN_REC_DELAY 10000
#define MIN_HEIGHT_TO_TRIGGER 100

#define SENS_ACT_PIN 27
#define SENS_TRIGGER_PIN 26 // Trigger
#define US_ECHO_PIN 39 // Echo

#endif