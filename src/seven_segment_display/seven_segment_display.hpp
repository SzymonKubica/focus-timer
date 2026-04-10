#include "SevSeg.h"

class SevenSegmentDisplay {
private:
  SevSeg sevseg;

public:
  void setup();
  void set_number(int number);
  void refresh();
};
