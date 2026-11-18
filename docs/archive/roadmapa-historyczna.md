> **Archiwum:** ten dokument opisuje wcześniejszy etap projektu. Ścieżki, piny i ustalenia mogą być nieaktualne; bieżące informacje są w [README](../../README.md) i [schemacie](../schemat-aktualny.svg).

# Samochód RC LEGO Technic — plan projektu

Dokument roboczy: zapisuje obecne ustalenia, pytania i kolejne kroki. Aktualizujemy go po pomiarach i ważnych decyzjach. Niezaznaczone punkty są planem, nie opisem ukończonych funkcji.

Ostatnia aktualizacja: 11 listopada 2026 r.

## Cel

Zbudować samochód LEGO Technic sterowany z telefonu przez Wi-Fi, z napędem z masażera, serwami skrętu i skrzyni biegów, obrazem z osobnej kamery oraz wyświetlaczem stanu. Projekt ma jednocześnie służyć do nauki elektroniki, ESP32 i programowania.

Najbliższy krok: **ustalić i zaplanować zasilanie oraz ładowanie samochodu**. Dalszą kalibrację silnika i FOUT wykonamy na zmontowanym aucie.

## Stan projektu

- [x] Główna płytka ESP32-S3 została uruchomiona w PlatformIO; działają programowanie przez USB i debugowanie przez USB-JTAG.
- [x] W projekcie testowym uruchomiono Wi-Fi oraz wgrywanie przez sieć OTA. Moduł ma rezerwację DHCP `192.168.0.15` w obecnej sieci.
- [x] Przetestowano joystick i matrycę LED 8×8 z MAX7219. To stanowisko eksperymentalne, nie docelowa elektronika auta.
- [x] Stary pakiet z masażera i jego BMS zostały zutylizowane. Nowy pakiet składa się z dwóch ogniw szeregowo (2S), do 8,4 V po pełnym naładowaniu.
- [x] Rozpoznać podstawy sterowania silnikiem i uruchomić go z ESP na stole. Dalsza kalibracja i testy pod obciążeniem są zaplanowane na działającym aucie.
- [x] OLED działa z głównym ESP; ESP32-CAM uruchomiono, a podgląd MJPEG działał płynnie w pobliżu routera. W słabszym Wi-Fi występowały zacięcia.
- [ ] Zbudować i sprawdzić zasilanie, elektronikę oraz oprogramowanie samochodu.
- [ ] Uruchomić kamerę, komunikację między ESP i interfejs sterowania.

## Komponenty

| Element | Rola i obecne informacje | Do ustalenia |
| --- | --- | --- |
| ESP32-S3 N16R8, płytka z dwoma USB-C | Główny sterownik: sieć, napęd, serwa, OLED i status auta. Płytka działa w projekcie testowym. | Dokładny model i schemat płytki, wyprowadzenia, zasilanie 5 V/USB; potwierdzić ustawienia pamięci w PlatformIO. |
| ESP32-CAM z OV2640 | Osobny moduł obrazu; planowany strumień na telefon i przekazywanie stanu głównemu ESP. | Potwierdzić posiadany wariant, sposób zasilania i programowania oraz docelowy sposób transmisji obrazu. |
| OLED 1,3 cala, 128×64, I²C | Wyświetlanie stanu samochodu, Wi-Fi i kamery. | Potwierdzić sterownik, napięcie zasilania i poziomy sygnałów. |
| Silnik z masażera | Napęd samochodu; ma przewody opisane `PWM`, `GN`, `NC`, `FOUT`, `M+`. | Model, typ, funkcje pinów, poziomy i przebiegi sygnałów, sposób sterowania i pobór prądu. |
| Dwa serwa SG90 | Skręt i skrzynia biegów; planowane zasilanie 5 V. | Prąd przy rozruchu i obciążeniu, zakresy ruchu i mechanika. |
| Nowy pakiet i BMS | Dwa ogniwa szeregowo, maksymalnie 8,4 V po naładowaniu. | Połączenia i zachowanie pod obciążeniem. |
| Płytka ładowania masażera z USB-C | Zgodna z nowym pakietem według użytkownika; ładowanie i praca działają. | Bez płytki przycisków zaobserwowano wzrost napięcia o ok. 0,10 V po podłączeniu ładowarki; sam wzrost nie potwierdza jeszcze przepływu prądu ładowania. |
| Oryginalna płytka masażera z wyświetlaczem i przyciskami | Źródło sygnałów referencyjnych podczas badania silnika. Wcześniej rozważano wykorzystanie jej wskaźnika baterii. | Docelowo planowane odłączenie od sterowania silnikiem; ewentualne ponowne użycie wyświetlacza pozostaje otwarte. |
| Przetwornica XL4015 step-down 5 A | Posiadana mniejsza przetwornica. | Producent XL4015 podaje wejście 8–36 V, więc nie jest właściwym docelowym wyborem dla pełnego zakresu pakietu 2S; wybrać inną przetwornicę 5 V. |
| Płytki uniwersalne, złącza i przewody | Organizacja oraz montaż połączeń. | Dobór przewodów i złączy do prądu; rozmieszczenie i mocowanie. |
| Arduino Uno i ESP-01S | Sprzęt z wcześniejszej wersji i nauki. | Ich użycie w docelowym samochodzie nie jest obecnie planowane. |
| WH-201 / EK1254 z LM393 | Opcjonalny czujnik przeszkody. | Sprawdzić konkretną płytkę; według dostępnego opisu daje sygnał wykrycia, a nie pomiar odległości. |

