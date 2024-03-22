#include "LEDs.h"
#include "Storage.h"

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void initLEDs()
{
    pixels.begin();
    pixels.clear();
    pixels.show();
}

void writeBufferToLeds(int8 indexBuffer[])
{
    uint8 brightness = fetchBrightness() * 255 / 100;

    for (uint i = 0; i < NUM_SEGMENTS; i++)
    {
        StatusColor statusColor = colors[indexBuffer[i]];
        uint8_t targetRed = statusColor.red * brightness;
        uint8_t targetGreen = statusColor.green * brightness;
        uint8_t targetBlue = statusColor.blue * brightness;
        uint32_t color = pixels.getPixelColor(i * ledsPerSegment);
        uint8_t red = (color >> 16) & 255;
        uint8_t green = (color >> 8) & 255;
        uint8_t blue = color & 255;

        Log.verboseln("current color: %d, %d, %d", red, green, blue);
        Log.verboseln("target color: %d, %d, %d", targetRed, targetGreen, targetBlue);

        if (red == targetRed && blue == targetBlue && green == targetGreen)
        {
            continue;
        }

        while (red != targetRed || blue != targetBlue || green != targetGreen)
        {
            approachTargetValue(red, targetRed);
            approachTargetValue(green, targetGreen);
            approachTargetValue(blue, targetBlue);

            pixels.fill(pixels.Color(red, green, blue), i * ledsPerSegment, ledsPerSegment);
            pixels.show();

            delay(7);
        }
    }
}

void turnOffLeds()
{

    for (uint i = 0; i < NUM_SEGMENTS; i++)
    {
        int32_t color = pixels.getPixelColor(i * ledsPerSegment);
        uint8_t red = (color >> 16) & 255;
        uint8_t green = (color >> 8) & 255;
        uint8_t blue = color & 255;

        while (red != 0 || blue != 0 || green != 0)
        {
            approachTargetValue(red, 0);
            approachTargetValue(green, 0);
            approachTargetValue(blue, 0);

            pixels.fill(pixels.Color(red, green, blue), i * ledsPerSegment, ledsPerSegment);
            pixels.show();

            delay(4);
        }
    }
}

int getColorIndexByName(const char *name)
{
    for (uint8 i = 0; i < sizeof(colors) / sizeof(StatusColor); i++)
    {
        if (strcmp(name, colors[i].name) == 0)
        {
            return i;
        }
    }
    Log.warningln("status color with name %s not found", name);
    return -1;
}

void approachTargetValue(uint8_t &value, uint8_t target) {
    if (value < target) {
        value++;
    } else if (value > target) {
        value--;
    }
}