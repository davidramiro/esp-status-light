#include "main.h"
#include <WifiCredentials.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif
#include "eeprom.h"
#include "server.h"
#include "leds.h"

void setup()
{
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  while (!Serial)
    ;

  initEEPROM();
  initLEDs();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int8_t wifiStatusCode = WiFi.waitForConnectResult();
  if (wifiStatusCode != WL_CONNECTED)
  {
    Log.errorln("Error connecting to WiFi, code %d", wifiStatusCode);
    return;
  }

  Log.infoln("IP: %p", WiFi.localIP());

  initServer();
}

void loop()
{
  yield();
}