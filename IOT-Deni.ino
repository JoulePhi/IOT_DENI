#include <ArduinoJson.h>
#include <Http.h>
#include <Geo.h>

#define RST_PIN 8
#define RX_PIN 10
#define TX_PIN 9

const char BEARER[] PROGMEM = "internet";

// the setup routine runs once when you press reset:
void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("Starting!");
}

// the loop routine runs over and over again forever:
void loop()
{
  HTTP http(9600, RX_PIN, TX_PIN, RST_PIN);
  char response[32];
  char body[124];
  Result result;
  // Notice the bearer must be a pointer to the PROGMEM
  result = http.connect(BEARER);
  Serial.print(F("HTTP connect: "));
  Serial.println(result);
  delay(1000);
  Serial.println("READING LOCATION");
  delay(500);
  String loc = http.getLocation();
//int v = http.readVoltage();
  Serial.println(loc);
  delay(2000);
  sprintf(body, "[[\"%s\",\"10\",\"10\",\"10\",\"10\",\"10\"]]", loc.c_str());
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
