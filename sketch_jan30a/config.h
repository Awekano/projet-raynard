#pragma once
#include <Arduino.h>
//Définition limite de fréquence 
#define F_MIN     0.1f
#define F_MAX_151 60.0f
#define F_MAX_157 2000.0f
#define F_MAX_158 2000.0f

static const bool ACTIVE_LOW = true;
//Pin de l'ESP32
static const int PIN_1A = 25;
static const int PIN_1B = 26;
static const int PIN_2A = 27;
static const int PIN_2B = 14;
static const int PIN_3A = 12;
static const int PIN_3B = 13;

static const uint32_t DEADTIME_US = 0;
