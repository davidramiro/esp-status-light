#include "leds.h"

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void initLEDs()
{
    pixels.begin();
    pixels.clear();
    pixels.show();
}

void writeBufferToLeds(int8 indexBuffer[])
{
    for (uint i = 0; i < NUM_SEGMENTS; i++)
    {
        StatusColor statusColor = colors[indexBuffer[i]];
        uint8_t targetRed = statusColor.red;
        uint8_t targetGreen = statusColor.green;
        uint8_t targetBlue = statusColor.blue;
        uint32_t color = pixels.getPixelColor(i * ledsPerSegment);
        uint8_t red = (color >> 16) & 255;
        uint8_t green = (color >> 8) & 255;
        uint8_t blue = color & 255;

        if (red == targetRed && blue == targetBlue && green == targetGreen)
        {
            continue;
        }

        while (red != targetRed || blue != targetBlue || green != targetGreen)
        {
            if (red < targetRed)
            {
                red++;
            }
            else if (red > targetRed)
            {
                red--;
            }
            if (green < targetGreen)
            {
                green++;
            }
            else if (green > targetGreen)
            {
                green--;
            }
            if (blue < targetBlue)
            {
                blue++;
            }
            else if (blue > targetBlue)
            {
                blue--;
            }

            pixels.fill(pixels.Color(red, green, blue), i * ledsPerSegment, ledsPerSegment);
            pixels.show();

            delay(10);
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
            if (red != 0)
            {
                red--;
            }
            if (blue != 0)
            {
                blue--;
            }
            if (green != 0)
            {
                green--;
            }

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