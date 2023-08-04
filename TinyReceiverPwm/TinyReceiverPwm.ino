/*
 *  TinyReceiver.cpp
 *
 *  Small memory footprint and no timer usage!
 *
 *  Receives IR protocol data of NEC protocol using pin change interrupts.
 *  On complete received IR command the function handleReceivedIRData(uint16_t aAddress, uint8_t aCommand, uint8_t aFlags)
 *  is called in Interrupt context but with interrupts being enabled to enable use of delay() etc.
 *  !!!!!!!!!!!!!!!!!!!!!!
 *  Functions called in interrupt context should be running as short as possible,
 *  so if you require longer action, save the data (address + command) and handle it in the main loop.
 *  !!!!!!!!!!!!!!!!!!!!!
 *
 * The FAST protocol is a proprietary modified JVC protocol without address, with parity and with a shorter header.
 *  FAST Protocol characteristics:
 * - Bit timing is like NEC or JVC
 * - The header is shorter, 3156 vs. 12500
 * - No address and 16 bit data, interpreted as 8 bit command and 8 bit inverted command,
 *     leading to a fixed protocol length of (6 + (16 * 3) + 1) * 526 = 55 * 526 = 28930 microseconds or 29 ms.
 * - Repeats are sent as complete frames but in a 50 ms period / with a 21 ms distance.
 *
 *
 *  This file is part of IRMP https://github.com/IRMP-org/IRMP.
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2022-2023 Armin Joachimsmeyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************************
 */

#include <Arduino.h>
#include <EEPROM.h>


#define TX_PIN         PB3
#define IR_RECEIVE_PIN PB4

#define LED_R_PIN      PB0
#define LED_G_PIN      PB2                                                                                                                                                    
#define LED_B_PIN      PB1

#include "ATtinySerialOut.hpp" // TX is at pin 2 - Available as Arduino library "ATtinySerialOut" - Saves up to 700 bytes program memory and 70 bytes RAM for ATtinyCore
#define DISABLE_PARITY_CHECKS // Disable parity checks. Saves 48 bytes of program memory.
#define USE_ONKYO_PROTOCOL    // Like NEC, but take the 16 bit address and command each as one 16 bit value and not as 8 bit normal and 8 bit inverted value.
#include "TinyIRReceiver.hpp"

/*
 * Helper macro for getting a macro definition as string
 */
#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

volatile struct TinyIRReceiverCallbackDataStruct sCallbackData;

// EEPROM DATA STORE
struct ConfigData {
    uint8_t identity; //stores what charactere the device is
};

struct ConfigData configData;
#define EE_CONFIG_ADDRESS 0


//Led control
struct LedDataStruct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

const struct LedDataStruct LED_OFF = {0, 0, 0};
const struct LedDataStruct LED_FULL = {255, 255, 255};

const struct LedDataStruct LED_RED = {0, 255, 0};
const struct LedDataStruct LED_GREEN = {0, 255, 0};
const struct LedDataStruct LED_BLUE = {0, 0, 255};

const struct LedDataStruct PAC_MAN = {255, 85, 0};
const struct LedDataStruct GHOST_BLINKY = {255, 0, 0};  // vermelho
const struct LedDataStruct GHOST_INKY = {0, 255, 180};  // ciano
const struct LedDataStruct GHOST_CLYDE = {255, 40, 0};  // laranja
const struct LedDataStruct GHOST_PINKY = {255, 0, 80};  // rosa

const struct LedDataStruct GHOST_AFRAID = {0, 0, 255}; 

uint8_t greenPWMValue = 0;

struct Identity {
    uint8_t character;
    LedDataStruct identityLedValues;
    LedDataStruct currentLedvalues;
    uint16_t blink_period;

};

Identity identity;


uint16_t blinkCount  = 0;
uint16_t blinkInterval  = 100;
uint8_t  blinkStatus = 0;
uint8_t  g_pwm_count = 0;


enum mode { MODE_NORMAL, MODE_AFRAID };
uint8_t mode = MODE_NORMAL;


//Remote Control
uint16_t address = 0;
uint16_t command = 0;