Pozycje z wcześniejszej listy zakupów nie muszą oznaczać, że każdy element już dotarł — stan i dokładne warianty sprawdzimy przed montażem.

## Ustalenia projektowe

- Głównym sterownikiem planujemy ESP32-S3; osobne ESP32-CAM odpowiada za obraz. Arduino Uno i ESP-01S pozostają dostępnymi narzędziami, ale nie są konieczne w nowej architekturze.
- Korzystamy z VS Code, PlatformIO i frameworka Arduino; kod aplikacji piszemy w C++. Istniejący kod przejrzymy przed wyborem elementów do ponownego użycia. Kompleksowy refaktoryzacyjny rewrite zostawiamy na później.
- Telefon ma służyć do sterowania i podglądu obrazu. Szczegóły interfejsu, topologii Wi-Fi i protokołu pozostają otwarte.
- Główne ESP ma dostawać od kamery informacje, które można pokazać na OLED: co najmniej dostępność modułu, stan Wi-Fi i ewentualnie adres IP oraz stan transmisji. Trzeba rozróżnić brak odpowiedzi kamery od zgłoszonego przez nią braku Wi-Fi.
- Stan baterii również chcemy pokazywać na OLED, docelowo bez oryginalnej płytki przycisków i wyświetlacza. Możemy rozważyć pomiar napięcia pakietu przez dzielnik i ADC ESP albo odczyt sygnałów oryginalnej ładowarki; sposób oraz dokładność wskazań są jeszcze do ustalenia.
- Komunikacja ESP ↔ ESP-CAM: dodano eksperymentalne ogłaszanie kamery po UDP w lokalnej podsieci, jej wykrywanie przez główne ESP i status na OLED. Strona sterowania odbiera MJPEG bezpośrednio z kamery. Integracja wymaga wgrania obu programów i testu na sprzęcie; funkcja odkrywania zależy od sieci i jest dostępne ręczne wpisanie IP.
- Planujemy zasilanie z nowego pakietu przez BMS i regulowaną przetwornicę. Serwa potrzebują odpowiednio wydajnej linii 5 V; małe odbiorniki 3,3 V mogą korzystać ze stabilizatora właściwej płytki tylko po sprawdzeniu zapasu prądu. Dzielnik rezystorowy nie jest zasilaczem dla ESP.
- Nie zakładamy pracy auta podczas ładowania. Połączenie ładowarki, BMS, pakietu i wyłącznika ustalimy po identyfikacji modułów.
- W docelowym aucie chcemy odłączyć oryginalną płytkę sterującą silnikiem, pozostawiając zgodny moduł ładowania. Główne ESP będzie sterować silnikiem po ustaleniu jego interfejsu; nie zakładamy, że GPIO może zasilać go bezpośrednio.
- OTA jest wygodne do rozwoju; obecny firmware testowy nie ma hasła OTA. Docelowe zabezpieczenie aktualizacji i sieci wybierzemy przed użyciem poza zaufaną siecią. Debugowanie z breakpointami obecnie działa przez USB-JTAG, nie przez OTA.

## Wstępna architektura

