#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "wifi_credentials.h"

constexpr uint8_t PWM_INPUT_PIN = 6;
constexpr uint8_t RGB_PIN = 48;
constexpr unsigned long REPORT_INTERVAL_MS = 1000;
constexpr unsigned long PULSE_TIMEOUT_US = 20000;
constexpr int SAMPLE_COUNT = 3;

bool otaReady = false;
wl_status_t lastWifiStatus = WL_IDLE_STATUS;
unsigned long lastReport = 0;

void updateNetwork() {
  const wl_status_t status = WiFi.status();
  if (status != lastWifiStatus) {
    lastWifiStatus = status;
    if (status == WL_CONNECTED) {
      Serial.print("Wi-Fi polaczone. IP: ");
      Serial.println(WiFi.localIP());
      if (!otaReady) {
        ArduinoOTA.begin();
        otaReady = true;
        Serial.println("OTA gotowe na porcie 3232.");
      }
    } else {
      Serial.printf("Brak polaczenia Wi-Fi (status %d).\n", int(status));
    }
  }
  static int lastLed = -1;
  const int led = status == WL_CONNECTED ? 2 : (millis() / 500) % 2;
  if (led != lastLed) {
    lastLed = led;
    if (led == 2) rgbLedWrite(RGB_PIN, 0, 20, 0);
    else if (led == 1) rgbLedWrite(RGB_PIN, 20, 0, 0);
    else rgbLedWrite(RGB_PIN, 0, 0, 0);
  }
  if (otaReady && status == WL_CONNECTED) ArduinoOTA.handle();
}

void reportPwm() {
  unsigned long highTotal = 0, lowTotal = 0;
  int valid = 0;
  for (int i = 0; i < SAMPLE_COUNT; ++i) {
    const unsigned long highUs = pulseIn(PWM_INPUT_PIN, HIGH, PULSE_TIMEOUT_US);
    const unsigned long lowUs = pulseIn(PWM_INPUT_PIN, LOW, PULSE_TIMEOUT_US);
    if (highUs != 0 && lowUs != 0) {
      highTotal += highUs;
      lowTotal += lowUs;
      ++valid;
    }
  }
  if (valid == 0) {
    Serial.printf("Brak kompletnych impulsow (GPIO %u: %s w chwili odczytu). Sprawdz sygnal, mase i zakres pomiaru.\n",
                  PWM_INPUT_PIN, digitalRead(PWM_INPUT_PIN) ? "HIGH" : "LOW");
    return;
  }
  const float highUs = float(highTotal) / valid;
  const float lowUs = float(lowTotal) / valid;
  const float periodUs = highUs + lowUs;
  const float dutyHigh = 100.0f * highUs / periodUs;
  Serial.printf("PWM: %.1f Hz | okres %.1f us | HIGH %.1f us (%.1f%%) | LOW %.1f us (%.1f%%) | probki %d/%d\n",
                1000000.0f / periodUs, periodUs, highUs, dutyHigh,
                lowUs, 100.0f - dutyHigh, valid, SAMPLE_COUNT);
}

void setup() {
  Serial.begin(115200);
  pinMode(PWM_INPUT_PIN, INPUT); // Bez wewnetrznego pull-up/pull-down.
  WiFi.mode(WIFI_STA);
  ArduinoOTA.setHostname("snake-esp32");
  ArduinoOTA.onStart([]() { Serial.println("Rozpoczeto aktualizacje OTA."); });
  ArduinoOTA.onEnd([]() { Serial.println("Aktualizacja OTA zakonczona."); });
  ArduinoOTA.onError([](ota_error_t error) { Serial.printf("Blad OTA: %u\n", unsigned(error)); });
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Pomiar PWM: wejscie GPIO 6, odczyt co okolo 1 s.");
}

void loop() {
  updateNetwork();
  const unsigned long now = millis();
  if (now - lastReport >= REPORT_INTERVAL_MS) {
    lastReport = now;
    reportPwm();
  }
  delay(5);
}
