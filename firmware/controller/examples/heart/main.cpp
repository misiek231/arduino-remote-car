#include <Arduino.h>

// Wstepna konfiguracja dla jednej matrycy ze sterownikiem MAX7219.
constexpr uint8_t PIN_DIN = 11;
constexpr uint8_t PIN_CLK = 12;
constexpr uint8_t PIN_CS = 10;

// Kolejne bajty to wiersze; bit 1 zapala piksel.
const uint8_t FRAMES[][8] = {
    {0x00, 0x00, 0x24, 0x7e, 0x7e, 0x3c, 0x18, 0x00}, // male serce
    {0x00, 0x66, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0x00}, // duze serce
};
constexpr size_t FRAME_COUNT = sizeof(FRAMES) / sizeof(FRAMES[0]);

void writeRegister(uint8_t address, uint8_t value) {
  digitalWrite(PIN_CS, LOW);
  shiftOut(PIN_DIN, PIN_CLK, MSBFIRST, address);
  shiftOut(PIN_DIN, PIN_CLK, MSBFIRST, value);
  digitalWrite(PIN_CS, HIGH);
}

void showFrame(const uint8_t frame[8]) {
  for (uint8_t row = 0; row < 8; ++row) {
    writeRegister(row + 1, frame[row]);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_DIN, OUTPUT);
  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH);
  digitalWrite(PIN_CLK, LOW);

  writeRegister(0x0f, 0); // test wyswietlacza wylaczony
  writeRegister(0x09, 0); // tryb pojedynczych pikseli
  writeRegister(0x0b, 7); // wszystkie osiem wierszy
  writeRegister(0x0a, 1); // jasnosc 0-15
  for (uint8_t row = 1; row <= 8; ++row) writeRegister(row, 0);
  writeRegister(0x0c, 1); // normalna praca
  Serial.println("Start animacji MAX7219");
}

void loop() {
  static size_t frame = 0;
  static unsigned long previous = 0;
  const unsigned long now = millis();
  if (now - previous >= 400) {
    previous = now;
    showFrame(FRAMES[frame]);
    frame = (frame + 1) % FRAME_COUNT;
  }
}
