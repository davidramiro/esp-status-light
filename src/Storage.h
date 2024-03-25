#pragma once

#include "Main.h"
#include <EEPROM.h>

class Storage {
public:
  Storage();

  bool saveSegmentName(uint8 segment, const char name[]);
  bool fetchSegmentName(uint8 segment, char *outStr);
  bool findSegmentByName(const char *name, uint8 *segment);
  bool saveBrightness(uint8 brightness);
  uint8 fetchBrightness();
};
