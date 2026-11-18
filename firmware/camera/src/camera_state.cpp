#include "camera_app.h"

// Typowe wyprowadzenia ESP32-CAM AI Thinker z kamera OV2640.
WebServer server(80);
bool cameraReady = false;
WiFiUDP discovery;
unsigned long lastAnnouncement = 0;
bool streamReady = false;
std::atomic<bool> otaUpdating{false};
bool flashReady = false;
int flashPercent = 0;
StreamStats streamStats;
portMUX_TYPE statsMux = portMUX_INITIALIZER_UNLOCKED;

