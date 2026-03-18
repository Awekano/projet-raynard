#include "modes.h"

const char* modeName(ModeId m) {
  switch (m) {
    case MODE_15_1: return "15.1 (Monophasé pleine onde)";
    case MODE_15_7: return "15.7 (Hacheur 4Q)";
    case MODE_15_8: return "15.8 (Hacheur serie)";
    default: return "?";
  }
}
//Menu d'aide
void printHelp() {
  Serial.println();
  Serial.println("MENU D'AIDE ");
  Serial.println("------------------------------------------------------------------");
  Serial.println("Commandes:");
  Serial.println("  h                : aide");
  Serial.println("  s                : status");
  Serial.println("  m 151|157|158    : changer de mode");
  Serial.println("  f <Hz>           : regler la frequence du mode actif");
  Serial.println("  a <0..1>         : regler alpha du mode actif (15.7 / 15.8)");
  Serial.println("------------------------------------------------------------------");
  Serial.println();
}
//Staut du mode actif
void printStatus() {
  Serial.println();
  Serial.print("Mode actif: "); Serial.println(modeName(currentMode));
  Serial.print("  f_hz   = "); Serial.println(params[currentMode].f_hz, 3);
  Serial.print("  alpha  = "); Serial.println(params[currentMode].alpha, 3);
  Serial.println();
}

String readLineNonBlocking() {
  static String buf;
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      String line = buf;
      buf = "";
      line.trim();
      return line;
    }
    buf += c;
    if (buf.length() > 120) buf.remove(0, 60);
  }
  return "";
}

void setMode(ModeId m) {
  currentMode = m;
  t0_us = micros();
  halfState = false;
  allOff();

  Serial.print("-> Mode change: ");
  Serial.println(modeName(currentMode));
  printStatus();
}

void handleCommand(const String& line) {
  if (line.length() == 0) return;

  String cmd = line;
  cmd.toLowerCase();

  if (cmd == "h" || cmd == "help") { printHelp(); return; }
  if (cmd == "s" || cmd == "status") { printStatus(); return; }

  if (cmd.startsWith("m")) {
    int v = cmd.substring(1).toInt();
    if (cmd.indexOf(' ') >= 0) v = cmd.substring(cmd.indexOf(' ') + 1).toInt();
    if (v == 151) setMode(MODE_15_1);
    else if (v == 157) setMode(MODE_15_7);
    else if (v == 158) setMode(MODE_15_8);
    else Serial.println("Mode inconnu. Utilise: m 151 | m 157 | m 158");
    return;
  }

  if (cmd.startsWith("f")) {
    float f = cmd.substring(1).toFloat();
    if (cmd.indexOf(' ') >= 0) f = cmd.substring(cmd.indexOf(' ') + 1).toFloat();
    if (f <= 0) { Serial.println("f invalide"); return; }

    float fmax = maxFreqForMode(currentMode);
    float f_clamped = constrain(f, F_MIN, fmax);

    if (f > fmax) {
      Serial.print("⚠ ");
      Serial.print(modeName(currentMode));
      Serial.print(" : frequence limitee a ");
      Serial.print(fmax, 1);
      Serial.println(" Hz");
    }

    params[currentMode].f_hz = f_clamped;
    Serial.println("OK (frequence mise a jour)");
    printStatus();
    return;
  }

  if (cmd.startsWith("a")) {
    float a = cmd.substring(1).toFloat();
    if (cmd.indexOf(' ') >= 0) a = cmd.substring(cmd.indexOf(' ') + 1).toFloat();
    params[currentMode].alpha = constrain(a, 0.0f, 1.0f);
    Serial.println("OK (alpha mis a jour)");
    printStatus();
    return;
  }

  Serial.println("Commande inconnue. Tape 'h' pour l'aide.");
}
