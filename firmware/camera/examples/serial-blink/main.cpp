#include <Arduino.h>

// Typowa ESP32-CAM AI Thinker: mala dioda sygnalizacyjna, aktywna stanem LOW.
constexpr uint8_t STATUS_LED = 33;

void setup() {
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, HIGH);
  Serial.begin(115200);
}

void loop() {
  static unsigned long count = 0;
  digitalWrite(STATUS_LED, LOW);
  Serial.printf("ESP32-CAM TEST nr=%lu millis=%lu\n", ++count, millis());
  delay(500);
  digitalWrite(STATUS_LED, HIGH);
  delay(500);
}
