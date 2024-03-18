#include <ArduinoLog.h>
#if defined(ESP8266)
#include <ESPAsyncTCP.h>
#elif defined(ESP32)
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include "main.h"

void handlePostSegment(uint8_t *data, size_t &len, AsyncWebServerRequest *request);
void handlePutStatus(uint8_t *data, size_t &len, AsyncWebServerRequest *request);
void handleGetSegments(AsyncWebServerRequest *request);
void handlePutLEDs(uint8_t *data, size_t &len, AsyncWebServerRequest *request);
void handleNotFound(AsyncWebServerRequest *request);
void initServer();