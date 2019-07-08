/*
 * Dumping time to EEPROM 
 * Modules used: MCP7940N RTC module, Arduino Mini
 * Durgesh Pachghare  9 January 2019
 */

#include <Wire.h>
#include<SoftwareSerial.h>
SoftwareSerial Serial1(5,6);

char *test = "$GPRMC";

//defines
#define RTCADDR B1101111//page11 datasheet
#define RTCSEC 0x00
#define RTCMIN 0x01
#define RTCHOUR 0x02
#define RTCWKDAY 0x03
#define RTCDATE 0x04
#define RTCMTH 0x05
#define RTCYEAR 0x06
#define CONTROL 0x07
#define OSCTRIM 0x08
#define ALM0SEC 0x0A
#define ALM0MIN 0x0B
#define ALM0HOUR 0x0C
#define ALM0WKDAY 0x0D
#define ALM0DATE 0x0E
#define ALM0MTH 0x0F
#define ALM1SEC 0x11
#define ALM1MIN 0x12
#define ALM1HOUR 0x13
#define ALM1WKDAY 0x14
#define ALM1DATE 0x15
#define ALM1MTH 0x16
#define PWRDNMIN 0x18
#define PWRDNHOUR 0x19
#define PWRDNDATE 0x1A
#define PWRDNMTH 0x1B
#define PWRUPMIN 0x1C
#define PWRUPHOUR 0x1D
#define PWRUPDATE 0x1E
#define PWRUPMTH 0x1F

//variables used here

byte rtcSeconds, rtcMinutes, rtcHours;
byte rtcWeekDay, rtcDay, rtcYear, rtcMonth;
byte alarmHour, alarmMinutes, alarmSeconds, alarmWeek, alarmDay, alarmMonth;
boolean rtc12hrMode, rtcPM, rtcOscRunning, rtcPowerFail, rtcVbatEn;
String weekDay[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};


boolean mfpPinTriggered = false;
const uint8_t SPRINTF_BUFFER_SIZE = 32;
char charBuffer[SPRINTF_BUFFER_SIZE];
char outputBuffer[64];

