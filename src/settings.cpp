#include <Arduino.h>
#include <ezButton.h>

#include "Pixels.h"

#define POT_PIN 36      // ADC1_0 - GPIO36 @ DevKit left-3
#define LED_PIN 25      // GPIO25 @ DevKit left-9
#define BTN_MODE_PIN 32 // GPIO32 @ DevKit left-7
#define BTN_ADJ_PIN 33  // GPIO33 @ DevKit left-8

namespace Settings
{

  uint8_t settingsMode = 0;

  ezButton modeButton(BTN_MODE_PIN);
  ezButton adjButton(BTN_ADJ_PIN);

  void readBrightness()
  {
    uint16_t potValue = analogRead(POT_PIN); // 12-bit value 0-4095
    Pixels::setGlobalBrightness(potValue);
  }

  void setup()
  {
    modeButton.setDebounceTime(100);
    adjButton.setDebounceTime(100);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    readBrightness();
  }

  void loop()
  {
    readBrightness();
    modeButton.loop(); // Update button state
    if (modeButton.isPressed())
    {
      settingsMode++;
    }

    if (settingsMode)
    {
      digitalWrite(LED_PIN, HIGH);
      adjButton.loop(); // Update button state
      switch (settingsMode)
      {
      case 1: // Mode 1: pixel channels (R-G-B)
        if (adjButton.isPressed())
        {
          Pixels::setChannels();
        }
        break;
      case 2: // Mode 2: Toggle Dithering
        if (adjButton.isPressed())
        {
          Pixels::toggleDithering();
        }
        break;
      default:
        settingsMode = 0;
      }
    }
    else
    {
      digitalWrite(LED_PIN, LOW);
    }
  }

};
