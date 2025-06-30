String inString;
#define PIN_LEDuno LED_BUILTIN
extern const float MAX_RPM;
// ------------------------------------------------------------------------------
// Inicialización y manejo de la COM serie con el ESP

void initCOM() {
  Serial.begin(115200);
  Serial.println("→ UNO listo, esperando comandos del ESP a 115200bps");
  pinMode(PIN_LEDuno, OUTPUT);
  digitalWrite(PIN_LEDuno, LOW);
}

void handleCOM() {
  while (Serial.available()) {
    char c = Serial.read();
    Serial.println(c);
    inString += c;
    if (c == ']') {
      int start = inString.indexOf('[');
      int end   = inString.indexOf(']');
      if (start >= 0 && end > start) {
        String cmd = inString.substring(start + 1, end);

        if (cmd == "C") {
          Serial.println("=== Iniciando calibración por GUI ===");
          calibrateGyro();
          Serial.println("=== Calibración completa ===\n");
          }

        // --- Comando MOTOR ---
        if (cmd.startsWith("M,")) {
          int idx1 = cmd.indexOf(',', 2);
          int idx2 = cmd.indexOf(',', idx1 + 1);
          if (idx1 > 2 && idx2 > idx1) {
            int v = cmd.substring(2, idx1).toInt();
            int t = cmd.substring(idx1 + 1, idx2).toInt();
            int m = cmd.substring(idx2 + 1).toInt();
            avanzar(v, t, m);
          }
        }

        // --- Comando MOTOR sin driver (nivel DC entre 0.0 y 1.0) ---
        else if (cmd.startsWith("DC,")) {
          // formato esperado: [DC,0.0] … [DC,1.0]
          float level = cmd.substring(3).toFloat();      // extrae desde el carácter 3 hasta el final
          level = constrain(level, 0.0f, 1.0f);           // asegurarse de que esté en [0,1]
          dcmotor(level);                                 // llama a tu nueva dcmotor(float)
        }


        // --- Comando SERVOS ---
        else if (cmd.startsWith("S,")) {
          int idx = cmd.indexOf(',', 2);
          if (idx > 2) {
            int a1 = cmd.substring(2, idx).toInt();
            int a2 = cmd.substring(idx + 1).toInt();
            setServoAngles(a1, a2);
          }
        }


        // --- Comando JOY_L: mapeo de -1..1 a 0..180 ---
        else if (cmd.startsWith("JOY_L:")) {
          String vals = cmd.substring(6);
          int comma = vals.indexOf(',');
          if (comma > 0) {
            // parseamos sólo el eje Y
            float fy = vals.substring(comma + 1).toFloat();
            // mapeamos [-1, +1] → [-MAX_RPM, +MAX_RPM]
            float rpm = fy * MAX_RPM;
            avanzarSteppers(rpm);
            
          }
         }


        // --- Comando JOY_R: mapeo de -1..1 a 0..180 ---
        else if (cmd.startsWith("JOY_R:")) {
          String vals = cmd.substring(6);
          int comma = vals.indexOf(',');
          if (comma > 0) {
            float fx = vals.substring(0, comma).toFloat();
            float fy = vals.substring(comma + 1).toFloat();
            // mapear [-1.0,1.0] → [0,180]
            int angle1 = roundf( (fx + 1.0f) * 90.0f );
            int angle2 = roundf( (fy + 1.0f) * 90.0f );
            setServoAngles(angle1, angle2);
          }
        }



        // --- Comandos de LED existentes ---
        else if (cmd == "UNOON") {
          digitalWrite(PIN_LEDuno, HIGH);
          Serial.println(" -> UNO LED ON");
        }
        else if (cmd == "UNOOFF") {
          digitalWrite(PIN_LEDuno, LOW);
          Serial.println(" -> UNO LED OFF");
        }
      }
      // limpiamos el buffer para el siguiente comando
      inString = "";
    }
  }
}