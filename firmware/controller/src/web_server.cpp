#include "app.h"
#include "control_page.h"

void initWebServer()
{
  webServer.on("/", HTTP_GET, []() { webServer.send_P(200, "text/html; charset=utf-8", CONTROL_PAGE); });
  webServer.on("/start", HTTP_POST, []() {
    const String speed = webServer.arg("speed");
    if (speed.length() == 0 || speed.length() > 3) { webServer.send(400, "text/plain", "Nieprawidlowa wartosc"); return; }
    for (unsigned int i = 0; i < speed.length(); ++i)
      if (speed[i] < '0' || speed[i] > '9') { webServer.send(400, "text/plain", "Nieprawidlowa wartosc"); return; }
    const int value = speed.toInt();
    const String requestedDirection = webServer.arg("direction");
    if (!motorReady || updating || value < 1 || value > 100 ||
        requestedDirection != (reverseDirection ? "reverse" : "forward")) {
      webServer.send(400, "text/plain", "Sterowanie niedostepne lub nieprawidlowa wartosc"); return;
    }
    webActive = true;
    setMotor(value);
    webServer.send(200, "text/plain", "OK");
  });
  webServer.on("/speed", HTTP_POST, []() {
    const String speed = webServer.arg("speed");
    if (!webActive || !motorReady || updating || speed.length() == 0 || speed.length() > 3) {
      webServer.send(409, "text/plain", "Sterowanie niedostepne"); return;
    }
    for (unsigned int i = 0; i < speed.length(); ++i)
      if (speed[i] < '0' || speed[i] > '9') { webServer.send(400, "text/plain", "Nieprawidlowa wartosc"); return; }
    const int value = speed.toInt();
    if (value < 1 || value > 100) { webServer.send(400, "text/plain", "Nieprawidlowa wartosc"); return; }
    setMotor(value);
    webServer.send(200, "text/plain", "OK");
  });
  webServer.on("/keep", HTTP_POST, []() {
    if (!webActive || !motorReady || updating) { webServer.send(409, "text/plain", "Silnik nie jest aktywny"); return; }
    lastCommand = millis();
    webServer.send(200, "text/plain", "OK");
  });
  webServer.on("/stop", HTTP_POST, []() { stopMotor(); webServer.send(200, "text/plain", "OK"); });
  webServer.on("/direction", HTTP_POST, []() {
    const String direction = webServer.arg("direction");
    if (direction != "forward" && direction != "reverse") { webServer.send(400, "text/plain", "Nieprawidlowy kierunek"); return; }
    if (updating || !motorReady) { webServer.send(409, "text/plain", "Sterowanie niedostepne"); return; }
    const bool requestedReverse = direction == "reverse";
    if (requestedReverse == reverseDirection) { webServer.send(200, "text/plain", "OK"); return; }
    if (motorTargetPercent != 0 || motorPercent != 0) stopMotor();
    digitalWrite(DIRECTION_PIN, requestedReverse ? HIGH : LOW);
    reverseDirection = requestedReverse;
    Serial.printf("Kierunek: %s.\n", reverseDirection ? "wstecz" : "przod");
    webServer.send(200, "text/plain", "OK");
  });
  webServer.on("/servo", HTTP_POST, []() {
    const String name = webServer.arg("servo"), angleText = webServer.arg("angle");
    if (updating || (name != "steering" && name != "gear") || angleText.length() == 0 || angleText.length() > 3) {
      webServer.send(400, "text/plain", "Nieprawidlowe polecenie"); return;
    }
    for (unsigned int i = 0; i < angleText.length(); ++i)
      if (angleText[i] < '0' || angleText[i] > '9') { webServer.send(400, "text/plain", "Nieprawidlowy kat"); return; }
    const int angle = angleText.toInt();
    const int minimum = name == "steering" ? STEERING_MIN_DEG : GEAR_MIN_DEG;
    const int maximum = name == "steering" ? STEERING_MAX_DEG : GEAR_MAX_DEG;
    if (angle < minimum || angle > maximum) { webServer.send(400, "text/plain", "Kat poza zakresem"); return; }
    int rate = 180;
    if (name == "steering" && webServer.hasArg("rate")) {
      const String rateText = webServer.arg("rate");
      if (rateText.length() == 0 || rateText.length() > 3) { webServer.send(400, "text/plain", "Nieprawidlowa predkosc"); return; }
      for (unsigned int i = 0; i < rateText.length(); ++i)
        if (rateText[i] < '0' || rateText[i] > '9') { webServer.send(400, "text/plain", "Nieprawidlowa predkosc"); return; }
      rate = rateText.toInt();
      if (rate < 20 || rate > 600) { webServer.send(400, "text/plain", "Predkosc poza zakresem"); return; }
    }
    const bool ok = name == "steering" ? setSteeringTarget(angle, rate)
                                        : setServoPosition(GEAR_SERVO_PIN, 3, gearServoReady, angle);
    if (!ok) { webServer.send(500, "text/plain", "Nie udalo sie ustawic serwa"); return; }
    Serial.printf("Serwo %s: zadana pozycja %d stopni.\n", name.c_str(), angle);
    webServer.send(200, "text/plain", "OK");
  });
  webServer.on("/status", HTTP_GET, []() {
    char body[128], camera[16] = "";
    if (cameraSeen && millis() - cameraLastSeen < CAMERA_TIMEOUT_MS)
      snprintf(camera, sizeof(camera), "%u.%u.%u.%u", cameraIp[0], cameraIp[1], cameraIp[2], cameraIp[3]);
    if (foutMeasured) snprintf(body, sizeof(body), "{\"pwm\":%d,\"target\":%d,\"fout\":%.1f,\"camera\":\"%s\",\"direction\":\"%s\"}", motorPercent, motorTargetPercent, lastGapRate, camera, reverseDirection ? "reverse" : "forward");
    else snprintf(body, sizeof(body), "{\"pwm\":%d,\"target\":%d,\"fout\":null,\"camera\":\"%s\",\"direction\":\"%s\"}", motorPercent, motorTargetPercent, camera, reverseDirection ? "reverse" : "forward");
    webServer.sendHeader("Cache-Control", "no-store");
    webServer.send(200, "application/json", body);
  });
  webServer.begin();
  webServerStarted = true;
}

