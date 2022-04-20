#include <Arduino.h>
#include <FastLED.h>

#define DATA_PIN 18

#define LEDS_MODULE 16
#define NUM_MODULES 15
#define NUM_LEDS (LEDS_MODULE * NUM_MODULES)

// All pixels data
CRGB leds[NUM_LEDS];

uint8_t offsetsA[] = {0, 1, 8, 9, 10, 11};
uint8_t offsetsB[] = {6, 7, 12, 13, 14, 15};
uint8_t offsetsC[] = {2, 3, 4, 5};

uint8_t gate = 0;

void setup()
{
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  FastLED.clear();
  FastLED.show();
  Serial.begin(115200);
}

void fade(CRGB leds[], uint8_t offsets[], uint8_t numLeds)
{
  for (int i = 0; i < numLeds; i++)
  {
    leds[offsets[i]].nscale8(240);
  }
}

void clear(CRGB leds[], uint8_t offsets[], uint8_t numLeds)
{
  for (int i = 0; i < numLeds; i++)
  {
    leds[offsets[i]] = CRGB(0, 0, 0);
  }
}

void ebbAndFlow(CRGB leds[], uint8_t offsets[], uint8_t numLeds)
{
  uint16_t t = millis() / 7;
  uint8_t scale = 15;

  for (int i = 0; i < numLeds; i++)
  {
    uint8_t noise = inoise8(i * scale, t);
    leds[offsets[i]] = CRGB(noise, noise, noise);
    // leds[offsets[i]].g = noise;
  }
}

void ebbAndFlowAll()
{
  uint16_t t = millis() / 7;
  uint8_t scale = 15;

  for (int i = 0; i < NUM_LEDS; i++)
  {
    uint8_t noise = inoise8(i * scale, t);
    leds[i] = CRGB(0, noise, 0);
  }
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

  // EVERY_N_SECONDS(5)
  // {
  //   gate ^= 1;
  // }

  EVERY_N_MILLIS(17)
  {
    ebbAndFlowAll();

    // if (gate)
    // {
    //   ebbAndFlow(leds, offsetsA, 6);
    //   fade(leds, offsetsB, 6);
    // }
    // else
    // {
    //   ebbAndFlow(leds, offsetsB, 6);
    //   fade(leds, offsetsA, 6);
    // }
    // lava(leds, offsetsB, 7);
    // leds[3].nscale8(240);
    // leds[4].nscale8(240);
    FastLED.show();
  }
}
