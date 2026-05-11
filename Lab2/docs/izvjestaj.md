# Izvještaj - Lab2

## Tema

Laboratorijska vježba prikazuje upravljanje potrošnjom energije mikrokontrolera pomoću sleep režima. Implementirana je varijanta A: pametni poštanski sandučić koji se budi samo kada se dogodi vanjski događaj, odnosno pritisak tipkala.

## Sažetak

| Stavka | Odgovor |
|---|---|
| Platforma | ESP32 DevKit V1 |
| Varijanta | A - Pametni poštanski sandučić |
| Sleep mode | ESP32 Deep Sleep; u Wokwiju simulirano sleep stanje |
| Buđenje | Vanjski GPIO događaj na GPIO33 |
| Čuvanje stanja | `RTC_DATA_ATTR eventCounter` |
| Debouncing | Vremensko ignoriranje ponovljenih događaja + čekanje otpuštanja tipkala |
| Wokwi link | https://wokwi.com/projects/463275793871923201 |

## Opis implementacije

Sustav koristi ESP32, tipkalo `POSTA` na GPIO33 i zelenu LED na GPIO25.

Rad sustava:

1. Nakon pokretanja program ulazi u sleep stanje.
2. Tipkalo `POSTA` predstavlja događaj ubacivanja pošte.
3. Pritiskom tipkala aktivira se obrada događaja.
4. Debounce logika osigurava da se jedan fizički pritisak broji kao jedan logički događaj.
5. Brojač događaja `eventCounter` se povećava za 1.
6. LED indikator treperi oko 3 sekunde kao aktivna faza rada.
7. Sustav čeka otpuštanje tipkala.
8. Nakon toga se vraća u sleep stanje.

Aktivna faza koristi `millis()` za vremensko upravljanje LED indikatorom, tako da se ne koristi samo `delay()` kao jedini mehanizam upravljanja vremenom.

## Sleep i wake-up

Za stvarni ESP32 hardver u kodu je predviđen Deep Sleep način rada:

```cpp
esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1);
esp_deep_sleep_start();
```

GPIO33 je RTC-capable pin, zbog čega se može koristiti kao EXT0 wake-up iz Deep Sleep režima. Tipkalo je spojeno tako da pritiskom podigne GPIO33 na HIGH.

Za Wokwi web simulator koristi se `WOKWI_DEMO_MODE = 1`. Taj način rada simulira sleep stanje jer Wokwi web ne simulira uvijek pouzdano pravo buđenje nakon `esp_deep_sleep_start()`. Time se u simulatoru jasno vidi logika sleep/wake ciklusa, što je naglasak zadatka.

## Debouncing

Mehanički prekidači mogu pri pritisku proizvesti više brzih promjena stanja. To može dovesti do toga da se jedan fizički pritisak pogrešno protumači kao više događaja.

Odabrano rješenje:

- nakon prihvaćenog događaja kratko se ignoriraju ponovljeni događaji,
- sustav čeka otpuštanje tipkala prije povratka u sleep,
- isti fizički pritisak ne može pokrenuti više obrada.

Ovo smanjuje nepotrebna buđenja i smanjuje vrijeme provedeno u aktivnom režimu, što pozitivno utječe na energetsku učinkovitost.

## Usporedba ESP32 sleep režima

| Režim | Logičko ponašanje | Vrijeme buđenja | Primjena |
|---|---|---|---|
| Light Sleep | CPU pauzira, RAM ostaje očuvan | Brzo | Kratke pauze |
| Deep Sleep | CPU se gasi, RTC domena ostaje dostupna | Sporije | Rijetki događaji, baterijski uređaji |
| Hibernation | Gasi se još više dijelova sustava | Najsporije | Najniža potrošnja uz minimalno očuvanje stanja |

Za pametni poštanski sandučić najprikladniji je Deep Sleep jer uređaj većinu vremena ne radi ništa i treba se probuditi samo na događaj.

## Ograničenja simulacije

Wokwi omogućuje testiranje logike programa, tipkala i LED indikatora, ali ne omogućuje stvarnu procjenu potrošnje energije. Također, podrška za napredne sleep/wake scenarije nije jednaka stvarnom hardveru.

Za preciznu energetsku analizu potreban je stvarni ESP32, baterija ili laboratorijsko napajanje i mjerenje struje.

## Zaključak

Implementacija prikazuje logiku upravljanja energijom, ali ne omogućuje stvarnu procjenu potrošnje energije.
