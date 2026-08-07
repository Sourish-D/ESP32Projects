#include <Arduino.h>

const int potPin = 33;
const int ledPin = 4;

void setup() {
  Serial.begin(115200);
  pinMode(potPin, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  Serial.println(analogRead(potPin));
  int potValue = analogRead(potPin);
  analogWrite(ledPin, potValue / 16);
  delay(50);
}
