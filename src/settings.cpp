#include <Arduino.h>
#include <ezButton.h>

#include "Pixels.h"

// Settings inputs
#define POT_PIN 36 // ADC1_0 - GPIO36 @ DevKit left-3

// Settings outputs
#define LED_PIN 25 // GPIO25 @ DevKit left-9

#define SETTINGS_MODES 1
// Mode 1: pixel selection (R-G-B)

namespace Settings
{

  static uint8_t settingsMode = 0;

  ezButton modeButton(32); // GPIO32 @ DevKit left-7
  ezButton adjButton(33);  // GPIO33 @ DevKit left-8

  void readBrightness()
  {
    int analogValue = analogRead(POT_PIN);
    int brightness = map(analogValue, 0, 4095, 0, 255);

    // Serial.print("Analog = ");
    // Serial.print(analogValue);
    // Serial.print(" => brightness = ");
    // Serial.println(brightness);

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
      readBrightness();
    }
    else
    {
      digitalWrite(LED_PIN, LOW);
    }
  }

};
