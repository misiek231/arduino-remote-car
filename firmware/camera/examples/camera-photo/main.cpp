#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "esp_camera.h"
#include "esp_system.h"
#include "wifi_credentials.h"

// Typowe wyprowadzenia ESP32-CAM AI Thinker z kamera OV2640.
WebServer server(80);
bool cameraReady = false;

void handlePhoto() {
  camera_fb_t *frame = esp_camera_fb_get();
  if (!frame) {
    server.send(500, "text/plain", "Nie udalo sie pobrac zdjecia.");
    return;
  }
  server.sendHeader("Cache-Control", "no-store");
  server.setContentLength(frame->len);
  server.send(200, "image/jpeg", "");
  server.client().write(frame->buf, frame->len);
  esp_camera_fb_return(frame);
}

void setup() {
  Serial.begin(115200);
  delay(2000); // Czas na otwarcie monitora po wgraniu i resecie.
  Serial.println("\nESP32-CAM: test kamery AI Thinker");
  Serial.printf("Powod ostatniego resetu (kod ESP-IDF): %d\n", static_cast<int>(esp_reset_reason()));
  Serial.printf("PSRAM: %s\n", psramFound() ? "dostepna" : "niedostepna");

  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;  config.pin_d1 = 18; config.pin_d2 = 19; config.pin_d3 = 21;
  config.pin_d4 = 36; config.pin_d5 = 39; config.pin_d6 = 34; config.pin_d7 = 35;
  config.pin_xclk = 0; config.pin_pclk = 22; config.pin_vsync = 25; config.pin_href = 23;
  config.pin_sccb_sda = 26; config.pin_sccb_scl = 27;
  config.pin_pwdn = 32; config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  config.fb_location = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  const esp_err_t result = esp_camera_init(&config);
  if (result != ESP_OK) {
    Serial.printf("Blad inicjalizacji kamery: 0x%x\n", static_cast<unsigned>(result));
    return;
  }
  cameraReady = true;
  Serial.println("Kamera zainicjalizowana.");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Laczenie z Wi-Fi 2,4 GHz...");
  const unsigned long start = millis();
  unsigned long lastProgress = start;
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(250);
    if (millis() - lastProgress >= 1000) {
      lastProgress = millis();
      Serial.printf("Laczenie: %lu s, status %d\n", (millis() - start) / 1000, static_cast<int>(WiFi.status()));
    }
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Nie polaczono z Wi-Fi. Sprawdz dane i zresetuj plytke.");
    return;
  }
  server.on("/", []() {
    server.send(200, "text/html; charset=utf-8",
      "<!doctype html><meta name='viewport' content='width=device-width,initial-scale=1'>"
      "<title>Test ESP32-CAM</title><h1>Test ESP32-CAM</h1>"
      "<p><a href='/photo'>Zrob zdjecie</a> — odswiez strone, aby zrobic kolejne.</p>");
  });
  server.on("/photo", handlePhoto);
  server.begin();
  Serial.print("Otworz na telefonie lub komputerze: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (cameraReady && WiFi.status() == WL_CONNECTED) server.handleClient();
  delay(2);
}
