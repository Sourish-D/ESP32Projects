#include <Arduino.h>
#include <ESP32Servo.h>

Servo myServo;

// Compatibility wrapper: some codebases call ledcAttach(channel, pin)
// while ESP32 Arduino core provides ledcAttachPin(pin, channel).
// Provide a simple alias to avoid "ledcAttach not declared" errors.
static inline void ledcAttach(int channel, int pin) {
  ledcAttachPin(pin, channel);
}

const int pwmChannel = 6;
const int IN1 = 26;
const int IN2 = 25;
const int IN3 = 33;
const int IN4 = 32;
const int servoPin = 14;
const int mEN  = 15;
const int mIN1 = 18;
const int mIN2 = 19;

int stepNumber = 0;

unsigned long lastStepTime = 0;
int stepDelay = 1; // milliseconds between steps

int sequence[8][4] = {
  {1,0,0,1},
  {1,0,0,0},
  {1,1,0,0},
  {0,1,0,0},
  {0,1,1,0},
  {0,0,1,0},
  {0,0,1,1},
  {0,0,0,1}
};

void setup() {
  /*
  Serial.begin(9600);
  */
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(mEN, OUTPUT);
  pinMode(mIN1, OUTPUT);
  pinMode(mIN2, OUTPUT);

  digitalWrite(mIN1, HIGH);
  digitalWrite(mIN2, LOW);
  ledcSetup(pwmChannel, 1000, 8);      // 1 kHz, 8-bit
  ledcAttachPin(mEN, pwmChannel);

  myServo.attach(servoPin);
}

void moveStepper() {

  if (millis() - lastStepTime >= stepDelay) {

    lastStepTime = millis();

    digitalWrite(IN1, sequence[stepNumber][0]);
    digitalWrite(IN2, sequence[stepNumber][1]);
    digitalWrite(IN3, sequence[stepNumber][2]);
    digitalWrite(IN4, sequence[stepNumber][3]);

    stepNumber--;

    if (stepNumber < 0)
      stepNumber = 7;
  }
}

void loop() {
  myServo.write(5);

  moveStepper();

  
  /*
  int pwm = map(potValue, 0, 4095, 0, 255);
  Serial.print("Pot: ");
  Serial.print(potValue);
  Serial.print("  PWM: ");
  Serial.println(pwm);
  */
  ledcWrite(pwmChannel, 140);

}


/* Buzzer Code
#include <Arduino.h>
const int D25 = 25;

void setup() {
  pinMode(D25, OUTPUT);
}

void loop() {
  tone(D25, 1000);
  delay(25);
}
*/ 