#pragma once

#include <FastLED.h>

namespace Pixels
{
  struct uvLoop;

  void setup();
  void loop();

  void buildLedSetup();

  void checkMinMaxNoise(uint8_t);
  void exciteNeighbors(uvLoop);
  void handleLoopActivation();
  void deactivateAllLoops();
  void exciteRandomLoop(bool);

  void setLoop(uvLoop, CRGB);
  void setChannels();
  uint32_t allChannels(uint8_t);
  void setGlobalBrightness(uint16_t);
  void fade(CRGB, uint8_t, uint8_t);
  void fadeAll();
  void clear(CRGB, uint8_t, uint8_t);

  void ebbAndFlowLoops(uvLoop);
  void ebbAndFlowLeds(CRGB[], uint8_t[], uint8_t);
  void ebbAndFlowAll();
  void lavaLoops(uvLoop);
  void lava(CRGB[], uint8_t[], uint8_t);
};
