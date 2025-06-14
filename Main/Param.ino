// Pines y variables globales

const int trigPin = 3;


const int echoPin = 2;
const int MPU_ADDR        = 0x68;

int16_t ax, ay, az, gx, gy, gz, tmp;
int16_t gx_offset = 0, gy_offset = 0, gz_offset = 0;
float   roll = 0, pitch = 0, yaw = 0, temperature = 0;
unsigned long lastTime;
float dt;

const int enA = 10;
const int enB = 11;
const int in1 = 8;
const int in2 = 9;
const int in3 = 10;
const int in4 = 11;



const int servo1pin = 9;
const int servo2pin = 6;
Servo servo1;
Servo servo2;

void initParams() {
  Wire.begin();
}
