#pragma once
#include <Arduino.h>

// ===================== LIMITES =====================
#define F_MIN            0.1f

// Modes pleine onde
#define F_MAX_151        60.0f   // 15.1 monophasé pleine onde
#define F_MAX_152        60.0f   // 15.2 monophasé décalé
#define F_MAX_155        60.0f   // 15.5 triphasé pleine onde

// Modes MLI
#define F_MOD_MIN        0.1f
#define F_MOD_MAX        60.0f   // fréquence modulante
#define F_CAR_MIN        500.0f
#define F_CAR_MAX        5000.0f // fréquence porteuse
#define AMP_MIN_PCT      0.0f
#define AMP_MAX_PCT      100.0f

// Hacheurs
#define F_MAX_157        2000.0f
#define F_MAX_158        2000.0f

// ===================== CONFIG HW =====================
static const bool ACTIVE_LOW = true;

static const int PIN_1A = 25;
static const int PIN_1B = 26;
static const int PIN_2A = 27;
static const int PIN_2B = 14;
static const int PIN_3A = 12;
static const int PIN_3B = 13;

// Dead-time logiciel éventuel
static const uint32_t DEADTIME_US = 0;
