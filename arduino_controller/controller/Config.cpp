#include "Config.h"
#include <stdint.h>

// === Defaults ===
const int steeringDefaultPos = 90;
const int engineDefaultSpeed = 100;
const int gearDefaultPos = 90;

// === Steering Constraints ===
const int STEERING_MIN = 40;
const int STEERING_MAX = 140;

// === WiFi Configuration ===
const char* WIFI_SSID = "UPC2558138";
const char* WIFI_PASSWORD = "Mpuff7bpjeku";

// === Engine Offset Tables ===

int8_t engineOffsetTable[128] = { 0 };             // Push offset
int8_t engineEaseTargetOffsetTable[128] = { 0 };   // Retract offset
int8_t gearOffsetTable[128] = { 0 };

inline void setupEngineOffsets() {
  engineOffsetTable['w'] = 30;    // Push forward
  engineEaseTargetOffsetTable['w'] = 10; // Retract forward

  engineOffsetTable['s'] = -20;   // Push reverse
  engineEaseTargetOffsetTable['s'] = -5;  // Retract reverse

  engineOffsetTable['c'] = 0;
  engineEaseTargetOffsetTable['c'] = 0;
}

// === Gear Mapping Table ===
inline void setupGearOffsets() {
  gearOffsetTable['v'] = 50;    // Gear 1
  gearOffsetTable['b'] = -50;   // Gear 2
  gearOffsetTable['c'] = 0;     // Neutral
}

void setupConfig() {
  setupEngineOffsets();
  setupGearOffsets();
}
