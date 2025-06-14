// Param.ino
extern const int trigPin;  // de Param.ino
extern const int echoPin;  // de Param.ino

// HCSR04.ino

// ===== Parámetros de lectura =====
static const unsigned long TIMEOUT_US   = 30000UL;  // 30 ms timeout (~5 m)
static const float        MIN_DIST_CM  =   2.0F;   // 2 cm mínima distancia fiable
static const float        MAX_DIST_CM  = 400.0F;   // 400 cm máxima distancia fiable
static const float        FILTER_ALPHA =   0.3F;   // coef. IIR (0<α<1)

static float filteredDistance = MAX_DIST_CM;  // valor inicial alto

void initHCSR04() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);
}

void handleHCSR04() {
  // 1) Trigger de 10 µs
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // 2) Medir eco con timeout
  unsigned long dur = pulseIn(echoPin, HIGH, TIMEOUT_US);
  if (dur == 0) {
    // timeout → no actualiza filtro
    Serial.print("Distance: ");
    Serial.print(filteredDistance);
    Serial.println(" cm");
    return;
  }

  // 3) Calcular distancia en cm
  float d = (dur * 0.0343F) / 2.0F;

  // 4) Validar rango
  if (d < MIN_DIST_CM || d > MAX_DIST_CM) {
    // lectura anómala → descarta
    Serial.print("Distance: ");
    Serial.print(filteredDistance);
    Serial.println(" cm");
    return;
  }

  // 5) Filtrar con IIR: y = α·x + (1–α)·y_prev
  filteredDistance = FILTER_ALPHA * d
                   + (1.0F - FILTER_ALPHA) * filteredDistance;

  // 6) Imprimir distancia filtrada
  Serial.print("Distance: ");
  Serial.print(filteredDistance);
  Serial.println(" cm");
}
