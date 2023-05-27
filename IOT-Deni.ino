// SIM800L
#include <Http.h>
#define RST_PIN 8
#define RX_PIN 10 
#define TX_PIN 9
const char BEARER[] PROGMEM = "internet";
const char URL[] PROGMEM = "https://v1.nocodeapi.com/datadeni/google_sheets/xzDjIQomcfxkFnkz?tabId=Sheet1";
HTTP http(9600, RX_PIN, TX_PIN, RST_PIN);

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

unsigned long 
void setup()
{
  Serial.begin(9600);
  URTCLIB_WIRE.begin();
  sensors.begin();
  ec.begin();
  while (!Serial);
  Serial.println(F("Starting!"));
}
void(* resetFunc) (void) = 0;
void loop()
{
  rtc.refresh();
  char response[32];
  char body[124];
  Result result;
  result = http.connect(BEARER);
  Serial.print(F("HTTP connect: "));
  Serial.println(result);
  delay(500);
  readLocation();
  delay(500);
  readTemp();
  delay(500);
  readPh();
  delay(500);
  readEc();
  delay(500);
  sprintf(body, "[[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]]", loc.c_str(),getNowDate().c_str(),getNowTime().c_str(),sPh.c_str(),sec.c_str(),sTemp.c_str());
  Serial.println("Body : " + String(body));
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
  delay(300000);
  resetFunc();
}

void readLocation(){
  Serial.println(F("READING LOCATION"));
  loc = http.getLocation();
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
