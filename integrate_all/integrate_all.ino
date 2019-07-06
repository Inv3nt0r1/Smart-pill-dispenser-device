/****************************************************************************************
 * Author: Durgesh pachghare
 * Date  : 17 Jan 2019 
 * Program for everything
 */
//-------------Include all Libraries-----------------------
#include<MCP7940.h>
//---------------------------------------------------------

//---------------------Declare all program constants and enumerated types-----------------
const uint32_t SERIAL_SPEED        = 115200;                                  // Set the baud rate for Serial I/O //
const uint8_t  LED_PIN             =     13;                                  // Arduino built-in LED pin number  //
const uint8_t  SPRINTF_BUFFER_SIZE =     32;                                  // Buffer size for sprintf()        //
const uint8_t  BUZZER_PIN          =      7;                                  // Pin no where Buzzer is attached  //
const uint8_t  LITMIT_SWITCH       =       ;                                  // Pin no where button is attached  //
const uint8_t  ALARM1_INTERVAL     =     15;                                  // Alarm interval                   //
enum alarmTypes { matchSeconds, matchMinutes, matchHours, matchDayOfWeek,     // Enumeration of alarm types       //
                  matchDayOfMonth, Unused1, Unused2, matchAll, Unknown };     //                                  //

//---------------------Declare all global variables and instance classes-------------------
MCP7940_Class MCP7940
char          inputBuffer[SPRINTF_BUFFER_SIZE];

//----------------------setup method--------------------------------------------------------
void setup() 
{
  Serial.begin(SERIAL_SPEED);
  attachInterrupt(0,rtcMFP_isr, FALLING);
  #ifdef  __AVR_ATmega32U4__                                                  
    delay(3000);                                                           
  #endif
  Serial.print(F("\nStarting SetAlarms program\n"));                          // Show program information         //
  Serial.print(F("- Compiled with c++ version "));                            //                                  //
  Serial.print(F(__VERSION__));                                               // Show compiler information        //
  Serial.print(F("\n- On "));                                                 //                                  //
  Serial.print(F(__DATE__));                                                  //                                  //
  Serial.print(F(" at "));                                                    //                                  //
  Serial.print(F(__TIME__));                                                  //                                  //
  Serial.print(F("\n"));                                                      //                                  //
  while (!MCP7940.begin()) {                                                  // Initialize RTC communications    //
    Serial.println(F("Unable to find MCP7940. Checking again in 3s."));       // Show error text                  //
    delay(3000);                                                              // wait a second                    //
  } // of loop until device is located                                        //                                  //
  Serial.println(F("MCP7940 initialized."));                                  //                                  //
  while (!MCP7940.deviceStatus()) {                                           // Turn oscillator on if necessary  //
    Serial.println(F("Oscillator is off, turning it on."));                   //                                  //
    bool deviceStatus = MCP7940.deviceStart();                                // Start oscillator and return state//
    if (!deviceStatus) {                                                      // If it didn't start               //
      Serial.println(F("Oscillator did not start, trying again."));           // Show error and                   //
      delay(1000);                                                            // wait for a second                //
    } // of if-then oscillator didn't start                                   //                                  //
  } // of while the oscillator is off                                         //                                  //
  Serial.println("Setting MCP7940M to date/time of library compile");         //                                  //
  MCP7940.adjust();                                                           // Use compile date/time for clock  //
  Serial.print("Date/Time set to ");                                          //                                  //
  DateTime now = MCP7940.now();                                               // get the current time             //
  sprintf(inputBuffer,"%04d-%02d-%02d %02d:%02d:%02d", now.year(),            // Use sprintf() to pretty print    //
          now.month(), now.day(), now.hour(), now.minute(), now.second());    // date/time with leading zeroes    //
  Serial.println(inputBuffer);                                                // Display the current date/time    //
  

}

void loop() {
  // put your main code here, to run repeatedly:

}
