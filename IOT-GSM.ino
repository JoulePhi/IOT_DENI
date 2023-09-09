#include <ArduinoJson.h>
#include <Http.h>
#include "uRTCLib.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DFRobot_EC.h"
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>

// Pin Definitions
#define RST_PIN 8
#define RX_PIN 10 
#define TX_PIN 9
#define EC_PIN A1
#define pHSense A0
#define ONE_WIRE_BUS 2
// Constants
const char BEARER[] PROGMEM = "internet";

// Global Objects
uRTCLib rtc(0x68);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DFRobot_EC ec;
File myFile;
SoftwareSerial ss(5, 6);
// Global Variables
char date[11];
char time[9];
char body[90];
char phs[10];
char ecs[10];
char temps[10];
char batt[10];
char response[80];
char lats[10];
char longs[10];
void setup() {
  wdt_disable();
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
  readSensors();
  getNowDateAndTime();
  composeBody();
  delay(3000);
  if (initializeSD()) {
    saveDataToSD();
    SD.end();
  }
  sendDataToGoogleSheets();
  clearMemory();
  delay(240000);
}

void readSensors() {
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  dtostrf(temp, 2, 2, temps);
  
  float measured = measure(10);
  float ph = calculatePh(measured);
  dtostrf(ph, 2, 2, phs);
  
  float ecRes = readEc(temp);
  dtostrf(ecRes, 2, 2, ecs);
  
  float battVoltage = analogRead(A3) * (5.0 / 1023.0);
  float battPercentage = (battVoltage / 4.2) * 100;
  dtostrf(battPercentage, 2, 2, batt);
}

void getNowDateAndTime() {
  snprintf_P(date, sizeof(date), PSTR("%i/%i/%i"), rtc.day(), rtc.month(), rtc.year());
  snprintf_P(time, sizeof(time), PSTR("%i:%i:%i"), rtc.hour(), rtc.minute(), rtc.second());
}

void composeBody() {
  snprintf_P(body, sizeof(body), PSTR("[[\"%s,%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]]"),
             "100","100", date, time, phs, ecs, temps, batt);
  Serial.print(F("Body: "));
  Serial.println(body);
}

bool initializeSD() {
  bool sdInitialized = false;
  for (int i = 0; i < 10; i++) {
    if (SD.begin(4)) {
      sdInitialized = true;
      break;
    }
    SD.end();
    delay(1000);
  }
  return sdInitialized;
}

void saveDataToSD() {
  myFile = SD.open("data.txt", FILE_WRITE);
  if (myFile) {
    myFile.println(body);
    myFile.close();
    Serial.println(F("Data saved to SD card."));
  } else {
    Serial.println(F("Error opening data.txt"));
  }
}

void sendDataToGoogleSheets() {
  HTTP http(4800, RX_PIN, TX_PIN, RST_PIN);
  Result result = http.connect(BEARER);
  if (result == SUCCESS) {
    result = http.post(PSTR("https://v1.nocodeapi.com/joulephi/google_sheets/fTERUIGtHpAvFPhK?tabId=Sheet1"), body, response);
    Serial.print(F("HTTP POST: "));
    Serial.println(result);
    if (result == SUCCESS) {
      Serial.println(response);
    } else {
      Serial.print(F("Error: "));
      Serial.println(response);
    }
    http.disconnect();
  }
}

void clearMemory() {
  memset(body, 0, sizeof(body));
  memset(response, 0, sizeof(response));
  memset(date, 0, sizeof(date));
  memset(time, 0, sizeof(time));
  memset(phs, 0, sizeof(phs));
  memset(ecs, 0, sizeof(ecs));
  memset(temps, 0, sizeof(temps));
  memset(batt, 0, sizeof(batt));
}

void resetSim() {
  digitalWrite(8, LOW);
  delay(2000);
  digitalWrite(8, HIGH);
}

void getLocation(){
  ss.begin(9600);
  byte getLoc = 0;
  dtostrf(0.000000,1,6,lats);
  dtostrf(0.000000,1,6,longs);
  while(getLoc < 5){
    char c = ss.read();
    static char sentence[100];
    static int sentenceIndex = 0;
    if (c == '$') {
      sentenceIndex = 0;
    }
    sentence[sentenceIndex++] = c;
    if (c == '\n') {
      sentence[sentenceIndex] = '\0';
      if (strstr_P(sentence, PSTR("$GPGGA"))) {
        char *token = strtok(sentence, ",");
        for (int i = 0; i < 6; i++) {
          token = strtok(NULL, ",");
        }
        float latitude = atof(token);
        token = strtok(NULL, ",");

        char latDirection = token[0];
        token = strtok(NULL, ",");

        float longitude = atof(token);
        token = strtok(NULL, ",");

        char lonDirection = token[0];

        if (latDirection == 'S') {
          latitude = -latitude;
        }
        if (lonDirection == 'W') {
          longitude = -longitude;
        }
        Serial.print(F("Latitude: "));
        Serial.print(latitude, 6);
        Serial.print(F("  Longitude: "));
        Serial.println(longitude, 6);
        dtostrf(latitude,1,6,lats);
        dtostrf(longitude,1,6,longs);
      }
      sentenceIndex = 0;
    }
    delay(1000);
    getLoc++;
  }
  
  ss.end();
}

void clearSD() {
  byte sd = 0;
  digitalWrite(4, LOW);
  while (sd != 255) {
    sd = SPI.transfer(255);
    Serial.print(F("sd="));
    Serial.println(sd);
  }
  digitalWrite(4, HIGH);
}
