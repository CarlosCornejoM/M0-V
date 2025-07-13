#include <Wire.h>
#include <Servo.h>
#include <AccelStepper.h>


void setup() {
  initParams();
  initSteppers();
  initMotors();
  initMPU();
  initCOM();
  //initPID();
}

void loop() {
  updateMPU();
  handleCOM();
  runSteppers();
  //handlePID();
}