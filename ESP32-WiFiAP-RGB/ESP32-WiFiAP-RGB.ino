/*
 
   
   Created for arduino-esp32 on 04 July, 2023
*/


// Libraries
#include <BluetoothA2DPSource.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// Includes
#include "eeprom.h"
#include "config.h"
#include "leds.h"
#include "filesystem.h"
#include "mesh.h"
#include "audio.h"
#include "serverHandlers.h"



uint8_t boardMode = 0;  

void setup() {

  Serial.begin(115200);

  led_setup();

  Serial.println(F("Loading config data from eeprom"));
  loadConfigData();

  changeDeviceIdentity( configData.identity );
  updateLed( identity.currentLedValues, bright_level );  
  

  // Initialize SPIFFS
  Serial.println(F("Mounting SPIFFS File system"));
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  } else
    Serial.println(F("SPIFFS File system mounted!"));


  setupMesh();



#if defined(CHARACTER_PACMAN)

    myAPIP = IPAddress(mesh.getAPIP());
    Serial.println("My AP IP is " + myAPIP.toString());

    myIP = getlocalIP();
    Serial.println("My Station IP is " + myIP.toString());
  
#endif


  fileSystemServerInit();

  /*
  //Main Page Handler
  server.on("/", HTTP_GET, []() {
    if (!handleFileRead("/index.html")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });


  //Service handlers
  server.on("/service/led/status", HTTP_GET, []() {
    server.send(200, "text/json", ledStatusJson() );
  });

  server.on("/service/led/set", HTTP_GET, handleLedSet );

  server.on("/service/identity", HTTP_GET, []() {
    server.send(200, "text/json", identityJson() );
  });

  server.on("/service/identity/set", HTTP_GET, handleIdentitySet );
*/
  //Async webserver
  setupWebServerHandlers();


  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  mesh.update();

 if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());
  }

  
  
}
