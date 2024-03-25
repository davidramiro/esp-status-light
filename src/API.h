#pragma once

#include <ArduinoLog.h>
#if defined(ESP8266)
#include <ESPAsyncTCP.h>
#elif defined(ESP32)
#include <AsyncTCP.h>
#endif
#include "LEDs.h"
#include "Main.h"
#include "Storage.h"
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>

class API {
public:
  API(LEDs &leds, Storage &storage);
  void init();
  static constexpr const char *MIME_TYPE_TEXT = "plain/text";

private:
  void handlePostSegment(uint8_t *data, size_t &len,
                         AsyncWebServerRequest *request);
  void handlePutStatus(uint8_t *data, size_t &len,
                       AsyncWebServerRequest *request);
  void handleGetSegments(AsyncWebServerRequest *request);
  void handlePutLEDs(uint8_t *data, size_t &len,
                     AsyncWebServerRequest *request);

  AsyncWebServer *server;
  LEDs *leds;
  Storage *storage;
  int8 statusColorIndexBuffer[NUM_SEGMENTS];
  char responseBuffer[100];
};
