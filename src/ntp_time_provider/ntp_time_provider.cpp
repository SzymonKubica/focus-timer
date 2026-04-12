#include "ntp_time_provider.hpp"
#include <Arduino.h>
#include <WiFiNINA.h>

const char *ntpServer = "pool.ntp.org";
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];

void TimeProvider::send_ntp_packet() {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);

  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  udp.beginPacket(ntpServer, 123);
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

unsigned long TimeProvider::get_current_time() {
  send_ntp_packet();
  delay(1000);

  if (udp.parsePacket()) {
    udp.read(packetBuffer, NTP_PACKET_SIZE);

    // TODO: understand how this works.
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = (highWord << 16) | lowWord;

    const unsigned long seventyYears = 2208988800UL;
    return secsSince1900 - seventyYears;
  }

  return 0;
}

void TimeProvider::setup() { udp.begin(2390); }
