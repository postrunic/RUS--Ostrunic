/**
 * @file main.cpp
 * @brief Lab2 - Pametni postanski sanducic, event-driven energy management.
 *
 */

#include <Arduino.h>
#include <driver/rtc_io.h>
#include <esp_sleep.h>

#ifndef WOKWI_DEMO_MODE
#define WOKWI_DEMO_MODE 1
#endif

static constexpr gpio_num_t WAKE_GPIO = GPIO_NUM_33;
static constexpr uint8_t WAKE_PIN = 33;
static constexpr uint8_t LED_PIN = 25;

static constexpr uint32_t SERIAL_BAUD = 115200;
static constexpr uint32_t DEBOUNCE_IGNORE_MS = 250;
static constexpr uint32_t ACTIVE_PHASE_MS = 3000;
static constexpr uint32_t LED_TOGGLE_MS = 250;
static constexpr uint32_t RELEASE_STABLE_MS = 80;

RTC_DATA_ATTR uint32_t eventCounter = 0;
uint32_t lastAcceptedEventMs = 0;
uint32_t lastSleepStatusMs = 0;
bool lowPowerState = false;

void configurePins() {
  rtc_gpio_deinit(WAKE_GPIO);
  pinMode(WAKE_PIN, INPUT_PULLDOWN);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

bool isEventPinActive() {
  return digitalRead(WAKE_PIN) == HIGH;
}

bool acceptDebouncedEvent() {
  const uint32_t now = millis();

  if (!isEventPinActive()) {
    return false;
  }

  if (lastAcceptedEventMs != 0 && now - lastAcceptedEventMs < DEBOUNCE_IGNORE_MS) {
    return false;
  }

  lastAcceptedEventMs = now;
  return true;
}

void runActivePhase() {
  Serial.println("[OBRADA] Evidentiram dogadaj pametnog postanskog sanducica.");
  Serial.print("[OBRADA] Ukupan broj prihvacenih dogadaja: ");
  Serial.println(eventCounter);
  Serial.println("[OBRADA] LED indikator treperi oko 3 sekunde.");

  const uint32_t phaseStart = millis();
  uint32_t lastToggle = phaseStart;
  bool ledState = false;

  while (millis() - phaseStart < ACTIVE_PHASE_MS) {
    const uint32_t now = millis();

    if (now - lastToggle >= LED_TOGGLE_MS) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);
      lastToggle = now;
    }

    delay(1);
  }

  digitalWrite(LED_PIN, LOW);
  Serial.println("[OBRADA] Kratka obrada je zavrsena.");
}

void waitForButtonRelease() {
  if (!isEventPinActive()) {
    return;
  }

  Serial.println("[DEBOUNCE] Cekam otpustanje tipkala prije povratka u sleep.");

  while (isEventPinActive()) {
    delay(20);
  }

  const uint32_t releaseStart = millis();
  while (millis() - releaseStart < RELEASE_STABLE_MS) {
    delay(5);
  }

  Serial.println("[DEBOUNCE] Tipkalo otpusteno, signal je stabilan.");
}

void enterLowPowerState() {
  digitalWrite(LED_PIN, LOW);

#if WOKWI_DEMO_MODE
  lowPowerState = true;
  Serial.println("[SLEEP] Wokwi: simuliram ESP32 Deep Sleep stanje.");
  Serial.println("[SLEEP] Sljedece budenje: pritisak tipkala POSTA na GPIO33.");
  Serial.flush();
  lastSleepStatusMs = millis();
#else
  Serial.println("[SLEEP] Gasim indikator i konfiguriram EXT0 wake-up.");
  pinMode(WAKE_PIN, INPUT_PULLDOWN);
  rtc_gpio_pullup_dis(WAKE_GPIO);
  rtc_gpio_pulldown_en(WAKE_GPIO);
  esp_sleep_enable_ext0_wakeup(WAKE_GPIO, 1);

  Serial.println("[SLEEP] Ulazim u ESP32 Deep Sleep.");
  Serial.println("[SLEEP] Sljedece budenje: pritisak tipkala na GPIO33.");
  Serial.flush();

  esp_deep_sleep_start();
#endif
}

void handleWakeEvent() {
  lowPowerState = false;

  Serial.println();
  Serial.println("[WAKE] Budenje vanjskim dogadajem na GPIO33.");

  if (!acceptDebouncedEvent()) {
    Serial.println("[DEBOUNCE] Dogadaj je odbacen kao nestabilan ili prebrzo ponovljen.");
    enterLowPowerState();
    return;
  }

  eventCounter++;
  Serial.println("[DEBOUNCE] Dogadaj je prihvacen kao jedan logicki dogadaj.");

  runActivePhase();
  waitForButtonRelease();
  enterLowPowerState();
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(500);

  configurePins();

  Serial.println();
  Serial.println("=== RUS Lab2 - Pametni postanski sanducic ===");

#if WOKWI_DEMO_MODE
  Serial.println("[BOOT] Wokwi demonstracija pokrenuta.");
  Serial.println("[BOOT] Tipkalo POSTA simulira ubacivanje poste.");
  enterLowPowerState();
#else
  const esp_sleep_wakeup_cause_t wakeReason = esp_sleep_get_wakeup_cause();

  if (wakeReason == ESP_SLEEP_WAKEUP_EXT0) {
    handleWakeEvent();
  } else {
    Serial.println("[BOOT] Prvo pokretanje/reset, dogadaj se ne broji.");
    enterLowPowerState();
  }
#endif
}

void loop() {
#if WOKWI_DEMO_MODE
  if (lowPowerState && isEventPinActive()) {
    handleWakeEvent();
  }

  if (lowPowerState && millis() - lastSleepStatusMs >= 2500) {
    Serial.println("[SLEEP] Cekam dogadaj na tipkalu POSTA...");
    Serial.flush();
    lastSleepStatusMs = millis();
  }

  delay(10);
#endif
}
