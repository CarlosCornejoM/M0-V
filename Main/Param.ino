// Param.ino
// Pines y variables globales

#include <Servo.h>
#include <Wire.h>

const int trigPin    = A3;
const int echoPin    = 6;
const int MPU_ADDR   = 0x68;

int16_t ax, ay, az, gx, gy, gz, tmp;
int16_t gx_offset = 0, gy_offset = 0, gz_offset = 0;
float   roll = 0, pitch = 0, yaw = 0, temperature = 0;
unsigned long lastTime;
float dt;

const int enA  = 4;
const int enB  = 7;
const int in1  = 8;
const int in2  = 12;
const int in3  = 10;
const int in4  = 13;

const int servo1pin = 5;
const int servo2pin = 9;
Servo servo1;
Servo servo2;

const int motordc = 11;

// PID params
float setPoint = 25.0;  // cm
float Kp       = 1.0;
float Ki       = 0.02;
float Kd       = 0.5;

float MAX_RPM = 30.0;  // usado por el stepper

void initParams() {
  Wire.begin();
  pinMode(motordc, OUTPUT);
}