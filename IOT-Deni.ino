#include <avr/wdt.h>
#include <Http.h>
#include "uRTCLib.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DFRobot_EC.h"
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

// Pin Definitions
#define RST_PIN 8
#define RX_PIN 10 
#define TX_PIN 9
#define EC_PIN A1
#define pHSense A0

// Constants
const char BEARER[] PROGMEM = "internet";
const char mainUrl[] PROGMEM = "https://script.google.com/macros/s/AKfycbzQy7kjuffUMs68NFF0XOpYns4X8yfoWHDXWdI72lbSrO8lKf7yv_CWfEACeCLmz4UqFw/exec?";
const char locParam[] PROGMEM = "loc=";
const char dateParam[] PROGMEM = "&date=";
const char jamParam[] PROGMEM = "&jam=" ;
const char phParam[] PROGMEM = "&ph=" ;
const char ecParam[] PROGMEM = "&cd=";
const char tempParam[] PROGMEM = "&temp=";
const char battParam[] PROGMEM = "&batt=" ;

// Global Variables
uRTCLib rtc(0x68);
OneWire oneWire(2);
DallasTemperature sensors(&oneWire);
DFRobot_EC ec;
SoftwareSerial ss(5, 6);
File myFile;
char response[16];
char phs[6];
char ecs[6];
char temps[6];
char batt[6];
char url[220];
char longs[11];
char lats[11];
char date[11];
char time[11];

void setup() {
  wdt_disable();
  delay(2000);
  Serial.begin(9600);
  URTCLIB_WIRE.begin();
  sensors.begin();
  ec.begin();
  pinMode(8, OUTPUT);
  pinMode(4, OUTPUT);
  resetSim();
  Serial.println(F("Starting!"));
}

void loop() {
  rtc.refresh();
  readTemperature();
  readPh();
  readEc();
  readBattery();
  readLocation();
  getNowDateAndTime();
  composeUrl();
  writeToSDCard();
  sendHttpRequest();
  resetVariables();
  delay(240000);
}

void readTemperature() {
  sensors.requestTemperatures(); 
  float temp = sensors.getTempCByIndex(0);  
  dtostrf(temp, 2, 2, temps);
}

void readPh() {
  float measured = measure(10);
  float ph = calculatePh(measured);
  dtostrf(ph, 2, 2, phs);
}

void readEc() {
  float temp = sensors.getTempCByIndex(0);
  float ecRes = readEc(temp);
  dtostrf(ecRes, 2, 2, ecs);
}

void readBattery() {
  float rawAnalog = analogRead(A3);
  float battVoltage = rawAnalog * (5.0 / 1023.0);
  float battPercentage = (battVoltage / 4.2) * 100;
  dtostrf(battPercentage, 2, 2, batt);
}

void readLocation() {
  ss.begin(9600);
  dtostrf(100, 1, 6, lats);
  dtostrf(100, 1, 6, longs);
  ss.end();
}

void getNowDateAndTime() {
  rtc.refresh();
  getNowDate();
  getNowTime();
}

void getNowDate() {
  snprintf(date, sizeof(date), "%02d-%02d-%04d", rtc.day(), rtc.month(), rtc.year());
}

void getNowTime() {
  snprintf(time, sizeof(time), "%02d:%02d:%02d", rtc.hour(), rtc.minute(), rtc.second());
}

void composeUrl() {
  strcpy_P(url, mainUrl);
  strcat_P(url, locParam);
  strcat(url, lats);
  strcat(url, ",");
  strcat(url, longs);
  strcat_P(url, dateParam);
  strcat(url, date);
  strcat_P(url, jamParam);
  strcat(url, time);
  strcat_P(url, phParam);
  strcat(url, phs);
  strcat_P(url, ecParam);
  strcat(url, "100");
  strcat_P(url, tempParam);
  strcat(url, temps);
  strcat_P(url, battParam);
  strcat(url, batt);
}

void writeToSDCard() {
  bool sd = SD.begin(4);
  if (!sd) {
    Serial.println(F("SD card initialization failed!"));
  }
  myFile = SD.open("data.txt", FILE_WRITE);
  if (myFile) {
    myFile.println(url);
    myFile.close();
  } else {
    Serial.println(F("Error opening data.txt"));
  }
  SD.end();
}

void sendHttpRequest() {
  HTTP http(9600, RX_PIN, TX_PIN, RST_PIN);
  Result result = http.connect(BEARER);
  if (result == SUCCESS) {
    result = http.get(url, response);
    if (result == SUCCESS) {
      Serial.println(response);
    } else {
      Serial.print(F("Error: "));
      Serial.println(response);
    }
    http.disconnect();
  }
}

void resetVariables() {
  memset(response, 0, sizeof(response));
  memset(date, 0, sizeof(date));
  memset(time, 0, sizeof(time));
  memset(phs, 0, sizeof(phs));
  memset(ecs, 0, sizeof(ecs));
  memset(temps, 0, sizeof(temps));
  memset(batt, 0, sizeof(batt));
  memset(url, 0, sizeof(url));
}

void resetSim() {
  digitalWrite(8, LOW);
  delay(2000);
  digitalWrite(8, HIGH);
}
