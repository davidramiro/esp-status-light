#include "API.h"
#include "LEDs.h"
#include "Storage.h"

API::API(LEDs &leds, Storage &storage) {
  this->server = new AsyncWebServer(80);
  this->leds = &leds;
  this->storage = &storage;
}

void API::init() {
  Log.verboseln("setting default LED color");
  int defaultIndex = LEDs::getColorIndexByName(DEFAULT_STATUS);
  if (defaultIndex == -1) {
    Log.fatalln("could not find default status %s", DEFAULT_STATUS);
    return;
  }

  for (int i = 0; i < NUM_SEGMENTS; i++) {
    statusColorIndexBuffer[i] = defaultIndex;
  }

  this->server->on(
      "/api/segments", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
      [this](AsyncWebServerRequest *request, uint8_t *data, size_t len,
             size_t index,
             size_t total) { this->handlePostSegment(data, len, request); });

  this->server->on(
      "/api/status", HTTP_PUT, [](AsyncWebServerRequest *request) {}, NULL,
      [this](AsyncWebServerRequest *request, uint8_t *data, size_t len,
             size_t index,
             size_t total) { this->handlePutStatus(data, len, request); });

  this->server->on("/api/segments", HTTP_GET,
                   [this](AsyncWebServerRequest *request) {
                     this->handleGetSegments(request);
                   });

  this->server->on(
      "/api/leds", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
      [this](AsyncWebServerRequest *request, uint8_t *data, size_t len,
             size_t index,
             size_t total) { this->handlePutLEDs(data, len, request); });

  this->server->onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, MIME_TYPE_TEXT, "not found");
  });

  this->server->begin();
}

void API::handleGetSegments(AsyncWebServerRequest *request) {
  Log.infoln("GET /api/segments");

  if (request->hasParam("segment")) {
    AsyncWebParameter *p = request->getParam("segment");
    Log.verboseln("requested segment: %s", p->value());

    char name[NAME_BUFFER_LEN];
    int segment = p->value().toInt();

    JsonDocument doc;

    bool success = this->storage->fetchSegmentName(segment, name);
    Log.verboseln("fetched segment name %s", name);
    if (success) {
      doc["name"] = name;
    } else {
      Log.warningln("could not fetch segment name for %s", name);
    }

    doc["segment"] = segment;
    doc["status"] = colors[statusColorIndexBuffer[segment]].name;

    String response;
    serializeJson(doc, response);

    Log.verboseln("sending response: %s", response.c_str());

    request->send(200, "application/json", response);
  } else {
    Log.verboseln("building response for segment info");
    JsonDocument doc;
    for (uint8 i = 0; i < NUM_SEGMENTS; i++) {
      char name[NAME_BUFFER_LEN];
      bool success = this->storage->fetchSegmentName(i, name);
      if (success) {
        Log.verboseln("found name for adding to response: %s", name);
        doc[i]["name"] = name;
        doc[i]["segment"] = i;
        doc[i]["status"] = colors[statusColorIndexBuffer[i]].name;
      }
    }

    String response;
    serializeJson(doc, response);

    Log.verboseln("sending response: %s", response.c_str());

    request->send(200, "application/json", response);
  }
}

void API::handlePutLEDs(uint8_t *data, size_t &len,
                        AsyncWebServerRequest *request) {
  Log.infoln("PUT /api/leds");

  JsonDocument obj;
  DeserializationError error = deserializeJson(obj, (const char *)data, len);

  if (error) {
    Log.errorln("error deserializing json body: %s", error.c_str());
    request->send(500, MIME_TYPE_TEXT, error.f_str());
    return;
  }

  uint8 brightness = this->storage->fetchBrightness();

  Log.verboseln("handler: eeprom read brightness %d", brightness);

  if (obj.containsKey("brightness")) {
    brightness = obj["brightness"];
    Log.verboseln("handler: new brightness from request: %d", brightness);
    if (brightness > 100 || brightness < 1) {
      request->send(400, MIME_TYPE_TEXT,
                    "invalid brightness, stay between 1 and 100");
      return;
    }
    bool success = this->storage->saveBrightness(brightness);
    if (!success) {
      request->send(500, MIME_TYPE_TEXT, "error saving brightness");
      return;
    }
  }

  bool ledStatus = true;

  if (obj.containsKey("leds")) {
    ledStatus = obj["leds"];
  }

  if (ledStatus) {
    this->leds->writeBufferToLeds(statusColorIndexBuffer, brightness);
  } else {
    brightness = 0;
    this->leds->turnOff();
  }

  sprintf(responseBuffer, "leds set to %d", brightness);
  request->send(200, MIME_TYPE_TEXT, String(responseBuffer));
}

