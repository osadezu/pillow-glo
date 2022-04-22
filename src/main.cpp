#include <Arduino.h>

#include "settings.h"
#include "pixels.h"

void setup()
{
  Pixels::setup();
  Settings::setup();

  // Debugging output
  Serial.begin(115200);
}

void loop()
{
  Settings::loop();
  Pixels::loop();
}
