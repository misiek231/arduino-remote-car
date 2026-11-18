#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "wifi_credentials.h"

constexpr uint8_t PIN_DIN = 11, PIN_CLK = 12, PIN_CS = 10;
constexpr uint8_t PIN_JOY_X = 4, PIN_JOY_Y = 5;
constexpr uint8_t PIN_RGB = 48;
constexpr int DEAD_ZONE = 350;
constexpr unsigned long STEP_MS = 450, FOOD_BLINK_MS = 250;
constexpr bool INVERT_X = false, INVERT_Y = false;

struct Point { int8_t x, y; };
Point snake[64];
int length = 0;
Point food;
int dx = 1, dy = 0;
int nextDx = 1, nextDy = 0;
int centerX, centerY;
unsigned long lastStep, lastBlink;
bool foodVisible = true;
bool playing = false;
bool waitForRelease = false;
bool turnQueued = false;
wl_status_t lastWifiStatus = WL_IDLE_STATUS;
bool otaReady = false;

void updateWifiLed() {
  static int lastState = -1;
  const int state = WiFi.status() == WL_CONNECTED ? 2 : (millis() / 500) % 2;
  if (state == lastState) return;
  lastState = state;
  if (state == 2) rgbLedWrite(PIN_RGB, 0, 20, 0); // Polaczono: zielony.
  else if (state == 1) rgbLedWrite(PIN_RGB, 20, 0, 0); // Brak polaczenia: miga na czerwono.
  else rgbLedWrite(PIN_RGB, 0, 0, 0);
}

void updateWifiStatus() {
  const wl_status_t status = WiFi.status();
  if (status == lastWifiStatus) return;
  lastWifiStatus = status;
  if (status == WL_CONNECTED) {
    Serial.print("Polaczono z Wi-Fi! Adres IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Adres MAC Wi-Fi: ");
    Serial.println(WiFi.macAddress());
    if (!otaReady) {
      ArduinoOTA.begin();
      otaReady = true;
      Serial.println("OTA gotowe na porcie 3232.");
    }
  } else {
    Serial.printf("Wi-Fi: brak polaczenia (status %d).\n", int(status));
  }
}

void writeRegister(uint8_t address, uint8_t value) {
  digitalWrite(PIN_CS, LOW);
  shiftOut(PIN_DIN, PIN_CLK, MSBFIRST, address);
  shiftOut(PIN_DIN, PIN_CLK, MSBFIRST, value);
  digitalWrite(PIN_CS, HIGH);
}

bool same(Point a, Point b) { return a.x == b.x && a.y == b.y; }

void render() {
  uint8_t rows[8] = {};
  for (int i = 0; i < length; ++i) rows[snake[i].y] |= 1U << snake[i].x;
  if (playing && foodVisible) rows[food.y] |= 1U << food.x;
  for (int y = 0; y < 8; ++y) writeRegister(y + 1, rows[y]);
}

void placeFood() {
  Point freeCells[64];
  int count = 0;
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      Point candidate = {int8_t(x), int8_t(y)};
      bool occupied = false;
      for (int i = 0; i < length; ++i) {
        if (same(candidate, snake[i])) { occupied = true; break; }
      }
      if (!occupied) freeCells[count++] = candidate;
    }
  }
  if (count > 0) food = freeCells[random(count)];
}

void startGame() {
  length = 3;
  snake[0] = {3, 3}; snake[1] = {2, 3}; snake[2] = {1, 3};
  dx = 1; dy = 0; nextDx = 1; nextDy = 0;
  turnQueued = false;
  playing = true;
  foodVisible = true;
  lastStep = lastBlink = millis();
  placeFood();
  render();
  Serial.println("Start! Zbieraj migajace jedzenie.");
}

void endGame(bool won) {
  playing = false;
  waitForRelease = true;
  Serial.printf("%s Wynik: %d. Pusc joystick, potem wychyl, by zagrac ponownie.\n",
                won ? "Wygrana!" : "Koniec gry!", length - 3);
  // Krotka animacja bezpiecznie tylko na matrycy.
  for (int i = 0; i < 3; ++i) {
    for (int row = 1; row <= 8; ++row) writeRegister(row, 0xff);
    delay(150);
    for (int row = 1; row <= 8; ++row) writeRegister(row, 0);
    delay(150);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  ArduinoOTA.setHostname("snake-esp32");
  ArduinoOTA.onStart([]() { Serial.println("Rozpoczeto aktualizacje OTA."); });
  ArduinoOTA.onEnd([]() { Serial.println("Aktualizacja OTA zakonczona."); });
  ArduinoOTA.onError([](ota_error_t error) { Serial.printf("Blad OTA: %u\n", unsigned(error)); });
  Serial.print("Adres MAC Wi-Fi: ");
  Serial.println(WiFi.macAddress());
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Laczenie z Wi-Fi w tle...");
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
    sumX += analogRead(PIN_JOY_X); sumY += analogRead(PIN_JOY_Y);
    delay(5);
  }
  centerX = sumX / 32; centerY = sumY / 32;
  randomSeed(esp_random());
  startGame();
}

void loop() {
  updateWifiStatus();
  updateWifiLed();
  if (otaReady && WiFi.status() == WL_CONNECTED) ArduinoOTA.handle();
  const int x = (analogRead(PIN_JOY_X) - centerX) * (INVERT_X ? -1 : 1);
  const int y = (analogRead(PIN_JOY_Y) - centerY) * (INVERT_Y ? -1 : 1);
  const bool neutral = abs(x) < DEAD_ZONE && abs(y) < DEAD_ZONE;
  if (!playing) {
    if (neutral) waitForRelease = false;
    else if (!waitForRelease) startGame();
    delay(10);
    return;
  }

  // Jedna zmiana kierunku na ruch; nigdy bezposrednio o 180 stopni.
  if (!turnQueued && !neutral) {
    int candidateX = 0, candidateY = 0;
    if (abs(x) >= abs(y)) candidateX = x > 0 ? 1 : -1;
    else candidateY = y > 0 ? 1 : -1;
    if (!(candidateX == -dx && candidateY == -dy) &&
        !(candidateX == dx && candidateY == dy)) {
      nextDx = candidateX; nextDy = candidateY; turnQueued = true;
    }
  }

  const unsigned long now = millis();
  if (now - lastBlink >= FOOD_BLINK_MS) {
    lastBlink = now; foodVisible = !foodVisible; render();
  }
  if (now - lastStep < STEP_MS) { delay(5); return; }
  lastStep = now;
  dx = nextDx; dy = nextDy; turnQueued = false;
  Point head = {int8_t(snake[0].x + dx), int8_t(snake[0].y + dy)};
  if (head.x < 0 || head.x >= 8 || head.y < 0 || head.y >= 8) {
    endGame(false); return;
  }
  const bool eating = same(head, food);
  // Przy zwyklym ruchu ogon zwalnia pole, wiec wolno na nie wejsc.
  const int collisionLength = eating ? length : length - 1;
  for (int i = 0; i < collisionLength; ++i) {
    if (same(head, snake[i])) { endGame(false); return; }
  }
  if (eating) ++length;
  for (int i = length - 1; i > 0; --i) snake[i] = snake[i - 1];
  snake[0] = head;
  if (eating) {
    Serial.printf("Punkt! Wynik: %d\n", length - 3);
    if (length == 64) { render(); endGame(true); return; }
    placeFood(); foodVisible = true; lastBlink = now;
  }
  render();
}
