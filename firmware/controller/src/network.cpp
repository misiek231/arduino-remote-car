#include "app.h"

void updateNetwork()
{
  const wl_status_t status = WiFi.status();
  if (status != lastWifiStatus)
  {
    lastWifiStatus = status;
    if (status == WL_CONNECTED)
    {
      Serial.print("Wi-Fi polaczone. IP: ");
      Serial.println(WiFi.localIP());
      cameraDiscoveryReady = cameraDiscovery.begin(42142) == 1;
      Serial.println(cameraDiscoveryReady ? "Nasluch kamery UDP 42142 gotowy." : "Nie mozna uruchomic nasluchu kamery.");
      if (!webServerStarted) initWebServer();
      Serial.println("Strona sterowania dostepna pod adresem IP na porcie 80.");
      if (!otaReady)
      {
        ArduinoOTA.begin();
        otaReady = true;
        Serial.println("OTA gotowe na porcie 3232.");
      }
    }
    else
    {
      if (webActive) stopMotor();
      cameraDiscovery.stop();
      cameraDiscoveryReady = false;
      cameraSeen = false;
      Serial.printf("Brak polaczenia Wi-Fi (status %d).\n", int(status));
    }
  }
  static int lastLed = -1;
  const int led = status == WL_CONNECTED ? 2 : (millis() / 500) % 2;
  if (led != lastLed)
  {
    lastLed = led;
    if (led == 2)
      rgbLedWrite(RGB_PIN, 0, 20, 0);
    else if (led == 1)
      rgbLedWrite(RGB_PIN, 20, 0, 0);
    else
      rgbLedWrite(RGB_PIN, 0, 0, 0);
  }
  if (otaReady && status == WL_CONNECTED)
    ArduinoOTA.handle();
}

void updateCameraDiscovery()
{
  if (!cameraDiscoveryReady || WiFi.status() != WL_CONNECTED) return;
  for (int i = 0; i < 5; ++i) {
    const int packetSize = cameraDiscovery.parsePacket();
    if (!packetSize) break;
    char message[16] = {};
    const int received = cameraDiscovery.read(message, sizeof(message) - 1);
    if (received == 9 && strcmp(message, "RC_CAM_V1") == 0) {
      cameraIp = cameraDiscovery.remoteIP();
      cameraLastSeen = millis();
      cameraSeen = true;
    }
  }
}

