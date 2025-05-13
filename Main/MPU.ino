#include <Wire.h>

extern const int MPU_ADDR;               // de Param.ino
extern int16_t ax, ay, az, gx, gy, gz, tmp;
extern int16_t gx_offset, gy_offset, gz_offset;
extern float roll, pitch, yaw, temperature;
extern unsigned long lastTime;
extern float dt;

void initMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  delay(100);
  calibrateGyro();
  Serial.println("=== Calibración automática completa ===");
  lastTime = millis();
}

void updateMPU() {
  readMPU6050();
  unsigned long now = millis();
  dt = (now - lastTime) / 1000.0;
  lastTime = now;
  float gyroZ = (gz - gz_offset) / 131.0;
  if (abs(gyroZ) > 0.5) yaw += gyroZ * dt;
  computeAngles();
  sendTelemetry();
}

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

void computeAngles() {
  float denom = max(sqrt(ay*ay + az*az), 0.0001f);
  roll  = atan2(ay, az) * RAD_TO_DEG;
  pitch = atan2(-ax, denom) * RAD_TO_DEG;
  temperature = (tmp - 1600) / 340.0 + 36.53;
}

void sendTelemetry() {
  Serial.print("Angle => Pitch: "); Serial.print(pitch,2);
  Serial.print(" | Roll: ");  Serial.print(roll,2);
  Serial.print(" | Yaw: ");   Serial.println(yaw,2);
  Serial.print("Accelerometer => X: "); Serial.print(ax);
  Serial.print(" | Y: "); Serial.print(ay);
  Serial.print(" | Z: "); Serial.println(az);
  Serial.print("Gyroscope => X: "); Serial.print(gx);
  Serial.print(" | Y: "); Serial.print(gy);
  Serial.print(" | Z: "); Serial.println(gz);
  Serial.print("Temperature => "); Serial.print(temperature,2);
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
