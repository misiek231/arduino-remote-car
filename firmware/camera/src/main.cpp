#include "camera_app.h"
#include "wifi_credentials.h"

extern WiFiUDP discovery;
extern unsigned long lastAnnouncement;

void setup() {
  pinMode(FLASH_PIN, OUTPUT);
  digitalWrite(FLASH_PIN, LOW);
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
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 2;
  config.fb_location = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;
  config.grab_mode = CAMERA_GRAB_LATEST;

  const esp_err_t result = esp_camera_init(&config);
  if (result != ESP_OK) {
    Serial.printf("Blad inicjalizacji kamery: 0x%x\n", static_cast<unsigned>(result));
    return;
  }
  cameraReady = true;
  Serial.println("Kamera zainicjalizowana.");
  flashReady = initFlash();
  Serial.println(flashReady ? "Lampa gotowa (GPIO 4), wylaczona." : "Blad inicjalizacji lampy.");
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
  ArduinoOTA.setHostname("esp32-cam-rc");
  ArduinoOTA.onStart([]() { otaUpdating = true; if (flashReady) setFlash(0); Serial.println("Rozpoczeto aktualizacje OTA; podglad i lampa zostana zatrzymane."); });
  ArduinoOTA.onEnd([]() { Serial.println("Aktualizacja OTA zakonczona."); });
  ArduinoOTA.onError([](ota_error_t error) { otaUpdating = false; Serial.printf("Blad OTA: %u\n", unsigned(error)); });
  ArduinoOTA.begin();
  Serial.println("OTA gotowe: esp32-cam-rc.local (port 3232).");
  initCameraRoutes();
  streamReady = startStreamServer();
  if (streamReady) Serial.println("Serwer podgladu uruchomiony na porcie 81.");
  else Serial.println("Blad uruchomienia serwera podgladu.");
  Serial.print("Otworz na telefonie lub komputerze: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (cameraReady && WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle();
    server.handleClient();
    if (streamReady && millis() - lastAnnouncement >= 1000) {
      lastAnnouncement = millis();
      const IPAddress local = WiFi.localIP(), mask = WiFi.subnetMask();
      IPAddress broadcast;
      for (int i = 0; i < 4; ++i) broadcast[i] = local[i] | static_cast<uint8_t>(~mask[i]);
      if (discovery.beginPacket(broadcast, 42142)) {
        discovery.print("RC_CAM_V1");
        discovery.endPacket();
      }
    }
  }
  delay(2);
}
