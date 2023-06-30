// SIM800L
#include "SIM900.h"
#include <SoftwareSerial.h>
#include "inetGSM.h"
InetGSM inet;
char msg[64];
int numdata;
char inSerial[50];
int i=0;
boolean started=false;
char body[64];

// RTC
#include "uRTCLib.h"
uRTCLib rtc(0x68);

// TEMP
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// EC
#include "DFRobot_EC.h"
#include <EEPROM.h>
#define EC_PIN A1
DFRobot_EC ec;


// PH
int pHSense = A0;

// VARIABLES
String loc,sTemp, sPh, sec;
float temp = 0,measured = 0, ph = 0,ecRes = 0;

#define delayReadLocaation 500
#define delayReadTemp 500
#define delayReadPh 500
#define delayReadEc 500
#define delaySaveData 5000

// Timer
unsigned int refreshRtc = 0;
unsigned int step1Time = 0;
unsigned int step2Time = 0;
unsigned int step3Time = 0;
unsigned int step4Time = 0;
unsigned int step5Time = 0;
unsigned int step6Time = 0;
unsigned int step7Time = 0;
unsigned int step8Time = 0;

unsigned int rtcInterval = 500; 
unsigned int step1Interval = 500; 
unsigned int step2Interval = step1Interval + 500; 
unsigned int step3Interval = step2Interval + 500; 
unsigned int step4Interval = step3Interval + 500; 
unsigned int step5Interval = step4Interval + 500; 
unsigned int step6Interval = step5Interval + 500; 
unsigned int step7Interval = step6Interval + 500; 
unsigned long step8Interval = step6Interval + 300000; 
bool isOn[7] = {0,0,0,0,0,0,0};
void setup()
{
  Serial.begin(9600);
  
  URTCLIB_WIRE.begin();
  sensors.begin();
  ec.begin();
  while (!Serial);
  Serial.println(F("Starting!"));
  pinMode(5, OUTPUT);
  if (gsm.begin(2400)){
    Serial.println("\nstatus=READY");
    started=true;  
  }else Serial.println("\nstatus=IDLE");
}
void(* resetFunc) (void) = 0;
void loop()
{
  unsigned int currTime = millis();
  if(currTime - refreshRtc >= rtcInterval){
    rtc.refresh();
    refreshRtc = currTime;
  }
  if(currTime - step2Time >= step2Interval && isOn[1] == 0){
    readLocation();
    isOn[1] = 1;
  }
  if(currTime - step3Time >= step3Interval && isOn[2] == 0){
    readTemp();
    isOn[2] = 1;
  }
  if(currTime - step4Time >= step4Interval && isOn[3] == 0){
    readPh();
    isOn[3] = 1;
  }
  if(currTime - step5Time >= step5Interval && isOn[4] == 0){
    readPh();
    isOn[4] = 1;
  }
  if(currTime - step6Time >= step6Interval && isOn[5] == 0){
    readEc();
    isOn[5] = 1;
  }
  if(currTime - step7Time >= step7Interval && isOn[6] == 0){
    sprintf(body, "[[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]]", loc.c_str(),getNowDate().c_str(),getNowTime().c_str(),sPh.c_str(),sec.c_str(),sTemp.c_str());
    Serial.println("Body : " + String(body));
    if(started){
    if (inet.attachGPRS("internet", "", ""))
      Serial.println(F("status=ATTACHED"));
    else Serial.println(F("status=ERROR"));
    delay(1000);
    gsm.SimpleWriteln("AT+CIFSR");
    delay(5000);
    gsm.WhileSimpleRead();
    numdata=inet.httpPOST("v1.nocodeapi.com", 443, "/datadeni/google_sheets/xzDjIQomcfxkFnkz?tabId=Sheet1",body, msg, 128);
    Serial.println(F("\nNumber of data received:"));
    Serial.println(numdata);  
    Serial.println(F("\nData received:")); 
    Serial.println(msg); 
    if (inet.dettachGPRS())
      Serial.println(F("status=DETTACHED"));
    else Serial.println(F("status=ERROR"));
  }
    isOn[6] = 1;
  }
  if(currTime - step8Time >= step8Interval){
    for(int i=0; i<8; i++){
      isOn[i] = 0;
    }
    step1Time = currTime;
    step2Time = currTime;
    step3Time = currTime;
    step4Time = currTime;
    step5Time = currTime;
    step6Time = currTime;
    step7Time = currTime;
    step8Time = currTime;
    resetFunc();
  }
}

void readLocation(){
  Serial.println(F("READING LOCATION"));
  loc = "107.685840,-6.913760";
  Serial.println(loc);
}

void readTemp(){
  Serial.println(F("READING TEMPERATURE"));
  for(int i=0; i < 10; i++){
    sensors.requestTemperatures(); 
    temp = sensors.getTempCByIndex(0);  
    delay(500);
  }
  sTemp = String(temp,2);
  Serial.println("Temp = " + sTemp);
}

void readPh(){
  Serial.println(F("MEASURING PH"));
  measured = measure(10);
  ph = calculatePh(measured);
  sPh = String(ph);
  Serial.println("Ph : " + sPh);
}

void readEc(){
  Serial.println(F("CALCULATING EC"));
  for(int i=0; i < 10; i++){
    ecRes = readEc(temp);  
    delay(500);
  }
  sec = String(ecRes,2);
  Serial.println("EC : " + sec);
}
