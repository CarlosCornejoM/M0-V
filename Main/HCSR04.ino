// HCSR04.ino

#include <Arduino.h>
#include <NewPing.h>

// Pines de Param.ino
extern const int trigPin, echoPin;

#define ITERATIONS     6
#define MAX_DISTANCE 300
#define PING_INTERVAL 10

NewPing sonar(trigPin, echoPin, MAX_DISTANCE);
float median = 0.0;

void initHCSR04() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);
}

void handleHCSR04() {
  static unsigned long lastMillis = 0;
  unsigned long now = millis();
  if (now - lastMillis < (unsigned long)PING_INTERVAL * ITERATIONS) return;
  lastMillis = now;

  float readings[ITERATIONS];
  uint8_t cnt = 0;
  for (uint8_t i=0; i<ITERATIONS; i++) {
    unsigned int d = sonar.ping();
    float dist = (d*0.0343F)/2.0F;
    if (d>0 && dist<=MAX_DISTANCE) readings[cnt++] = dist;
    delay(PING_INTERVAL);
  }
  if (!cnt) return;

  for (uint8_t i=1; i<cnt; i++) {
    float key = readings[i];
    int8_t j = i-1;
    while (j>=0 && readings[j]>key) {
      readings[j+1] = readings[j];
      j--;
    }
    readings[j+1] = key;
  }
  median = readings[cnt/2];
  Serial.print("[US,");
  Serial.print(median,2);
  Serial.println("]");
}

