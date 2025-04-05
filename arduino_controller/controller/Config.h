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

// === Engine Offset Table ===
#include <stdint.h>

int8_t engineOffsetTable[128] = { 0 };  // Initialize all to 0

inline void setupEngineOffsets() {
  engineOffsetTable['w'] = 50;   // Forward
  engineOffsetTable['s'] = -20;  // Reverse
  engineOffsetTable['c'] = 0;    // Coast
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
