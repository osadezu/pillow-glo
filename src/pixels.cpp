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
#define NOISE_LOWER_BOUND = 30;  // 34 much sooner
#define NOISE_UPPER_BOUND = 228; // 218 much sooner

namespace Pixels
{
  CRGB leds[TOTAL_LEDS];

  struct uvLoop
  {
    uint8_t ledCount = LEDS_MODULE;
    CRGB *leds;
    bool isActive = false;
    uint8_t excitation = 0;
    uint8_t cascadedExcitation = 0;
    uint8_t deferred = 0;
    uint8_t neighborCount = 0;
    uvLoop *neighbors[3];
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
  const int loopGridHeight = 2;
  const int loopGridWidth = 8;
  uvLoop *loopGrid[2][8] = {
      {&allLoops[0], &allLoops[3], &allLoops[4], &allLoops[7], &allLoops[8], &allLoops[11], &allLoops[12], &allLoops[15]},
      {&allLoops[1], &allLoops[2], &allLoops[5], &allLoops[6], &allLoops[9], &allLoops[10], &allLoops[13], &allLoops[14]}};

  void buildSetup()
  {
    allLoops[0].neighborCount = 1;
    allLoops[0].neighbors[0] = &allLoops[1];

    allLoops[1].neighborCount = 1;
    allLoops[1].neighbors[0] = &allLoops[0];

    allLoops[2].neighborCount = 2;
    allLoops[2].neighbors[0] = &allLoops[3];
    allLoops[2].neighbors[1] = &allLoops[4];

    allLoops[3].neighborCount = 2;
    allLoops[3].neighbors[0] = &allLoops[2];
    allLoops[3].neighbors[1] = &allLoops[4];

    allLoops[4].neighborCount = 2;
    allLoops[4].neighbors[0] = &allLoops[2];
    allLoops[4].neighbors[1] = &allLoops[3];

    allLoops[5].neighborCount = 1;
    allLoops[5].neighbors[0] = &allLoops[6];

    allLoops[6].neighborCount = 1;
    allLoops[6].neighbors[0] = &allLoops[5];

    allLoops[7].neighborCount = 3;
    allLoops[7].neighbors[0] = &allLoops[8];
    allLoops[7].neighbors[1] = &allLoops[9];
    allLoops[7].neighbors[2] = &allLoops[10];

    allLoops[8].neighborCount = 3;
    allLoops[8].neighbors[0] = &allLoops[7];
    allLoops[8].neighbors[1] = &allLoops[9];
    allLoops[8].neighbors[2] = &allLoops[10];

    allLoops[9].neighborCount = 3;
    allLoops[9].neighbors[0] = &allLoops[7];
    allLoops[9].neighbors[1] = &allLoops[8];
    allLoops[9].neighbors[2] = &allLoops[10];

    allLoops[10].neighborCount = 3;
    allLoops[10].neighbors[0] = &allLoops[7];
    allLoops[10].neighbors[1] = &allLoops[8];
    allLoops[10].neighbors[2] = &allLoops[9];

    allLoops[11].neighborCount = 3;
    allLoops[11].neighbors[0] = &allLoops[2];
    allLoops[11].neighbors[1] = &allLoops[10];
    allLoops[11].neighbors[2] = &allLoops[12];

    allLoops[12].neighborCount = 2;
    allLoops[12].neighbors[0] = &allLoops[13];
    allLoops[12].neighbors[1] = &allLoops[14];

    allLoops[13].neighborCount = 2;
    allLoops[13].neighbors[0] = &allLoops[12];
    allLoops[13].neighbors[1] = &allLoops[14];

    allLoops[14].neighborCount = 2;
    allLoops[14].neighbors[0] = &allLoops[12];
    allLoops[14].neighbors[1] = &allLoops[13];
  }
#endif

  // Behavior Variables
  const uint8_t decay = 254;               // proportion of 255 to fade inactive loops each frame
  const uint8_t activationThreshold = 170; // excitation level at which loops can turn on
  const uint8_t maxActiveLoops = 6;
  const uint8_t cascadingRate = 51; // proportion of 255 to fade inactive loops each frame

  // State Variables
  uint8_t activeLoops = 0;

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

  void exciteNeighbors(uvLoop activeLoop)
  {
    for (int i = 0; i < activeLoop.neighborCount; i++)
    {
      activeLoop.neighbors[i]->cascadedExcitation = cascadingRate * activeLoop.excitation;
    }
  }

  // Check all loops excitation
  void handleLoopActivation()
  {
    Serial.println("Active loops");
    int countActiveLoops = 0;
    for (int i = 0; i < NUM_MODULES; i++)
    {
      if (allLoops[i].excitation > activationThreshold)
      {
        allLoops[i].isActive = true;
        countActiveLoops++;
        Serial.println(i);
        // exciteNeighbors(allLoops[i]);
      }
      else
      {
        allLoops[i].isActive = false;
      }
    }
    activeLoops = countActiveLoops;
  }

  // Assign random excitation level to randomly chosen loop
  void exciteRandomLoop()
  {
    uint8_t excitation = random8();
    if (excitation > activationThreshold && activeLoops >= maxActiveLoops)
      return;
    uint8_t thisLoop = random8(NUM_MODULES - 1);
    allLoops[thisLoop].excitation = excitation;
  }

  void exciteLoopGridWithNoise()
  {
    uint16_t t = millis();
    uint8_t yScale = 50;
    uint8_t xScale = 20;

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
    buildSetup();
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
    EVERY_N_SECONDS(3)
    {
      // exciteLoopGridWithNoise();
      exciteRandomLoop();
      handleLoopActivation();
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
        if (allLoops[i].isActive)
        {
          ebbAndFlowLoops(allLoops[i]);
          exciteNeighbors(allLoops[i]);
        }
        if (!allLoops[i].isActive && allLoops[i].cascadedExcitation > 0)
        {
          ebbAndFlowLoops(allLoops[i]);
          allLoops[i].cascadedExcitation = 0;
        }
        // lavaLoops(allLoops[i]);
      }

      FastLED.show();
    }
  }
};
