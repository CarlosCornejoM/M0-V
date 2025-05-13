// Comunicación serial y parsing de comandos

void initCOM() {
  Serial.begin(115200);
  Serial.println("=== Sistema iniciado ===");
}

void handleCOM() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    Serial.print(">> CMD RECIBIDO: "); Serial.println(cmd);

    if (cmd == "C") {
      Serial.println("=== Iniciando calibración por GUI ===");
      calibrateGyro();
      Serial.println("=== Calibración completa ===\n");
    }
    else if (cmd.startsWith("S,")) {
      String payload = cmd.substring(2);
      int comma = payload.indexOf(',');
      if (comma > 0) {
        int a1 = payload.substring(0, comma).toInt();
        int a2 = payload.substring(comma + 1).toInt();
        setServoAngles(a1, a2);
      }
    }
    else if (cmd.startsWith("M,")) {
      int parts[3], p = 0, start = 2;
      for (int i = 2; i <= cmd.length() && p < 3; i++) {
        if (i == cmd.length() || cmd.charAt(i) == ',') {
          parts[p++] = cmd.substring(start, i).toInt();
          start = i + 1;
        }
      }
      if (p == 3) {
        avanzar(parts[0], parts[1], parts[2]);
      }
    }
  }
}
