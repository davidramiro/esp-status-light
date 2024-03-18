#include "server.h"
#include "eeprom.h"
#include "leds.h"

const char *mimeTypeText = "plain/text";

char responseBuffer[100];
int8 statusColorIndexBuffer[NUM_SEGMENTS];

AsyncWebServer server(80);

void initServer()
{
    Log.verboseln("setting default LED color");
    int defaultIndex = getColorIndexByName(DEFAULT_STATUS);
    if (defaultIndex == -1)
    {
        Log.fatalln("could not find default status %s", DEFAULT_STATUS);
        return;
    }

    for (int i = 0; i < NUM_SEGMENTS; i++)
    {
        statusColorIndexBuffer[i] = defaultIndex;
    }

    server.on(
        "/api/segments", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        { handlePostSegment(data, len, request); });

    server.on(
        "/api/status", HTTP_PUT, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        { handlePutStatus(data, len, request); });

    server.on("/api/segments", HTTP_GET, [](AsyncWebServerRequest *request)
              { handleGetSegments(request); });

    server.on(
        "/api/leds", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        { handlePutLEDs(data, len, request); });

    server.onNotFound(handleNotFound);

    server.begin();
}

void handleGetSegments(AsyncWebServerRequest *request)
{
    Log.infoln("GET /api/segments");

    if (request->hasParam("segment"))
    {
        AsyncWebParameter *p = request->getParam("segment");
        Log.verboseln("requested segment: %s", p->value());

        char name[NAME_BUFFER_LEN];
        int segment = p->value().toInt();

        JsonDocument doc;

        bool success = getNameFromEEPROM(segment, name);
        if (success)
        {
            doc["name"] = name;
        }

        doc["segment"] = segment;
        doc["status"] = colors[statusColorIndexBuffer[segment]].name;

        String response;
        serializeJson(doc, response);

        Log.verboseln("sending response: %s", response.c_str());

        request->send(200, "application/json", response);
    }
    else
    {
        Log.verboseln("building response for segment info");
        JsonDocument doc;
        for (uint8 i = 0; i < NUM_SEGMENTS; i++)
        {
            char name[NAME_BUFFER_LEN];
            bool success = getNameFromEEPROM(i, name);
            if (success)
            {
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

void handlePutLEDs(uint8_t *data, size_t &len, AsyncWebServerRequest *request)
{
    Log.infoln("PUT /api/leds");

    JsonDocument obj;
    DeserializationError error = deserializeJson(obj, (const char *)data, len);

    if (error)
    {
        Log.errorln("error deserializing json body: %s", error.c_str());
        request->send(500, mimeTypeText, error.f_str());
        return;
    }

    uint8 brightness = getBrightnessFromEEPROM();

    if (obj.containsKey("brightness")) {
        brightness = obj["brightness"];
        if (brightness > 100 || brightness < 1) {
            request->send(400, mimeTypeText,"invalid brightness, stay between 1 and 100");
            return;
        }
        bool success = saveBrightnessToEEPROM(brightness);
        if (!success) {
            request->send(500, mimeTypeText,"error saving brightness");
            return;
        }
    }

    bool ledStatus = true;

    if (obj.containsKey("leds")) {
        ledStatus = obj["leds"];
    }

    if (ledStatus)
    {
        writeBufferToLeds(statusColorIndexBuffer);
    }
    else
    {
        brightness = 0;
        turnOffLeds();
    }

    sprintf(responseBuffer, "leds set to %d", brightness);
    request->send(200, mimeTypeText, String(responseBuffer));
}

void handlePostSegment(uint8_t *data, size_t &len, AsyncWebServerRequest *request)
{
    Log.infoln("POST /api/segments");

    JsonDocument obj;
    DeserializationError error = deserializeJson(obj, (const char *)data, len);

    if (error)
    {
        Log.errorln("error deserializing json body: %s", error.c_str());
        request->send(500, mimeTypeText, error.f_str());
        return;
    }

    uint8_t segment = obj["segment"];
    if (ledsPerSegment * (segment + 1) > NUMPIXELS)
    {
        sprintf(responseBuffer, "requested segment starts at LED: %d, available LEDS: %d", ledsPerSegment * (segment + 1), NUMPIXELS);
        Log.warningln(responseBuffer);
        request->send(400, mimeTypeText, String(responseBuffer));
        return;
    }

    const char *name = obj["name"];
    if (strlen(name) > NAME_BUFFER_LEN - 2)
    {
        Log.warningln("requested name with length of: %d, maximum is %d", strlen(name), NAME_BUFFER_LEN - 2);

        sprintf(responseBuffer, "max chars for name: %d", (NAME_BUFFER_LEN - 2));
        request->send(400, mimeTypeText, String(responseBuffer));
        return;
    }

    bool success = writeSegmentNameToEEPROM(segment, name);
    if (!success)
    {
        Log.errorln("error writing name to eeprom");
        request->send(500, mimeTypeText, "internal error saving segment name");
        return;
    }

    sprintf(responseBuffer, "saved %s to segment %d", name, segment);
    request->send(200, mimeTypeText, String(responseBuffer));
}

void handlePutStatus(uint8_t *data, size_t &len, AsyncWebServerRequest *request)
{
    Log.infoln("PUT /api/status");

    JsonDocument obj;
    DeserializationError error = deserializeJson(obj, (const char *)data, len);

    if (error)
    {
        Log.errorln("error deserializing request: %s", error.c_str());
        request->send(500, mimeTypeText, error.f_str());
        return;
    }

    uint8 segment = 8;

    if (obj.containsKey("segment"))
    {
        segment = obj["segment"];
        Log.verboseln("status change request for segment %d", segment);
    }
    else if (obj.containsKey("name"))
    {
        const char *name = obj["name"];
        bool found = findSegmentFromName(name, &segment);
        if (!found)
        {
            Log.warningln("segment for name %s not found", name);
            request->send(404, mimeTypeText, "name not found");
            return;
        }

        Log.verboseln("status change request for name %s, found on segment %d", name, segment);
    }

    if (ledsPerSegment * (segment + 1) > NUMPIXELS)
    {
        request->send(400, mimeTypeText, "invalid segment number");
        return;
    }

    const char *status = obj["status"];

    int index = getColorIndexByName(status);
    if (index == -1)
    {
        request->send(400, mimeTypeText, "invalid status identifier");
        return;
    }

    statusColorIndexBuffer[segment] = index;

    writeBufferToLeds(statusColorIndexBuffer);

    sprintf(responseBuffer, "changed segment %d to status %s with brightness %d", segment, status, getBrightnessFromEEPROM());
    request->send(200, mimeTypeText, String(responseBuffer));
}

void handleNotFound(AsyncWebServerRequest *request)
{
    request->send(404, mimeTypeText, "not found");
}