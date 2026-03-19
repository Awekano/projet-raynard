#include "modes.h"
#include <math.h>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

ModeParams params[MODE_COUNT] = {
  {10.0f, 0.5f,   0.0f, 1000.0f,  80.0f}, // 15.1
  {10.0f, 0.5f,  90.0f, 1000.0f,  80.0f}, // 15.2
  {10.0f, 0.5f,   0.0f, 1000.0f,  80.0f}, // 15.3
  {10.0f, 0.5f,   0.0f, 1000.0f,  80.0f}, // 15.4
  {10.0f, 0.5f,   0.0f, 1000.0f,  80.0f}, // 15.5
  {10.0f, 0.5f,   0.0f, 1000.0f,  80.0f}, // 15.6
  {100.0f,0.5f,   0.0f, 1000.0f,  80.0f}, // 15.7
  {100.0f,0.5f,   0.0f, 1000.0f,  80.0f}  // 15.8
};

ModeId currentMode = MODE_15_1;
uint32_t t0_us = 0;

static bool warn151 = false;
static bool warn152 = false;
static bool warn153f = false;
static bool warn153c = false;
static bool warn154f = false;
static bool warn154c = false;
static bool warn155 = false;
static bool warn156f = false;
static bool warn156c = false;
static bool warn157 = false;
static bool warn158 = false;

static float warnClampOnce(float value, float vmin, float vmax, const char* lowMsg, const char* highMsg, bool& latch) {
  float out = value;
  if (value < vmin) {
    if (!latch && lowMsg) Serial.println(lowMsg);
    latch = true;
    out = vmin;
  } else if (value > vmax) {
    if (!latch && highMsg) Serial.println(highMsg);
    latch = true;
    out = vmax;
  } else {
    latch = false;
  }
  return out;
}

static inline float clamp01(float x) {
  return constrain(x, 0.0f, 1.0f);
}

static inline float nowSeconds(uint32_t now_us) {
  return now_us * 1e-6f;
}

static inline float triangleNormalized(float t_sec, float freq_hz) {
  float x = t_sec * freq_hz;
  float frac = x - floorf(x);
  return 4.0f * fabsf(frac - 0.5f) - 1.0f; // [-1 ; +1]
}

static inline float sineNormalized(float t_sec, float freq_hz, float phase_rad, float amp_pct) {
  float amp = constrain(amp_pct, AMP_MIN_PCT, AMP_MAX_PCT) / 100.0f;
  return amp * sinf(2.0f * PI * freq_hz * t_sec + phase_rad);
}

static inline void gateWrite(int pin, bool commandOn) {
  bool level = ACTIVE_LOW ? !commandOn : commandOn;
  digitalWrite(pin, level ? HIGH : LOW);
}

static inline void applyHalfBridge(int pinHigh, int pinLow, bool highSideOn) {
  if (DEADTIME_US > 0) {
    gateWrite(pinHigh, false);
    gateWrite(pinLow, false);
    delayMicroseconds(DEADTIME_US);
  }
  gateWrite(pinHigh, highSideOn);
  gateWrite(pinLow, !highSideOn);
}