```text
                              telefon
                         sterowanie + obraz
                            /          \
                         Wi-Fi        Wi-Fi
                           /              \
                   główne ESP32-S3 ←→ ESP32-CAM
                    │   │   │   │        status kamery
                    │   │   │   └── OLED I²C
                    │   │   └────── serwo skrzyni
                    │   └────────── serwo skrętu
                    └────────────── interfejs sterowania silnikiem

pakiet → BMS → zasilanie silnika oraz przetwornica 5 V → ESP i serwa
   └── tor ładowania USB-C — szczegóły po sprawdzeniu zgodności modułów
```

To schemat funkcjonalny, nie schemat połączeń: nie określa pinów, bezpieczników ani sposobu podłączenia ładowarki. Docelowe masy, rozgałęzienia, wyłącznik i kolejność elementów ustalimy na podstawie konkretnych urządzeń.

## Roadmapa

### 1. Rozpoznanie i sterowanie silnikiem — zakończone na obecnym etapie

- [x] Ustalono, że `GN` silnika jest połączone z `P−` BMS; `M+` jest zasilaniem silnika, w teście około 7 V względem `GN`.
- [x] Zmierzono oryginalny sygnał `PWM`: około 7,9 kHz, okres około 126–128 µs. Przy wzroście prędkości wzrasta udział stanu LOW; przy włączonym urządzeniu i zatrzymanym silniku zmierzono około 5 V oraz stan HIGH w chwili odczytu.
- [x] Przy odłączonym przewodzie `PWM` od oryginalnej płytki, z zasilanym silnikiem, po stronie silnika było około 5 V; z rezystorem 100 kΩ do `GN` około 4,8 V. Silnik nie ruszył. Wskazuje to na podciąganie wejścia, lecz nie wyznacza jego dokładnych progów ani wszystkich stanów awaryjnych.
- [x] Linia oznaczona wcześniej `NC` zmienia kierunek obrotów: po odłączeniu około 5 V; przez 10 kΩ do GN około 2 V bez zmiany kierunku; przez 330 Ω około 0,07 V i kierunek przeciwny. Funkcjonalnie działa więc jako wejście kierunku w przeprowadzonym teście; docelowy interfejs tranzystorowy i zmianę kierunku po zatrzymaniu trzeba jeszcze przetestować.
- [ ] Przetestować sterowanie kierunkiem przez GPIO 12 i drugi 2N2222A: baza przez 10 kΩ, 100 kΩ baza–masa, kolektor przez 330 Ω do `NC`, emiter do `GN`. Zmiana kierunku wyłącza PWM i przełącza linię bez dodatkowego oczekiwania; fizyczne zatrzymanie wału nie jest weryfikowane.
- [x] Uruchomiono sterowanie z ESP32-S3 przez tranzystor 2N2222A bez oryginalnej płytki sterującej. W teście GPIO 7 przez 10 kΩ steruje bazą, 100 kΩ łączy bazę z masą, emiter jest na `GN`, a kolektor na `PWM` silnika. GPIO HIGH ściąga linię silnika w stan LOW; GPIO LOW wyłącza tranzystor. Kolejność nóżek należy sprawdzać dla konkretnego tranzystora.
- [x] Potwierdzono zmianę prędkości z ESP. Na stanowisku bez dodatkowego obciążenia prędkość przestała zauważalnie rosnąć powyżej około 60% udziału LOW; wpływ większego wypełnienia pod obciążeniem pozostaje nieznany. To poziomy sterowania PWM, nie biegi mechaniczne skrzyni.
- [x] Zbadano `FOUT`: zmienia się przy spowolnieniu silnika. Surowy licznik około 4 kHz obejmował także szybkie zbocza o odstępach zbliżonych do okresu PWM. Eksperymentalny licznik przerw ≥1000 µs dawał po ustabilizowaniu około 53/s przy 10%, 109/s przy 20%, 151/s przy 30% i spadał do zera po zatrzymaniu. Przelicznik na RPM, charakter sygnału i działanie progu przy większych prędkościach są nieznane.
- [x] Powstał program testowy `esp-test/esp-test/src/main.cpp`: sterowanie GPIO 7, pomiar FOUT na GPIO 6 przez dzielnik, OTA oraz stop po 10 s bez polecenia. To narzędzie testowe, nie gotowy sterownik samochodu.

