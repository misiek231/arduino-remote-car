#include "app.h"
#include "wifi_credentials.h"

void handleCommands()
{
  while (Serial.available())
  {
    char c = Serial.read();
    if (c == '0' || c == 's' || c == 'S')
      stopMotor();
    else if (c >= '1' && c <= '9')
    {
      if (!motorReady || updating)
        Serial.println("Sterowanie niedostepne.");
      else
      { webActive = false; setMotor((c - '0') * 10); }
    }
    else if (c == 'f' || c == 'F')
    {
      if (!motorReady || updating)
        Serial.println("Sterowanie niedostepne.");
      else
      { webActive = false; setMotor(100); }
    }
    else if (c == 'h' || c == 'H')
    {
      if (motorTargetPercent > 0)
      {
        lastCommand = millis();
        Serial.printf("Przedluzono prace o %lu s.\n", MOTOR_TIMEOUT_MS / 1000);
      }
    }
    else if (c != '\r' && c != '\n')
    {
      Serial.println("Polecenia: 0/s=stop, 1-9=10-90%, f=100%, h=przedluz czas.");
    }
  }
}

void setup()
{
  digitalWrite(DIRECTION_PIN, LOW);
  pinMode(DIRECTION_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);
  pinMode(MOTOR_PIN, OUTPUT);
  Serial.begin(115200);
  pinMode(FOUT_PIN, INPUT); // Bez wewnetrznego pull-up/pull-down.
  attachInterrupt(digitalPinToInterrupt(FOUT_PIN), onFoutFalling, FALLING);
  lastReport = millis();
  motorReady = ledcAttach(MOTOR_PIN, MOTOR_FREQ_HZ, PWM_BITS);
  if (motorReady)
    ledcWrite(MOTOR_PIN, 0);
  else
    Serial.println("BLAD: nie udalo sie uruchomic PWM. Silnik pozostaje w stanie stop.");
  initOled();
  drawOled();
  WiFi.mode(WIFI_STA);
  ArduinoOTA.setHostname("snake-esp32");
  ArduinoOTA.onStart([]()
                     { updating = true; stopMotor(); Serial.println("Rozpoczeto aktualizacje OTA."); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("Aktualizacja OTA zakonczona."); });
  ArduinoOTA.onError([](ota_error_t error)
                     { updating = false; stopMotor(); Serial.printf("Blad OTA: %u\n", unsigned(error)); });
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("Pomiar FOUT: zbocza opadajace na GPIO %u, odczyt co okolo 1 s.\n", FOUT_PIN);
  Serial.printf("Silnik GPIO %u: 0/s=stop, 1-9=10-90%%, f=100%%, h=przedluz. Auto-stop po %lu s.\n", MOTOR_PIN, MOTOR_TIMEOUT_MS / 1000);
}

void loop()
{
  handleCommands();
  if (motorTargetPercent > 0 && millis() - lastCommand >= (webActive ? WEB_TIMEOUT_MS : MOTOR_TIMEOUT_MS))
  {
    stopMotor();
    Serial.println("Auto-stop: brak aktualnych polecen.");
  }
  updateNetwork();
  updateMotorRamp();
  updateSteering();
  updateCameraDiscovery();
  if (webServerStarted && WiFi.status() == WL_CONNECTED) webServer.handleClient();
  const unsigned long now = millis();
  if (now - lastReport >= REPORT_INTERVAL_MS)
  {
    const unsigned long elapsed = now - lastReport;
    lastReport = now;
    reportFout(elapsed);
  }
  if (now - lastOledUpdate >= 250)
  {
    lastOledUpdate = now;
    drawOled();
  }
  delay(5);
}
