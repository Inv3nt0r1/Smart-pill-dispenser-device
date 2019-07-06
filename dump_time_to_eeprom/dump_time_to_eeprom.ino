/* Program to dump time to EEPROM using RTC module
 * Durgesh Pachghare
 * 9 January 2019
 */
#include<MCP7940.h>
#include <EEPROM.h>

const byte LED_PIN             = 13;
const byte MCP7940_MEMORY_SIZE = 64;
const byte SPRINTF_BUFFER_SIZE = 32;
int addr = 0;
MCP7940_Class MCP7940;
char inputBuffer[SPRINTF_BUFFER_SIZE];

void setup() 
{
  Serial.begin(115200);
  while (!MCP7940.begin())
  {                                                  
    Serial.println(F("Unable to find MCP7940N. Checking again in 3s."));     
    delay(3000);                                                              
  }                                      
  Serial.println(F("MCP7940 initialized."));                                  
  while (!MCP7940.deviceStatus()) 
  {                                           
    Serial.println(F("Oscillator is off, turning it on."));                   
    bool deviceStatus = MCP7940.deviceStart();                           
    if (!deviceStatus) 
    {                                                           
      Serial.println(F("Oscillator did not start, trying again."));           
      delay(1000);                                                           
    }                          
  }
  MCP7940.adjust();
   pinMode(LED_PIN,OUTPUT);
}

void loop() 
{
  static byte secs;
  static byte mins;
  static byte hrs;
  static byte memoryAddress = 0;
  int readAddr;
  int incomingByte;
  DateTime now = MCP7940.now();
  if(secs != now.second())
  {
    sprintf(inputBuffer,"%04d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
    if(secs%10 == 0)
      Serial.println(inputBuffer);
    secs = now.second();
    mins = now.minute();
    hrs = now.hour();
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    
    EEPROM.write(addr,hrs);
    addr++;
    EEPROM.write(addr,mins);
    addr++;
    EEPROM.write(addr,secs);
    addr++;
    if(addr >= EEPROM.length())
    {
      addr = 0;
      Serial.println("EEPROM full! Overwriting from first address!");
    }
    if(Serial.available() > 0)
    {
      incomingByte = Serial.read();
    }
    if(incomingByte == 1)
    {
      Serial.println("Printing all the values in EEPROM!");
      for(int i=0;i<addr;i+=3)
      {
        hrs = EEPROM.read(i);
        mins = EEPROM.read(i+1);
        secs = EEPROM.read(i+2);
        sprintf(inputBuffer,"%02d:%02d:%02d", hrs,mins,secs);
        Serial.println(inputBuffer);
      }
    }

    
    delay(1);
    
  }
}
