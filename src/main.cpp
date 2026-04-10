#include "seven_segment_display/seven_segment_display.hpp"
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#define ACTION_BUTTON 15
#define PRESSED 1
#define NOT_PRESSED 0

int buttonState = 0;
#define DISPLAY_UPDATE_INTERVAL 1000 // ms

inline bool button_press_registered() {
  int pinValue = digitalRead(ACTION_BUTTON);
  delay(1); // debounce filter
  if (buttonState != pinValue) {
    bool test = pinValue == PRESSED && buttonState == NOT_PRESSED;
    buttonState = pinValue;
    return test;
  }
  return false;
}

void setup() {
  pinMode(ACTION_BUTTON, INPUT_PULLUP);
  Serial.begin(115200);
  Wire.begin();
  delay(500);
  Serial.println("Nano Ready!");
}

void loop() {
  int counter = 0;

  SevenSegmentDisplay display{};
  display.setup();

  bool counting = false;
  int start = millis();
  while (true) {
    if (button_press_registered()) {
      Serial.println("Button press registered");
      if (counting) {
        counting = false;
      } else {
        start = millis();
        counting = true;
      }
    }

    if (counting && counter % DISPLAY_UPDATE_INTERVAL == 0) {
      int elapsed = (millis() - start) / 1000;
      display.set_number(elapsed);
    }
    counter = (counter + 1) % 1000;

    display.refresh();
  }
}
