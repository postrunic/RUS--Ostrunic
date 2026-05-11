# Analiza teorijskog trajanja baterije

## Pretpostavke

| Parametar | Vrijednost |
|---|---:|
| Kapacitet baterije | 2500 mAh |
| Aktivna struja, `I_active` | 80 mA |
| Sleep struja, `I_sleep` | 0.15 mA |
| Broj događaja dnevno | 10 |
| Aktivno vrijeme po događaju | 3 s |
| Ukupno aktivno vrijeme dnevno | 30 s |
| Ukupno sleep vrijeme dnevno | 86 370 s |
| Ukupno promatrano vrijeme | 86 400 s |

## Formula

```text
I_avg = (I_active × t_active + I_sleep × t_sleep) / ukupno vrijeme
```

## Izračun

```text
I_avg = (80 mA × 30 s + 0.15 mA × 86 370 s) / 86 400 s
I_avg = (2400 + 12 955.5) / 86 400
I_avg = 0.1777 mA
```

Teorijsko trajanje baterije:

```text
t = 2500 mAh / 0.1777 mA
t = 14 068.7 h
t = 586.2 dana
t = 1.61 godina
```

## Zaključak analize

Prema pojednostavljenom teorijskom modelu baterija od 2500 mAh trajala bi oko 586 dana, odnosno oko 1.6 godina.

Ovaj rezultat je samo procjena. Stvarno trajanje može biti kraće zbog potrošnje regulatora napona, USB-UART sklopa na razvojnoj pločici, kvalitete baterije, temperature, samopražnjenja i razlika između stvarnog hardvera i simulatora.

