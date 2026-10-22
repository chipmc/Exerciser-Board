#include <Arduino.h>

// Color-order blinky for the exerciser board: red, green, blue in turn.
const uint8_t LED_R = PIN_PC3;
const uint8_t LED_G = PIN_PC2;
const uint8_t LED_B = PIN_PC1;

void pulse(uint8_t pin) {
  digitalWrite(pin, HIGH);
  delay(20);
  digitalWrite(pin, LOW);
}

void setup() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
}

void loop() {
  pulse(LED_R); delay(480);
  pulse(LED_G); delay(480);
  pulse(LED_B); delay(480);
}
