#include <Wire.h>

extern const int MPU_ADDR;                // de Param.ino
extern int16_t ax, ay, az, gx, gy, gz, tmp;
extern int16_t gx_offset, gy_offset, gz_offset;
extern float roll, pitch, yaw, temperature;
extern unsigned long lastTime;
extern float dt;

// --- PARÁMETROS DE SENSIBILIDAD ---
// ACCEL_FS_SEL: 0=>±2g, 1=>±4g, 2=>±8g, 3=>±16g
const uint8_t ACCEL_FS_SEL = 0;
// Sensibilidad en LSB por g según ACCEL_FS_SEL
const float ACCEL_SENS[4] = {16384.0f, 8192.0f, 4096.0f, 2048.0f};

// GYRO_FS_SEL: 0=>±250°/s, 1=>±500°/s, 2=>±1000°/s, 3=>±2000°/s
const uint8_t GYRO_FS_SEL = 0;
// Sensibilidad en LSB por (°/s) según GYRO_FS_SEL
const float GYRO_SENS[4]  = {131.0f, 65.5f, 32.8f, 16.4f};

// Filtro complementario (0..1): más alto más peso al giroscopio
const float alpha = 0.98f;

// Variables de filtro
float rollAcc, rollGyro, rollFiltered = 0;

void initMPU() {
  Wire.begin();
  // Despertar MPU
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  delay(100);

  // Configurar full‑scale del acelerómetro
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C);
  Wire.write(ACCEL_FS_SEL << 3);
  Wire.endTransmission(true);

  // Configurar full‑scale del giroscopio
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B);
  Wire.write(GYRO_FS_SEL << 3);
  Wire.endTransmission(true);
  delay(50);

  // Calibración de giroscopio
  calibrateGyro();
  Serial.println("=== Calibración automática completa ===");
  lastTime = millis();
}

void updateMPU() {
  readMPU6050();
  unsigned long now = millis();
  dt = (now - lastTime) / 1000.0f;
  lastTime = now;

  // Integración yaw simple
  float gyroZ = (gz - gz_offset) / GYRO_SENS[GYRO_FS_SEL];
  if (abs(gyroZ) > 0.5f) yaw += gyroZ * dt;

  computeAngles();
  sendTelemetry();
}

void readMPU6050() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);
  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
  tmp = Wire.read() << 8 | Wire.read();
  gx = Wire.read() << 8 | Wire.read();
  gy = Wire.read() << 8 | Wire.read();
  gz = Wire.read() << 8 | Wire.read();
}

void computeAngles() {
  // 1) Convertir acelerómetro crudo a g:
  //    a_g = raw_acc / (LSB/g)
  float ax_g = ax / ACCEL_SENS[ACCEL_FS_SEL];
  float ay_g = ay / ACCEL_SENS[ACCEL_FS_SEL];
  float az_g = az / ACCEL_SENS[ACCEL_FS_SEL];

  // 2) Ángulos por acelerómetro:
  //    rollAcc = atan2(ay, az) * (180/PI)
  rollAcc = atan2(ay_g, az_g) * RAD_TO_DEG;
  float denom = max(sqrt(ay_g*ay_g + az_g*az_g), 1e-6f);
  pitch = atan2(-ax_g, denom) * RAD_TO_DEG;

  // 3) Integración del giroscopio en X:
  //    ω_x = (gx - offset) / (LSB/(°/s))
  float gyroX = (gx - gx_offset) / GYRO_SENS[GYRO_FS_SEL];
  rollGyro = rollFiltered + gyroX * dt;

  // 4) Filtro complementario:
  //    roll = α*(roll_prev + ω_x·dt) + (1-α)*rollAcc
  rollFiltered = alpha * rollGyro + (1.0f - alpha) * rollAcc;
  roll = rollFiltered;

  // 5) Temperatura:
  //    T = (raw_temp - 1600)/340 + 36.53
  temperature = (tmp - 1600) / 340.0f + 36.53f;
}

void sendTelemetry() {

  
  Serial.print("Pitch: "); Serial.print(pitch,2);
  Serial.print(" | Roll: "); Serial.print(roll,2);
  Serial.print(" | Yaw: "); Serial.println(yaw,2);

  // Mostrar aceleración en g y velocidad angular
  Serial.print("Accel (g): X="); Serial.print(ax/ACCEL_SENS[ACCEL_FS_SEL],3);
  Serial.print(" Y="); Serial.print(ay/ACCEL_SENS[ACCEL_FS_SEL],3);
  Serial.print(" Z="); Serial.println(az/ACCEL_SENS[ACCEL_FS_SEL],3);

  Serial.print("Gyro (°/s): X="); Serial.print((gx-gx_offset)/GYRO_SENS[GYRO_FS_SEL],2);
  Serial.print(" Y="); Serial.print((gy-gy_offset)/GYRO_SENS[GYRO_FS_SEL],2);
  Serial.print(" Z="); Serial.println((gz-gz_offset)/GYRO_SENS[GYRO_FS_SEL],2);

  Serial.print("Temp: "); Serial.print(temperature,2);
  Serial.println(" °C\n");
}

void calibrateGyro() {
  long sumX = 0, sumY = 0, sumZ = 0;
  const int samples = 1000;
  for (int i = 0; i < samples; i++) {
    readMPU6050();
    sumX += gx; sumY += gy; sumZ += gz;
    delay(5);
  }
  gx_offset = sumX / samples;
  gy_offset = sumY / samples;
  gz_offset = sumZ / samples;
}
