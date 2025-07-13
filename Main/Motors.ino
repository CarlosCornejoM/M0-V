// dual_stepper_control.ino
#include <Servo.h>
#include <AccelStepper.h>

// Externs de Param.ino
extern const int enA, enB, in1, in2, in3, in4;
extern Servo servo1, servo2;
extern const int servo1pin, servo2pin;
extern const int motordc;
extern float MAX_RPM;

// Pines driver de paso a paso motor 1
const int dirPin1    = 2;
const int stepPin1   = 3;

// Pines driver de paso a paso motor 2
const int dirPin2    = 4;
const int stepPin2   = 5;

// Enable compartido (A0)
const int enablePin1  = A0;
const int enablePin2 = A1;
// Parámetros del stepper
const float STEPS_PER_REV = 2048.0;

// Objetos AccelStepper para motores 1 y 2 (DRIVER)
AccelStepper stepper1(AccelStepper::DRIVER, stepPin1, dirPin1);
AccelStepper stepper2(AccelStepper::DRIVER, stepPin2, dirPin2);

bool steppersEnabled = false;
float currentRPM1    = 0.0;
float currentRPM2    = 0.0;

// ----------------------- Inicialización -----------------------
void initSteppers() {
  pinMode(enablePin1, OUTPUT);
  pinMode(enablePin2, OUTPUT);
  disableStepper(1);
  disableStepper(2);
  
  // Configuración de velocidades y aceleraciones
  float maxStepsPerSec = (MAX_RPM / 60.0f) * STEPS_PER_REV;
  stepper1.setMaxSpeed( maxStepsPerSec );
  stepper1.setAcceleration( 10 * maxStepsPerSec );
  stepper1.setSpeed(0);

  stepper2.setMaxSpeed( maxStepsPerSec );
  stepper2.setAcceleration( 10 * maxStepsPerSec );
  stepper2.setSpeed(0);
}

void enableStepper(int m) {
  if (m == 1) digitalWrite(enablePin1, LOW);
  else        digitalWrite(enablePin2, LOW);
}

void disableStepper(int m) {
  if (m == 1) {
    digitalWrite(enablePin1, HIGH);
    stepper1.setSpeed(0);
    currentRPM1 = 0;
  } else {
    digitalWrite(enablePin2, HIGH);
    stepper2.setSpeed(0);
    currentRPM2 = 0;
  }
}

void setStepperRPM(int m, float rpm) {
  rpm = constrain(rpm, -MAX_RPM, MAX_RPM);
  if (fabs(rpm) < 0.01f) {
    disableStepper(m);
    return;
  }
  enableStepper(m);
  float stepsPerSec = (rpm / 60.0f) * STEPS_PER_REV;
  if (m == 1) {
    currentRPM1 = rpm;
    stepper1.setSpeed(stepsPerSec);
  } else {
    currentRPM2 = rpm;
    stepper2.setSpeed(stepsPerSec);
  }
}
// Llamar desde loop(): mueve ambos steppers y reporta RPM
void runSteppers() {
  static float lastSent1 = -1.0f;
  static float lastSent2 = -1.0f;

  stepper1.runSpeed();
  stepper2.runSpeed();

  if (fabs(currentRPM1 - lastSent1) > 1.0f) {
    Serial.print("[RPM1,"); Serial.print(currentRPM1,1); Serial.println("]");
    lastSent1 = currentRPM1;
  }
  if (fabs(currentRPM2 - lastSent2) > 1.0f) {
    Serial.print("[RPM2,"); Serial.print(currentRPM2,1); Serial.println("]");
    lastSent2 = currentRPM2;
  }
}

// ------------------- Control DC y servos (sin cambios) -------------------
bool motorDCRunning = false;
int  lastDCLevel    = -1;

void initMotors() {
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  servo1.attach(servo1pin);
  servo2.attach(servo2pin);
  setServoAngles(90, 90);
  detenerDC();
  dcmotor(0);
}

void avanzar(int velocidad, int tiempo, int motor) {
  detenerDC();
  velocidad = constrain(velocidad, -255, 255);

  if (motor == 1 || motor == 3) {
    digitalWrite(in1, velocidad >= 0);
    digitalWrite(in2, velocidad <  0);
    analogWrite(enA, abs(velocidad));
  }
  if (motor == 2 || motor == 3) {
    digitalWrite(in3, velocidad >= 0);
    digitalWrite(in4, velocidad <  0);
    analogWrite(enB, abs(velocidad));
  }

  delay(tiempo);
  detenerDC();
}

void detenerDC() {
  analogWrite(enA, 0);
  analogWrite(enB, 0);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

void setServoAngles(int a1, int a2) {
  a1 = constrain(a1, 0, 180);
  a2 = constrain(a2, 0, 180);
  servo1.write(a1);
  servo2.write(a2);
  Serial.print("Servos -> ");
  Serial.print(a1);
  Serial.print("°, ");
  Serial.print(a2);
  Serial.println("°");
}

void dcmotor(float level) {
  level = constrain(level, 0.0f, 1.0f);
  int pwm = round(level * 255.0f);
  if (pwm == lastDCLevel) return;
  analogWrite(motordc, pwm);
  lastDCLevel    = pwm;
  motorDCRunning = (pwm > 0);
}
