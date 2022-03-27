#include <Arduino.h>
#include <FastLED.h>

#define DATA_PIN 18

#define NUM_LEDS 16
#define NUM_MODULES 2
// CRGB leds[NUM_MODULES][NUM_LEDS];
CRGB leds[NUM_LEDS];

uint16_t x;
int scale;
uint16_t t;

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
  t = millis() / 7;
  // scale = beatsin8(1, 0, 60);
  scale = 15;

  for (int i = 0; i < numLeds; i++)
  {
    uint8_t noise = inoise8(i * scale, t);
    noise = map(noise, 50, 190, 0, 255);
    // leds[i] = CRGB(noise, noise, noise);
    leds[i].g = noise;
  }

  for (int in = 0; in < 256; in++)
  {
    Serial.print(in);
    Serial.print(", ");
    uint8_t out = map(in, 50, 190, 0, 255);
    Serial.println(out);
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
