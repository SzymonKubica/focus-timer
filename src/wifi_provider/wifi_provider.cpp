#include "wifi_provider.hpp"
#include <Arduino.h>
#include <WiFiNINA.h>

char DEFAULT_SSID[] = "RK";
char DEFAULT_PASSWORD[] = "rjsb62930";

void WifiProvider::connect() { connect(DEFAULT_SSID, DEFAULT_PASSWORD); }

void WifiProvider::connect(const char *ssid, const char *password) {
  Serial.print("Connecting to WiFi...");
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
}
