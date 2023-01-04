#ifndef COAP_UTILS_H /*== INCLUDE ==*/
#define COAP_UTILS_H /*=== GUARD ===*/

#include "COAPConnectivityImports.h"
#include <ArduinoJson.h>

#define COAP_LOOP_DELAY 1000
#define ALARM_INIT_ATTEMPT_DELAY 500

void COAPResponse(CoapPacket &packet, IPAddress ip, int port);
void COAPSwitchAlarmRequest(CoapPacket &packet, IPAddress ip, int port);
void COAPAlarmTrigger(CoapPacket &packet, IPAddress ip, int port);
#endif