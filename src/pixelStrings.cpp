#include <Arduino.h>
#include <FastLED.h>

// WS2811 output
#define DATA_PIN 18 // GPIO18 @ DevKit right-9

/* Comment this line for final pixel setup */
// #define TESTGRID

#ifndef TESTGRID
// Trailer setup values
#define NUM_STRINGS 2
#define LEDS_STRING 100
#else
// Test grid setup values
#define NUM_STRINGS 2
#define LEDS_STRING 32
#endif

#define TOTAL_LEDS (LEDS_STRING * NUM_STRINGS)

// Behavior Parameters
#define FRAME_PERIOD_MS 20 // 20 gives 50 frames per second
#define EXCITE_PERIOD_S 2
#define DARKNESS_WATCHDOG 5                             // Seconds before forcing an activation
#define DARKNESS_WATCHDOG_MS (DARKNESS_WATCHDOG * 1000) // Milliseconds before forcing an activation

#define DECAY 250                // proportion of 255 to fade inactive loops each frame
#define ACTIVATION_THRESHOLD 190 // excitation level at which loops can turn on
#define MAX_ACTIVE_LOOPS 5       // max number of randomly excited loops
#define CASCADING_RATE 20        // proportion of 255 to fade inactive loops each frame

// Observed values from noise8
#define NOISE_LOWER_BOUND 30  // 34 much sooner
#define NOISE_UPPER_BOUND 228 // 218 much sooner

namespace PixelStrings
{
  CRGB leds[TOTAL_LEDS];

  uint8_t channelMask = 2;

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

  void setChannels()
  {
    Serial.println("Mask was:");
    Serial.println(channelMask);

    channelMask = (channelMask) % 7 + 1; // All channel combinations RGB

    Serial.println("Mask now:");
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

  void unexcite()
  {
    // TODO: make parametric
    uint8_t decay = 200; // out of 255

    for (int i = 0; i < TOTAL_LEDS; i++)
    {
      leds[i].r = scale8(leds[i].r, decay);
      leds[i].b = scale8(leds[i].b, dim8_raw(100));
    }
  }

  void clear(CRGB leds[], uint8_t offsets[], uint8_t numLeds)
  {
    for (int i = 0; i < numLeds; i++)
    {
      leds[offsets[i]] = CRGB(0, 0, 0);
    }
  }

  void ebbAndFlowAll()
  {
    uint16_t t = millis() / 1; // / 7
    uint8_t scale = 15;        // length of wave

    for (int i = 0; i < TOTAL_LEDS; i++)
    {
      uint8_t noise = inoise8(i * scale, t);
      uint32_t noiseRGB = allChannels(noise);
      leds[i] = CRGB(noiseRGB);
    }
  }

  void exciteSection(uint8_t start, uint8_t length)
  {
    uint16_t t = millis() / 1; // / 7
    uint8_t scale = 15;        // length of wave

    for (int i = start; i < length; i++)
    {
      uint8_t noise = inoise8(i * scale, t);
      // TODO: optimize maths
      leds[i].r = scale8(noise, 128);
      leds[i].g = scale8(noise, 128);
      leds[i].b = scale8(noise, 128);
    }
  }

  void ebbAndFlowGreen()
  {
    uint16_t t = millis() / 2; // / 7
    uint8_t scale = 5;         // length of wave

    for (int i = 0; i < TOTAL_LEDS; i++)
    {
      uint8_t noise = inoise8(i * scale, t);
      // TODO: make parametric
      // leds[i].g = noise;
      leds[i].g = scale8(noise, 64);
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
    // FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, TOTAL_LEDS);
    FastLED.setBrightness(128);
    FastLED.clear();
    FastLED.show();
  }

  void loop()
  {
    // EVERY_N_SECONDS(1)
    // {
    //   Serial.println("Pixel 0");
    //   Serial.println(leds[0]);
    // }

    // EVERY_N_SECONDS(EXCITE_PERIOD_S)
    // {
    //   // exciteLoopGridWithNoise();
    //   // uint32_t now = millis();
    //   uint8_t start = random8(TOTAL_LEDS);
    //   uint8_t length = random8(start, TOTAL_LEDS);
    //   exciteSection(start, length);
    // }

    // EVERY_N_MILLIS(1000)
    // {
    //   unexcite();
    // }

    // EVERY_N_MILLIS(FRAME_PERIOD_MS)
    // {
    //   ebbAndFlowGreen();
    // }

    uint8_t val = 128;
    for (int i = 0; i < TOTAL_LEDS; i++)
    {
      // if (i % 3 == 0)
      // {
      //   leds[i].setRGB(0, 0, val);
      // }
      // else if (i % 2 == 0)
      // {
      //   leds[i].setRGB(0, val, 0);
      // }
      // else
      // {
      //   leds[i].setRGB(val, 0, 0);
      // }
      leds[i].setRGB(val, 0, 0);
    }

    // fadeAll();
    FastLED.show();
  }
};
