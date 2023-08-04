

#define   HOSTNAME        "HTTP_BRIDGE"
#define   MESH_SSID       "PacMan_Mesh"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define   MESH_CHANNEL    6

//#define   STATION_SSID     "Familia Teo 2g"
//#define   STATION_PASSWORD "aguadoce"

#define   STATION_SSID     "motorola edge 20_8911"
#define   STATION_PASSWORD "aguadoce2022"

//#define   STATION_SSID     "DOBMoto2.4GHz"
//#define   STATION_PASSWORD "oficinademotobmw"

#define   BLINK_PERIOD    3000 // milliseconds until cycle repeat
#define   BLINK_DURATION  50  // milliseconds LED is on for

// Prototype
void processMeshMessage( String & msg );
void sendMessage(uint32_t to, String message); 
void sendHelloMessage();
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback(); 
void nodeTimeAdjustedCallback(int32_t offset); 
void delayReceivedCallback(uint32_t from, int32_t delay);
void taskBlinkCallBack();

IPAddress getlocalIP();
IPAddress myIP(0,0,0,0);
IPAddress myAPIP(0,0,0,0);
painlessMesh  mesh;

uint16_t blink_period = 500;
uint16_t blink_duration = 200;
Task taskBlink;
bool onFlagBlink = false;

Task taskSendMessage( TASK_SECOND * 1, TASK_FOREVER, &sendHelloMessage ); // start with a one second interval

Task blinkNoNodes;
bool onFlag = false;
bool calc_delay = false;

SimpleList<uint32_t> nodes;

String mesh_status;
String mesh_last_message_received;

void setupMesh(){

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION | DEBUG );  // set before init() so that you can see startup messages
  
#ifdef CHARACTER_PACMAN 
  Serial.println("Mesh initializing as Root Node and AP Bridge");
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL);
  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);
#else
  Serial.println("Mesh initializing");
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_STA, MESH_CHANNEL);
#endif

  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  //add task to send Messages
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
    
  blinkNoNodes.set(BLINK_PERIOD, (mesh.getNodeList().size() + 1) * 2, []() {
      // If on, switch off, else switch on (led built in )
      if (onFlag)
        onFlag = false;
      else
        onFlag = true;

      blinkNoNodes.delay(BLINK_DURATION);

      if (blinkNoNodes.isLastIteration()) {
        // Finished blinking. Reset task for next run 
        // blink number of nodes (including this node) times
        blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
        // Calculate delay based on current mesh time and BLINK_PERIOD
        // This results in blinks between nodes being synced
        blinkNoNodes.enableDelayed(BLINK_PERIOD - 
            (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
      }
  });
  userScheduler.addTask(blinkNoNodes);
  blinkNoNodes.enable();

  randomSeed(analogRead(A0));

#ifdef CHARACTER_PACMAN
  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);

    //init task for general blink
    blink_period = 500;

#else

 blink_period = 100;

#endif

  taskBlink.set(blink_period, TASK_FOREVER, taskBlinkCallBack);
  userScheduler.addTask(taskBlink);
  
  Serial.print("Sou o node = ");
  Serial.println( mesh.getNodeId() );

}

void activateBlink(){
  blinking = true;
  taskBlink.enable();
}

void deactivateBlink(){
  blinking = false;
  taskBlink.disable();
}

void taskBlinkCallBack(){
   if (onFlagBlink)
        onFlagBlink = false;
      else
        onFlagBlink = true;
}


void receivedCallback(uint32_t from, String & msg) {
  char message[256] = "";
  sprintf(message, "Received from %u message=>%s\n", from, msg.c_str());
  mesh_last_message_received = message;
  mesh_last_message_received += message;
  sprintf(message, "%s", msg.c_str());
  processMeshMessage( msg );
}

void newConnectionCallback(uint32_t nodeId) {
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
 
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
  Serial.printf("New Connection, %s\n", mesh.subConnectionJson(true).c_str());
}

