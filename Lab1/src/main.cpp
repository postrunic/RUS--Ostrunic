/**
 * @file main.cpp
 * @brief Sustav za nadzor prostorije s višestrukim prekidima na ESP32
 *
 * Ovaj program demonstrira rad s višestrukim prekidima i njihovim prioritetima.
 * Koriste se 2 tipkala, HC-SR04 ultrazvučni senzor i Timer za generiranje prekida.
 *
 * Prioriteti prekida (od najvišeg prema najnižem):
 * 1. Timer    - najviši prioritet, trepće bijela LED svake 2 sekunde
 * 2. Tipkalo1 - visoki prioritet, aktivira alarm (crvena LED)
 * 3. Tipkalo2 - srednji prioritet, resetira alarm (zelena LED)
 * 4. Senzor   - niski prioritet, upozorenje na blizinu (žuta LED)
 *
 * @author Patrik Ostrunić
 */

#include <Arduino.h>

// ─── Definicija pinova ───────────────────────────────────────────────────────
#define PIN_BTN_ALARM   18   ///< Tipkalo 1 - aktivacija alarma (visoki prioritet)
#define PIN_BTN_RESET   19   ///< Tipkalo 2 - reset alarma (srednji prioritet)
#define PIN_LED_ALARM   25   ///< Crvena LED - indikator alarma
#define PIN_LED_RESET   26   ///< Zelena LED - potvrda reseta
#define PIN_LED_SENSOR  27   ///< Žuta LED - upozorenje senzora
#define PIN_LED_TIMER   14   ///< Bijela LED - indikator timera
#define PIN_TRIG        32   ///< HC-SR04 Trigger pin
#define PIN_ECHO        33   ///< HC-SR04 Echo pin

// ─── Konstante ───────────────────────────────────────────────────────────────
#define DEBOUNCE_MS     200  ///< Debounce vrijeme za tipkala (ms)
#define ALARM_DISTANCE  20   ///< Prag udaljenosti za aktivaciju upozorenja (cm)
#define TIMER_INTERVAL  2000 ///< Interval Timer prekida (ms)

// ─── Zastavice prekida (volatile jer ih mijenjaju ISR funkcije) ──────────────
volatile bool flagAlarm  = false;  ///< Zastavica za alarm tipkalo
volatile bool flagReset  = false;  ///< Zastavica za reset tipkalo
volatile bool flagTimer  = false;  ///< Zastavica za timer prekid

// ─── Vremenske varijable za debounce ─────────────────────────────────────────
volatile unsigned long lastAlarmTime = 0;
volatile unsigned long lastResetTime = 0;

// ─── Stanje sustava ──────────────────────────────────────────────────────────
bool alarmActive = false;  ///< Je li alarm trenutno aktivan

// ─── Timer objekt ────────────────────────────────────────────────────────────
hw_timer_t *timer = NULL;

// ─── ISR funkcije ────────────────────────────────────────────────────────────

/**
 * @brief ISR za Timer prekid (najviši prioritet)
 * Postavlja zastavicu koja se obrađuje u glavnoj petlji.
 */
void IRAM_ATTR onTimer() {
  flagTimer = true;
}

/**
 * @brief ISR za Tipkalo 1 - Alarm (visoki prioritet)
 * Provjerava debounce i postavlja zastavicu alarma.
 */
void IRAM_ATTR ISR_Alarm() {
  unsigned long now = millis();
  if (now - lastAlarmTime > DEBOUNCE_MS) {
    lastAlarmTime = now;
    flagAlarm = true;
  }
}

/**
 * @brief ISR za Tipkalo 2 - Reset (srednji prioritet)
 * Provjerava debounce i postavlja zastavicu reseta.
 */
void IRAM_ATTR ISR_Reset() {
  unsigned long now = millis();
  if (now - lastResetTime > DEBOUNCE_MS) {
    lastResetTime = now;
    flagReset = true;
  }
}

// ─── Pomoćne funkcije ────────────────────────────────────────────────────────

/**
 * @brief Mjeri udaljenost pomoću HC-SR04 senzora
 * @return Udaljenost u centimetrima, ili -1 ako mjerenje nije uspjelo
 */
