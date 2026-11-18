#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "wifi_credentials.h"

constexpr uint8_t PIN_DIN = 11, PIN_CLK = 12, PIN_CS = 10;
constexpr uint8_t PIN_JOY_X = 4, PIN_RGB = 48;
constexpr int DEAD_ZONE = 350;
constexpr bool INVERT_X = false;
constexpr unsigned long BALL_MS = 250, PADDLE_MS = 120;

constexpr int PADDLE_WIDTH = 4;
int centerX = 0, paddle = 2;
int ballX = 3, ballY = 5, dx = 1, dy = -1;
uint8_t bricks[2];
bool playing = false, waitForRelease = false, otaReady = false;
unsigned long lastBall = 0, lastPaddle = 0;
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

void render() {
  uint8_t rows[8] = {};
  rows[0] = bricks[0]; rows[1] = bricks[1];
  if (playing) {
    rows[ballY] |= 1U << ballX;
    rows[7] |= ((1U << PADDLE_WIDTH) - 1) << paddle;
  }
  for (int y = 0; y < 8; ++y) writeRegister(y + 1, rows[y]);
}

void startGame() {
  bricks[0] = bricks[1] = 0xff;
  paddle = 2; ballX = 3; ballY = 5; dx = 1; dy = -1;
  playing = true;
  lastBall = lastPaddle = millis();
  Serial.println("Breakout! Odbijaj pilke i rozbij cegielki.");
  render();
}

void endGame(bool won) {
  playing = false;
  waitForRelease = true;
  Serial.println(won ? "Wygrana! Pusc joystick, potem wychyl, by zaczac." :
                       "Koniec gry! Pusc joystick, potem wychyl, by zaczac.");
  // Znacznik zwyciestwa albo porazki, bez zatrzymywania obslugi OTA.
  const uint8_t win[8] = {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81};
  const uint8_t lose[8] = {0, 0x42, 0x42, 0, 0x3c, 0x42, 0, 0};
  for (int y = 0; y < 8; ++y) writeRegister(y + 1, won ? win[y] : lose[y]);
}

void moveBall() {
  int nextX = ballX + dx;
  if (nextX < 0 || nextX > 7) { dx = -dx; nextX = ballX + dx; }
  int nextY = ballY + dy;
  if (nextY < 0) { dy = 1; nextY = ballY + dy; }
  if (nextY <= 1 && (bricks[nextY] & (1U << nextX))) {
    bricks[nextY] &= ~(1U << nextX);
    if (!(bricks[0] | bricks[1])) { endGame(true); return; }
    dy = -dy;
    nextY = ballY + dy;
    if (nextY < 0) { dy = 1; nextY = ballY + dy; }
  }
  if (nextY == 7) {
    if (nextX < paddle || nextX >= paddle + PADDLE_WIDTH) { endGame(false); return; }
    dy = -1;
    dx = nextX < paddle + PADDLE_WIDTH / 2 ? -1 : 1;
    nextY = 6;
  }
  ballX = nextX; ballY = nextY;
  render();
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
  Serial.println("Kalibracja: zostaw joystick w spoczynku.");
  delay(1000);
  long sum = 0;
  for (int i = 0; i < 32; ++i) { sum += analogRead(PIN_JOY_X); delay(5); }
  centerX = sum / 32;
  startGame();
}

void loop() {
  updateNetwork();
  const int x = (analogRead(PIN_JOY_X) - centerX) * (INVERT_X ? -1 : 1);
  if (!playing) {
    if (abs(x) < DEAD_ZONE) waitForRelease = false;
    else if (!waitForRelease) startGame();
    delay(5);
    return;
  }
  const unsigned long now = millis();
  if (abs(x) >= DEAD_ZONE && now - lastPaddle >= PADDLE_MS) {
    lastPaddle = now;
    paddle = constrain(paddle + (x > 0 ? 1 : -1), 0, 8 - PADDLE_WIDTH);
    render();
  }
  if (now - lastBall >= BALL_MS) { lastBall = now; moveBall(); }
  delay(5);
}
