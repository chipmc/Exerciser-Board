#include <Arduino.h>

// V1 exerciser: look like a PIR to the Generalized Core Counter (WO-002, WO-004).
// One 20-minute cycle, repeated forever: a cluster of pulses, then silence.
// The LED shows where the counter should be in its occupancy session.
//
// Expected values:
//   session   = (PULSES-1) x SPACING + DEBOUNCE = 9 x 30 + 60 = 330 s = 5.5 min
//   per hour  = 3 sessions = 16.5 min
//   per open day = whole cycles completed x 5.5 min
//   tolerance = about +/-1 s per session (the counter credits whole seconds)
const uint8_t  PIN_PULSE = PIN_PA6;      // IO1, J2 pin 5: active-high, idle LOW
const uint8_t  PIN_RED = PIN_PC3;        // LEDs: HIGH = on
const uint8_t  PIN_GREEN = PIN_PC2;
const uint8_t  PIN_BLUE = PIN_PC1;
const uint32_t STARTUP_QUIET_MS = 6000;  // LOW after setup() before the first cluster
const uint8_t  PULSES_PER_CLUSTER = 10;
const uint32_t PULSE_HIGH_MS = 2000;
const uint32_t PULSE_SPACING_MS = 30000; // pulse start to pulse start
const uint32_t CYCLE_MS = 1200000;       // cluster start to next cluster start (20 min)
// Must equal the counter's sensor.setting1. Used only for the LED session marker
// and the expected-value comment; does not affect pulse timing.
const uint32_t DEBOUNCE_MS = 60000;

const uint32_t LAST_RISE_MS = (PULSES_PER_CLUSTER - 1) * PULSE_SPACING_MS;
const uint32_t HEARTBEAT_START_MS =      // first spacing boundary after the session ends
    ((LAST_RISE_MS + DEBOUNCE_MS) / PULSE_SPACING_MS + 1) * PULSE_SPACING_MS;

uint32_t clusterStart;                   // absolute time (millis) of the current cluster

// Which LED channel is on at pos ms into the cycle (none, or exactly one).
uint8_t ledPin(uint32_t pos) {
  uint32_t k = pos / PULSE_SPACING_MS, r = pos % PULSE_SPACING_MS;
  if (k < PULSES_PER_CLUSTER - 1) {                 // pulses 1 to 9
    uint8_t c = (k == 0 && r < PULSE_HIGH_MS) ? PIN_GREEN : PIN_BLUE;
    if (r < PULSE_HIGH_MS) return c;
    if (r >= 5000 && r % 5000 < 50) return c;
    return 0;
  }
  uint32_t tl = pos - LAST_RISE_MS;                 // ms since the last rise
  if (tl < PULSE_HIGH_MS) return PIN_BLUE;
  if (tl < DEBOUNCE_MS) return (tl >= 5000 && tl % 5000 < 50) ? PIN_BLUE : 0;
  if (pos >= HEARTBEAT_START_MS && r < 20) return PIN_RED;
  return 0;
}

void setup() {
  pinMode(PIN_PULSE, OUTPUT);            // first: output and LOW before anything else
  digitalWrite(PIN_PULSE, LOW);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  clusterStart = millis() + STARTUP_QUIET_MS;
}

void loop() {
  uint32_t elapsed = millis() - clusterStart;
  bool high = false;
  uint8_t led = 0;
  if ((int32_t)elapsed < 0) {                       // quiet time: one 200 ms blue "armed" blink
    if (elapsed + STARTUP_QUIET_MS < 200) led = PIN_BLUE;
  } else if (elapsed >= CYCLE_MS) {                 // next cluster: absolute, no drift
    clusterStart += CYCLE_MS;
    return;
  } else {
    high = (elapsed / PULSE_SPACING_MS < PULSES_PER_CLUSTER) &&
           (elapsed % PULSE_SPACING_MS < PULSE_HIGH_MS);
    led = ledPin(elapsed);
  }
  digitalWrite(PIN_PULSE, high);
  digitalWrite(PIN_RED, led == PIN_RED);
  digitalWrite(PIN_GREEN, led == PIN_GREEN);
  digitalWrite(PIN_BLUE, led == PIN_BLUE);
}