//row,col commands represent each button
constexpr uint16_t iRcommands[6][4] = {{65280,65025,64770,64515},
                                      {64260,64005,63750,63495},
                                      {63240,62985,62730,62475},
                                      {62220,61965,61710,61455},
                                      {61200,60945,60690,60435},
                                      {60180,59925,59670,59415}};




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


  analogWrite(LED_R_PIN, levels_mapped.red);
  greenPWMValue = levels_mapped.green;
  analogWrite(LED_B_PIN, levels_mapped.blue);
}


uint8_t bright_level = 200;




void changeDeviceIdentity( uint8_t character ){
  identity.character = character;
  switch( character ){
    case 0:
      identity.identityLedValues = PAC_MAN;
      identity.currentLedvalues = PAC_MAN;
      Serial.println(F("I am Pac Man"));
      break;
    case 1: 
      identity.identityLedValues = GHOST_BLINKY;
      identity.currentLedvalues = GHOST_BLINKY;
      Serial.println(F("I am Blinky"));
      break;
    case 2: 
      identity.identityLedValues =  GHOST_INKY;
      identity.currentLedvalues = GHOST_INKY;
      Serial.println(F("I am Inky"));
      break;
    case 3: 
      identity.identityLedValues =  GHOST_PINKY;
      identity.currentLedvalues = GHOST_PINKY;
      Serial.println(F("I am Pinky"));
      break;
    case 4: 
      identity.identityLedValues =  GHOST_CLYDE;
      identity.currentLedvalues = GHOST_CLYDE;
      Serial.println(F("I am Clyde"));
      break;
    default:
      break;
  }

}



void setup() {

  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println(F("STARTING..."));

  //read from eemprom
  EEPROM.get(EE_CONFIG_ADDRESS, configData);
  Serial.print(F(" - EEPROM Config: Identity "));
  Serial.println( configData.identity );
  if( configData.identity > 4 )
    configData.identity = 0;
  changeDeviceIdentity( configData.identity );
  updateLed( identity.currentLedvalues, bright_level );

  // Enables the interrupt generation on change of IR input signal
  if (!initPCIInterruptForTinyReceiver()) {
    Serial.println(F("No interrupt available for pin " STR(IR_RECEIVE_PIN))); // optimized out by the compiler, if not required :-)
  }

    Serial.println(F("Ready to receive NEC IR signals at pin " STR(IR_RECEIVE_PIN)));

}