void setup()
{
  Serial.begin(9600);
  attachInterrupt(0 , rtcMFP_isr, FALLING);//interrupt for rtc Alarm
  rtcInit();
  pinMode(7,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(9,OUTPUT);
  digitalWrite(4,OUTPUT);
  digitalWrite(9,OUTPUT);

  //Disable ADC - don't forget to flip back after waking up if using ADC in your application ADCSRA |= (1 << 7);
    ADCSRA &= ~(1 << 7);
    //ENABLE SLEEP - this enables the sleep mode
    SMCR |= (1 << 2); //power down mode
    SMCR |= 1;//enable sleep

    //Starting and initializing GSM module
    Serial1.begin(9600);
    Serial.println("Initializing....");
    initGSMModule("AT","OK",1000);
    initGSMModule("ATE1","OK",1000);
    initGSMModule("AT+CPIN?","READY",1000);  
    initGSMModule("AT+CMGF=1","OK",1000);     
    initGSMModule("AT+CNMI=2,2,0,0,0","OK",1000);  
    Serial.println("Initialized GSM Module Successfully");
}
void loop()
{
  rtcGetTime();
  Serial.println("Current Date and Time:");
  sprintf(charBuffer, "%04d-%02d-%02d %02d:%02d:%02d", rtcYear, rtcDay, rtcMonth, rtcHours, rtcMinutes, rtcSeconds);
  Serial.println(charBuffer);
  alarmHour = rtcHours;
  alarmMinutes = rtcMinutes;
  alarmSeconds = rtcSeconds+20;
  alarmDay = rtcDay;
  alarmWeek = rtcWeekDay;
  alarmMonth = rtcMonth;
  //sprintf(outputBuffer,"Alarm set for: %02d:%02d:%02d", alarmHour, alarmMinutes, alarmSeconds);
  Wire.beginTransmission(RTCADDR);//set the alarm time
    Wire.write(ALM0SEC);
    Wire.write(0x10);//seconds only
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.write(0x00);//clears flag
    Wire.endTransmission();
  //BOD DISABLE - this must be called right before the __asm__ sleep instruction
    MCUCR |= (3 << 5); //set both BODS and BODSE at the same time
    MCUCR = (MCUCR & ~(1 << 5)) | (1 << 6); //then set the BODS bit and clear the BODSE bit at the same time
    __asm__  __volatile__("sleep");//in line assembler to go to sleep

  while (!checkAlarm()) 
  {
      //BOD DISABLE - this must be called right before the __asm__ sleep instruction
      MCUCR |= (3 << 5); //set both BODS and BODSE at the same time
      MCUCR = (MCUCR & ~(1 << 5)) | (1 << 6); //then set the BODS bit and clear the BODSE bit at the same time
      __asm__  __volatile__("sleep");//in line assembler to go to sleep
  }
  for(int i=0;i<30;i++)
  {
    digitalWrite(7,HIGH);
    delay(500);
    digitalWrite(7,LOW);
    delay(500);
  }
  Send();

}

boolean checkAlarm() 
{
    if (mfpPinTriggered == true) 
    {
      mfpPinTriggered = false;
      Wire.beginTransmission(RTCADDR);
      Wire.write(ALM0WKDAY);//check the RT Flag
      Wire.endTransmission();
      Wire.requestFrom(RTCADDR, 1);
      delay(1);
      byte alarm0Check = Wire.read();
      if (((alarm0Check >> 3) & 0x01) == 1)//Set?
        return 1;
      else
          return 0;
    }
}

void rtcMFP_isr() {
  mfpPinTriggered = true;
}


void rtcInit() 
{   // RTC Initialize
  //sets up I2C at 100kHz
  Wire.setClock(100000);
  Wire.begin();
  
  Wire.beginTransmission(RTCADDR);
  Wire.write(CONTROL);
  Wire.write(B00000000);//clear out the entire control register
  Wire.endTransmission();

  Wire.beginTransmission(RTCADDR);
  Wire.write(RTCWKDAY);
    Wire.endTransmission();
    Wire.requestFrom(RTCADDR, 1);
  delay(1);
  byte rtcWeekdayRegister = Wire.read();
  rtcWeekdayRegister |= 0x08; //enable Battery backup
    Wire.beginTransmission(RTCADDR);
    Wire.write(RTCWKDAY);
    Wire.write(rtcWeekdayRegister);
    Wire.endTransmission();


    Wire.beginTransmission(RTCADDR);
    Wire.write(RTCSEC);
    Wire.endTransmission();
    Wire.requestFrom(RTCADDR, 1);
    delay(1);
    byte rtcSecondRegister = Wire.read();   //read out seconds
    rtcSecondRegister |= 0x80;    // flip the start bit to ON
    Wire.beginTransmission(RTCADDR);
    Wire.write(RTCSEC);
    Wire.write(rtcSecondRegister);    //write it back in... now the RTC is running
    Wire.endTransmission();

  Wire.beginTransmission(RTCADDR);//Enable Alarm0
    Wire.write(CONTROL);
    Wire.endTransmission();
    Wire.requestFrom(RTCADDR, 1);
    delay(1);
    byte rtcControlRegeister = Wire.read();
    rtcControlRegeister |= 0x10;//enable alm0
    Wire.beginTransmission(RTCADDR);
    Wire.write(CONTROL);
    Wire.write(rtcControlRegeister);
    Wire.endTransmission();
}

void rtcGetTime() 
{
  Wire.beginTransmission(RTCADDR);
    Wire.write(RTCSEC);
    Wire.endTransmission();
    Wire.requestFrom(RTCADDR, 7);//pull out all timekeeping registers
    delay(1);//little delay

    //now read each byte in and clear off bits we don't need, hence the AND operations
    rtcSeconds = Wire.read() & 0x7F;
    rtcMinutes = Wire.read() & 0x7F;
    rtcHours = Wire.read() & 0x7F;
    rtcWeekDay = Wire.read() & 0x3F;
    rtcDay = Wire.read() & 0x3F;
    rtcMonth = Wire.read() & 0x3F;
    rtcYear = Wire.read();

    //now format the data, combine lower and upper parts of byte to give decimal number
  rtcSeconds = (rtcSeconds >> 4) * 10 + (rtcSeconds & 0x0F);
  rtcMinutes = (rtcMinutes >> 4) * 10 + (rtcMinutes & 0x0F);

  if ((rtcHours >> 6) == 1)//check for 12hr mode
      rtc12hrMode = true;
    else rtc12hrMode = false;

    // 12hr check and formatting of Hours
    if (rtc12hrMode) //12 hr mode so get PM/AM
  {         //12 hr mode so get PM/AM
      if ((rtcHours >> 5) & 0x01 == 1)
          rtcPM = true;
      else rtcPM = false;
      rtcHours = ((rtcHours >> 4) & 0x01) * 10 + (rtcHours & 0x0F);//only up to 12
    }
    else //24hr mode
  { 
      rtcPM = false;
      rtcHours = ((rtcHours >> 4) & 0x03) * 10 + (rtcHours & 0x0F);//uses both Tens digits, '23'
    }

    //weekday register has some other bits in it, that are pulled out here
    if ((rtcWeekDay >> 5) & 0x01 == 1)
      rtcOscRunning = true;// good thing to check to make sure the RTC is running
    else rtcOscRunning = false;
    if ((rtcWeekDay >> 4) & 0x01 == 1)
      rtcPowerFail = true;// if the power fail bit is set, we can then go pull the timestamp for when it happened
    else rtcPowerFail = false;
    if ((rtcWeekDay >> 3) & 0x01 == 1)//check to make sure the battery backup is enabled
      rtcVbatEn = true;
    else rtcVbatEn = false;

    rtcWeekDay = rtcWeekDay & 0x07;//only the bottom 3 bits for the actual weekday value

    //more formatting bytes into decimal numbers
    rtcDay = (rtcDay >> 4) * 10 + (rtcDay & 0x0F);
    rtcMonth = ((rtcMonth >> 4) & 0x01) * 10 + (rtcMonth & 0x0F);
    rtcYear = (rtcYear >> 4) * 10 + (rtcYear & 0x0F);

    //print everything out
    //Serial.print(rtcHours);
    //Serial.print(":");
    //Serial.print(rtcMinutes);
    //Serial.print(":");
    //Serial.print(rtcSeconds);

    if (rtc12hrMode == true && rtcPM == true)
      Serial.print(" PM ");
    else if (rtc12hrMode == true && rtcPM == false)
      Serial.print(" AM ");

    if (rtc12hrMode == false)
      Serial.print(" 24hr ");

    //Serial.print("WeekDay=");
    //Serial.print(rtcWeekDay);
    //Serial.print(weekDay[rtcWeekDay - 1]);
    //Serial.print(" ");
    //Serial.print(rtcMonth);
  //Serial.print("/");
  //Serial.print(rtcDay);
    //Serial.print("/");
    //Serial.print(rtcYear);
    //Serial.println("");
}

void initGSMModule(String cmd, char *res, int t)
{
    while(1)
    {
        Serial.println(cmd);
        Serial1.println(cmd);
        delay(100);
        while(Serial1.available()>0)
        {
            if(Serial1.find(res))
            {
                Serial.println(res);
                delay(t);
                return;
            }

            else
            {
                Serial.println("Error");
            }
        }
        delay(t);
    }
}
void serial1Print()
{
    while(Serial1.available()>0)
    {
        Serial.print(Serial1.read());
    }
}
void Send()
{ 
    Serial1.println("AT");
    delay(500);
    serial1Print();
    Serial1.println("AT+CMGF=1");
    delay(500);
    serial1Print();
    Serial1.print("AT+CMGS=");
    Serial1.print('"');
    Serial1.print("7057676280");    //mobile no. for SMS alert
    Serial1.println('"');
    delay(500);
    serial1Print();
    Serial1.print("Just checking");
    Serial1.write(26);
    delay(2000);
    serial1Print();
}
