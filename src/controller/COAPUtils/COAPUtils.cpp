#include "COAPUtils.h"

extern bool alarm_active;
extern bool alarm_to_startup;
extern bool alarm_act_init_success;
extern bool alarm_triggered;
extern TaskHandle_t task_handle_Coordinator;
extern Coap coap;

void COAPResponse(CoapPacket &packet, IPAddress ip, int port){
    Serial.println("[Coap Response got]");
  
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = '\0';
  
  Serial.println(p);
}

void COAPSwitchAlarmRequest(CoapPacket &packet, IPAddress ip, int port){
    Serial.println("ALARM: Switching the status of the alarm [ON/OFF]");

    // send response
    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = '\0';

    Serial.println(p);

    if(strcmp(p, "true") == 0 && !alarm_active){
        alarm_active = true;
        alarm_to_startup = true;
        while(!alarm_act_init_success){
            vTaskResume(task_handle_Coordinator);
            if (!alarm_act_init_success)
                vTaskDelay(ALARM_INIT_ATTEMPT_DELAY / portTICK_PERIOD_MS);
        }
        Serial.println("ACTIVATING ALARM");
    } else if(strcmp(p, "false") == 0 && alarm_active){
        alarm_active = false;
        alarm_act_init_success = false;
        alarm_triggered = false;
        Serial.println("DEACTIVATING ALARM");
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

}

void COAPAlarmTrigger(CoapPacket &packet, IPAddress ip, int port){
    Serial.println("ALARM: TRIGGERING THE ALARM!");

    if(alarm_active && !alarm_to_startup){
        alarm_triggered = true;
        Serial.println("**ALARM: TRIGGERED!**");
    } else {
        Serial.println("ALARM: TRIGGER FAILED!");
    }
        

    coap.sendResponse(ip, 
                      port, \
                      packet.messageid, 
                      NULL, 0, 
                      COAP_CONTENT, 
                      COAP_TEXT_PLAIN, 
                      packet.token, packet.tokenlen
                      );
}
