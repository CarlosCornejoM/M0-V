#include <Servo.h>
#include <AccelStepper.h>

// —————— Parámetros externos (de Param.ino) ——————
extern const int enA, enB, in1, in2, in3, in4;
extern Servo servo1, servo2;
extern const int servo1pin, servo2pin;

// —————— Configuración del stepper ——————
#define MotorInterfaceType 4
const float stepPerRevolution = 2048.0;  // pasos/rev eje de salida
const float MAX_RPM           = 20.0;   // tope en RPM
const float MAX_ACCEL         = 500.0;  // pasos/s², ajusta a tu gusto

// Pines según tu ULN2003 wiring: IN1→pin, IN2→pin, etc.
AccelStepper myStepper(MotorInterfaceType, 8, 10, 9, 1);

// —————— Inicialización ——————
void initSteppers(){
  // Fijamos aceleración y velocidad punta
  myStepper.setAcceleration(MAX_ACCEL);
  float maxStepsPerSec = (MAX_RPM / 60.0) * stepPerRevolution;
  myStepper.setMaxSpeed(maxStepsPerSec);
  myStepper.setSpeed(0);
}

// —————— Control por RPM ——————
/**
 * @param rpm  Velocidad en RPM (-MAX_RPM..+MAX_RPM)
 */
void avanzarSteppers(float rpm){
  // Convertimos RPM a pasos/segundo
  float stepsPerSec = (rpm / 60.0) * stepPerRevolution;
  // Ajustamos como objetivo de velocidad
  myStepper.setSpeed(stepsPerSec);
}

// —————— Ejecución en bucle ——————
/**
 * Debe llamarse en cada iteración de loop() para aplicar
 * aceleración y mantener la velocidad objetivo.
 */
void stepper(){
  myStepper.run();  
}

// —————— Resto de motores y servos ——————
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
  detener();
}

void avanzar(int velocidad, int tiempo, int motor) {
  detener();
  velocidad = constrain(velocidad, -255, 255);
  if (motor == 1 || motor == 3) {
    digitalWrite(in1, velocidad >= 0 ? HIGH : LOW);
    digitalWrite(in2, velocidad >= 0 ? LOW  : HIGH);
    analogWrite(enA, abs(velocidad));
  }
  if (motor == 2 || motor == 3) {
    digitalWrite(in3, velocidad >= 0 ? HIGH : LOW);
    digitalWrite(in4, velocidad >= 0 ? LOW  : HIGH);
    analogWrite(enB, abs(velocidad));
  }
  delay(tiempo);
  detener();
}

void detener() {
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
  Serial.print("Servos -> "); Serial.print(a1);
  Serial.print("°, "); Serial.print(a2); Serial.println("°");
}
