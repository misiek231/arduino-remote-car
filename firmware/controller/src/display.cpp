#include "app.h"

void initOled()
{
  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setTimeOut(20);
  uint8_t address = 0;
  for (uint8_t candidate = 0x3c; candidate <= 0x3d; ++candidate)
  {
    Wire.beginTransmission(candidate);
    if (Wire.endTransmission() == 0) { address = candidate; break; }
  }
  if (address == 0)
  {
    Serial.println("OLED nie odpowiada pod 0x3C ani 0x3D. Sprawdz polaczenia.");
    return;
  }
  Serial.printf("Znaleziono urzadzenie I2C pod adresem 0x%02X; probuje OLED SH1106.\n", address);
  oled.setI2CAddress(address << 1);
  oled.begin();
  oledReady = true;
}

void drawOled()
{
  if (!oledReady) return;
  char line[32];
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(0, 10, "STEROWNIK SILNIKA");
  oled.drawHLine(0, 13, 128);
  snprintf(line, sizeof(line), "PWM: %d%% / cel %d%%", motorPercent, motorTargetPercent);
  oled.drawStr(0, 25, line);
  if (foutMeasured) snprintf(line, sizeof(line), "FOUT: %.1f przerw/s", lastGapRate);
  else snprintf(line, sizeof(line), "FOUT: oczekiwanie");
  oled.drawStr(0, 38, line);
  oled.drawStr(0, 51, motorTargetPercent ? "Silnik: STEROWANIE" : "Silnik: STOP");
  oled.drawStr(0, 63, WiFi.status() != WL_CONNECTED ? "WiFi: brak polaczenia" :
               (cameraSeen && millis() - cameraLastSeen < CAMERA_TIMEOUT_MS ? "WiFi: OK  CAM: OK" : "WiFi: OK  CAM: --"));
  oled.sendBuffer();
}

