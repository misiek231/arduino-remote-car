#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <math.h>
#include "wifi_credentials.h"

constexpr uint8_t PIN_DIN = 11, PIN_CLK = 12, PIN_CS = 10;
constexpr uint8_t PIN_JOY_X = 4, PIN_JOY_Y = 5, PIN_RGB = 48;
constexpr bool INVERT_X = false, INVERT_Y = false;
constexpr int DEAD_ZONE = 250;
constexpr unsigned long FRAME_MS = 40;
int centerX = 0, centerY = 0;
bool otaReady = false;
wl_status_t lastWifiStatus = WL_IDLE_STATUS;

void writeRegister(uint8_t address, uint8_t value) {
  digitalWrite(PIN_CS, LOW);
  shiftOut(PIN_DIN, PIN_CLK, MSBFIRST, address);
  shiftOut(PIN_DIN, PIN_CLK, MSBFIRST, value);
  digitalWrite(PIN_CS, HIGH);
}

void updateNetwork() {
  const wl_status_t status = WiFi.status();
  if (status != lastWifiStatus) {
    lastWifiStatus = status;
    if (status == WL_CONNECTED) {
      Serial.print("Wi-Fi polaczone. IP: "); Serial.println(WiFi.localIP());
      Serial.print("MAC: "); Serial.println(WiFi.macAddress());
      if (!otaReady) {
        ArduinoOTA.begin();
        otaReady = true;
        Serial.println("OTA gotowe na porcie 3232.");
      }
    } else Serial.printf("Brak polaczenia Wi-Fi (status %d).\n", int(status));
  }
  static int lastLed = -1;
  const int led = status == WL_CONNECTED ? 2 : (millis() / 500) % 2;
  if (led != lastLed) {
    lastLed = led;
    if (led == 2) rgbLedWrite(PIN_RGB, 0, 20, 0);
    else if (led == 1) rgbLedWrite(PIN_RGB, 20, 0, 0);
    else rgbLedWrite(PIN_RGB, 0, 0, 0);
  }
  if (otaReady && status == WL_CONNECTED) ArduinoOTA.handle();
}

float axisValue(int reading, int center, bool invert) {
  int delta = reading - center;
  if (abs(delta) <= DEAD_ZONE) return 0.0f;
  const int range = delta > 0 ? 4095 - center : center;
  if (range <= DEAD_ZONE) return 0.0f;
  float magnitude = float(abs(delta) - DEAD_ZONE) / (range - DEAD_ZONE);
  magnitude = constrain(magnitude, 0.0f, 1.0f);
  return magnitude * (delta > 0 ? 1.0f : -1.0f) * (invert ? -1.0f : 1.0f);
}

void render(float x, float y) {
  // Kontur zaokraglonego ksztaltu: okrag wydluzony w kierunku joysticka.
  const float vx = x * 2.0f, vy = y * 2.0f;
  const float lengthSquared = vx * vx + vy * vy;
  uint8_t rows[8] = {};
  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      const float px = col - 3.5f, py = row - 3.5f;
      float t = lengthSquared > 0.0001f ? (px * vx + py * vy) / lengthSquared : 0.0f;
      t = constrain(t, 0.0f, 1.0f);
      const float ex = px - t * vx, ey = py - t * vy;
      const float distance = sqrtf(ex * ex + ey * ey);
      if (distance >= 0.95f && distance <= 1.9f) rows[row] |= 1U << col;
    }
  }
  for (int row = 0; row < 8; ++row) writeRegister(row + 1, rows[row]);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  ArduinoOTA.setHostname("snake-esp32");
  ArduinoOTA.onStart([]() { Serial.println("Rozpoczeto aktualizacje OTA."); });
  ArduinoOTA.onEnd([]() { Serial.println("Aktualizacja OTA zakonczona."); });
  ArduinoOTA.onError([](ota_error_t error) { Serial.printf("Blad OTA: %u\n", unsigned(error)); });
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  pinMode(PIN_DIN, OUTPUT); pinMode(PIN_CLK, OUTPUT); pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH); digitalWrite(PIN_CLK, LOW);
  writeRegister(0x0f, 0); writeRegister(0x09, 0);
  writeRegister(0x0b, 7); writeRegister(0x0a, 1); writeRegister(0x0c, 1);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_JOY_X, ADC_11db);
  analogSetPinAttenuation(PIN_JOY_Y, ADC_11db);
  Serial.println("Kalibracja: zostaw joystick w spoczynku.");
  delay(1000);
  long sumX = 0, sumY = 0;
  for (int i = 0; i < 32; ++i) {
    sumX += analogRead(PIN_JOY_X); sumY += analogRead(PIN_JOY_Y); delay(5);
  }
  centerX = sumX / 32; centerY = sumY / 32;
  render(0, 0);
}

void loop() {
  updateNetwork();
  static unsigned long lastFrame = 0;
  const unsigned long now = millis();
  if (now - lastFrame >= FRAME_MS) {
    lastFrame = now;
    render(axisValue(analogRead(PIN_JOY_X), centerX, INVERT_X),
           axisValue(analogRead(PIN_JOY_Y), centerY, INVERT_Y));
  }
  delay(5);
}
