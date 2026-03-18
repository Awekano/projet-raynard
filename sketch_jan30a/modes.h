#pragma once
#include <Arduino.h>
#include "config.h"
//Liste des modes
enum ModeId : uint8_t {
  MODE_15_1 = 0,
  MODE_15_7 = 1,
  MODE_15_8 = 2
};
//Paramètres des modes
struct ModeParams {
  float f_hz;
  float alpha; // 0..1
};

extern ModeParams params[3];
extern ModeId currentMode;

extern uint32_t t0_us;
extern bool halfState;

float maxFreqForMode(ModeId m);

void allOff();
void setLeg12(bool on_1A_2B, bool on_1B_2A);

void stepMode15_1(uint32_t now_us);
void stepMode15_7(uint32_t now_us);
void stepMode15_8(uint32_t now_us);
