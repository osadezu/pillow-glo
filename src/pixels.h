#pragma once

#include <FastLED.h>

namespace Pixels
{
  void setup();
  void loop();

  void setChannels();
  void setGlobalBrightness(uint8_t);
  void fade(CRGB, uint8_t, uint8_t);
  void clear(CRGB, uint8_t, uint8_t);
  void ebbAndFlow(CRGB[], uint8_t[], uint8_t);
  void ebbAndFlowAll();
  void lava(CRGB[], uint8_t[], uint8_t);
};
