#include "Main.h"

#if __has_include("WifiCredentials.h")
#include "WifiCredentials.h"
#else
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"
#endif

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "Storage.h"
#include "API.h"
#include "LEDs.h"

void setup()
{
  Serial.begin(115200);
  Log.begin(LOG_LEVEL, &Serial);
  while (!Serial)
    ;

  initStorage();
  initLEDs();

  WiFi.mode(WIFI_STA);
  WiFi.hostname(ESP_HOST_NAME);
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