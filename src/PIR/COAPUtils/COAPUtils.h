#ifndef COAP_UTILS_H /*== INCLUDE ==*/
#define COAP_UTILS_H /*=== GUARD ===*/

#include "COAPConnectivityImports.h"
#include <ArduinoJson.h>
#include <Preferences.h>
#include "PIR/PreferencesUtils/PreferencesUtils.h"
#include "PIR/MQTTUtils/MQTTUtils.h"

#define COAP_LOOP_DELAY 1000
#define ALARM_INIT_ATTEMPT_DELAY 500

void COAPResponse(CoapPacket &packet, IPAddress ip, int port);
void COAPSetTier(CoapPacket &packet, IPAddress ip, int port);
void COAPSetGroup(CoapPacket &packet, IPAddress ip, int port);
void COAPActivateSensor(CoapPacket &packet, IPAddress ip, int port);
#endif