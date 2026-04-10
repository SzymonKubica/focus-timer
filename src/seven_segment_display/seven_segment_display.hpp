#include "SevSeg.h"

class SevenSegmentDisplay {
private:
  SevSeg driver;

public:
  void setup();
  void set_number(int number);
  void refresh();
};