void changedConnectionCallback() {

  char node_data[64] = "";
  mesh_status = "";
  
  Serial.printf("Changed connections\n");
  // Reset blink task
  onFlag = false;
  
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
 
  nodes = mesh.getNodeList();

  sprintf(node_data,"Num nodes: %d + 1 (me)\n", nodes.size());
  mesh_status += node_data;
  mesh_status += "Connection list:\n";
  sprintf(node_data," - %u (me)\n", mesh.getNodeId());
  mesh_status += node_data;
  
  Serial.printf("Num nodes: %d\n", nodes.size());
  Serial.printf("Connection list:");

  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) {
    
    sprintf(node_data," - %u\n", *node);
    mesh_status += node_data;
    Serial.printf(" %u", *node);
    node++;
  }
  Serial.println();
  calc_delay = true;
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.printf("Delay to node %u is %d us\n", from, delay);
}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}


void sendMessage( uint32_t to, String message){
  Serial.printf("Sent data to node %u - %s\n", to, message);
  mesh.sendSingle(to, message);
}

void sendMessageToAll(  String message){
  mesh.sendBroadcast(message);
}


void sendHelloMessage() {
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  msg += " myFreeMemory: " + String(ESP.getFreeHeap());
  
  //disabled broadcast
  //mesh.sendBroadcast(msg);

  if (calc_delay) {
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end()) {
      mesh.startDelayMeas(*node);
      node++;
    }
    calc_delay = false;
  }

  //Serial.printf("Sending message: %s\n", msg.c_str());
  taskSendMessage.setInterval( random(TASK_SECOND * 10, TASK_SECOND * 60));  // between 1 and 5 seconds
}

// Prototype
void processMeshMessage( String & msg ){

  //cut string in two part, command and value
  int commaIndex = msg.indexOf(',');
  //Search for the next comma just after the first
  //int secondCommaIndex = myString.indexOf(',', commaIndex + 1);

  //Then you could use that index to create a substring using the String class's substring() method. This returns a new String beginning at a particular starting index, and ending just before a second index (Or the end of a file if none is given). So you would type something akin to:
  String msg_command = msg.substring(0, commaIndex);
  String msg_value = msg.substring(commaIndex + 1);

/*
  Serial.print("Commnand =");
  Serial.print(msg_command);
  Serial.print("  Value =");
  Serial.println( msg_value );
*/

  //ID,x
  if( msg_command.equals( "ID" ) ){
      bright_level = 200;
      changeDeviceIdentity( msg_value.toInt() );
      updateLed( identity.currentLedValues, bright_level );  
  } else if( msg_command.equals( "AFRAID" ) ){
      identity.currentLedValues = GHOST_AFRAID;
      updateLed( identity.currentLedValues, bright_level ); 
      blink_period =  msg_value.toInt();
      activateBlink();
  } else if( msg_command.equals( "BLINK" ) ){
      if( msg_value.equals("true") ){
        activateBlink();
      } else {
        deactivateBlink();
      }
  } else if( msg_command.equals( "SYNCBLINK" ) ){
      if( msg_value.equals("true") ){
        sync_blink = true;
      } else {
        sync_blink = false;
      }
  } else if( msg_command.equals( "RED" ) ){
      identity.currentLedValues.red = msg_value.toInt();
      updateLed( identity.currentLedValues, bright_level);
  } else if( msg_command.equals( "GREEN" ) ){
      identity.currentLedValues.green = msg_value.toInt();
      updateLed( identity.currentLedValues, bright_level);
  } else if( msg_command.equals( "BLUE" ) ){
      identity.currentLedValues.blue = msg_value.toInt();
      updateLed( identity.currentLedValues, bright_level);
  } else if( msg_command.equals( "BRIGHT" ) ){
      bright_level = msg_value.toInt();
      updateLed( identity.currentLedValues, bright_level);
  }

}



void sendAfraidCommand(uint16_t blink_period){

    //set Pacman blick too


    //for each node, set Afraid Mode
    int x=1;
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end() && x < 5 ) {
      String id = "AFRAID," + blink_period;
      sendMessage( *node, id);
      node++;
      x++;
    }

}






