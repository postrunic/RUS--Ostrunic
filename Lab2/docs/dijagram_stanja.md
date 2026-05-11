# Dijagram stanja

```mermaid
stateDiagram-v2
    [*] --> Pokretanje
    Pokretanje --> Sleep

    Sleep --> Budenje: pritisak tipkala POSTA na GPIO33
    Budenje --> Debounce

    Debounce --> DogadjajOdbacen: prebrzo ponavljanje ili nestabilan signal
    Debounce --> ObradaDogadjaja: prihvacen jedan logicki dogadjaj

    ObradaDogadjaja --> CekanjeOtpustanja: LED treperi oko 3 s
    DogadjajOdbacen --> Sleep

    CekanjeOtpustanja --> Sleep: tipkalo otpusteno
```

## Objašnjenje

Sustav nakon pokretanja ulazi u sleep stanje. Pritisak tipkala `POSTA` predstavlja vanjski događaj koji budi sustav. Nakon debounce provjere događaj se prihvaća kao jedan logički događaj, LED indikator treperi oko 3 sekunde, a sustav zatim čeka otpuštanje tipkala i vraća se u sleep.
