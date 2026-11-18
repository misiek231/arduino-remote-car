#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <math.h>
constexpr uint8_t FOUT_PIN = 10;
constexpr uint8_t MOTOR_PIN = 11;
constexpr uint8_t DIRECTION_PIN = 12;
constexpr uint8_t STEERING_SERVO_PIN = 8;
constexpr uint8_t GEAR_SERVO_PIN = 17;
constexpr uint8_t RGB_PIN = 48;
constexpr uint8_t OLED_SDA = 4, OLED_SCL = 5;
constexpr unsigned int MOTOR_FREQ_HZ = 7900;
constexpr uint8_t PWM_BITS = 8;
constexpr uint8_t SERVO_PWM_BITS = 14;
constexpr uint32_t SERVO_FREQ_HZ = 50;
constexpr int STEERING_MIN_DEG = 0, STEERING_MAX_DEG = 180;
constexpr int GEAR_MIN_DEG = 0, GEAR_MAX_DEG = 180;
constexpr unsigned long MOTOR_TIMEOUT_MS = 10000;
constexpr unsigned long WEB_TIMEOUT_MS = 2000;
constexpr unsigned long MOTOR_RAMP_STEP_MS = 50; // 1% na krok: 0–100% w ok. 5 s.
constexpr unsigned long REPORT_INTERVAL_MS = 1000;
constexpr uint32_t FOUT_GAP_THRESHOLD_US = 1000; // Prog eksperymentalny, nie jest pomiarem RPM.
constexpr unsigned long CAMERA_TIMEOUT_MS = 5000;

extern U8G2_SH1106_128X64_NONAME_F_HW_I2C oled;
extern portMUX_TYPE foutMux;
extern volatile uint32_t foutEdges, foutLongGaps, foutMinUs, foutMaxUs, previousEdgeUs;
extern volatile bool havePreviousEdge;
extern bool otaReady, motorReady, updating, changedDuringWindow, webActive, webServerStarted;
extern wl_status_t lastWifiStatus;
extern unsigned long lastReport, lastCommand, lastMotorRampStep;
extern int motorPercent, motorTargetPercent;
extern bool reverseDirection;
extern WebServer webServer;
extern WiFiUDP cameraDiscovery;
extern bool cameraDiscoveryReady, cameraSeen;
extern IPAddress cameraIp;
extern unsigned long cameraLastSeen;
extern bool steeringServoReady, gearServoReady;
extern float steeringCurrent, steeringTarget;
extern int steeringRateDegPerSec;
extern unsigned long steeringLastUpdate;
extern bool oledReady, foutMeasured;
extern float lastGapRate;
extern unsigned long lastOledUpdate;
void onFoutFalling();
void initOled(); void drawOled();
void setMotor(int percent); void updateMotorRamp(); void stopMotor();
bool setServoPosition(uint8_t pin, uint8_t channel, bool &ready, float angle);
bool setSteeringTarget(int angle, int rate); void updateSteering();
void initWebServer(); void handleCommands(); void updateNetwork();
void updateCameraDiscovery(); void reportFout(unsigned long elapsedMs);
