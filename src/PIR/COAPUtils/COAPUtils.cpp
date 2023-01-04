#include "COAPUtils.h"

extern Preferences preferences;
extern uint8_t tier;
extern char group;
extern TaskHandle_t task_handle_Coordinator;
extern Coap coap;
extern bool sensorActive;
extern bool sensorToActivate;


void COAPResponse(CoapPacket &packet, IPAddress ip, int port){
    Serial.println("[Coap Response got]");
  
    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = '\0';
    
    Serial.println(p);
}

void COAPSetTier(CoapPacket &packet, IPAddress ip, int port){
    preferences.begin(DYNAMIC_CONFIG_STORAGE_NAME, false);

    Serial.println("DYN CONFIG: SETTING TIER");
    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = '\0';

    Serial.println(p);

    tier = (uint8_t) strtoul(p, NULL, 10);
    preferences.putUChar(TIER_STOR_KEY, tier);
    Serial.println("DYN CONFIG: STORED NEW TIER VALUE");
    preferences.end();

    if(tier > 0)
    {
        sensorActive = false;
        Serial.println(" - Sensor DEACTIVATED!");
    } else {
        sensorActive = true;
        Serial.println(" - Sensor ACTIVATED!");
    }

    coap.sendResponse(ip, 
                      port, \
                      packet.messageid, 
                      p, strlen(p), 
                      COAP_CONTENT, 
                      COAP_TEXT_PLAIN, 
                      packet.token, packet.tokenlen
                      );
    Serial.println("RESPONSE SENT");
    MQTTPubConnectMsg();

}

void COAPSetGroup(CoapPacket &packet, IPAddress ip, int port){
    preferences.begin(DYNAMIC_CONFIG_STORAGE_NAME, false);

    Serial.println("DYN CONFIG: SETTING GROUP");
    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = '\0';

    Serial.println(p);

    group = p[0];
    preferences.putUChar(GROUP_STOR_KEY, group);
    Serial.println("DYN CONFIG: STORED NEW GROUP VALUE");
    preferences.end();

    coap.sendResponse(ip, 
                      port, \
                      packet.messageid, 
                      p, strlen(p), 
                      COAP_CONTENT, 
                      COAP_TEXT_PLAIN, 
                      packet.token, packet.tokenlen
                      );
    Serial.println("RESPONSE SENT");
    MQTTPubConnectMsg();

}

void COAPSetGroupAndTier(CoapPacket &packet, IPAddress ip, int port){
    preferences.begin(DYNAMIC_CONFIG_STORAGE_NAME, false);

    Serial.println("DYN CONFIG: SETTING GROUP AND TIER");
    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = '\0';

    Serial.println(p);

    if(packet.payloadlen > 1){
        group = p[0];
        tier = (uint8_t) strtoul(p, NULL, 10);
    }

    coap.sendResponse(ip, 
                      port, \
                      packet.messageid, 
                      p, strlen(p), 
                      COAP_CONTENT, 
                      COAP_TEXT_PLAIN, 
                      packet.token, packet.tokenlen
                      );
    Serial.println("RESPONSE SENT");

    preferences.end();
}

void COAPActivateSensor(CoapPacket &packet, IPAddress ip, int port) {
    Serial.println(" -- Received sensor activation request!");
    if(tier > 0) {
        Serial.println(" - Activation request ACCEPTED!");
        sensorToActivate = true;
    } else {
        Serial.println(" - Activation request REFUSED!");
    }

    coap.sendResponse(ip, 
                      port, \
                      packet.messageid, 
                      NULL, 0, 
                      COAP_CONTENT, 
                      COAP_TEXT_PLAIN, 
                      packet.token, packet.tokenlen
                      );

    Serial.println("RESPONSE SENT");
}


