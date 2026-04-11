#include <Arduino.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>

char ssid[] = "RK";
char pass[] = "rjsb62930";

WiFiUDP udp;

const char *ntpServer = "pool.ntp.org";
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];

unsigned long sendNTPpacket() {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);

  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  udp.beginPacket(ntpServer, 123);
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();

  return millis();
}

unsigned long getNtpTime() {
  sendNTPpacket();
  delay(1000);

  if (udp.parsePacket()) {
    udp.read(packetBuffer, NTP_PACKET_SIZE);

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = (highWord << 16) | lowWord;

    const unsigned long seventyYears = 2208988800UL;
    return secsSince1900 - seventyYears;
  }

  return 0;
}

void setup() {
  Serial.begin(57600);

  Serial.print("Connecting to WiFi...");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  udp.begin(2390);

  unsigned long epoch = getNtpTime();
  Serial.print("Unix time: ");
  Serial.println(epoch);
}

void loop() {}
