#include <Arduino.h>
#include <FastLED.h>

#define DATA_PIN 18

// #define LEDS_MODULE 16
// #define NUM_MODULES 15
// #define NUM_LEDS (LEDS_MODULE * NUM_MODULES)
#define NUM_LEDS 240

// All pixels data
CRGB leds[NUM_LEDS];

uint8_t offsetsA[] = {0, 1, 2, 8, 9, 10, 11};
uint8_t offsetsB[] = {5, 6, 7, 12, 13, 14, 15};
uint8_t offsetsC[] = {3, 4};

void setup()
{
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  FastLED.clear();
  FastLED.show();
  Serial.begin(115200);
}

void fade(CRGB leds[], int numLeds)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i].nscale8(220);
  }
}

void ebbAndFlow(CRGB leds[], uint8_t offsets[], uint8_t numLeds)
{
  uint16_t t = millis() / 7;
  uint8_t scale = 15;

  for (int i = 0; i < numLeds; i++)
  {
    uint8_t noise = inoise8(i * scale, t);
    // leds[i] = CRGB(noise, noise, noise);
    leds[offsets[i]].g = noise;
  }

  // FastLED.show();
}

void lava(CRGB leds[], uint8_t offsets[], uint8_t numLeds)
{
  uint16_t t = millis() / 5;
  uint8_t scale = 150;

  for (int i = 0; i < numLeds; i++)
  {
    uint8_t val = inoise8(i * scale, t);
    leds[offsets[i]] = CHSV(CRGB::Red, 255, val);
  }

  // FastLED.show();
}

void loop()
{
  EVERY_N_SECONDS(3)
  {
    leds[3].r = 255;
    leds[4].r = 255;
  }

  EVERY_N_MILLIS(17)
  {
    ebbAndFlow(leds, offsetsA, 7);
    lava(leds, offsetsB, 7);
    leds[3].nscale8(240);
    leds[4].nscale8(240);
    FastLED.show();
  }
}
