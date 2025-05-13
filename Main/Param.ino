// Pines y variables globales
const int IR_RECEIVE_PIN = 2;   // Pin del receptor IR
const int MPU_ADDR        = 0x68;

int16_t ax, ay, az, gx, gy, gz, tmp;
int16_t gx_offset = 0, gy_offset = 0, gz_offset = 0;
float   roll = 0, pitch = 0, yaw = 0, temperature = 0;
unsigned long lastTime;
float dt;

const int enA = 5;
const int enB = 6;
const int in1 = 4;
const int in2 = 7;
const int in3 = 8;
const int in4 = 12;

Servo servo1;
Servo servo2;

void initParams() {
  Wire.begin();
}
