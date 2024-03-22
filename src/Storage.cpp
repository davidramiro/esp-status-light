#include "Storage.h"

void initStorage() {
  EEPROM.begin(4096);

  if (fetchBrightness() == 0) {
    saveBrightness(100);
  }
}

bool saveSegmentName(uint8 segment, const char name[]) {
  if (segment > 7) {
    Log.errorln("out of bounds, segment %d greater than 7 allowed", segment);
    return false;
  }

  uint8 startAddr = segment * NAME_BUFFER_LEN;
  size_t length = strlen(name);

  Log.verboseln("writing length of %d to address %d", length, startAddr);
  EEPROM.write(startAddr, length + 1);

  for (uint8 i = 0; i <= length; i++) {
    uint8 addrToWrite = startAddr + i + 1;
    Log.verboseln("writing char %d: %c to addr %d", i, name[i], addrToWrite);
    EEPROM.write(addrToWrite, name[i]);
  }

  Log.infoln("saving name %s to address %d", name, startAddr);

  return EEPROM.commit();
}

bool fetchSegmentName(uint8 segment, char *outStr) {
  uint8 startAddr = segment * NAME_BUFFER_LEN;
  uint8 length = EEPROM.read(startAddr);

  Log.verboseln("reading name from addr %d with length %d", startAddr, length);

  if (length > NAME_BUFFER_LEN - 2) {
    Log.errorln("not a name, length invalid");
    return false;
  }

  for (uint8 i = 0; i < length; i++) {
    uint8 addrToRead = startAddr + i + 1;

    outStr[i] = EEPROM.read(addrToRead);
    Log.verboseln("read char %d: %c from addr %d", i, outStr[i], addrToRead);
  }

  return true;
}

bool findSegmentByName(const char *name, uint8 *segment) {
  Log.infoln("looking up name %s in eeprom", name);

  for (uint8 i = 0; i < NUM_SEGMENTS; i++) {
    Log.verboseln("trying segment %d", i);

    char eepromName[NAME_BUFFER_LEN] = {0};
    bool success = fetchSegmentName(i, eepromName);
    if (!success) {
      Log.errorln("could not read name from eeprom for segment %d", i);
      return false;
    }

    Log.verboseln("read eeprom name: %s", eepromName);

    if (strcmp(name, eepromName) == 0) {
      Log.verboseln("found match! returning segment %d", i);
      *segment = i;
      return true;
    }
  }

  return false;
}

bool saveBrightness(uint8 brightness) {
  Log.verboseln("saving brightness value %d to eeprom", brightness);
  EEPROM.write(BRIGHTNESS_EEPROM_ADDR, brightness);
  return EEPROM.commit();
}

uint8 fetchBrightness() {
  uint8 brightness = EEPROM.read(BRIGHTNESS_EEPROM_ADDR);
  Log.verboseln("read brightness %d from eeprom", brightness);
  return brightness;
}