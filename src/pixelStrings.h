#pragma once

#include <FastLED.h>

namespace PixelStrings
{
  struct uvLoop;

  void setup();
  void loop();

  void checkMinMaxNoise(uint8_t);

  void setChannels();
  uint32_t allChannels(uint8_t);
  void setGlobalBrightness(uint16_t);
  void fade(CRGB, uint8_t, uint8_t);
  void fadeAll();
  void clear(CRGB, uint8_t, uint8_t);

  void ebbAndFlowAll();
  void lava(CRGB[], uint8_t[], uint8_t);

  void toggleDithering();
};
