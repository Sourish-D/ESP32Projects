#include <Arduino.h>
#include <ESP32Servo.h>

Servo myServo;

const int potPin = 33;
const int servoPin = 4;

void setup() {
  myServo.attach(servoPin);
  pinMode(potPin, INPUT);
}

void loop() {
  int potValue = analogRead(potPin);
  int angle = map(potValue, 0, 4095, 180, 0); //180 to 0 because the directions are reversed
  myServo.write(angle);
  delay(50);
}