float measureDistance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duration = pulseIn(PIN_ECHO, HIGH, 30000); // timeout 30ms
  if (duration == 0) return -1;
  return (duration / 2.0) * 0.0343;
}

/**
 * @brief Obrada Timer prekida - trepće bijela LED
 * Ima najviši prioritet, izvršava se uvijek.
 */
void handleTimer() {
  flagTimer = false;
  digitalWrite(PIN_LED_TIMER, HIGH);
  delay(100);
  digitalWrite(PIN_LED_TIMER, LOW);
  Serial.println("[TIMER] Prekid timera - najviši prioritet");
}

/**
 * @brief Obrada Alarm prekida - pali crvenu LED
 * Visoki prioritet, blokira obradu nižih prekida.
 */
void handleAlarm() {
  flagAlarm = false;
  alarmActive = true;
  digitalWrite(PIN_LED_ALARM, HIGH);
  digitalWrite(PIN_LED_SENSOR, LOW); // ugasi senzor upozorenje
  Serial.println("[ALARM] Alarm aktiviran - visoki prioritet!");
}

/**
 * @brief Obrada Reset prekida - gasi alarm, pali zelenu LED
 * Srednji prioritet, radi samo ako je alarm aktivan.
 */
void handleReset() {
  flagReset = false;
  if (alarmActive) {
    alarmActive = false;
    digitalWrite(PIN_LED_ALARM, LOW);
    digitalWrite(PIN_LED_RESET, HIGH);
    delay(300);
    digitalWrite(PIN_LED_RESET, LOW);
    Serial.println("[RESET] Alarm resetiran - srednji prioritet");
  }
}

/**
 * @brief Obrada senzora - pali žutu LED ako je objekt blizu
 * Niski prioritet, izvršava se samo ako nema aktivnog alarma.
 */
void handleSensor() {
  float distance = measureDistance();
  if (distance > 0 && distance < ALARM_DISTANCE) {
    digitalWrite(PIN_LED_SENSOR, HIGH);
    Serial.print("[SENZOR] Objekt detektiran na ");
    Serial.print(distance);
    Serial.println(" cm - niski prioritet");
  } else {
    digitalWrite(PIN_LED_SENSOR, LOW);
  }
}

// ─── Setup i Loop ─────────────────────────────────────────────────────────────

/**
 * @brief Inicijalizacija sustava
 */
void setup() {
  Serial.begin(115200);

  // Postavljanje pinova
  pinMode(PIN_BTN_ALARM, INPUT_PULLUP);
  pinMode(PIN_BTN_RESET, INPUT_PULLUP);
  pinMode(PIN_LED_ALARM,  OUTPUT);
  pinMode(PIN_LED_RESET,  OUTPUT);
  pinMode(PIN_LED_SENSOR, OUTPUT);
  pinMode(PIN_LED_TIMER,  OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  // Spajanje ISR funkcija na pinove tipkala
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_ALARM), ISR_Alarm, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_RESET), ISR_Reset, FALLING);

  // Konfiguracija hardware timera (Timer0, prescaler 80 → 1 tick = 1µs)
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, TIMER_INTERVAL * 1000, true); // µs
  timerAlarmEnable(timer);

  Serial.println("=== Sustav za nadzor prostorije pokrenut ===");
  Serial.println("Prioriteti: Timer > Tipkalo1(Alarm) > Tipkalo2(Reset) > Senzor");
}

/**
 * @brief Glavna petlja - obrađuje zastavice prema prioritetu
 *
 * Redoslijed obrade:
 * 1. Timer  (uvijek ima prednost)
 * 2. Alarm  (visoki prioritet)
 * 3. Reset  (srednji prioritet)
 * 4. Senzor (niski prioritet, samo ako nema alarma)
 */
void loop() {
  // 1. NAJVIŠI PRIORITET - Timer
  if (flagTimer) {
    handleTimer();
  }

  // 2. VISOKI PRIORITET - Alarm tipkalo
  if (flagAlarm) {
    handleAlarm();
  }

  // 3. SREDNJI PRIORITET - Reset tipkalo
  if (flagReset) {
    handleReset();
  }

  // 4. NISKI PRIORITET - Senzor (samo ako alarm nije aktivan)
  if (!alarmActive) {
    handleSensor();
  }
}