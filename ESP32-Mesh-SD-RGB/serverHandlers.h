
#ifdef CHARACTER_PACMAN

void setupWebServerHandlers(){

    //Transmit Mesh Message
    server.on("/broadcast", HTTP_GET, [](AsyncWebServerRequest *request){
    
    request->send(200, "text/html", "<form>Text to Broadcast<br><input type='text' name='BROADCAST'><br><br><input type='submit' value='Submit'></form>");
    
    if (request->hasArg("BROADCAST")){
      String msg = request->arg("BROADCAST");
      mesh.sendBroadcast(msg);
    }
  });
  
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  //Index - pagina principal
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");


 //Service handlers
  server.on("/service/led/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/json", ledStatusJson() );
  });

  server.on("/service/led/set", HTTP_GET, [](AsyncWebServerRequest *request) {
    char str_id[128];
    if ( request->hasArg("device") && request->arg("device").equals("0") ) {    
      handleLedSet(request);
    } else {
        sprintf(str_id,"Selected Device = %s",request->arg("device"));
        Serial.println( str_id );

        int n=1;
        int d = request->arg("device").toInt();
        
        SimpleList<uint32_t>::iterator node = nodes.begin();
        while (node != nodes.end() ) {

          Serial.println(n);
          
          if( n == d ){  
            sprintf(str_id,"RED,%s",request->arg("red"));
            sendMessage( *node, str_id);
            sprintf(str_id,"BLUE,%s",request->arg("blue"));
            sendMessage( *node, str_id);
            sprintf(str_id,"GREEN,%s",request->arg("green"));
            sendMessage( *node, str_id);
            sprintf(str_id,"BRIGHT,%s",request->arg("bright"));
            sendMessage( *node, str_id);
          }

          node++;
          n++;
        }

    }

    request->send(200, "text/json", identityJson() );
  }); 

  

  server.on("/service/identity", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/json", identityJson() );
  });

  server.on("/service/mesh/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", mesh_status );
  });

  server.on("/service/mesh/lastMessageReceived", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", mesh_last_message_received );
  });


  server.on("/service/mesh/send", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasArg("message") && request->hasArg("toNode") ){
      String m = request->arg("message");
      String to = request->arg("toNode");
      sendMessage( to.toInt(), m );
      Serial.println("message sent node");
      request->send(200, "text/json", "{\"ok\":1}" );
    }
        
    request->send(400, "text/json", "{\"nok\":0}");
  });


  server.on("/service/idforall", HTTP_GET, [](AsyncWebServerRequest *request) {

    //Pacman Sempre Pacman...
    bright_level = 255;
    changeDeviceIdentity( 0 );
    updateLed( identity.currentLedValues, bright_level );  
  
    //for each node, one identity
    int x=1;
    char str_id[32];
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end() && x < 5 ) {
      sprintf(str_id,"ID,%d",x);
      sendMessage( *node, str_id);
      node++;
      x++;
    }

    request->send(200, "text/json", "{\"message\":\"Identity setted for all nodes\"}");
  });


  server.on("/service/blink/all", HTTP_GET, [](AsyncWebServerRequest *request) {

      String m = "false";
      if (request->hasArg("blink_state") ){
        m = request->arg("blink_state");
      }

    //for Pacman
    if( m.equals("true") ){
        activateBlink();
    } else {
        deactivateBlink();
    }

    //for each node, one identity
    int x=1;
    char str_id[128];
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end() && x < 5 ) {
      sprintf(str_id,"BLINK,%s",m);
      sendMessage( *node, str_id);
      node++;
      x++;
    }

    sprintf(str_id,"{\"message\":\"All nodes blinking = %s \"}",m);
    request->send(200, "text/json", str_id);
  });


server.on("/service/blink/onsync", HTTP_GET, [](AsyncWebServerRequest *request) {

      String m = "false";
      if (request->hasArg("blink_state") ){
        m = request->arg("blink_state");
      }

    //for Pacman
    if( m.equals("true") ){
        sync_blink = true;
    } else {
        sync_blink = false;
    }

    //for each node, one identity
    int x=1;
    char str_id[128];
    sprintf(str_id,"SYNCBLINK,%s",m);
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end() && x < 5 ) {
      sendMessage( *node, str_id);
      node++;
      x++;
    }

    sprintf(str_id,"{\"message\":\"All nodes blinking IN SYNC = %s \"}",m);
    request->send(200, "text/json", str_id);
  });

  server.on("/service/afraid", HTTP_GET, [](AsyncWebServerRequest *request) {
    sendAfraidCommand(200);
    request->send(200, "text/json", "{\"message\":\"Identity setted for all nodes\"}");
  });


  server.on("/service/audio/start", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(AUDIO_START_PIN, HIGH);
    delay(1);
    digitalWrite(AUDIO_START_PIN, LOW);

    request->send(200, "text/json", "{\"message\":\"Identity setted for all nodes\"}");
});


}


#endif
