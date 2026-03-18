#include "modes.h"

//Valeur variables de base 
ModeParams params[3] = {
  { 10.0f, 0.5f },
  { 100.0f, 0.5f },
  { 100.0f, 0.5f }
};
//Mode de base
ModeId currentMode = MODE_15_1;

uint32_t t0_us = 0;
bool halfState = false;

static bool warn151 = false;
static bool warn157 = false;
static bool warn158 = false;
//Avertissement (sans boucle)
static float warnClampOnce(float f_cmd, float f_max, const char* msg, bool& latch) {
  if (f_cmd > f_max) {
    if (!latch) {
      Serial.println(msg);
      latch = true;
    }
    return f_max;
  } else {
    latch = false;
    return f_cmd;
  }
}

inline void gateWrite(int pin, bool commandOn) {
  bool level = ACTIVE_LOW ? !commandOn : commandOn;
  digitalWrite(pin, level ? HIGH : LOW);
}
//Retourne fréquence max si dépassé
float maxFreqForMode(ModeId m) {
  switch (m) {
    case MODE_15_1: return F_MAX_151;
    case MODE_15_7: return F_MAX_157;
    case MODE_15_8: return F_MAX_158;
    default:        return F_MAX_151;
  }
}

void allOff() {
  gateWrite(PIN_1A, false);
  gateWrite(PIN_1B, false);
  gateWrite(PIN_2A, false);
  gateWrite(PIN_2B, false);
  gateWrite(PIN_3A, false);
  gateWrite(PIN_3B, false);
}

void setLeg12(bool on_1A_2B, bool on_1B_2A) {
  if (DEADTIME_US > 0) {
    gateWrite(PIN_1A, false); gateWrite(PIN_1B, false);
    gateWrite(PIN_2A, false); gateWrite(PIN_2B, false);
    delayMicroseconds(DEADTIME_US);
  }

  gateWrite(PIN_1A, on_1A_2B);
  gateWrite(PIN_2B, on_1A_2B);

  gateWrite(PIN_1B, on_1B_2A);
  gateWrite(PIN_2A, on_1B_2A);
}

void stepMode15_1(uint32_t now_us) {
  float f_cmd = params[MODE_15_1].f_hz;
  f_cmd = warnClampOnce(f_cmd, F_MAX_151, "⚠ Mode 15.1 : fréquence limitée à 60 Hz", warn151);

  float f = constrain(max(F_MIN, f_cmd), F_MIN, F_MAX_151);
  uint32_t T_us  = (uint32_t)(1000000.0f / f);
  uint32_t halfT = T_us / 2;

  uint32_t dt = now_us - t0_us;
  if (dt >= halfT) { t0_us += halfT; halfState = !halfState; }

  if (!halfState) setLeg12(true, false);
  else            setLeg12(false, true);
}

void stepMode15_7(uint32_t now_us) {
  float f_cmd = params[MODE_15_7].f_hz;
  f_cmd = warnClampOnce(f_cmd, F_MAX_157, "⚠ Mode 15.7 : fréquence limitée à 2000 Hz", warn157);

  float f = constrain(max(F_MIN, f_cmd), F_MIN, F_MAX_157);
  float a = constrain(params[MODE_15_7].alpha, 0.0f, 1.0f);

  uint32_t T_us  = (uint32_t)(1000000.0f / f);
  uint32_t on_us = (uint32_t)(a * T_us);

  uint32_t dt = now_us - t0_us;
  if (dt >= T_us) { t0_us += T_us; dt = now_us - t0_us; }

  if (dt < on_us) setLeg12(true, false);
  else            setLeg12(false, true);
}

void stepMode15_8(uint32_t now_us) {
  float f_cmd = params[MODE_15_8].f_hz;
  f_cmd = warnClampOnce(f_cmd, F_MAX_158, "⚠ Mode 15.8 : fréquence limitée à 2000 Hz", warn158);

  float f = constrain(max(F_MIN, f_cmd), F_MIN, F_MAX_158);
  float a = constrain(params[MODE_15_8].alpha, 0.0f, 1.0f);

  uint32_t T_us  = (uint32_t)(1000000.0f / f);
  uint32_t on_us = (uint32_t)(a * T_us);

  uint32_t dt = now_us - t0_us;
  if (dt >= T_us) { t0_us += T_us; dt = now_us - t0_us; }

  gateWrite(PIN_1B, false);
  gateWrite(PIN_2A, false);
  gateWrite(PIN_2B, false);
  gateWrite(PIN_3A, false);
  gateWrite(PIN_3B, false);

  gateWrite(PIN_1A, (dt < on_us));
}
