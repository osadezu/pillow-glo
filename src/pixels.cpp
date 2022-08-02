#include <Arduino.h>
#include <FastLED.h>

// WS2811 output
#define DATA_PIN 18 // GPIO18 @ DevKit right-9
#define LEDS_MODULE 16
#define NUM_MODULES 15
#define NUM_LEDS (LEDS_MODULE * NUM_MODULES)

namespace Pixels
{
  CRGB leds[NUM_LEDS];

  struct uvLoop
  {
    uint8_t excitation = 1;
    CRGB *leds;
  };

  uint8_t offsetsA[] = {0, 1, 8, 9, 10, 11};
  uint8_t offsetsB[] = {6, 7, 12, 13, 14, 15};
  uint8_t offsetsC[] = {2, 3, 4, 5};

  uint8_t channelMask = 2;

  void setup()
  {
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    FastLED.setBrightness(128);
    FastLED.clear();
    FastLED.show();
  }

  void setChannels()
  {
    channelMask = (channelMask + 1) % 8;
    Serial.println(channelMask);
  }

  uint32_t allChannels(uint8_t colorcode)
  {
    uint32_t channels = 0;
    if (channelMask & 0b001)
      channels |= (colorcode << 0);
    if (channelMask & 0b010)
      channels |= (colorcode << 8);
    if (channelMask & 0b100)
      channels |= (colorcode << 16);
    return channels;
  }

  void setGlobalBrightness(uint8_t brightness)
  {
    FastLED.setBrightness(brightness);
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
      uint32_t noiseRGB = allChannels(noise);
      leds[offsets[i]] = CRGB(noiseRGB);
    }
  }

  void ebbAndFlowAll()
  {
    uint16_t t = millis() / 7;
    uint8_t scale = 15;

    for (int i = 0; i < NUM_LEDS; i++)
    {
      uint8_t noise = inoise8(i * scale, t);
      uint32_t noiseRGB = allChannels(noise);
      leds[i] = CRGB(noiseRGB);
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
  }

  void loop()
  {
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

};
