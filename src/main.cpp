#include <Arduino.h>
#include <FastLED.h>

// #define NUM_LEDS 240
#define NUM_LEDS 32
#define DATA_PIN 18

CRGB leds[NUM_LEDS];

uint16_t x;
int scale;
uint16_t t;

void setup()
{
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  FastLED.clear();
  FastLED.show();
  Serial.begin(115200);
}

void fadeall()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i].nscale8(220);
  }
}

void ebbAndFlow(CRGB leds[], int numLeds)
{
  t = millis() / 7;
  // scale = beatsin8(1, 0, 60);
  scale = 15;

  for (int i = 0; i < numLeds; i++)
  {
    uint8_t noise = inoise8(i * scale, t);
    // leds[i] = CRGB(noise, noise, noise);
    leds[i].g = noise;
  }

  FastLED.show();
}

void loop()
{
  EVERY_N_MILLIS(6)
  {
    ebbAndFlow(leds, NUM_LEDS);
  }
}
