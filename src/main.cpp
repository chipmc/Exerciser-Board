#include <Arduino.h>

// V1 exerciser: look like a PIR to the Generalized Core Counter (WO-002).
// One 20-minute cycle, repeated forever: a cluster of pulses, then silence.
const uint8_t  PIN_PULSE = PIN_PA6;      // IO1, J2 pin 5: active-high, idle LOW
const uint8_t  PIN_LED = PIN_PC3;        // red LED mirrors each pulse
const uint32_t QUIET_MS = 6000;          // LOW after setup() before the first cluster
const uint8_t  PULSES_PER_CLUSTER = 10;
const uint32_t PULSE_HIGH_MS = 200;
const uint32_t PULSE_SPACING_MS = 30000; // pulse start to pulse start
const uint32_t CYCLE_MS = 1200000;       // cluster start to next cluster start (20 min)

uint32_t clusterStart;                   // absolute time (millis) of the current cluster

void setup() {
  pinMode(PIN_PULSE, OUTPUT);            // first: output and LOW before anything else
  digitalWrite(PIN_PULSE, LOW);
  pinMode(PIN_LED, OUTPUT);
  clusterStart = millis() + QUIET_MS;
}

void loop() {
  uint32_t elapsed = millis() - clusterStart;
  bool high = false;
  if ((int32_t)elapsed >= 0) {                      // false during the quiet time
    if (elapsed >= CYCLE_MS) {                      // next cluster: absolute, no drift
      clusterStart += CYCLE_MS;
      return;
    }
    high = (elapsed / PULSE_SPACING_MS < PULSES_PER_CLUSTER) &&
           (elapsed % PULSE_SPACING_MS < PULSE_HIGH_MS);
  }
  digitalWrite(PIN_PULSE, high);
  digitalWrite(PIN_LED, high);
}
