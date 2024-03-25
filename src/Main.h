#pragma once

#include <Adafruit_NeoPixel.h>
#include <ArduinoLog.h>

#define PIN D2
#define NUMPIXELS 24
#define NUM_SEGMENTS 2
#define NAME_BUFFER_LEN 30
#define DEFAULT_STATUS "off"
#define ESP_HOST_NAME "ESPStatusLight"
#define LOG_LEVEL LOG_LEVEL_INFO
#define BRIGHTNESS_EEPROM_ADDR (NUM_SEGMENTS * NAME_BUFFER_LEN + 2)