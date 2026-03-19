#pragma once
#include <Arduino.h>
#include "config.h"

enum ModeId : uint8_t {
  MODE_15_1 = 0,
  MODE_15_2,
  MODE_15_3,
  MODE_15_4,
  MODE_15_5,
  MODE_15_6,
  MODE_15_7,
  MODE_15_8,
  MODE_COUNT
};

struct ModeParams {
  float f_hz;          // fréquence fondamentale ou hacheur selon le mode
  float alpha;         // 0..1 pour hacheurs
  float angle_deg;     // 0..360 pour 15.2
  float carrier_hz;    // PWM/Mli
  float amplitude_pct; // 0..100
};

extern ModeParams params[MODE_COUNT];
extern ModeId currentMode;
extern uint32_t t0_us;

const char* modeName(ModeId m);
float maxFreqForMode(ModeId m);

void allOff();
void setLeg1(bool highSideOn);
void setLeg2(bool highSideOn);
void setLeg3(bool highSideOn);
void setLeg12(bool on_1A_2B, bool on_1B_2A);

void stepMode15_1(uint32_t now_us);
void stepMode15_2(uint32_t now_us);
void stepMode15_3(uint32_t now_us);
void stepMode15_4(uint32_t now_us);
void stepMode15_5(uint32_t now_us);
void stepMode15_6(uint32_t now_us);
void stepMode15_7(uint32_t now_us);
void stepMode15_8(uint32_t now_us);
