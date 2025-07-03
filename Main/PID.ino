// PID.ino

#include <Servo.h>

// Externs de Param.ino y HCSR04
extern float median;
extern const int servo2pin;
extern float setPoint, Kp, Ki, Kd;

float integral = 0;
float previousError = 0;
Servo beamServo;

void initPID() {
  beamServo.attach(servo2pin);
  beamServo.write(90);
}

void resetPIDState() {
  integral = 0;
  previousError = 0;
}

void handlePID() {
  float measured = median;
  if (measured < 0.1) return;
  float error = setPoint - measured;
  integral += error;
  integral = constrain(integral,-1000,1000);
  float derivative = error - previousError;
  float output = Kp*error + Ki*integral + Kd*derivative;
  previousError = error;

  int angle = constrain(90 + int(output), 60, 120);
  beamServo.write(angle);

  Serial.print("[PID,");
  Serial.print(error,2); Serial.print(",");
  Serial.print(output,2); Serial.print(",");
  Serial.print(angle);
  Serial.println("]");
}

