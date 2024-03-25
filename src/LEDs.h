#pragma once

#include "Main.h"

struct StatusColor {
  char *name;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

const StatusColor colors[] = {{strdup("free"), 0, 127, 0},
                              {strdup("dnd"), 127, 0, 0},
                              {strdup("occupied"), 127, 127, 0},
                              {strdup("off"), 0, 0, 0}};

class LEDs {

public:
  LEDs();

  void init();
  void writeBufferToLeds(int8 indexBuffer[], int brightness);
  void turnOff();
  static int getColorIndexByName(const char *name);
  static constexpr const int LEDS_PER_SEGMENT = NUMPIXELS / NUM_SEGMENTS;

private:
  static void approachTargetValue(uint8_t &value, uint8_t target);
};
