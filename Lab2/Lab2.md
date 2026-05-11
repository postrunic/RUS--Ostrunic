# RUS Lab2 - Pametni poštanski sandučić

## Opis rješenja

Implementirana je 1. varijanta laboratorijske vježbe: pametni poštanski sandučić, odnosno događajno vođeni sustav (*event-driven*).

Uređaj je većinu vremena u sleep stanju. Kada se pritisne tipkalo `POSTA`, sustav se budi, evidentira jedan događaj, kratko odradi obradu paljenjem/treperenjem LED indikatora i vraća se u sleep.

Wokwi link: https://wokwi.com/projects/463275793871923201

## Platforma

| Stavka | Vrijednost |
|---|---|
| Mikrokontroler | ESP32 DevKit V1 |
| Varijanta | A - Pametni poštanski sandučić |
| Sleep mode | ESP32 Deep Sleep, u Wokwiju simuliran kao sleep stanje zbog ograničenja simulatora |
| Buđenje | Vanjski GPIO događaj na GPIO33 |
| Obrada | Zelena LED na GPIO25 treperi oko 3 sekunde |
| Debouncing | Vremensko ignoriranje ponovljenih događaja i čekanje otpuštanja tipkala |
| Čuvanje stanja | `RTC_DATA_ATTR eventCounter` u kodu |

## Važna napomena za Wokwi

Datoteka `wokwi/main.cpp` i `src/main.cpp` su usklađene. Zadani način rada je:

```cpp
#define WOKWI_DEMO_MODE 1
```

Taj način rada je namijenjen Wokwi web simulatoru. Sleep se prikazuje kao stanje programa jer Wokwi web simulator ne simulira pouzdano stvarno buđenje nakon `esp_deep_sleep_start()`. Logika koju zadatak traži ostaje ista: sleep, vanjski događaj, debounce, obrada i povratak u sleep.

U istom kodu postoji i stvarni ESP32 Deep Sleep način rada. Za stvarni hardver potrebno je postaviti:

```cpp
#define WOKWI_DEMO_MODE 0
```

Tada se koristi `esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1)` i `esp_deep_sleep_start()`.

## Wokwi test

1. U Wokwiju odabrati ESP32 Arduino projekt.
2. U `sketch.ino` zalijepiti sadržaj iz `wokwi/main.cpp`.
3. U `diagram.json` zalijepiti sadržaj iz `wokwi/diagram.json`.
4. Pokrenuti simulaciju.
5. Pritisnuti tipkalo `POSTA`.
6. Zelena LED `OBRADA` treba treperiti oko 3 sekunde.
7. Ako se tipkalo drži pritisnuto, događaj se ne smije brojati više puta.
8. Nakon otpuštanja i ponovnog pritiska LED ponovno treperi.

Serial Monitor je konfiguriran u `wokwi/diagram.json` s `"display": "always"` i spojen je na ESP32 `TX0/RX0` pinove. Zbog toga se u Wokwiju treba otvoriti odmah nakon pokretanja simulacije i prikazivati `Serial.println()` ispis iz programa. U Wokwi demo modu program svake 2.5 sekunde ispiše da čeka događaj kako bi Serial Monitor sigurno prikazao output.

## Struktura projekta

```text
Lab2/
├── Lab2.md
├── platformio.ini
├── src/
│   └── main.cpp
├── docs/
│   ├── analiza_baterije.md
│   ├── dijagram_stanja.md
│   └── izvjestaj.md
├── wokwi/
│   ├── diagram.json
│   ├── main.cpp
│   └── wokwi.toml
└── rezultat/
    └── serial_output.txt
```
