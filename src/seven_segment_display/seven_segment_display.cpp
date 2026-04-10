#include "SevSeg.h"

class SevenSegmentDisplay {
private:
  SevSeg sevseg;

public:
  void setup() {
    byte numDigits = 4;
    byte digitPins[] = {14, 16, 13, 12};
    byte segmentPins[] = {17, 21, 6, 4, 3, 20, 7, 5};
    bool resistorsOnSegments = true;
    byte hardwareConfig = COMMON_CATHODE;
    sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins,
                 resistorsOnSegments);
    sevseg.setBrightness(100);
  }

  void set_number(int number) { sevseg.setNumber(number, 2); }
  void refresh() { sevseg.refreshDisplay(); }
};
