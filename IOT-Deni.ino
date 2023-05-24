#include <ArduinoJson.h>
#include <Http.h>
#include "uRTCLib.h"
#define RST_PIN 8
#define RX_PIN 10
#define TX_PIN 9

const char BEARER[] PROGMEM = "internet";
uRTCLib rtc(0x68);
// the setup routine runs once when you press reset:
void setup()
{
  Serial.begin(9600);
  URTCLIB_WIRE.begin();
  while (!Serial)
    ;
  Serial.println("Starting!");
}

// the loop routine runs over and over again forever:
void loop()
{
  rtc.refresh();
  HTTP http(9600, RX_PIN, TX_PIN, RST_PIN);
  char response[32];
  char body[124];
  Result result;
  result = http.connect(BEARER);
  Serial.print(F("HTTP connect: "));
  Serial.println(result);
  delay(1000);
  Serial.println("READING LOCATION");
  delay(500);
  String loc = http.getLocation();
  Serial.println(loc);
  sprintf(body, "[[\"%s\",\"%s\",\"%s\",\"10\",\"10\",\"10\"]]", loc.c_str(),getNowDate().c_str(),getNowTime().c_str());
  Serial.println("Body : " + String(body) );
  result = http.post("https://v1.nocodeapi.com/datadeni/google_sheets/xzDjIQomcfxkFnkz?tabId=Sheet1", body, response);
  Serial.print(F("HTTP POST: "));
  Serial.println(result);
  if (result == SUCCESS)
  {
    Serial.println(response);
    StaticJsonBuffer<64> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(response);

    const char *id = root[F("id")];
    Serial.print(F("ID: "));
    Serial.println(id);
  }else{
    Serial.print("Error : ");
    Serial.println(response);
  }
  Serial.print(F("HTTP disconnect: "));
  Serial.print(http.disconnect());
}
String getNowDate(){
  String date = "";
  date += String(rtc.year());
  date += "/";
  date += String(rtc.month());
  date += "/";
  date += String(rtc.day());
  return date;
}
String getNowTime(){
  String time = "";
  time += String(rtc.hour());
  time += ":";
  time += String(rtc.minute());
  time += ":";
  time += String(rtc.second());
  return time;
}
