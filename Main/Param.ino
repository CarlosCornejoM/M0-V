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

const int enA  = 14;
const int enB  = 7;
const int in1  = 8;
const int in2  = 12;
const int in3  = 10;
const int in4  = 13;

const int servo1pin = 6;
const int servo2pin = 9;
Servo servo1;
Servo servo2;

const int motordc = 11;

// PID params
float setPoint = 0;
float Kp       = 0;
float Ki       = 0;
float Kd       = 0;

float MAX_RPM = 30.0;

void initParams() {
  Wire.begin();
  pinMode(motordc, OUTPUT);
}