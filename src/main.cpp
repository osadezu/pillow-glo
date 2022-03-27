#include <Arduino.h>
#include <FastLED.h>

#define DATA_PIN 18

#define NUM_LEDS 32
#define NUM_MODULES 2
// CRGB leds[NUM_MODULES][NUM_LEDS];
CRGB leds[NUM_LEDS];

void setup()
{
  // FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds[0], NUM_LEDS);
  // FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds[1], NUM_LEDS);
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

void ebbAndFlow(CRGB leds[], int numLeds)
{
  uint16_t t = millis() / 7;
  uint8_t scale = 15;

  for (int i = 0; i < numLeds; i++)
  {
    uint8_t noise = inoise8(i * scale, t);
    // leds[i] = CRGB(noise, noise, noise);
    leds[i].g = noise;
  }

  FastLED.show();
}

void lava(CRGB leds[], int numLeds)
{
  uint16_t t = millis() / 5;
  uint8_t scale = 150;

  for (int i = 0; i < numLeds; i++)
  {
    uint8_t val = inoise8(i * scale, t);
    leds[i] = CHSV(CRGB::Red, 255, val);
  }

  FastLED.show();
}

void loop()
{
  EVERY_N_MILLIS(6)
  {
    // ebbAndFlow(leds, NUM_LEDS);
    lava(leds, NUM_LEDS);
  }
}
