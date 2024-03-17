#include <EEPROM.h>
#include "main.h"

bool writeSegmentNameToEEPROM(uint8 segment, const char name[]);
bool getNameFromEEPROM(uint8 segment, char *outStr);
bool findSegmentFromName(const char *name, uint8 *segment);
void initEEPROM();