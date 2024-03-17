#include <Adafruit_NeoPixel.h>
#include "main.h"

struct StatusColor
{
    char *name;
    int8_t red;
    int8_t green;
    int8_t blue;
};

const int ledsPerSegment = NUMPIXELS / NUM_SEGMENTS;
const StatusColor colors[] = {{strdup("free"), 0, 80, 0}, {strdup("dnd"), 80, 0, 0}, {strdup("occupied"), 60, 40, 0}, {strdup("off"), 0, 0, 0}};

void initLEDs();
void writeBufferToLeds(int8 indexBuffer[]);
void turnOffLeds();
int getColorIndexByName(const char *name);