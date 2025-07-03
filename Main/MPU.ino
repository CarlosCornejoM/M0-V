// MPU.ino

#include <Wire.h>

// Externs de Param.ino
extern const int MPU_ADDR;
extern int16_t ax, ay, az, gx, gy, gz, tmp;
extern int16_t gx_offset, gy_offset, gz_offset;
extern float roll, pitch, yaw, temperature;
extern unsigned long lastTime;
extern float dt;

const uint8_t ACCEL_FS_SEL = 0;
const float ACCEL_SENS[4] = {16384,8192,4096,2048};
const uint8_t GYRO_FS_SEL  = 0;
const float GYRO_SENS[4]  = {131,65.5,32.8,16.4};
const float alpha = 0.98f;

float rollAcc, rollGyro, rollFiltered = 0;

void initMPU() {
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); Wire.write(0);
  Wire.endTransmission(true);
  delay(100);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C); Wire.write(ACCEL_FS_SEL<<3);
  Wire.endTransmission(true);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B); Wire.write(GYRO_FS_SEL<<3);
  Wire.endTransmission(true);
  delay(50);
  calibrateGyro();
  Serial.println("=== Calibración automática completa ===");
  lastTime = millis();
}

void updateMPU() {
  readMPU6050();
  unsigned long now = millis();
  dt = (now-lastTime)/1000.0f;
  lastTime = now;
  float gz_d = (gz-gz_offset)/GYRO_SENS[GYRO_FS_SEL];
  if (abs(gz_d)>0.5f) yaw += gz_d*dt;
  computeAngles();
  sendTelemetry();
}

void readMPU6050() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR,14,true);
  ax = Wire.read()<<8|Wire.read();
  ay = Wire.read()<<8|Wire.read();
  az = Wire.read()<<8|Wire.read();
  tmp= Wire.read()<<8|Wire.read();
  gx = Wire.read()<<8|Wire.read();
  gy = Wire.read()<<8|Wire.read();
  gz = Wire.read()<<8|Wire.read();
}

void computeAngles() {
  float ax_g = ax/ACCEL_SENS[ACCEL_FS_SEL];
  float ay_g = ay/ACCEL_SENS[ACCEL_FS_SEL];
  float az_g = az/ACCEL_SENS[ACCEL_FS_SEL];
  rollAcc = atan2(ay_g,az_g)*RAD_TO_DEG;
  float denom = max(sqrt(ay_g*ay_g+az_g*az_g),1e-6f);
  pitch = atan2(-ax_g,denom)*RAD_TO_DEG;
  float gx_d = (gx-gx_offset)/GYRO_SENS[GYRO_FS_SEL];
  rollGyro = rollFiltered + gx_d*dt;
  rollFiltered = alpha * rollGyro + (1-alpha)*rollAcc;
  roll = rollFiltered;
  temperature = (tmp-1600)/340.0f + 36.53f;
}

void sendTelemetry() {
  return;
  Serial.print("[ANGLES,"); Serial.print(pitch,2);
  Serial.print(","); Serial.print(roll,2);
  Serial.print(","); Serial.print(yaw,2);
  Serial.println("]");
  Serial.print("[ACCEL,");
  Serial.print(ax/ACCEL_SENS[ACCEL_FS_SEL],3);
  Serial.print(","); Serial.print(ay/ACCEL_SENS[ACCEL_FS_SEL],3);
  Serial.print(","); Serial.print(az/ACCEL_SENS[ACCEL_FS_SEL],3);
  Serial.println("]");
  Serial.print("[GYRO,");
  Serial.print((gx-gx_offset)/GYRO_SENS[GYRO_FS_SEL],2);
  Serial.print(","); Serial.print((gy-gy_offset)/GYRO_SENS[GYRO_FS_SEL],2);
  Serial.print(","); Serial.print((gz-gz_offset)/GYRO_SENS[GYRO_FS_SEL],2);
  Serial.println("]");
  Serial.print("[TEMP,"); Serial.print(temperature,2); Serial.println("]");
}

void calibrateGyro() {
  long sx=0, sy=0, sz=0;
  const int n=1000;
  for (int i=0;i<n;i++) {
    readMPU6050();
    sx+=gx; sy+=gy; sz+=gz;
    delay(5);
  }
  gx_offset = sx/n;
  gy_offset = sy/n;
  gz_offset = sz/n;
}

