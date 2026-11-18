#include <Arduino.h>

// W wielu wersjach DevKitC-1 dioda RGB jest na GPIO48.
// Jesli nie swieci, sprawdz oznaczenie plytki i sprobuj GPIO38.
constexpr uint8_t RGB_PIN = 48;
constexpr uint8_t MAX_BRIGHTNESS = 40; // 0-255
constexpr unsigned long FRAME_MS = 20;
constexpr unsigned long COLOR_CHANGE_MS = 300;

struct Color
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

constexpr Color COLORS[] = {
    {255, 20, 80},
    {100, 0, 255},
    {0, 80, 255},
    {0, 255, 160},
    {255, 120, 0},
};
constexpr size_t COLOR_COUNT = sizeof(COLORS) / sizeof(COLORS[0]);

void setup()
{
  Serial.begin(115200);
  Serial.println("RGB: plynna zorza startuje!");
}

void loop()
{
  static unsigned long lastFrame = 0;
  const unsigned long now = millis();
  if (now - lastFrame < FRAME_MS)
    return;
  lastFrame = now;

  const size_t current = (now / COLOR_CHANGE_MS) % COLOR_COUNT;
  const size_t next = (current + 1) % COLOR_COUNT;
  const float blend = float(now % COLOR_CHANGE_MS) / COLOR_CHANGE_MS;
  const Color &a = COLORS[current];
  const Color &b = COLORS[next];

  // Lagodne przejscie miedzy sasiednimi kolorami.
  const uint8_t red = (a.red + (int(b.red) - a.red) * blend) * MAX_BRIGHTNESS / 255;
  const uint8_t green = (a.green + (int(b.green) - a.green) * blend) * MAX_BRIGHTNESS / 255;
  const uint8_t blue = (a.blue + (int(b.blue) - a.blue) * blend) * MAX_BRIGHTNESS / 255;
  rgbLedWrite(RGB_PIN, red, green, blue);
}
