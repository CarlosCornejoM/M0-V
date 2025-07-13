#include <Wire.h>

// Externs de Param.ino
extern const int MPU_ADDR;
extern int16_t ax, ay, az, gx, gy, gz, tmp;
extern int16_t gx_offset, gy_offset, gz_offset;
extern float roll, pitch, yaw, temperature;
extern unsigned long lastTime;
extern float dt;
// Externs de RPM de motores
extern float currentRPM1;
extern float currentRPM2;
// Parámetros del sensor
const uint8_t ACCEL_FS_SEL = 0;
const float   ACCEL_SENS[4] = {16384, 8192, 4096, 2048};
const uint8_t GYRO_FS_SEL  = 0;
const float   GYRO_SENS[4]  = {131, 65.5, 32.8, 16.4};
const float   alpha = 0.98f;    // Complementary filter

// --- Parámetros de filtrado exponencial (EMA) ---
const float beta = 0.8f;        // Entre 0 (mucho suavizado) y 1 (sin suavizar)
float pitchFilt = 0.0f;
float rollFilt  = 0.0f;
float yawFilt   = 0.0f;

// Inicializa MPU6050 y calibra el giroscopio
void initMPU() {
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B); Wire.write(0);
  Wire.endTransmission(true);
  delay(100);

  Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1C); Wire.write(ACCEL_FS_SEL << 3);
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1B); Wire.write(GYRO_FS_SEL << 3);
  Wire.endTransmission(true);
  delay(50);

  calibrateGyro();
  Serial.println("=== Calibración automática completa ===");
  lastTime = millis();
}

// Lee datos, actualiza ángulos y envía telemetría
void updateMPU() {
   // Si los motores están girando, omitir lectura del MPU
  if (currentRPM1 != 0.0f || currentRPM2 != 0.0f) {
    return;
  }
  readMPU6050();
  unsigned long now = millis();
  dt = (now - lastTime) / 100.0f;
  lastTime = now;

  float gz_d = (gz - gz_offset) / GYRO_SENS[GYRO_FS_SEL];
  if (abs(gz_d) > 0.5f) yaw += gz_d * dt;

  computeAngles();
  sendTelemetry();
}

// Lee aceleración, temperatura y giro crudos
void readMPU6050() {
  Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);
  ax  = Wire.read() << 8 | Wire.read();
  ay  = Wire.read() << 8 | Wire.read();
  az  = Wire.read() << 8 | Wire.read();
  tmp = Wire.read() << 8 | Wire.read();
  gx  = Wire.read() << 8 | Wire.read();
  gy  = Wire.read() << 8 | Wire.read();
  gz  = Wire.read() << 8 | Wire.read();
}

// Calcula ángulos con complementary filter y aplica EMA
void computeAngles() {
  float ax_g = ax / ACCEL_SENS[ACCEL_FS_SEL];
  float ay_g = ay / ACCEL_SENS[ACCEL_FS_SEL];
  float az_g = az / ACCEL_SENS[ACCEL_FS_SEL];

  // Ángulo de acelerómetro
  float rollAcc  = atan2(ay_g, az_g) * RAD_TO_DEG;
  float denom    = max(sqrt(ay_g*ay_g + az_g*az_g), 1e-6f);
  pitch = atan2(-ax_g, denom) * RAD_TO_DEG;

  // Giro de giroscopio
  float gx_d = (gx - gx_offset) / GYRO_SENS[GYRO_FS_SEL];
  float rollGyro = roll + gx_d * dt;

  // Complementary filter para roll
  roll = alpha * rollGyro + (1.0f - alpha) * rollAcc;

  // Temperatura
  temperature = (tmp - 1600) / 340.0f + 36.53f;

  // Filtro exponencial (EMA) sobre los ángulos
  pitchFilt = beta * pitchFilt + (1.0f - beta) * pitch;
  rollFilt  = beta * rollFilt  + (1.0f - beta) * roll;
  yawFilt   = beta * yawFilt   + (1.0f - beta) * yaw;
}

// Envía telemetría cada 200 ms usando los valores filtrados
void sendTelemetry() {
  static unsigned long lastSend = 0;
  const unsigned long interval = 200;
  unsigned long now = millis();
  if (now - lastSend < interval) return;
  lastSend = now;

  char buf[16];

  Serial.print("[ANGLES,");
  dtostrf(pitchFilt, 6, 2, buf); Serial.print(buf);
  Serial.print(",");
  dtostrf(rollFilt,  6, 2, buf); Serial.print(buf);
  Serial.print(",");
  dtostrf(yawFilt,   6, 2, buf); Serial.print(buf);
  Serial.println("]");
}

// Calibración automática del giroscopio
void calibrateGyro() {
  long sx = 0, sy = 0, sz = 0;
  const int n = 1000;
  Serial.println("Calibrando giroscopio...");
  for (int i = 0; i < n; i++) {
    readMPU6050();
    sx += gx;  sy += gy;  sz += gz;
    delay(5);
    if (i % 100 == 0) {
      Serial.print("Calibración: ");
      Serial.print((i * 100) / n);
      Serial.println("%");
    }
  }
  gx_offset = sx / n;
  gy_offset = sy / n;
  gz_offset = sz / n;

  Serial.println("Calibración completada");
  Serial.print("Offsets - GX: "); Serial.print(gx_offset);
  Serial.print(" GY: "); Serial.print(gy_offset);
  Serial.print(" GZ: "); Serial.println(gz_offset);
}
