#include "modes.h"

const char* modeName(ModeId m) {
  switch (m) {
    case MODE_15_1: return "15.1 (Monophase pleine onde)";
    case MODE_15_2: return "15.2 (Monophase decalee)";
    case MODE_15_3: return "15.3 (Monophase MLI bipolaire)";
    case MODE_15_4: return "15.4 (Monophase MLI unipolaire)";
    case MODE_15_5: return "15.5 (Triphase pleine onde)";
    case MODE_15_6: return "15.6 (Triphase MLI bipolaire)";
    case MODE_15_7: return "15.7 (Hacheur 4Q)";
    case MODE_15_8: return "15.8 (Hacheur serie)";
    default:        return "?";
  }
}

void printHelp() {
  Serial.println();
  Serial.println("MENU D'AIDE");
  Serial.println("----------------------------------------------------------------------------");
  Serial.println("h                  : aide");
  Serial.println("s                  : status");
  Serial.println("m 151..158         : changer de mode");
  Serial.println("f <Hz>             : frequence fondamentale/modulante/hacheur");
  Serial.println("c <Hz>             : frequence porteuse MLI (modes 15.3 / 15.4 / 15.6)");
  Serial.println("u <0..100>         : amplitude modulante en % (modes 15.3 / 15.4 / 15.6)");
  Serial.println("a <0..1>           : alpha hacheur (modes 15.7 / 15.8)");
  Serial.println("p <0..360>         : angle de decalage en degres (mode 15.2)");
  Serial.println("----------------------------------------------------------------------------");
  Serial.println();
}

void printStatus() {
  ModeParams &p = params[currentMode];
  Serial.println();
  Serial.print("Mode actif     : "); Serial.println(modeName(currentMode));
  Serial.print("f_hz           : "); Serial.println(p.f_hz, 3);
  Serial.print("carrier_hz     : "); Serial.println(p.carrier_hz, 3);
  Serial.print("amplitude_pct  : "); Serial.println(p.amplitude_pct, 1);
  Serial.print("alpha          : "); Serial.println(p.alpha, 3);
  Serial.print("angle_deg      : "); Serial.println(p.angle_deg, 1);
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
  allOff();

  Serial.print("-> Mode change : ");
  Serial.println(modeName(currentMode));
  printStatus();
}

static float valueAfterCommand(const String& cmd) {
  if (cmd.indexOf(' ') >= 0) return cmd.substring(cmd.indexOf(' ') + 1).toFloat();
  return cmd.substring(1).toFloat();
}

void handleCommand(const String& line) {
  if (line.length() == 0) return;

  String cmd = line;
  cmd.toLowerCase();

  if (cmd == "h" || cmd == "help")   { printHelp(); return; }
  if (cmd == "s" || cmd == "status") { printStatus(); return; }

  if (cmd.startsWith("m")) {
    int v = (int)valueAfterCommand(cmd);
    switch (v) {
      case 151: setMode(MODE_15_1); break;
      case 152: setMode(MODE_15_2); break;
      case 153: setMode(MODE_15_3); break;
      case 154: setMode(MODE_15_4); break;
      case 155: setMode(MODE_15_5); break;
      case 156: setMode(MODE_15_6); break;
      case 157: setMode(MODE_15_7); break;
      case 158: setMode(MODE_15_8); break;
      default: Serial.println("Mode inconnu. Utilise : m 151 | ... | m 158"); break;
    }
    return;
  }

  if (cmd.startsWith("f")) {
    float f = valueAfterCommand(cmd);
    if (f <= 0.0f) { Serial.println("Frequence invalide"); return; }
    params[currentMode].f_hz = f;
    Serial.println("OK (frequence mise a jour)");
    printStatus();
    return;
  }

  if (cmd.startsWith("c")) {
    float c = valueAfterCommand(cmd);
    if (c <= 0.0f) { Serial.println("Porteuse invalide"); return; }
    params[currentMode].carrier_hz = c;
    Serial.println("OK (porteuse mise a jour)");
    printStatus();
    return;
  }

  if (cmd.startsWith("u")) {
    float u = valueAfterCommand(cmd);
    params[currentMode].amplitude_pct = constrain(u, AMP_MIN_PCT, AMP_MAX_PCT);
    Serial.println("OK (amplitude mise a jour)");
    printStatus();
    return;
  }

  if (cmd.startsWith("a")) {
    float a = valueAfterCommand(cmd);
    params[currentMode].alpha = constrain(a, 0.0f, 1.0f);
    Serial.println("OK (alpha mis a jour)");
    printStatus();
    return;
  }

  if (cmd.startsWith("p")) {
    float p = valueAfterCommand(cmd);
    while (p < 0.0f) p += 360.0f;
    while (p >= 360.0f) p -= 360.0f;
    params[currentMode].angle_deg = p;
    Serial.println("OK (angle mis a jour)");
    printStatus();
    return;
  }

  Serial.println("Commande inconnue. Tape 'h' pour l'aide.");
}
