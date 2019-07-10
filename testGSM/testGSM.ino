#include<SoftwareSerial.h>
SoftwareSerial Serial1(5,6);

void initModule(String cmd, char *res, int t)
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


void setup() 
{
  Serial1.begin(9600);
  Serial.begin(9600);
  pinMode(4,OUTPUT);
  pinMode(9,OUTPUT);
  digitalWrite(4,HIGH);
  digitalWrite(9,HIGH);
  // put your setup code here, to run once:
  Serial.println("Initializing....");
  initModule("AT","OK",1000);
  initModule("ATE1","OK",1000);
  initModule("AT+CPIN?","READY",1000);  
  initModule("AT+CMGF=1","OK",1000);     
  initModule("AT+CNMI=2,2,0,0,0","OK",1000);  
  Serial.println("Initialized Successfully");

}

void loop() {
  // put your main code here, to run repeatedly:
  Send();

}

void Send()
{ 
   
   Serial1.println("AT");
   delay(500);
   serialPrint();
   Serial1.println("AT+CMGF=1");
   delay(500);
   serialPrint();
   Serial1.print("AT+CMGS=");
   Serial1.print('"');
   Serial1.print("7057676280");    //mobile no. for SMS alert
   Serial1.println('"');
   delay(500);
   serialPrint();
   Serial1.print("Check message");
   //Serial1.write(26);
   delay(500);
   serialPrint();
}

void serialPrint()
{
  while(Serial1.available()>0)
  {
    Serial.print(Serial1.read());
  }
}
