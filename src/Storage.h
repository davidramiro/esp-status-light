#include <EEPROM.h>
#include "Main.h"

bool saveSegmentName(uint8 segment, const char name[]);
bool fetchSegmentName(uint8 segment, char *outStr);
bool findSegmentByName(const char *name, uint8 *segment);
void initStorage();
bool saveBrightness(uint8 brightness);
uint8 fetchBrightness();