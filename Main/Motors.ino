// Motors.ino

#include <Servo.h>
#include <AccelStepper.h>

// Externs de Param.ino
extern const int enA, enB, in1, in2, in3, in4;
extern Servo servo1, servo2;
extern const int servo1pin, servo2pin;
extern const int motordc;
extern float MAX_RPM;

// Define pin connections para el driver de paso a paso
const int dirPin     = 2;
const int stepPin    = 3;
const int enablePin  = A0;    // A0 = ENABLE del driver

// Parámetros del stepper
const float STEPS_PER_REV = 2048.0; // pasos por revolución (p.ej. 28BYJ‑48 con reduccion)

AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);

bool stepperEnabled = false;
float currentRPM    = 0.0;

// ----------------------- Inicialización -----------------------
void initSteppers() {
  pinMode(enablePin, OUTPUT);
  disableSteppers();
  
  stepper.setMaxSpeed( (MAX_RPM/60.0) * STEPS_PER_REV );   // velocidad máxima en pasos/s
  stepper.setAcceleration( 10 * (MAX_RPM/60.0) * STEPS_PER_REV ); // aceleración en pasos/s²
  stepper.setSpeed(0);
}

// Habilita el driver (LOW = habilitado en muchos módulos)
void enableSteppers() {
  digitalWrite(enablePin, LOW);
  stepperEnabled = true;
}

// Deshabilita el driver (HIGH = deshabilitado)
void disableSteppers() {
  digitalWrite(enablePin, HIGH);
  stepperEnabled = false;
  stepper.setSpeed(0);
  currentRPM = 0.0;
}

// Ajusta la velocidad en RPM; si rpm == 0 detiene y deshabilita
void setStepperRPM(float rpm) {
  rpm = constrain(rpm, -MAX_RPM, MAX_RPM);
  if (fabs(rpm) < 0.01f) {
    // detener completamente
    disableSteppers();
    currentRPM = 0.0;
    return;
  }

  if (!stepperEnabled) {
    enableSteppers();
  }

  currentRPM = rpm;
  // convertir RPM a pasos/segundo
  float stepsPerSec = (rpm / 60.0f) * STEPS_PER_REV;
  Serial.println(stepsPerSec);
  stepper.setSpeed(stepsPerSec);
}

// Debe llamarse desde loop(); ejecuta un paso si hay velocidad
// Debe llamarse desde loop(); ejecuta un paso si hay velocidad
void runSteppers() {
  static float lastSentRPM    = -1.0f;       // último RPM enviado
  static unsigned long lastTs = 0;
  unsigned long now = millis();

  float rpmToSend = stepperEnabled ? currentRPM : 0.0f;

    // Si cambió al menos 1 RPM, lo imprimimos
    if (fabs(rpmToSend - lastSentRPM) > 1) {
      Serial.print("[RPM,");
      Serial.print(rpmToSend, 1);    // 1 decimal
      Serial.println("]");
      lastSentRPM = rpmToSend;
    }

  // ---- mover el stepper solo si está habilitado ----
  if (stepperEnabled) {
    stepper.runSpeed();
  }
}


// ------------------- Control DC y servos -------------------
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


