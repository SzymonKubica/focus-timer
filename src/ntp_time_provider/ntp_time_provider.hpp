#include <WiFiUdp.h>

class TimeProvider {
  WiFiUDP udp;

  void send_ntp_packet();

public:
  unsigned long get_current_time();
  void setup();
};