**Dalsze prace na zmontowanym aucie:** kalibracja `FOUT` i całego zakresu PWM przy rzeczywistym obciążeniu, sprawdzenie poboru prądu, rozruchu i zachowania podczas resetu ESP, utraty sterowania oraz awarii. Dobór i weryfikacja docelowej ochrony wejścia `FOUT` pozostają otwarte: dzielnik użyty w teście nie zapewnia pełnej ochrony przed przepięciami. Dokładny model i typ silnika nie zostały potwierdzone.

**Pomysł na przyszłość — automatyczna redukcja:** po sprawdzeniu wiarygodności `FOUT` i zależności od obciążenia sterownik mógłby wykrywać utrzymujący się spadek prędkości względem zadanego napędu, zmniejszać napęd na czas zmiany biegu, przełączać serwo skrzyni i łagodnie przywracać moc. Potrzebne będą kalibracja progów, opóźnienie/histereza zapobiegające ciągłemu przełączaniu oraz bezpieczne zachowanie po utracie sygnału. To pomysł do testów, nie potwierdzona funkcja.

**Pomysł na przyszłość — pomiar obrotów koła:** dodać czujnik mierzący obroty koła lub wału wyjściowego skrzyni. Porównanie jego wskazań z obrotami silnika (jeśli uda się wiarygodnie skalibrować `FOUT`) może pomóc ocenić, czy skrzynia załączyła bieg, i oszacować przełożenie silnik–koło. Do ustalenia: rodzaj i montaż czujnika, liczba impulsów na obrót, kalibracja `FOUT`, charakterystyka poszczególnych biegów oraz warunki wykrywania — np. przy postoju, poślizgu lub toczeniu różnicowym sam stosunek obrotów może być niemiarodajny.

### 2. Zasilanie i ładowanie

Schemat ideowy: [docs/schemat-ideowy.svg](docs/schemat-ideowy.svg). Opis założeń i plan testów: [docs/schemat-wstepny.md](docs/schemat-wstepny.md).

- [x] Ustalić podstawowe elementy: pakiet 2S do 8,4 V, dwa serwa SG90, przetwornica XL4015 ustawiana na 5 V oraz płytka ładowania masażera.
- [x] Potwierdzić zgodność ładowarki z nowym pakietem według informacji użytkownika; ładowanie i praca działają z oryginalnym układem.
- [ ] Ustalić kolejność połączeń, wyłącznik i zabezpieczenia.
- [ ] Rozpoznać złącze między płytką ładowania a płytką przycisków: `GND`, `PWM1`, `PWM2`, `STAT`, `P_ON`, `P+`. Funkcji linii sygnałowych nie ustalono; sprawdzić także, czy ładowanie działa przy odłączonej płytce przycisków.
- [ ] Wybrać sposób wskazywania baterii na OLED. Rozważyć pomiar całego pakietu przez dzielnik i ADC, kalibrację oraz szacowanie poziomu z uwzględnieniem obciążenia; napięcie pakietu nie ujawnia osobno napięcia ogniw.
- [ ] Dobrać napięcie i wydajność przetwornicy, przewody oraz złącza do rzeczywistych obciążeń; uwzględnić rozruch/zablokowanie serw i silnika.
- [ ] Dobrać przetwornicę 5 V o gwarantowanym zakresie wejściowym obejmującym pełny zakres 2S pod obciążeniem; XL4015 jest poza specyfikacją poniżej 8 V według producenta.
- [ ] Ustalić zasilanie głównego ESP i ESP-CAM, w tym zachowanie przy jednoczesnym podłączeniu zewnętrznego 5 V i USB.
- [ ] Narysować schemat ideowy i przetestować napięcia bez obciążeń, a następnie etapami pod obciążeniem.

**Gotowe, gdy:** znamy i udokumentowaliśmy zasilanie oraz ładowanie, a ESP i serwa działają stabilnie w planowanych warunkach.

### 3. Stanowisko sterownika i serwa

Aktualna kalibracja mechanizmu (zadane pozycje w programie testowym):

- **Skręt:** domyślnie zakres 0–180°, pozycja środkowa 80°; granice można zmieniać na stronie w zakresie 0–180° z zachowaniem środka 80°. Poprzednio zmierzony zakres 10–120° warto ponownie sprawdzić przy kalibracji mechanizmu. Ustawienia strony są tymczasowe i nie zapisują się po jej odświeżeniu; GPIO 8.
- **Skrzynia:** 1. bieg 35°, luz 100°, 2. bieg 160°; sygnał serwa na GPIO 17. Ręczny suwak pozostaje dostępny w zakresie 0–180° do dalszej kalibracji.

