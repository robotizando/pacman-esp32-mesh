#include <EEPROM.h>

// EEPROM DATA STORE
struct ConfigData {
    uint8_t identity; //stores what charactere the device is
    uint8_t board_mode; // 0=AP mode  1=Connect Mode
    char    ssid[16];
};

struct ConfigData configData;
#define EE_CONFIG_ADDRESS 0

void saveConfigData(ConfigData c) {
  Serial.print(F("Save Config to EEPROM "));
  EEPROM.put(EE_CONFIG_ADDRESS, c);
}

  //read from eemprom
ConfigData loadConfigData(){
  EEPROM.get(EE_CONFIG_ADDRESS, configData);
  Serial.print(F(" - EEPROM Config: Identity "));
  if( configData.identity > 4 ){
    configData.identity = 0;
    saveConfigData( configData );
  }
  
  return configData;
}
