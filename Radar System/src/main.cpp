#include <Arduino.h>
#include <ESP32Servo.h>

Servo myServo;

const int trigPin = 5;
const int echoPin = 19;
const int servoPin = 13;

long duration;
float distance;
// put function declarations here:

float getDistance();

void setup() {
  Serial.begin(9600);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  myServo.attach(servoPin);
  delay(500);
}
 
void loop() {
  
  // Sweep 0 → 180
  for (int angle = 0; angle <= 180; angle += 2) {
    myServo.write(angle);
    delay(30);

    distance = getDistance();

    Serial.print(angle);
    Serial.print(",");
    Serial.println(distance);
  }

  // Sweep back 180 → 0
  for (int angle = 180; angle >= 0; angle -= 2) {
    myServo.write(angle);
    delay(30);

    distance = getDistance();

    Serial.print(angle);
    Serial.print(",");
    Serial.println(distance);
  }
}

// put function definitions here:
float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) return -1;

  return duration * 0.034 / 2;
}