void API::handlePostSegment(uint8_t *data, size_t &len,
                            AsyncWebServerRequest *request) {
  Log.infoln("POST /api/segments");

  JsonDocument obj;
  DeserializationError error = deserializeJson(obj, (const char *)data, len);

  if (error) {
    Log.errorln("error deserializing json body: %s", error.c_str());
    request->send(500, MIME_TYPE_TEXT, error.f_str());
    return;
  }

  uint8_t segment = obj["segment"];
  if (LEDs::LEDS_PER_SEGMENT * (segment + 1) > NUMPIXELS) {
    sprintf(responseBuffer,
            "requested segment starts at LED: %d, available LEDS: %d",
            LEDs::LEDS_PER_SEGMENT * (segment + 1), NUMPIXELS);
    Log.warningln(responseBuffer);
    request->send(400, MIME_TYPE_TEXT, String(responseBuffer));
    return;
  }

  const char *name = obj["name"];
  if (strlen(name) > NAME_BUFFER_LEN - 2) {
    Log.warningln("requested name with length of: %d, maximum is %d",
                  strlen(name), NAME_BUFFER_LEN - 2);

    sprintf(responseBuffer, "max chars for name: %d", (NAME_BUFFER_LEN - 2));
    request->send(400, MIME_TYPE_TEXT, String(responseBuffer));
    return;
  }

  bool success = this->storage->saveSegmentName(segment, name);
  if (!success) {
    Log.errorln("error writing name to eeprom");
    request->send(500, MIME_TYPE_TEXT, "internal error saving segment name");
    return;
  }

  sprintf(responseBuffer, "saved %s to segment %d", name, segment);
  request->send(200, MIME_TYPE_TEXT, String(responseBuffer));
}

void API::handlePutStatus(uint8_t *data, size_t &len,
                          AsyncWebServerRequest *request) {
  Log.infoln("PUT /api/status");

  JsonDocument obj;
  DeserializationError error = deserializeJson(obj, (const char *)data, len);

  if (error) {
    Log.errorln("error deserializing request: %s", error.c_str());
    request->send(500, MIME_TYPE_TEXT, error.f_str());
    return;
  }

  uint8 segment = 8;

  if (obj.containsKey("segment")) {
    segment = obj["segment"];
    Log.verboseln("status change request for segment %d", segment);
  } else if (obj.containsKey("name")) {
    const char *name = obj["name"];
    bool found = this->storage->findSegmentByName(name, &segment);
    if (!found) {
      Log.warningln("segment for name %s not found", name);
      request->send(404, MIME_TYPE_TEXT, "name not found");
      return;
    }

    Log.verboseln("status change request for name %s, found on segment %d",
                  name, segment);
  }

  if (LEDs::LEDS_PER_SEGMENT * (segment + 1) > NUMPIXELS) {
    request->send(400, MIME_TYPE_TEXT, "invalid segment number");
    return;
  }

  const char *status = obj["status"];

  int index = LEDs::getColorIndexByName(status);
  if (index == -1) {
    request->send(400, MIME_TYPE_TEXT, "invalid status identifier");
    return;
  }

  statusColorIndexBuffer[segment] = index;

  this->leds->writeBufferToLeds(statusColorIndexBuffer,
                                storage->fetchBrightness());

  sprintf(responseBuffer, "changed segment %d to status %s with brightness %d",
          segment, status, this->storage->fetchBrightness());
  request->send(200, MIME_TYPE_TEXT, String(responseBuffer));
}
