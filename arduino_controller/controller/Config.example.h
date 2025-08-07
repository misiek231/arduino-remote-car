#ifndef CONFIG_H
#define CONFIG_H

// === Configuration ===
#define ESP_COMMAND_TIMEOUT 20000 // ms
#define USE_SOFT_SERIAL 0

// === Pin Configuration ===
#define STEERING_SERVO_PIN 9
#define ENGINE_SERVO_PIN 10
#define GEAR_SERVO_PIN 11

// === Defaults ===
const int steeringDefaultPos = 90;
const int engineDefaultSpeed = 100;
const int gearDefaultPos = 90;

// === Steering Constraints ===
const int STEERING_MIN = 40;
const int STEERING_MAX = 140;

// === WiFi Configuration ===
const char* WIFI_SSID = "---";
const char* WIFI_PASSWORD = "---";

// === Engine Offset Tables ===
#include <stdint.h>

int8_t engineOffsetTable[128] = { 0 };             // Push offset
int8_t engineEaseTargetOffsetTable[128] = { 0 };   // Retract offset

inline void setupEngineOffsets() {
  engineOffsetTable['w'] = 30;    // Push forward
  engineEaseTargetOffsetTable['w'] = 10; // Retract forward

  engineOffsetTable['s'] = -20;   // Push reverse
  engineEaseTargetOffsetTable['s'] = -5;  // Retract reverse

  engineOffsetTable['c'] = 0;
  engineEaseTargetOffsetTable['c'] = 0;
}

// === Gear Mapping Table ===
int8_t gearOffsetTable[128] = { 0 };

inline void setupGearOffsets() {
  gearOffsetTable['v'] = 50;    // Gear 1
  gearOffsetTable['b'] = -50;   // Gear 2
  gearOffsetTable['c'] = 0;     // Neutral
}

inline void setupConfig() {
  setupEngineOffsets();
  setupGearOffsets();
}

#endif // CONFIG_H
