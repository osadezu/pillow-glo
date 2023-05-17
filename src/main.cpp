#include <Arduino.h>

#include "settings.h"
#include "pixelStrings.h"

void setup()
{
  PixelStrings::setup();
  Settings::setup();

  // Debugging output
  Serial.begin(115200);
}

void loop()
{
  Settings::loop();
  PixelStrings::loop();
}
