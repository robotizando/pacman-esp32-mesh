

#define LED_BUILTIN 2   // Set the GPIO pin where you connected your test LED or comment this line out if your dev board has a built-in LED


// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0
#define LCHANNEL_RED       1
#define LCHANNEL_GREEN     2
#define LCHANNEL_BLUE      3

// use 12 bit precission for LEDC timer
#define LEDC_TIMER_12_BIT  12

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     5000

//PIN definições
#define LED_RED            14
#define LED_BLU            12
#define LED_GRE            13


//Led control
struct LedDataStruct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct Identity {
    uint8_t character;
    LedDataStruct identityLedValues;
    LedDataStruct currentLedValues;
    uint16_t blink_period;

};

Identity identity;

const struct LedDataStruct LED_OFF = {0, 0, 0};
const struct LedDataStruct LED_FULL = {255, 255, 255};

const struct LedDataStruct PAC_MAN = {255, 85, 0};
const struct LedDataStruct GHOST_BLINKY = {255, 0, 0};  // vermelho
const struct LedDataStruct GHOST_INKY = {0, 255, 180};  // ciano
const struct LedDataStruct GHOST_CLYDE = {255, 40, 0};  // laranja
const struct LedDataStruct GHOST_PINKY = {255, 0, 80};  // rosa

const struct LedDataStruct GHOST_AFRAID = {0, 0, 255}; 

uint8_t bright_level = 200;



// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 4095 from 2 ^ 12 - 1
  uint32_t duty = (4095 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}

void led_setup() {

  // Setup timer and attach timer to a led pin
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
  ledcAttachPin(LED_BUILTIN, LEDC_CHANNEL_0);

  ledcSetup(LCHANNEL_RED, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
  ledcAttachPin(LED_RED, LCHANNEL_RED);

  ledcSetup(LCHANNEL_GREEN, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
  ledcAttachPin(LED_GRE, LCHANNEL_GREEN);

  ledcSetup(LCHANNEL_BLUE, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
  ledcAttachPin(LED_BLU, LCHANNEL_BLUE);


}




void updateLed( LedDataStruct levels, uint8_t bright_level ){

  //SCALE to bright_level
  LedDataStruct levels_mapped;
  levels_mapped.red = map(levels.red, 0, 255, 0, bright_level);
  levels_mapped.green = map(levels.green, 0, 255, 0, bright_level);
  levels_mapped.blue = map(levels.blue, 0, 255, 0, bright_level);

  Serial.print(F("Bright Level = "));
  Serial.print( bright_level );
  Serial.print(F("\n\r"));

  Serial.print(F("Normal Red = "));
  Serial.print( levels.red );
  Serial.print(F("   Green = "));
  Serial.print( levels.green );
  Serial.print(F("   Blue  = "));
  Serial.print( levels.blue );
  Serial.print(F("\n\r"));

  Serial.print(F("Mapped Red = "));
  Serial.print( levels_mapped.red );
  Serial.print(F("   Green = "));
  Serial.print( levels_mapped.green );
  Serial.print(F("   Blue  = "));
  Serial.print( levels_mapped.blue );
  Serial.print(F("\n\r"));


  ledcAnalogWrite(LCHANNEL_RED,  levels_mapped.red );
  ledcAnalogWrite(LCHANNEL_GREEN,  levels_mapped.green );
  ledcAnalogWrite(LCHANNEL_BLUE,  levels_mapped.blue );
    
}

String ledStatusJson(){

  String output = "";
          output += "{\"red\":\"";
          output += identity.currentLedValues.red;
          output += "\",\"green\":\"";
          output += identity.currentLedValues.green;
          output += "\",\"blue\":\"";   
          output += identity.currentLedValues.blue;
          output += "\",\"bright\":\"";   
          output += bright_level;
          output += "\"}";

  return output;
}


void changeDeviceIdentity( uint8_t character ){
  identity.character = character;
  switch( character ){
    case 0:
      identity.identityLedValues = PAC_MAN;
      identity.currentLedValues = PAC_MAN;
      Serial.println(F("I am Pac Man"));
      break;
    case 1: 
      identity.identityLedValues = GHOST_BLINKY;
      identity.currentLedValues = GHOST_BLINKY;
      Serial.println(F("I am Blinky"));
      break;
    case 2: 
      identity.identityLedValues =  GHOST_INKY;
      identity.currentLedValues = GHOST_INKY;
      Serial.println(F("I am Inky"));
      break;
    case 3: 
      identity.identityLedValues =  GHOST_PINKY;
      identity.currentLedValues = GHOST_PINKY;
      Serial.println(F("I am Pinky"));
      break;
    case 4: 
      identity.identityLedValues =  GHOST_CLYDE;
      identity.currentLedValues = GHOST_CLYDE;
      Serial.println(F("I am Clyde"));
      break;
    default:
      break;
  }

}


String identityJson(){

  uint8_t character = identity.character;
  
  String output = "";
  output += "{";
  output += "\"identity_id\":\"";
  output += character;
  output += "\",";

  output += "\"identity_message\":\"";
  switch( character ){
    case 0:
       output += F("I am Pac Man");
      break;
    case 1: 
       output += F("I am Blinky");
      break;
    case 2: 
       output += F("I am Inky");
      break;
    case 3: 
       output += F("I am Pinky");

      break;
    case 4: 
       output += F("I am Clyde");
      break;
    default:
      break;
  }

  output += "\"";
  output += "}";

  return output;
}

void handleLedSet(AsyncWebServerRequest *request) {

    String red = "0"; // server.arg("red");
    String green = "0"; //server.arg("green");
    String blue = "0"; //server.arg("blue");
    String bright = "0"; //server.arg("bright");

    if (request->hasArg("red"))
      red = request->arg("red");

    if (request->hasArg("green"))
      green = request->arg("green");

    if (request->hasArg("blue"))
      blue = request->arg("blue");

    if (request->hasArg("bright"))
      bright = request->arg("bright");
   
    identity.currentLedValues.red = red.toInt();
    identity.currentLedValues.green = green.toInt(); 
    identity.currentLedValues.blue = blue.toInt();
    bright_level = bright.toInt();

    updateLed( identity.currentLedValues, bright_level );  
    
    request->send(200, "text/json", identityJson() );
}


void handleIdentitySet() {

    String id = "0"; //server.arg("id");
   
    identity.character = id.toInt();
    changeDeviceIdentity( identity.character );
    
    configData.identity = identity.character;
    saveConfigData( configData );

    updateLed( identity.currentLedValues, bright_level );  
    
    //server.send(200, "text/json", ledStatusJson() );
}
