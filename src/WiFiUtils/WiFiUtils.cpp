#include "WiFiUtils.h"

extern char local_ip[IP_STRING_SIZE];
extern TimerHandle_t mqttReconnectTimer;
extern char mqtt_broker_ip[IP_STRING_SIZE];
extern uint16_t mqtt_broker_port;

bool WiFiVerifyConnectionSuccess(void){
    bool connection_success = false;

    Serial.print("WIFI: ATTEMPTING CONNECTION");
    for (uint8_t i = 0 ; 
         i < WIFI_CONNECT_MAX_TRIES && WiFi.status() != WL_CONNECTED; 
         i++ 
        ) 
    {
        vTaskDelay(WIFI_CONNECTION_SUCCESS_CHECKS_DELAY_MS / portTICK_PERIOD_MS);
        Serial.print(".");
    }
    Serial.println("");

    if (WiFi.status() == WL_CONNECTED)
    {
        WiFi.localIP().toString().toCharArray(local_ip, 16);
        Serial.println("WIFI: Connected");

        Serial.print("IP Address: ");
        Serial.println(local_ip);
        connection_success = true;
    } 
    return connection_success;
}

void WiFiSmartConfig(void){
    WiFi.beginSmartConfig();

    // Wait for SmartConfig packet from mobile
    Serial.println("WIFI: Waiting for SmartConfig");
    while (!WiFi.smartConfigDone()) {
        vTaskDelay(WIFI_SMARTCONFIG_SUCCESS_CHECKS_DELAY_MS / portTICK_PERIOD_MS);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WIFI: SmartConfig received");
}

void WiFiConnect(void){
    WiFi.begin();

    if(!WiFiVerifyConnectionSuccess()){
        Serial.println("WIFI: FAILED TO CONNECT TO THE CONFIGURED NETWORK!");

        while(!WiFiVerifyConnectionSuccess()){
            WiFiSmartConfig();
        }
    }
}

// -- WiFi Callbacks

void WiFiEvent(WiFiEvent_t event){
    Serial.printf("[WiFi-event] event: %d\n", event);

    switch(event) {
        case SYSTEM_EVENT_STA_GOT_IP:
            MDNSQueryForMQTTBroker();
            MQTTConnect();
            MQTTSetLastWill();
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            Serial.println("WIFI: lost connection");
            xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
            WiFiConnect();
            break;
        default:
            break;
    }
}

// ** MDNS **
void MDNSSetup(void){
    Serial.println("MDNS: Starting the MDNS!");
    char hostString[16] = { 0 };
    sprintf(hostString, "ESP_%06X", (uint32_t) ESP.getEfuseMac());
    WiFi.hostname(hostString);

    if (!MDNS.begin(hostString)) {
        Serial.println("MDNS: Error setting up MDNS responder!");
    }

    Serial.println("MDNS: Setup complete!");
}

void MDNSQueryForMQTTBroker(void){
    bool found = false;
    int n = 0;

    MDNSSetup();

    while(!found){
        n = MDNS.queryService("mcmqtt", "tcp");
        if(n == 0){
            Serial.println("MDNS: No MQTT service found - Retrying. . .");
        } else {
            found = true;
        }
    }

    MDNS.IP(0).toString().toCharArray(mqtt_broker_ip, 51);
    mqtt_broker_port = MDNS.port(0);
    char buffer[121];
    sprintf(buffer, 
            "MDNS: Found MQTT broker  IP - %s  Port - %u",
            mqtt_broker_ip,
            mqtt_broker_port
            );        
    Serial.println( buffer );

    MDNS.end();
    Serial.println("MDNS: Turned off!");
}
