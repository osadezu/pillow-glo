#include <Arduino.h>
#include <FastLED.h>

// #define NUM_LEDS 240
#define NUM_LEDS 16
#define DATA_PIN 18

CRGB leds[NUM_LEDS];

uint16_t x;
int scale;
uint16_t t;

void setup()
{
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
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

  t = millis() / 5;
  // scale = beatsin8(1, 0, 60);
  scale = 15;

  for (int i = 0; i < NUM_LEDS; i++)
  {
    uint8_t noise = inoise8(i * scale, t);
    leds[i] = CRGB(noise, 0, 0);
  }

  // Serial.print(0);
  // Serial.print(" ");
  // Serial.print(scale * 4);
  // Serial.print(" ");
  // Serial.print(map(t, 0, 65535, 0, 240));
  // Serial.print(" ");
  // Serial.print(leds[0].r);
  // Serial.print(" ");
  // Serial.print(leds[1].r);
  // Serial.print(" ");
  // Serial.println(240);

  FastLED.show();
}
