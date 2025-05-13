extern const int enA, enB, in1, in2, in3, in4;  // de Param.ino
extern Servo servo1, servo2;                   // de Param.ino

void initMotors() {
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  servo1.attach(9);
  servo2.attach(10);
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
