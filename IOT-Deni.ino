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
char body[80];

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

// BATTERY
float battVoltage = 0 , battPercentage = 0;

// SD CARD
#include <SPI.h>
#include <SD.h>
File myFile;
#define CS_PIN 4

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Starting!"));
  pinMode(8, OUTPUT);
  resetSim();
  URTCLIB_WIRE.begin();
  sensors.begin();
  ec.begin();
  if (!SD.begin(CS_PIN)) {
    Serial.println(F("initialization failed!"));
    while (1);
  }
  
}
void(* resetFunc) (void) = 0;
void loop()
{
  if (gsm.begin(2400)){
    Serial.println(F("\nstatus=READY"));
    started=true;  
  }else Serial.println(F("\nstatus=IDLE"));
  rtc.refresh();
  delay(500);
  readLocation();
  delay(500);
  readTemp();
  delay(500);
  readPh();
  delay(500);
  readEc();
  delay(500);
  rtc.refresh();
  delay(1000);
  sprintf(body, "[[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]]",
  loc.c_str(),
  getNowDate().c_str(),
  getNowTime().c_str(),
  sPh.c_str(),sec.c_str(),
  sTemp.c_str(),
  String(battVoltage).c_str()
  );
  Serial.println("Body : " + String(body));
  myFile = SD.open("data.txt", FILE_WRITE);
  if (myFile) {
    Serial.print(F("Writing to data.txt..."));
    myFile.println(body);
    myFile.close();
    Serial.println(F("done."));
  } else {
    // if the file didn't open, print an error:
    Serial.println(F("error opening test.txt"));
  }
  if(started){
    if (inet.attachGPRS("internet", "", "")) Serial.println(F("status=ATTACHED"));
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
    memset(body,0,sizeof(body));
    memset(msg,0,sizeof(msg));
    if (inet.dettachGPRS())
      Serial.println(F("status=DETTACHED"));
    else Serial.println(F("status=ERROR"));
  }
  gsm.SimpleWriteln("AT+CSCLK=1");
  delay(300000);
  resetSim();
  gsm.SimpleWriteln("AT");
  gsm.SimpleWriteln("AT+CSCLK=0");
  delay(1000);
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

void readBattery(){
  float rawAnalog = analogRead(A3);
  battVoltage= rawAnalog * (5.0 / 1023.0);
  battPercentage = (battVoltage/4.2) * 100;
}

void resetSim(){
  digitalWrite(8, 0);
  delay(2000);
  digitalWrite(8,1);
}
