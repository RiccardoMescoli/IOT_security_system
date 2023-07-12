#include "PreferencesUtils.h"

extern Preferences preferences;
extern uint8_t tier;
extern char group;

void dynConfigSetUp(void){
    preferences.begin(DYNAMIC_CONFIG_STORAGE_NAME, false);

    uint8_t stored_value = UINT8_MAX;

    stored_value = preferences.getUChar(TIER_STOR_KEY, UINT8_MAX);
    if(stored_value == UINT8_MAX){
        Serial.println("DYN CONFIG: Storing Tier in flash memory");
        preferences.putUChar(TIER_STOR_KEY, tier);
    } else {
        Serial.println("DYN CONFIG: Retrieving Tier from flash memory");
        tier = stored_value;
    }
    Serial.print("Tier: ");
    Serial.println(tier);

    stored_value = preferences.getUChar(GROUP_STOR_KEY, UINT8_MAX);
    if(stored_value == UINT8_MAX){
        Serial.println("DYN CONFIG: Storing Group in flash memory");
        preferences.putUChar(GROUP_STOR_KEY, group);
    } else {
        Serial.println("DYN CONFIG: Retrieving Group from flash memory");
        group = (char) stored_value;
    }
    Serial.print("Group: ");
    Serial.println(group);
    
    preferences.end();
}