#include <Wire.h>
#include <Servo.h>
#include <AccelStepper.h>


void setup() {
  initParams();
  initSteppers();
  initHCSR04();
  initMotors();
  initMPU();
  initCOM();
  
  initPID();
}

void loop() {
  //handleHCSR04();
  //updateMPU();
  handleCOM();
  runSteppers();
  //handlePID();
}