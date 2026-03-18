#include "config.h"
#include "modes.h"

// fonctions console.cpp
String readLineNonBlocking();
void handleCommand(const String& line);
void printHelp();
void printStatus();

void setup() {
  delay(500);
  Serial.begin(115200);
  delay(200);

  pinMode(PIN_1A, OUTPUT);
  pinMode(PIN_1B, OUTPUT);
  pinMode(PIN_2A, OUTPUT);
  pinMode(PIN_2B, OUTPUT);
  pinMode(PIN_3A, OUTPUT);
  pinMode(PIN_3B, OUTPUT);

  allOff();
  t0_us = micros();

  Serial.println("ESP32 - Modes 15.1 / 15.7 / 15.8 (console serie)");
  printHelp();
  printStatus();
}

void loop() {
  String line = readLineNonBlocking();
  if (line.length()) handleCommand(line);

  uint32_t now_us = micros();
  switch (currentMode) {
    case MODE_15_1: stepMode15_1(now_us); break;
    case MODE_15_7: stepMode15_7(now_us); break;
    case MODE_15_8: stepMode15_8(now_us); break;
  }
}
