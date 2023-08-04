


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

  server.on("/service/led/set", HTTP_GET, handleLedSet );


  server.on("/service/identity", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/json", identityJson() );
  });

  server.on("/service/mesh/device/identity", HTTP_PUT, [](AsyncWebServerRequest *request) {
    if (request->hasArg("toDevice") && request->hasArg("identity") ){

      String to = request->arg("toDevice");
      String id = request->arg("identity");

      String response = "{\"toDevice\":\"" + to + "\",\"identity\":" + to +"\"}";
      request->send(200, "text/json", response );
    }


    request->send(400, "text/plain", "Deu bad" );
  });

    server.on("/service/sound/connect", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasArg("device")){
      String d = request->arg("device");
      a2dp_source.start( d.c_str() );  
      a2dp_source.set_volume(40);
      String response = "{\"connected_to\":\"" + d + "\"}";
      request->send(200, "text/json", response );
    }
        
    request->send(400, "text/json", "{\"nok\":0}");
  });

  server.on("/service/sound/volume", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasArg("volume")){
      String v = request->arg("volume");
      a2dp_source.set_volume(v.toInt());
      String response = "{\"ok\":" + v;
      response += "}";
      request->send(200, "text/json", response );
    }
        
    request->send(400, "text/json", "{\"nok\":0}");
  });

  server.on("/service/sound/0", HTTP_GET, [](AsyncWebServerRequest *request) {
    a2dp_source.write_data(beginning);
    request->send(200, "text/json", "{\"ok\":1}");
  });

  server.on("/service/mesh/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", mesh_status );
  });

  server.on("/service/mesh/lastMessageReceived", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", mesh_last_message_received );
  });




}
