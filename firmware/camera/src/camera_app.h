#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <atomic>
#include "esp_camera.h"
#include "esp_system.h"
#include "driver/ledc.h"

constexpr int FLASH_PIN = 4;
struct StreamStats {
  bool active = false, valid = false;
  unsigned long updated = 0;
  float fps = 0, kb = 0, mbps = 0, capture = 0, send = 0;
  uint32_t captureMax = 0, sendMax = 0;
};
extern WebServer server;
extern bool cameraReady, streamReady, flashReady;
extern int flashPercent;
extern std::atomic<bool> otaUpdating;
extern StreamStats streamStats;
extern portMUX_TYPE statsMux;
bool initFlash(); bool setFlash(int percent);
bool startStreamServer();
void initCameraRoutes();
