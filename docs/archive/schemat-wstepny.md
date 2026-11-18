> **Archiwum:** ten dokument opisuje wcześniejszy etap projektu. Ścieżki, piny i ustalenia mogą być nieaktualne; bieżące informacje są w [README](../../README.md) i [schemacie](../schemat-aktualny.svg).

# Samochód RC — wstępny schemat połączeń

Stan: 11 listopada 2026. To schemat do planowania i kolejnych prób, a nie zweryfikowany projekt do ostatecznego montażu. Nieznane piny i wartości zabezpieczeń oznaczono jako wymagające ustalenia.

## Zasilanie i ładowanie

```text
  pakiet 2S (maks. 8,4 V)
       │
       └── BMS 2S: ogniwa do B− / punktu środkowego / B+
                       │
                      P+  P− ─────────────────────────────── GND wspólne
                       │
                       ├── płytka ładowania z USB-C
                       │   wejście baterii zgodnie z jej rzeczywistym złączem
                       │   (połączenie i samodzielną pracę jeszcze potwierdzić)
                       │
                       └── bezpiecznik* ── wyłącznik jazdy* ── VBAT po wyłączniku
                                                                │
                               ┌────────────────────────────────┴────────────┐
                               │                                             │
                         M+ silnika                                  przetwornica 5 V**
                         GN ── GND                                   IN+ ← VBAT
                                                                     IN− ← GND
                                                                     OUT− → GND
                                                                     OUT+ → 5 V
                                                                              │
                                     ┌──────────────────┬─────────────────────┼───────────────┐
                                     │                  │                     │               │
                               ESP32-S3 5V/VIN    ESP32-CAM 5V          SG90 skręt +5 V  SG90 biegi +5 V
                               GND → GND          GND → GND             GND → GND        GND → GND
```

\* Umiejscowienie bezpiecznika i wyłącznika to koncepcja: dobór rodzaju, wartości, prądu ciągłego oraz lokalizacji zależy od schematu płytki ładowania, przewodów i rzeczywistych prądów. Celem jest wyłączenie jazdy bez odłączania toru ładowania. Nie mostkować BMS ani nie podłączać toru ładowania do zacisków ogniw w ciemno.

\*\* XL4015 z koszyka nie nadaje się jako gwarantowane źródło 5 V w pełnym zakresie 2S: [producent podaje zakres wejściowy 8–36 V](https://www.xlsemi.com/datasheet/XL4015-EN.pdf). Potrzebny jest odpowiednio dobrany moduł o zakresie obejmującym minimalne napięcie 2S pod obciążeniem, z zapasem prądowym na dwa serwa i oba ESP. Buck z odpowiednio niskim minimalnym wejściem może wystarczyć; buck-boost jest inną możliwością. Oceniać wydajność całego modułu, nie tylko napis na aukcji.

Podłączenie obu płytek do 5 V jest projektem wstępnym — sprawdzić oznaczenia i schemat konkretnego ESP32-S3 oraz ESP32-CAM. Przed jednoczesnym zasilaniem przez USB i zewnętrzne 5 V ustalić sposób przełączania źródeł lub zasilać płytkę tylko z jednego. Masy serw, ESP i silnika są wspólne, ale przewody prądowe silnika i serw prowadzić do punktu rozdziału, nie przez płytkę ESP.

## Sterowanie i sygnały

```text
ESP32-S3                                  silnik / moduły
GPIO 7 ── 10 kΩ ──┬── baza 2N2222A
                  └── 100 kΩ ── GND
                         emiter ── GND / GN silnika
                         kolektor ── PWM silnika

FOUT silnika → 10 kΩ → węzeł → GPIO 6; węzeł → 10 kΩ → 5 kΩ → GND
GPIO 8 SDA ─────────────────────────── SDA OLED
GPIO 9 SCL ─────────────────────────── SCL OLED
3V3 ESP ────────────────────────────── VCC OLED (przetestowano)
GND ESP ────────────────────────────── GND OLED

GPIO serwa skrętu (do wyboru) ──────── sygnał SG90 skrętu
GPIO serwa biegów (do wyboru) ──────── sygnał SG90 biegów
ADC (do wyboru) ← dzielnik napięcia i ochrona ← VBAT po wyłączniku
```

GPIO 7, 8 i 9 to połączenia przetestowane; GPIO 6 służyło do pomiaru FOUT przez podany dzielnik. Dzielnik działał w teście, ale nie zapewnia pełnej ochrony przed nieznanymi przepięciami; zabezpieczenia docelowego toru wymagają weryfikacji. GPIO serw i ADC trzeba dobrać do rzeczywistego pinoutu płytki; nie przydzielać pinów USB i pamięci. [Espressif opisuje ograniczenia GPIO oraz GPIO19/20 dla USB-JTAG](https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s3/api-reference/peripherals/gpio.html). Potwierdzić rozkład E/B/C użytego tranzystora.

Sygnały serw z ESP mają 3,3 V: działanie konkretnego SG90 przy zasilaniu 5 V należy sprawdzić; jeśli poziom HIGH nie jest akceptowany, dobrać odpowiedni bufor/konwerter. Nie podłączać sygnału 5 V do GPIO ESP.

Komunikacja ESP32-CAM ↔ ESP32-S3 i przekazywanie stanu kamery: protokół oraz połączenia nie są jeszcze wybrane. Transmisja obrazu MJPEG przez Wi-Fi działała w próbie blisko routera.

## Pomiar baterii — zasada

Dzielnik do ADC ma tylko dostarczać sygnał pomiarowy, nie zasilać ESP. Projektując go, uwzględnić maksymalne napięcie, tolerancję elementów, zakres ADC, filtrowanie, pobór z baterii oraz ochronę i możliwość zasilania ESP przez wejście pomiarowe przy wyłączonym ESP. Napięcie całego pakietu daje przybliżenie stanu, a nie pomiar poszczególnych ogniw czy dokładny procent. BMS pozostaje odpowiedzialny za swoją funkcję ochronną.

## Kolejność uruchamiania

1. Zidentyfikować zaciski i zakresy zasilania oraz potwierdzić tor ładowania. Bez podłączania nieznanych przewodów `PWM1`, `PWM2`, `STAT`, `P_ON` do ESP.
2. Dobrać przetwornicę, ustawić 5 V **bez podłączonych odbiorników** i zmierzyć napięcie oraz polaryzację; sprawdzić stabilność pod odpowiednim obciążeniem i w zakresie napięć wejściowych pakietu.
3. Zasilić jeden element naraz: główne ESP, następnie jedno SG90 i drugie. Każde serwo sprawdzić bez zamontowania w mechanizmie, z ograniczonym zakresem ruchu; zbadać napięcie 5 V podczas ruchu.
4. Podłączyć ESP-CAM, OLED i sterowanie silnikiem etapami; przed ruchem silnika upewnić się, że GPIO podczas startu/resetu daje stan zatrzymania. Sam reset nie zastępuje fizycznego odcięcia napędu.
5. Dopiero po sprawdzeniu wspólnego działania zamontować układ w aucie; najpierw testować z uniesionymi kołami i łatwym dostępem do odłączenia zasilania.

Nie łączyć równocześnie ładowania z jazdą, dopóki cały tor nie zostanie rozpoznany i przetestowany.
