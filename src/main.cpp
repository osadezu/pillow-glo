#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 16
#define DATA_PIN 18

CRGB leds[NUM_LEDS];

void setup()
{
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(128);
  randomSeed(analogRead(0));
  Serial.begin(115200);
}

void fadeall()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i].nscale8(220);
  }
}

void loop()
{
  uint8_t scale;

  for (int i = 0; i < NUM_LEDS; i++)
  {
    scale = beatsin8(30, 0, 255, 0, i * 256 / 16);
    leds[i] = CRGB(scale, scale, scale);
  }

  FastLED.show();
}
