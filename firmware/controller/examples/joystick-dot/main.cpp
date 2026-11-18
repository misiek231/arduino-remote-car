#include <Arduino.h>

// MAX7219: piny z poprzedniego eksperymentu.
constexpr uint8_t PIN_DIN = 11;
constexpr uint8_t PIN_CLK = 12;
constexpr uint8_t PIN_CS = 10;

// Joystick: obie osie podlaczone do przetwornika ADC1.
constexpr uint8_t PIN_JOY_X = 4;
constexpr uint8_t PIN_JOY_Y = 5;
constexpr int DEAD_ZONE = 350;
constexpr unsigned long MOVE_INTERVAL_MS = 180;
constexpr unsigned long LOG_INTERVAL_MS = 500;

int centerX = 0;
int centerY = 0;
int dotX = 3;
int dotY = 3;

void writeRegister(uint8_t address, uint8_t value) {
  digitalWrite(PIN_CS, LOW);
  shiftOut(PIN_DIN, PIN_CLK, MSBFIRST, address);
  shiftOut(PIN_DIN, PIN_CLK, MSBFIRST, value);
  digitalWrite(PIN_CS, HIGH);
}

void drawDot() {
  for (int row = 0; row < 8; ++row) {
    writeRegister(row + 1, row == dotY ? uint8_t(1U << dotX) : 0);
  }
}

void calibrateJoystick() {
  long sumX = 0;
  long sumY = 0;
  Serial.println("Kalibracja: pusc joystick na chwile.");
  delay(1000);
  for (int i = 0; i < 32; ++i) {
    sumX += analogRead(PIN_JOY_X);
    sumY += analogRead(PIN_JOY_Y);
    delay(5);
  }
  centerX = sumX / 32;
  centerY = sumY / 32;
  Serial.printf("Srodek: X=%d Y=%d\n", centerX, centerY);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_DIN, OUTPUT);
  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH);
  digitalWrite(PIN_CLK, LOW);

  writeRegister(0x0f, 0); // test wylaczony
  writeRegister(0x09, 0); // pojedyncze piksele
  writeRegister(0x0b, 7); // osiem wierszy
  writeRegister(0x0a, 1); // niewielka jasnosc
  writeRegister(0x0c, 1); // normalna praca
  drawDot();

  analogReadResolution(12);
  analogSetPinAttenuation(PIN_JOY_X, ADC_11db);
  analogSetPinAttenuation(PIN_JOY_Y, ADC_11db);
  calibrateJoystick();
}

void loop() {
  static unsigned long lastMove = 0;
  static unsigned long lastLog = 0;
  const unsigned long now = millis();
  const int rawX = analogRead(PIN_JOY_X);
  const int rawY = analogRead(PIN_JOY_Y);

  if (now - lastMove >= MOVE_INTERVAL_MS) {
    lastMove = now;
    if (rawX > centerX + DEAD_ZONE) ++dotX;
    else if (rawX < centerX - DEAD_ZONE) --dotX;
    if (rawY > centerY + DEAD_ZONE) ++dotY;
    else if (rawY < centerY - DEAD_ZONE) --dotY;
    dotX = constrain(dotX, 0, 7);
    dotY = constrain(dotY, 0, 7);
    drawDot();
  }

  if (now - lastLog >= LOG_INTERVAL_MS) {
    lastLog = now;
    Serial.printf("X=%d Y=%d | kropka=(%d,%d)\n", rawX, rawY, dotX, dotY);
  }
  delay(5);
}
