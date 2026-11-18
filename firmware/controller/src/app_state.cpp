#include "app.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);
portMUX_TYPE foutMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t foutEdges = 0;
volatile uint32_t foutLongGaps = 0;
volatile uint32_t foutMinUs = UINT32_MAX;
volatile uint32_t foutMaxUs = 0;
volatile uint32_t previousEdgeUs = 0;
volatile bool havePreviousEdge = false;

bool otaReady = false;
wl_status_t lastWifiStatus = WL_IDLE_STATUS;
unsigned long lastReport = 0;
unsigned long lastCommand = 0;
int motorPercent = 0;
int motorTargetPercent = 0;
bool reverseDirection = false;
unsigned long lastMotorRampStep = 0;
bool motorReady = false;
bool updating = false;
bool changedDuringWindow = false;
bool webActive = false;
bool webServerStarted = false;
WebServer webServer(80);
WiFiUDP cameraDiscovery;
bool cameraDiscoveryReady = false, cameraSeen = false;
IPAddress cameraIp;
unsigned long cameraLastSeen = 0;
bool steeringServoReady = false, gearServoReady = false;
float steeringCurrent = 80.0f, steeringTarget = 80.0f;
int steeringRateDegPerSec = 180;
unsigned long steeringLastUpdate = 0;
bool oledReady = false;
bool foutMeasured = false;
float lastGapRate = 0;
unsigned long lastOledUpdate = 0;

