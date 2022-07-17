#include <Arduino.h>
#include <ezButton.h>

#include "Pixels.h"

#define POT_PIN 36      // ADC1_0 - GPIO36 @ DevKit left-3
#define LED_PIN 25      // GPIO25 @ DevKit left-9
#define BTN_MODE_PIN 32 // GPIO32 @ DevKit left-7
#define BTN_ADJ_PIN 33  // GPIO33 @ DevKit left-8
#define SETTINGS_MODES 1
// Mode 1: pixel channels (R-G-B)

namespace Settings
{

  uint8_t settingsMode = 0;

  ezButton modeButton(BTN_MODE_PIN);
  ezButton adjButton(BTN_ADJ_PIN);

  void readBrightness()
  {
    int analogValue = analogRead(POT_PIN);
    int brightness = map(analogValue, 0, 4095, 0, 255);
    Pixels::setGlobalBrightness(brightness);
  }

  void setup()
  {
    modeButton.setDebounceTime(100);
    modeButton.setCountMode(COUNT_FALLING);
    adjButton.setDebounceTime(100);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    readBrightness();
  }

  void loop()
  {
    modeButton.loop(); // Update button state
    settingsMode = modeButton.getCount() % (SETTINGS_MODES + 1);

    if (settingsMode)
    {
      digitalWrite(LED_PIN, HIGH);

      adjButton.loop(); // Update button state
      if (adjButton.isPressed())
      {
        switch (settingsMode)
        {
        case 1:
          Pixels::setChannels();
          break;

        default:
          break;
        }
      }

      readBrightness();
    }
    else
    {
      digitalWrite(LED_PIN, LOW);
    }
  }

};
