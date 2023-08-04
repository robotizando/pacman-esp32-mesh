#include "painlessMesh.h"

#define   HOSTNAME        "HTTP_BRIDGE"
#define   MESH_SSID       "PacMan_Mesh"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define   MESH_CHANNEL    6

#define   STATION_SSID     "Familia Teo 2g"
#define   STATION_PASSWORD "aguadoce"

#define   BLINK_PERIOD    3000 // milliseconds until cycle repeat
#define   BLINK_DURATION  200  // milliseconds LED is on for


painlessMesh  mesh;

// Prototype
void processMeshMessage( String & msg );
void sendMessage(); 
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback(); 
void nodeTimeAdjustedCallback(int32_t offset); 
void delayReceivedCallback(uint32_t from, int32_t delay);

IPAddress getlocalIP();
IPAddress myIP(0,0,0,0);
IPAddress myAPIP(0,0,0,0);
Scheduler     userScheduler; // to control your personal task
Task blinkNoNodes;
bool onFlag = false;

bool calc_delay = false;
SimpleList<uint32_t> nodes;

String mesh_status;
String mesh_last_message_received;

void setupMesh(){

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION | DEBUG );  // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  
  //mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_STA, 6 );
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL);
  
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  changedConnectionCallback();

#if defined(CHARACTER_PACMAN)
  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);
#endif

  Serial.print("Sou o node = ");
  Serial.println( mesh.getNodeId() );

}


void receivedCallback(uint32_t from, String & msg) {
  char message[256] = "";
  sprintf(message, "Received from %u message=>%s\n", from, msg.c_str());
  mesh_last_message_received = message;
  //mesh_last_message_received += message;

  sprintf(message, "%s", msg.c_str());
  processMeshMessage( msg );

}

void newConnectionCallback(uint32_t nodeId) {
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
 
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
  Serial.printf("--> startHere: New Connection, %s\n", mesh.subConnectionJson(true).c_str());
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
  mesh.sendSingle(to, message);
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

  Serial.print("Commnand =");
  Serial.print(msg_command);
  Serial.print("  Value =");
  Serial.println( msg_value );

  


}



void sendAfraidCommand(){

  for( uint8_t x; x < 4; x++ ){


  }

}






