#include <Arduino.h>
#include <FastLED.h>

// WS2811 output
#define DATA_PIN 18 // GPIO18 @ DevKit right-9

/* Comment this line for final pixel setup */
// #define TESTGRID

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

// Behavior Parameters
#define FRAME_PERIOD_MS 20 // 20 gives 50 frames per second
#ifndef TESTGRID
#define EXCITE_PERIOD_S 5
#else
#define EXCITE_PERIOD_S 3
#endif
#define DARKNESS_WATCHDOG 5                             // Seconds before forcing an activation
#define DARKNESS_WATCHDOG_MS (DARKNESS_WATCHDOG * 1000) // Milliseconds before forcing an activation

#define DECAY 254                // proportion of 255 to fade inactive loops each frame
#define ACTIVATION_THRESHOLD 190 // excitation level at which loops can turn on
#define MAX_ACTIVE_LOOPS 5       // max number of randomly excited loops
#define CASCADING_RATE 20        // proportion of 255 to fade inactive loops each frame

// Observed values from noise8
#define NOISE_LOWER_BOUND 30  // 34 much sooner
#define NOISE_UPPER_BOUND 228 // 218 much sooner

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
    uint8_t deferringExcitation = 0;
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
  // const int loopGridHeight = 2;
  // const int loopGridWidth = 8;
  // uvLoop *loopGrid[2][8] = {
  //     {&allLoops[0], &allLoops[3], &allLoops[4], &allLoops[7], &allLoops[8], &allLoops[11], &allLoops[12], &allLoops[15]},
  //     {&allLoops[1], &allLoops[2], &allLoops[5], &allLoops[6], &allLoops[9], &allLoops[10], &allLoops[13], &allLoops[14]}};

  void buildLedSetup()
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

    // Back Corners 5 & 6
    // Artificial neighbors
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

    // Solo unit 11 (Ruth)
    // Artificial neighbors
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
#else
  // Test grid setup
  // const int loopGridHeight = 2;
  // const int loopGridWidth = 8;
  // uvLoop *loopGrid[2][8] = {
  //     {&allLoops[0], &allLoops[3], &allLoops[4], &allLoops[7], &allLoops[8], &allLoops[11], &allLoops[12], &allLoops[15]},
  //     {&allLoops[1], &allLoops[2], &allLoops[5], &allLoops[6], &allLoops[9], &allLoops[10], &allLoops[13], &allLoops[14]}};

  void buildLedSetup()
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

  // State Variables
  uint32_t darkSince = millis();
  uint16_t globalBrightness = 2048;
  uint8_t activeLoops = 0;

  // void checkMinMaxNoise(uint8_t noiseSample)
  // {
  //   static uint8_t minNoise = 255;
  //   static uint8_t maxNoise = 0;

  //   if (noiseSample < minNoise)
  //     minNoise = noiseSample;
  //   if (noiseSample > maxNoise)
  //     maxNoise = noiseSample;

  //   Serial.println("MinMaxNoise");
  //   Serial.println(minNoise);
  //   Serial.println(maxNoise);
  // }

  void exciteNeighbors(uvLoop activeLoop)
  {
    for (int i = 0; i < activeLoop.neighborCount; i++)
    {
      uint8_t currExcitation = sub8(activeLoop.excitation, activeLoop.deferringExcitation);
      activeLoop.neighbors[i]->cascadedExcitation = scale8(currExcitation, CASCADING_RATE);
    }
  }

  // Check all loops excitation and count active
  void handleLoopActivation()
  {
    // Serial.println("Active loops");
    int countActiveLoops = 0;
    bool deactivatedLoop = false;

    for (int i = 0; i < NUM_MODULES; i++)
    {
      if (allLoops[i].excitation >= ACTIVATION_THRESHOLD)
      {
        if (!allLoops[i].isActive)
        {
          allLoops[i].isActive = true;
          allLoops[i].deferringExcitation = allLoops[i].excitation;
        }
        countActiveLoops++;
        // Serial.println(i);
        // exciteNeighbors(allLoops[i]);
      }
      else if (allLoops[i].isActive)
      {
        allLoops[i].isActive = false;
        allLoops[i].deferringExcitation = 0;
        deactivatedLoop = true;
      }
    }

    activeLoops = countActiveLoops;

    if (!activeLoops && deactivatedLoop)
    {
      darkSince = millis();
    }
  }

  void deactivateAllLoops()
  {
    for (int i = 0; i < NUM_MODULES; i++)
    {
      allLoops[i].excitation = 0;
    }
    handleLoopActivation();
  }

  // Assign random excitation level to randomly chosen loop
  void exciteRandomLoop(bool force = false)
  {
    uint8_t excitation;

    if (force)
      excitation = random8(ACTIVATION_THRESHOLD, 255);
    else
      excitation = random8();

    if (excitation > ACTIVATION_THRESHOLD && activeLoops >= MAX_ACTIVE_LOOPS)
      return;

    uint8_t thisLoop = random8(NUM_MODULES);
    allLoops[thisLoop].excitation = excitation;
  }

  // void exciteLoopGridWithNoise()
  // {
  //   uint16_t t = millis();
  //   uint8_t yScale = 50;
  //   uint8_t xScale = 20;

  //   for (int y = 0; y < loopGridHeight; y++)
  //   {
  //     for (int x = 0; x < loopGridWidth; x++)
  //     {
  //       uint8_t noise = inoise8(y * yScale, x * xScale, t);
  //       checkMinMaxNoise(noise);
  //       loopGrid[y][x]->excitation = noise;
  //     }
  //   }
  // }

  void setLoop(uvLoop loop, CRGB value)
  {
    for (int i = 0; i < loop.ledCount; i++)
    {
      loop.leds[i] = value;
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

  void setGlobalBrightness(uint16_t analogValue)
  {
    globalBrightness = analogValue;
    uint8_t brightness = map(analogValue, 0, 4095, 0, 255);
    // uint8_t brightness = dim8_raw(analogValue);
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
      leds[i].nscale8(DECAY);
    }
  }

  void clear(CRGB leds[], uint8_t offsets[], uint8_t numLeds)
  {
    for (int i = 0; i < numLeds; i++)
    {
      leds[offsets[i]] = CRGB(0, 0, 0);
    }
  }

  void ebbAndFlowLoops(uvLoop &loop)
  {
    uint16_t t = millis() / 2;
    uint8_t scale = 50;
    uint8_t currExcitation = 255;

    if (loop.isActive)
    {
      // Ease-in newly active loop
      if (loop.deferringExcitation > 0)
      {
        currExcitation = sub8(loop.excitation, loop.deferringExcitation);
        loop.deferringExcitation = scale8(loop.deferringExcitation, 254);
      }
      else
      {
        currExcitation = loop.excitation;
      }
      exciteNeighbors(loop);
    }

    // Trigger cascaded loop and clear cascaded
    if (!loop.isActive && loop.cascadedExcitation > 0)
    {
      currExcitation = loop.cascadedExcitation;
      loop.cascadedExcitation = 0;
    }

    for (int i = 0; i < loop.ledCount; i++)
    {
      uint8_t noise = inoise8(i * scale, t);
      noise = scale8(noise, currExcitation);
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

  void toggleDithering()
  {
    static bool dither = true;
    dither = !dither;
    FastLED.setDither(dither);
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
    buildLedSetup();
  }

  void loop()
  {
    EVERY_N_SECONDS(EXCITE_PERIOD_S)
    {
      // exciteLoopGridWithNoise();
      uint32_t now = millis();
      bool force = false;
      if (!activeLoops && now - darkSince >= DARKNESS_WATCHDOG_MS)
      {
        force = true;
      }
      exciteRandomLoop(force);
      handleLoopActivation();
    }

    EVERY_N_MILLIS(FRAME_PERIOD_MS * 3)
    {
      fadeAll();
    }

    EVERY_N_MILLIS(FRAME_PERIOD_MS)
    {
      // fadeAll();

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
        }
        if (!allLoops[i].isActive && allLoops[i].cascadedExcitation > 0)
        {
          ebbAndFlowLoops(allLoops[i]);
        }
        // lavaLoops(allLoops[i]);
      }
    }

    FastLED.show();
  }
};
