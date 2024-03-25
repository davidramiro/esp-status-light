#include "LEDs.h"
#include "Storage.h"

Adafruit_NeoPixel pixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

LEDs::LEDs() {
  pixel.begin();
  pixel.clear();
  pixel.show();
}

void LEDs::init() {}

void LEDs::writeBufferToLeds(int8 indexBuffer[], int brightness) {
  int brightnessFactor = brightness * 255 / 100;

  for (uint i = 0; i < NUM_SEGMENTS; i++) {
    StatusColor statusColor = colors[indexBuffer[i]];
    uint8_t targetRed = statusColor.red * brightnessFactor;
    uint8_t targetGreen = statusColor.green * brightnessFactor;
    uint8_t targetBlue = statusColor.blue * brightnessFactor;
    uint32_t color = pixel.getPixelColor(i * LEDS_PER_SEGMENT);
    uint8_t red = (color >> 16) & 255;
    uint8_t green = (color >> 8) & 255;
    uint8_t blue = color & 255;

    Log.verboseln("current color: %d, %d, %d", red, green, blue);
    Log.verboseln("target color: %d, %d, %d", targetRed, targetGreen,
                  targetBlue);

    if (red == targetRed && blue == targetBlue && green == targetGreen) {
      continue;
    }

    while (red != targetRed || blue != targetBlue || green != targetGreen) {
      LEDs::approachTargetValue(red, targetRed);
      LEDs::approachTargetValue(green, targetGreen);
      LEDs::approachTargetValue(blue, targetBlue);

      pixel.fill(pixel.Color(red, green, blue), i * LEDS_PER_SEGMENT,
                 LEDS_PER_SEGMENT);
      pixel.show();

      delay(7);
    }
  }
}

void LEDs::turnOff() {

  for (uint i = 0; i < NUM_SEGMENTS; i++) {
    int32_t color = pixel.getPixelColor(i * LEDS_PER_SEGMENT);
    uint8_t red = (color >> 16) & 255;
    uint8_t green = (color >> 8) & 255;
    uint8_t blue = color & 255;

    while (red != 0 || blue != 0 || green != 0) {
      LEDs::approachTargetValue(red, 0);
      LEDs::approachTargetValue(green, 0);
      LEDs::approachTargetValue(blue, 0);

      pixel.fill(Adafruit_NeoPixel::Color(red, green, blue),
                 i * LEDS_PER_SEGMENT, LEDS_PER_SEGMENT);
      pixel.show();

      delay(4);
    }
  }
}

int LEDs::getColorIndexByName(const char *name) {
  for (uint8 i = 0; i < sizeof(colors) / sizeof(StatusColor); i++) {
    if (strcmp(name, colors[i].name) == 0) {
      return i;
    }
  }
  Log.warningln("status color with name %s not found", name);
  return -1;
}

void LEDs::approachTargetValue(uint8_t &value, uint8_t target) {
  if (value < target) {
    value++;
  } else if (value > target) {
    value--;
  }
}