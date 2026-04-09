#include "SevSeg.h"
SevSeg sevseg;
#include <DS3231.h>
#include <Wire.h>

RTClib myRTC;

#define ACTION_BUTTON 15
#define PRESSED 1
#define NOT_PRESSED 0

int buttonState = 0;
typedef enum Mode { COUNTING, STOPPED, CLOCK } Mode;
#define DISPLAY_UPDATE_INTERVAL 1000 // ms

void log_time(DateTime now) {
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  Serial.print(" since midnight 1/1/1970 = ");
  Serial.print(now.unixtime());
  Serial.print("s = ");
  Serial.print(now.unixtime() / 86400L);
  Serial.println("d");
}

int get_elapsed_seconds_display(int elapsed) {
  return (elapsed / 60) * 100 + elapsed % 60;
}

int get_hours_minutes_display(DateTime time) {
  return time.hour() * 100 + time.minute();
}

inline bool button_press_registered() {
  int pinValue = digitalRead(ACTION_BUTTON);
  delay(1); // debounce filter
  if (buttonState != pinValue) {
    bool test =  pinValue == PRESSED && buttonState == NOT_PRESSED;
    buttonState = pinValue;
    return test;
  }
  return false;
}

enum Mode get_next_mode(enum Mode mode) {
  switch (mode) {
  case CLOCK:
    return COUNTING;
  case COUNTING:
    return STOPPED;
  case STOPPED:
    return CLOCK;
  }
}


void setup() {
  // Configure the 4-digit segment display.
  byte numDigits = 4;
  byte digitPins[] = {14, 16, 13, 12};
  byte segmentPins[] = {17, 21, 6, 4, 3, 20, 7, 5};
  bool resistorsOnSegments = true;
  bool updateWithDelaysIn = true;
  byte hardwareConfig = COMMON_CATHODE;
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins,
               resistorsOnSegments);
  sevseg.setBrightness(100);

  pinMode(ACTION_BUTTON, INPUT_PULLUP);
  // Serial port for logging
  Serial.begin(57600);
  // Communication with the DS3231 via SCL and SDA
  Wire.begin();
  delay(500);
  Serial.println("Nano Ready!");
}



void loop() {
  int counter = 0;
  Mode mode = CLOCK;
  DateTime now = myRTC.now();
  int start_seconds = now.unixtime();

  while (true) {
    if (button_press_registered()) {
      Serial.println("Button press registered");
      mode = get_next_mode(mode);
      if (mode == COUNTING) {
          now = myRTC.now();
          start_seconds = now.unixtime();
        }
    }

    if (counter % DISPLAY_UPDATE_INTERVAL == 0) {
      DateTime now = myRTC.now();
      log_time(now);
      int now_seconds = now.unixtime();
      int elapsed = now_seconds - start_seconds;

     switch (mode) {
      case CLOCK:
        sevseg.setNumber(get_hours_minutes_display(now), 2);
        break;
      case COUNTING:
        sevseg.setNumber(get_elapsed_seconds_display(elapsed), 2);
        break;
      case STOPPED:
        break;
      }
    }
    counter = (counter + 1) % 1000;

    sevseg.refreshDisplay();
  }
}

