#include <Wire.h>
#include <Servo.h>

void setup() {
  initParams();
  initCOM();
  initIR();
  initMotors();
  initMPU();
}

void loop() {
  handleCOM();
  handleIR();
  updateMPU();
  delay(10);
}
