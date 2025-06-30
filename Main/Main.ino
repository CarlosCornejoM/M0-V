#include <Wire.h>
#include <Servo.h>
#include <AccelStepper.h>

void setup() {
  initParams();
  initCOM();
  initHCSR04();
  initMotors();
  //initSteppers();
  initMPU();
}

void loop() {
  handleCOM();
  //stepper();
  handleHCSR04();
  updateMPU();
  //actualizarMotors();  // ver despues el orden para PID
  
}
