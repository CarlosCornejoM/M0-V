String inString;
#define PIN_LEDuno LED_BUILTIN
extern float MAX_RPM;

extern float setPoint;
extern float Kp;
extern float Ki;
extern float Kd;

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
          float level = cmd.substring(3).toFloat();
          level = constrain(level, 0.0f, 1.0f);
          dcmotor(level);
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

        // --- Comando PID-TUNING con setPoint y constantes Kp, Ki, Kd ---
        else if (cmd.startsWith("PIDT,")) {
          int p1 = cmd.indexOf(',');
          int p2 = cmd.indexOf(',', p1 + 1);
          int p3 = cmd.indexOf(',', p2 + 1);
          int p4 = cmd.indexOf(',', p3 + 1);
          
          float sp = cmd.substring(p1 + 1, p2).toFloat();
          float KP = cmd.substring(p2 + 1, p3).toFloat();
          float KI = cmd.substring(p3 + 1, p4).toFloat();
          float KD = cmd.substring(p4 + 1).toFloat();
          
          setPoint = sp;
          Kp = KP;
          Ki = KI;
          Kd = KD;
          
          resetPIDState();
        }
        

        // --- Comando JOY_L: control de los dos steppers ---
        else if (cmd.startsWith("JOY_L:")) {
  // cmd = "JOY_L:fx,fy"
  String vals = cmd.substring(6);
  int comma = vals.indexOf(',');
  if (comma > 0) {
    float fx = vals.substring(0, comma).toFloat();    // eje X
    float fy = vals.substring(comma + 1).toFloat();   // eje Y

    // Ahora intercambiamos: el eje Y actúa como diferencial
    //   left  = fx + fy
    //   right = fx - fy
    float vL = fx + fy;
    float vR = fx - fy;

    // Escalar a RPM máximo
    float rpmL = constrain(vL * MAX_RPM, -MAX_RPM, MAX_RPM);
    float rpmR = constrain(vR * MAX_RPM, -MAX_RPM, MAX_RPM);

    // Llamadas a tu función de control
    setStepperRPM(1, rpmL);
    setStepperRPM(2, rpmR);
  }
}




        // --- Comando JOY_R: control de servos ---
        else if (cmd.startsWith("JOY_R:")) {
          String vals = cmd.substring(6);
          int comma = vals.indexOf(',');
          if (comma > 0) {
            float fx = vals.substring(0, comma).toFloat();
            float fy = vals.substring(comma + 1).toFloat();
            int angle1 = roundf( (fx + 1.0f) * 90.0f );
            int angle2 = roundf( (fy + 1.0f) * 90.0f );
            setServoAngles(angle1, angle2);
          }
        }
        

        else if (cmd.startsWith("MRPM,")) {
          // extraer el valor después de "MRPM,"
          float mrpm = cmd.substring(5).toFloat();
          // función tuya que fija el límite de rpm del driver:
          MAX_RPM=mrpm;
          }

        // --- Comandos de LED ---
        else if (cmd == "UNOON") {
          digitalWrite(PIN_LEDuno, HIGH);
          Serial.println(" -> UNO LED ON");
        }
        else if (cmd == "UNOOFF") {
          digitalWrite(PIN_LEDuno, LOW);
          Serial.println(" -> UNO LED OFF");
        }
      }
      inString = "";
    }
  }
} 