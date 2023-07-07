#include <ArduinoJson.h>
// SIM800L
#include <Http.h>
#include "uRTCLib.h"
#define RST_PIN 8
#define RX_PIN 10 
#define TX_PIN 9

const char BEARER[] PROGMEM = "internet";


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


char date[10];
char time[10];
#define delayReadLocaation 500
#define delayReadTemp 500
#define delayReadPh 500
#define delayReadEc 500
#define delaySaveData 5000
// BATTERY


// SDCARD

#include <SPI.h>
#include <SD.h>
File myFile;

char response[32];
char body[100];
char phs[10];
char ecs[10];
char temps[10];
char batt[10];
void setup()
{
  Serial.begin(9600);
  URTCLIB_WIRE.begin();
  sensors.begin();
  ec.begin();
  pinMode(8, OUTPUT);
  pinMode(4, OUTPUT);
  resetSim();
  Serial.println(F("Starting!"));
   
}
void loop()
{
  Serial.println(F("Connecting..."));
  rtc.refresh();
  delay(2000);
  readTemp();
  delay(2000);
  readPh();
  delay(2000);
  readEcs();
  delay(2000);
  readBattery();
  delay(2000);
  rtc.refresh();
  getNowDate();
  getNowTime();
  delay(1000);
  sprintf(body, "[[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]]",
  "100",
  date,
  time,
  phs,
  ecs,
  temps,
  batt
  );
  Serial.print(F("Body : ") );
  Serial.println(body);
  delay(3000);
  int i = 0;
  SPI.begin();
  while(true){
    bool sd = SD.begin(4);
    if (!sd) {
      Serial.println(F("initialization failed!"));
    }
    delay(1000);
    clearSD();
    i++;
    if(i > 10 || sd){
      break;
    }
  }
  int u = 0;
  while(true){
    myFile = SD.open("data.txt", FILE_WRITE);
    if (myFile) {
      Serial.print(F("Writing to data.txt..."));
      myFile.println(body);
      myFile.close();
      Serial.println(F("done."));
      break;
    } else {
      u++;
      Serial.println(F("error opening data.txt"));
    }
    if(u > 5){
      break;
    }
    delay(1000);
  }
  HTTP http(9600, RX_PIN, TX_PIN, RST_PIN);
  Result result;
  result = http.connect(BEARER);
  Serial.print(F("HTTP connect: "));
  Serial.println(result);
  delay(2000);
  result = http.post("https://v1.nocodeapi.com/datadeni/google_sheets/xzDjIQomcfxkFnkz?tabId=Sheet1", body, response);
  Serial.print(F("HTTP POST: "));
  Serial.println(result);
  if (result == SUCCESS)
  {
    Serial.println(response);
  }else{
    Serial.print(F("Error : "));
    Serial.println(response);
  }
  Serial.print(F("HTTP disconnect: "));
  Serial.print(http.disconnect());
  memset(body,0,sizeof(body));
  memset(response,0,sizeof(response));
  memset(date,0,sizeof(date));
  memset(time,0,sizeof(time));
  memset(phs,0,sizeof(phs));
  memset(ecs,0,sizeof(ecs));
  memset(temps,0,sizeof(temps));
  memset(batt,0,sizeof(batt));
  delay(240000);
}

void readLocation(){
  
}
void readTemp(){
  float temp = 0.0;
  Serial.println(F("READING TEMPERATURE"));
  sensors.requestTemperatures(); 
  temp = sensors.getTempCByIndex(0);  
  dtostrf(temp,2,2,temps);
  Serial.print(F("Temp = "));
  Serial.println(temps);
}

void readPh(){
  float measured = 0.0, ph = 0.0;
  Serial.println(F("MEASURING PH"));
  measured = measure(10);
  ph = calculatePh(measured);
  dtostrf(ph,2,2,phs);
  Serial.print(F("Ph = "));
  Serial.println(phs);
}

void readEcs(){
  float ecRes = 0.0;
  Serial.println(F("CALCULATING EC"));
  float temp = 0.0;
  sensors.requestTemperatures(); 
  temp = sensors.getTempCByIndex(0);  
  ecRes = readEc(temp);  
  dtostrf(ecRes,2,2,ecs);
  Serial.print(F("EC = "));
  Serial.println(ecs);
}

void readBattery(){
  float battVoltage = 0 , battPercentage = 0;
  float rawAnalog = analogRead(A3);
  battVoltage= rawAnalog * (5.0 / 1023.0);
  battPercentage = (battVoltage/4.2) * 100;
  dtostrf(battPercentage,2,2,batt);
}
void resetSim(){
  digitalWrite(8, 0);
  delay(2000);
  digitalWrite(8,1);
}


void clearSD()
{
  byte sd = 0;
  digitalWrite(4, LOW);
  while (sd != 255)
  {
    sd = SPI.transfer(255);
    Serial.print(F("sd="));
    Serial.println(sd);
  }
  digitalWrite(4, HIGH);
}
