#include "seven_segment_display.hpp"

void SevenSegmentDisplay::setup() {
  byte numDigits = 4;
  byte digitPins[] = {14, 16, 13, 12};
  byte segmentPins[] = {17, 21, 6, 4, 3, 20, 7, 5};
  bool resistorsOnSegments = true;
  byte hardwareConfig = COMMON_CATHODE;
  driver.begin(hardwareConfig, numDigits, digitPins, segmentPins,
               resistorsOnSegments);
  driver.setBrightness(100);
}

void SevenSegmentDisplay::set_number(int number) {
  driver.setNumber(number, 2);
}
void SevenSegmentDisplay::refresh() { driver.refreshDisplay(); }
