#include <Arduino.h>
#include <FastLED.h>

// WS2811 output
#define DATA_PIN 18 // GPIO18 @ DevKit right-9

/* Comment this line for final pixel setup */
#define TESTGRID

#ifndef TESTGRID
// Trailer setup values
#define NUM_MODULES 15
#define LEDS_MODULE 16
#else
// Test grid setup values
#define NUM_MODULES 16
#define LEDS_MODULE 4
#endif

#define TOTAL_LEDS (LEDS_MODULE * NUM_MODULES)

// Observed values from noise8
#define NOISE_LOWER_BOUND = 34;
#define NOISE_UPPER_BOUND = 218;

namespace Pixels
{
  CRGB leds[TOTAL_LEDS];

  struct uvLoop
  {
    uint8_t ledCount = LEDS_MODULE;
    CRGB *leds;
    uint8_t excitation = 128;
    uint8_t deferred = 0;
    uvLoop *up = NULL;
    uvLoop *right = NULL;
    uvLoop *down = NULL;
    uvLoop *left = NULL;
  };

  uint8_t offsetsA[] = {0, 1, 8, 9, 10, 11};
  uint8_t offsetsB[] = {6, 7, 12, 13, 14, 15};
  uint8_t offsetsC[] = {2, 3, 4, 5};

  uint8_t channelMask = 2;

  uvLoop allLoops[NUM_MODULES];

// TODO: Move definitions to independent files
#ifndef TESTGRID
// Trailer setup
#else
  // Test grid setup
  int loopGridHeight = 2;
  int loopGridWidth = 8;
  uvLoop *loopGrid[2][8] = {
      {&allLoops[0], &allLoops[3], &allLoops[4], &allLoops[7], &allLoops[8], &allLoops[11], &allLoops[12], &allLoops[15]},
      {&allLoops[1], &allLoops[2], &allLoops[5], &allLoops[6], &allLoops[9], &allLoops[10], &allLoops[13], &allLoops[14]}};
#endif

  // Behavior Variables
  uint8_t decay = 254;               // proportion of 255 to fade inactive loops each frame
  uint8_t activationThreshold = 127; // excitation level at which loops can turn on

  void checkMinMaxNoise(uint8_t noiseSample)
  {
    static uint8_t minNoise = 255;
    static uint8_t maxNoise = 0;

    if (noiseSample < minNoise)
      minNoise = noiseSample;
    if (noiseSample > maxNoise)
      maxNoise = noiseSample;

    Serial.println("MinMaxNoise");
    Serial.println(minNoise);
    Serial.println(maxNoise);
  }

  void loopExcitation()
  {
    uint16_t t = millis();
    uint8_t yScale = 50;
    uint8_t xScale = 50;

    for (int y = 0; y < loopGridHeight; y++)
    {
      for (int x = 0; x < loopGridWidth; x++)
      {
        uint8_t noise = inoise8(y * yScale, x * xScale, t);
        checkMinMaxNoise(noise);
        loopGrid[y][x]->excitation = noise;
      }
    }
  }

  void setLoop(uvLoop loop, CRGB value)
  {
    for (int i = 0; i < loop.ledCount; i++)
    {
      loop.leds[i] = value;
    }
  }

  void setup()
  {
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, TOTAL_LEDS);
    FastLED.setBrightness(128);
    FastLED.clear();
    FastLED.show();

    for (int i = 0; i < NUM_MODULES; i++)
    {
      allLoops[i].leds = &leds[i * LEDS_MODULE];
    }
  }

  void setChannels()
  {
    channelMask = (channelMask) % 7 + 1; // All channel combinations RGB
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

  void fadeAll()
  {
    for (int i = 0; i < TOTAL_LEDS; i++)
    {
      leds[i].nscale8(decay);
    }
  }

  void clear(CRGB leds[], uint8_t offsets[], uint8_t numLeds)
  {
    for (int i = 0; i < numLeds; i++)
    {
      leds[offsets[i]] = CRGB(0, 0, 0);
    }
  }

  void ebbAndFlowLoops(uvLoop loop)
  {
    uint16_t t = millis() / 2;
    uint8_t scale = 50;

    for (int i = 0; i < loop.ledCount; i++)
    {
      uint8_t noise = inoise8(i * scale, t);
      uint32_t noiseRGB = allChannels(noise);
      loop.leds[i] = CRGB(noiseRGB);
    }
  }

  void ebbAndFlowLeds(CRGB leds[], uint8_t offsets[], uint8_t numLeds)
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

    for (int i = 0; i < TOTAL_LEDS; i++)
    {
      uint8_t noise = inoise8(i * scale, t);
      uint32_t noiseRGB = allChannels(noise);
      leds[i] = CRGB(noiseRGB);
    }
  }

  void lavaLoops(uvLoop loop)
  {
    uint16_t t = millis() / 5;
    uint8_t scale = 150;

    for (int i = 0; i < loop.ledCount; i++)
    {
      uint8_t val = inoise8(i * scale, t);
      loop.leds[i] = CHSV(CRGB::Red, 255, val);
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
    EVERY_N_SECONDS(1)
    {
      // uint8_t thisLoop = random8(NUM_MODULES - 1);
      // allLoops[thisLoop].excitation = random8();
      loopExcitation();
    }

    EVERY_N_MILLIS(20)
    {
      fadeAll();

      // ebbAndFlowAll();

      // CRGB colors[3] = {CRGB::Red,
      //                   CRGB::Green,
      //                   CRGB::Blue};

      // for (int i = 0; i < NUM_MODULES; i++)
      // {
      //   setLoop(allLoops[i], colors[i % 3]);
      // }

      for (int i = 0; i < NUM_MODULES; i++)
      {
        if (allLoops[i].excitation > activationThreshold)
        {
          ebbAndFlowLoops(allLoops[i]);
        }
        // lavaLoops(allLoops[i]);
      }

      FastLED.show();
    }
  }
};