void loop() {

    if (sCallbackData.justWritten) {
      sCallbackData.justWritten = false;
      command = sCallbackData.Command;
      address = sCallbackData.Address;

      Serial.print(F("Address="));
      Serial.print(address);
      Serial.print(F(" Command="));
      Serial.print(command);
      Serial.println();

      switch( command ){
        case iRcommands[0][0]:
          if( bright_level < 255 )
              bright_level++;
          break;
        case iRcommands[0][1]:
        if( bright_level > 0 )
          bright_level--;
                  break;
        case iRcommands[0][2]:
     
        bright_level = 0;
        Serial.println(F("Shutdown leds"));
                  break;
        case iRcommands[0][3]:
        bright_level = 255;
        Serial.println(F("Brigth leds"));
                  break;
        case iRcommands[1][0]: //RED Plus
          if( identity.currentLedvalues.red < 255 )
            identity.currentLedvalues.red = identity.currentLedvalues.red + 1;
                  break;
        case iRcommands[1][1]: // GREEN PLUS
                if( identity.currentLedvalues.green < 255 )
           identity.currentLedvalues.green = identity.currentLedvalues.green + 1;
                  break;
        case iRcommands[1][2]:

                  break;
        case iRcommands[1][3]:
          mode = MODE_NORMAL;
          identity.blink_period = 0;
          changeDeviceIdentity( identity.character );
                  break;
        case iRcommands[2][0]: //RED MINUS
                if( identity.currentLedvalues.red > 0 )
           identity.currentLedvalues.red = identity.currentLedvalues.red - 1;
                    break;
        case iRcommands[2][1]: //GREEN MINUS
                if( identity.currentLedvalues.green > 0 )
           identity.currentLedvalues.green = identity.currentLedvalues.green - 1;
                  break;
        case iRcommands[2][2]: //BLUE MINUS
                        if( identity.currentLedvalues.blue > 0 )
                   identity.currentLedvalues.blue = identity.currentLedvalues.blue - 1;
                  break;
        case iRcommands[2][3]: // Put in Afraid Mode
          mode = MODE_AFRAID;
          if( identity.character > 0 ){
            identity.currentLedvalues = GHOST_AFRAID;
            identity.blink_period = 300;
          } else {
            identity.blink_period = 20;
          }
          Serial.print( F("Blink Period "));
          Serial.println( identity.blink_period );

                  break;
        case iRcommands[3][0]: //RED PLUS 10
          if( identity.currentLedvalues.red < 255 )
            identity.currentLedvalues.red = identity.currentLedvalues.red + 10;
                  break;
        case iRcommands[3][1]: // GREEN PLUS 10
                if( identity.currentLedvalues.green < 255 )
           identity.currentLedvalues.green = identity.currentLedvalues.green + 10;
                  break;
        case iRcommands[3][2]: // BLUE PLUS 10
                        if( identity.currentLedvalues.blue < 255 )
           identity.currentLedvalues.blue = identity.currentLedvalues.blue + 10;
                  break;
        case iRcommands[3][3]: // CHANGE DEVICE IDENTITY - só ativa sem o repeat
          if(  sCallbackData.Flags != IRDATA_FLAGS_IS_REPEAT ){
            if( identity.character < 4 )
              identity.character++;
            else
              identity.character=0;            

          changeDeviceIdentity( identity.character );
          configData.identity = identity.character;
          EEPROM.put(EE_CONFIG_ADDRESS, configData);
          }

          break;
        case iRcommands[4][0]: //RED MINUS 10
          if( identity.currentLedvalues.red > 0 )
            identity.currentLedvalues.red = identity.currentLedvalues.red - 10;
          break;
        case iRcommands[4][1]: //GREEN MINUS 10
          if( identity.currentLedvalues.green > 0 )
           identity.currentLedvalues.green = identity.currentLedvalues.green - 10;
          break;  
        case iRcommands[4][2]: //BLUE MINUS 10
          if( identity.currentLedvalues.blue > 0 )
           identity.currentLedvalues.blue = identity.currentLedvalues.blue - 10;
          break;
        case iRcommands[4][3]:
          break;
        case iRcommands[5][0]:     
          if( identity.character == 1){
            identity.currentLedvalues = LED_OFF;
          } 
        break;        
        case iRcommands[5][1]:  
          if( identity.character == 2){
            identity.currentLedvalues = LED_OFF;
          }     
        break;        
        case iRcommands[5][2]:      
          if( identity.character == 3){
            identity.currentLedvalues = LED_OFF;
          }  
        break;
        case iRcommands[5][3]:
                  if( identity.character == 4){
            identity.currentLedvalues = LED_OFF;
          }        
        break;
      }






      //if (sCallbackData.Flags == IRDATA_FLAGS_IS_REPEAT)
      //     Serial.print(F(" Repeat"));

      //  if (sCallbackData.Flags == IRDATA_FLAGS_PARITY_FAILED)
      //      Serial.print(F(" Parity failed"));



      updateLed( identity.currentLedvalues, bright_level );

    } // Fim da rotina de tratamento do controle remoto

    /*
     * Green PWM Main loop routine
     */
    if( greenPWMValue <= g_pwm_count ){
      digitalWrite(LED_G_PIN, LOW );
    } else {
      digitalWrite(LED_G_PIN, HIGH );
    }
    g_pwm_count++;


    //Blink Routine
    if( identity.blink_period > 0 && blinkCount > ((uint16_t) (identity.blink_period * blinkInterval)) ){ 
      if( blinkStatus == 0 ){
      
        blinkStatus = 1;
        if( identity.blink_period > 0)
          updateLed( identity.currentLedvalues, bright_level );
      
      } else {
        blinkStatus = 0;   
        updateLed( LED_OFF, 0 );
      }

      blinkCount = 0;
    } else
      blinkCount++;
 
}




 
void handleReceivedTinyIRData(uint16_t aAddress, uint16_t aCommand, uint8_t aFlags){
  //printTinyReceiverResultMinimal(&Serial, aAddress, aCommand, aFlags);
  sCallbackData.Command = aCommand;
  sCallbackData.Flags = aFlags;
  sCallbackData.justWritten = true;
}