Pozycje wymagają ponownego sprawdzenia po zmianie montażu mechanizmu. Wartości to ustawienia sygnału, nie pomiar rzeczywistego kąta ani potwierdzenie załączenia biegu.

- [ ] Przydzielić piny po sprawdzeniu ograniczeń konkretnych płytek i urządzeń.
- [ ] Uruchomić każde serwo osobno; ustalić bezpieczne zakresy ruchu i pozycje początkowe mechaniki.
- [ ] Uruchomić napęd i serwa razem, sprawdzając stabilność zasilania i sterowania.
- [ ] Ustalić zachowanie po uruchomieniu, resecie, zaniku łączności i utracie sygnału: napęd ma przejść do bezpiecznego zatrzymania.

### 4. Kamera i status na OLED

- [x] Uruchomić ESP32-CAM i podgląd obrazu na telefonie; przy routerze użytkownik zaobserwował płynny obraz, natomiast słabe Wi-Fi powodowało zacięcia. Dokładnego opóźnienia nie mierzono.
- [x] Podłączyć i przetestować OLED z głównym ESP (SDA GPIO 8, SCL GPIO 9).
- [ ] Wybrać i wdrożyć komunikację statusu między modułami. Ustalić format danych, okres aktualizacji i timeout.
- [ ] Wyświetlać co najmniej: stan połączenia głównego ESP, dostępność kamery, stan Wi-Fi kamery i ewentualnie stan strumienia/IP. Przy braku aktualizacji pokazywać informację o utracie kontaktu.
- [ ] Przetestować uruchamianie modułów w różnej kolejności, restart kamery oraz zanik i powrót sieci.

### 5. Sterowanie z telefonu

- [ ] Przejrzeć istniejącą aplikację i poprzedni prototyp sterowania; zdecydować, co wykorzystać.
- [ ] Wybrać sposób połączenia telefonu z autem i sposób równoczesnego oglądania obrazu.
- [ ] Wdrożyć sterowanie napędem, skrętem i skrzynią; dodać informację o połączeniu i opóźnieniu, jeśli potrzebna.
- [ ] Przetestować zatrzymanie napędu po rozłączeniu, zamknięciu aplikacji i zaniku poleceń.

### 6. Montaż, porządkowanie i testy pojazdu

- [ ] Rozplanować rozmieszczenie płytek, dostęp do ładowania i mocowanie elementów w konstrukcji LEGO.
- [ ] Wykonać płytkę połączeniową, opisać przewody i złącza, ograniczyć plątaninę kabli.
- [ ] Sprawdzić działanie napędu, serw, kamery i łączności razem, najpierw z uniesionymi kołami, następnie podczas jazdy.
- [ ] Skalibrować PWM i FOUT na aucie pod obciążeniem oraz ocenić możliwość automatycznej redukcji.
- [ ] Rozważyć czujnik obrotów koła / wału wyjściowego i kalibrację przełożeń oraz wykrywania załączonego biegu.
- [ ] Uporządkować kod i dokumentację na podstawie działającego rozwiązania.

## Katalogi w repozytorium

- `esp-test/esp-test/` — niezależny projekt PlatformIO do testów ESP32-S3, matrycy, joysticka, Wi-Fi i OTA. Przykłady poprzednich programów są w `examples/`.
- `esp-cam-test/` — osobny test ESP32-CAM, pojedynczych zdjęć, podglądu MJPEG oraz OTA. Środowisko `esp32cam` wgrywa przez USB; `esp32cam-ota` przez sieć. Pierwsze wgranie wersji OTA i nowego układu partycji wymaga USB. OTA bez hasła, do testu w zaufanej sieci.
- `arduino_controller/` i `CarController/` — wcześniejszy kod sterownika Arduino; jego przydatność do nowej wersji wymaga przeglądu.
- `flutter_web_rtc/` i `node_web_rtc/` — istniejące projekty związane z aplikacją i komunikacją. Ich obecność nie przesądza o docelowej technologii.

Hasła i dane dostępowe trzymamy poza wersjonowanymi plikami. Szczegółowe wyniki pomiarów, modele i podjęte później decyzje dopisujemy do tego dokumentu lub do osobnych notatek, gdy materiał się rozrośnie.
