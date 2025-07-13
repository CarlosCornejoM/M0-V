// PID.ino — Control PID para dos steppers usando el ángulo 'pitch' del MPU6050

#include <Wire.h>             // Necesario para leer 'pitch' si está en el mismo proyecto
#include <AccelStepper.h>     // Para declarar extern setStepperRPM()

// Externs de Param.ino y tu módulo MPU/Steppers
extern float pitch;           // Ángulo medido por el MPU (°)
extern float setPoint;        // Punto de consigna (°)
extern float Kp, Ki, Kd;      // Ganancias PID
extern float MAX_RPM;         // Límite máximo de RPM para los steppers

// Prototipo de la función de control de steppers
void setStepperRPM(int motor, float rpm);

// Estado interno del PID
static float integral      = 0.0f;
static float previousError = 0.0f;

// -------------------------------------------------------------------------
// Inicializa el PID (se llama en setup())
void initPID() {
  integral      = 0.0f;
  previousError = 0.0f;
}

// Resetea el estado interno del PID (útil cuando cambies setPoint)
void resetPIDState() {
  integral      = 0.0f;
  previousError = 0.0f;
}

// -------------------------------------------------------------------------
// Calcula la diferencia angular mínima entre target y current, normalizada
// al rango [-180°, +180°].
static float angleError(float target, float current) {
  float err = target - current;
  if (err >  180.0f) err -= 360.0f;
  if (err < -180.0f) err += 360.0f;
  return err;
}

// -------------------------------------------------------------------------
// Debe llamarse periódicamente tras actualizar 'pitch'.
// Calcula el PID y ajusta la velocidad de ambos steppers.
void handlePID() {
  // Descartar lecturas inválidas o demasiado cercanas a cero
  if (fabs(pitch) < 0.1f) return;

  // 1) Error angular “wrappeado”
  float error = angleError(setPoint, pitch);

  // 2) Término integral (con tope para evitar wind‑up)
  integral += error;
  integral = constrain(integral, -1000.0f, 1000.0f);

  // 3) Término derivativo
  float derivative = error - previousError;

  // 4) Salida PID
  float output = Kp * error
               + Ki * integral
               + Kd * derivative;

  previousError = error;

  // 5) Convertir salida a RPM y limitar
  float rpm = constrain(output, -MAX_RPM, MAX_RPM);

  // 6) Aplicar a ambos steppers
  setStepperRPM(1, rpm);
  setStepperRPM(2, -rpm);

  // 7) Debug por serie (opcional)
  Serial.print("[PID,err=");
  Serial.print(error,  2);
  Serial.print(",int=");
  Serial.print(integral, 1);
  Serial.print(",der=");
  Serial.print(derivative, 2);
  Serial.print(",out=");
  Serial.print(output, 2);
  Serial.print(",rpm=");
  Serial.print(rpm, 1);
  Serial.println("]");
}