float maxFreqForMode(ModeId m) {
  switch (m) {
    case MODE_15_1: return F_MAX_151;
    case MODE_15_2: return F_MAX_152;
    case MODE_15_3: return F_MOD_MAX;
    case MODE_15_4: return F_MOD_MAX;
    case MODE_15_5: return F_MAX_155;
    case MODE_15_6: return F_MOD_MAX;
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

void setLeg1(bool highSideOn) { applyHalfBridge(PIN_1A, PIN_1B, highSideOn); }
void setLeg2(bool highSideOn) { applyHalfBridge(PIN_2A, PIN_2B, highSideOn); }
void setLeg3(bool highSideOn) { applyHalfBridge(PIN_3A, PIN_3B, highSideOn); }

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

  gateWrite(PIN_3A, false);
  gateWrite(PIN_3B, false);
}

void stepMode15_1(uint32_t now_us) {
  float f = params[MODE_15_1].f_hz;
  f = warnClampOnce(f, F_MIN, F_MAX_151,
                    "⚠ Mode 15.1 : fréquence minimale atteinte",
                    "⚠ Mode 15.1 : fréquence limitée à 60 Hz",
                    warn151);

  float t = nowSeconds(now_us - t0_us);
  float s = sinf(2.0f * PI * f * t);
  setLeg12(s >= 0.0f, s < 0.0f);
}

void stepMode15_2(uint32_t now_us) {
  float f = params[MODE_15_2].f_hz;
  f = warnClampOnce(f, F_MIN, F_MAX_152,
                    "⚠ Mode 15.2 : fréquence minimale atteinte",
                    "⚠ Mode 15.2 : fréquence limitée à 60 Hz",
                    warn152);

  float angle = params[MODE_15_2].angle_deg;
  while (angle < 0.0f) angle += 360.0f;
  while (angle >= 360.0f) angle -= 360.0f;

  float t = nowSeconds(now_us - t0_us);
  float w = 2.0f * PI * f * t;
  bool leg1High = sinf(w) >= 0.0f;
  bool leg2High = sinf(w - angle * PI / 180.0f) >= 0.0f;

  if (DEADTIME_US > 0) {
    gateWrite(PIN_1A, false); gateWrite(PIN_1B, false);
    gateWrite(PIN_2A, false); gateWrite(PIN_2B, false);
    delayMicroseconds(DEADTIME_US);
  }

  gateWrite(PIN_1A, leg1High);
  gateWrite(PIN_1B, !leg1High);
  gateWrite(PIN_2A, leg2High);
  gateWrite(PIN_2B, !leg2High);
  gateWrite(PIN_3A, false);
  gateWrite(PIN_3B, false);
}

void stepMode15_3(uint32_t now_us) {
  float fmod = params[MODE_15_3].f_hz;
  float fcar = params[MODE_15_3].carrier_hz;
  fmod = warnClampOnce(fmod, F_MOD_MIN, F_MOD_MAX,
                       "⚠ Mode 15.3 : fréquence modulante minimale atteinte",
                       "⚠ Mode 15.3 : fréquence modulante limitée à 60 Hz",
                       warn153f);
  fcar = warnClampOnce(fcar, F_CAR_MIN, F_CAR_MAX,
                       "⚠ Mode 15.3 : porteuse limitée au minimum 500 Hz",
                       "⚠ Mode 15.3 : porteuse limitée à 5000 Hz",
                       warn153c);

  float t = nowSeconds(now_us - t0_us);
  float vm = sineNormalized(t, fmod, 0.0f, params[MODE_15_3].amplitude_pct);
  float vp = triangleNormalized(t, fcar);
  bool positiveState = (vm > vp);
  setLeg12(positiveState, !positiveState);
}

void stepMode15_4(uint32_t now_us) {
  // Loi non détaillée dans le CCTP : implémentation standard d'une MLI unipolaire monophasée.
  float fmod = params[MODE_15_4].f_hz;
  float fcar = params[MODE_15_4].carrier_hz;
  fmod = warnClampOnce(fmod, F_MOD_MIN, F_MOD_MAX,
                       "⚠ Mode 15.4 : fréquence modulante minimale atteinte",
                       "⚠ Mode 15.4 : fréquence modulante limitée à 60 Hz",
                       warn154f);
  fcar = warnClampOnce(fcar, F_CAR_MIN, F_CAR_MAX,
                       "⚠ Mode 15.4 : porteuse limitée au minimum 500 Hz",
                       "⚠ Mode 15.4 : porteuse limitée à 5000 Hz",
                       warn154c);

  float t = nowSeconds(now_us - t0_us);
  float vm = sineNormalized(t, fmod, 0.0f, params[MODE_15_4].amplitude_pct);
  float vp = triangleNormalized(t, fcar);

  bool leg1High = (vm > vp);
  bool leg2High = ((-vm) > vp);

  setLeg1(leg1High);
  setLeg2(leg2High);
  gateWrite(PIN_3A, false);
  gateWrite(PIN_3B, false);
}

void stepMode15_5(uint32_t now_us) {
  float f = params[MODE_15_5].f_hz;
  f = warnClampOnce(f, F_MIN, F_MAX_155,
                    "⚠ Mode 15.5 : fréquence minimale atteinte",
                    "⚠ Mode 15.5 : fréquence limitée à 60 Hz",
                    warn155);

  float t = nowSeconds(now_us - t0_us);
  float w = 2.0f * PI * f * t;

  setLeg1(sinf(w) >= 0.0f);
  setLeg2(sinf(w - 2.0f * PI / 3.0f) >= 0.0f);
  setLeg3(sinf(w - 4.0f * PI / 3.0f) >= 0.0f);
}

void stepMode15_6(uint32_t now_us) {
  float fmod = params[MODE_15_6].f_hz;
  float fcar = params[MODE_15_6].carrier_hz;
  fmod = warnClampOnce(fmod, F_MOD_MIN, F_MOD_MAX,
                       "⚠ Mode 15.6 : fréquence modulante minimale atteinte",
                       "⚠ Mode 15.6 : fréquence modulante limitée à 60 Hz",
                       warn156f);
  fcar = warnClampOnce(fcar, F_CAR_MIN, F_CAR_MAX,
                       "⚠ Mode 15.6 : porteuse limitée au minimum 500 Hz",
                       "⚠ Mode 15.6 : porteuse limitée à 5000 Hz",
                       warn156c);

  float t = nowSeconds(now_us - t0_us);
  float vp  = triangleNormalized(t, fcar);
  float vm1 = sineNormalized(t, fmod, 0.0f,             params[MODE_15_6].amplitude_pct);
  float vm2 = sineNormalized(t, fmod, -2.0f * PI / 3.0f, params[MODE_15_6].amplitude_pct);
  float vm3 = sineNormalized(t, fmod, -4.0f * PI / 3.0f, params[MODE_15_6].amplitude_pct);

  setLeg1(vm1 > vp);
  setLeg2(vm2 > vp);
  setLeg3(vm3 > vp);
}

void stepMode15_7(uint32_t now_us) {
  float f = params[MODE_15_7].f_hz;
  f = warnClampOnce(f, F_MIN, F_MAX_157,
                    "⚠ Mode 15.7 : fréquence minimale atteinte",
                    "⚠ Mode 15.7 : fréquence limitée à 2000 Hz",
                    warn157);
  float a = clamp01(params[MODE_15_7].alpha);

  uint32_t T_us = (uint32_t)(1000000.0f / f);
  uint32_t on_us = (uint32_t)(a * T_us);
  uint32_t dt = (T_us > 0) ? ((now_us - t0_us) % T_us) : 0;

  if (dt < on_us) setLeg12(true, false);
  else            setLeg12(false, true);
}

void stepMode15_8(uint32_t now_us) {
  float f = params[MODE_15_8].f_hz;
  f = warnClampOnce(f, F_MIN, F_MAX_158,
                    "⚠ Mode 15.8 : fréquence minimale atteinte",
                    "⚠ Mode 15.8 : fréquence limitée à 2000 Hz",
                    warn158);
  float a = clamp01(params[MODE_15_8].alpha);

  uint32_t T_us = (uint32_t)(1000000.0f / f);
  uint32_t on_us = (uint32_t)(a * T_us);
  uint32_t dt = (T_us > 0) ? ((now_us - t0_us) % T_us) : 0;

  gateWrite(PIN_1B, false);
  gateWrite(PIN_2A, false);
  gateWrite(PIN_2B, false);
  gateWrite(PIN_3A, false);
  gateWrite(PIN_3B, false);
  gateWrite(PIN_1A, dt < on_us);
}
