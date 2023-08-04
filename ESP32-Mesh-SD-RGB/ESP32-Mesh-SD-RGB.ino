/*
 
   
   Created for arduino-esp32 on 04 July, 2023
*/


// Libraries
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "painlessMesh.h"

// Includes
#include "eeprom.h"
#include "config.h"
#include "leds.h"
#include "filesystem.h"
#include "mesh.h"
#include "serverHandlers.h"



uint8_t boardMode = 0;  

void setup() {

  Serial.begin(115200);

  //setupSD();

  led_setup();

  Serial.println(F("Loading config data from eeprom"));
  loadConfigData();

  //inicializa desligado
  bright_level = 0;
  changeDeviceIdentity( configData.identity );
  updateLed( identity.currentLedValues, bright_level );  
  
#if defined(CHARACTER_PACMAN)  

  pinMode( AUDIO_START_PIN, OUTPUT );
  digitalWrite(AUDIO_START_PIN, LOW);

  // Initialize SPIFFS
  Serial.println(F("Mounting SPIFFS File system"));
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  } else
    Serial.println(F("SPIFFS File system mounted!"));

#endif

  setupMesh();

#if defined(CHARACTER_PACMAN)

    myAPIP = IPAddress(mesh.getAPIP());
    Serial.println("My AP IP is " + myAPIP.toString());

    myIP = getlocalIP();
    Serial.println("My Station IP is " + myIP.toString());
  
  //Async webserver
  setupWebServerHandlers();
  server.begin();
  Serial.println("HTTP server started");

#endif




}


void loop() {
  mesh.update();

  //sync blink for builtin led
  digitalWrite(LED_BUILTIN, onFlag);

  //blink identity on sync
  if( sync_blink ){
     if( onFlag ){
      updateLed(identity.currentLedValues, bright_level);
    } else
      updateLed(identity.currentLedValues, 0);
  }


  if(blinking)
    if( onFlagBlink ){
      updateLed(identity.currentLedValues, 0);
    } else
      updateLed(identity.currentLedValues, bright_level);

  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());
  }
}
