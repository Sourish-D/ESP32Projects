#include <Arduino.h>
#include <Bluepad32.h>
#include <ESP32Servo.h>

Servo myServo;
ControllerPtr myControllers[BP32_MAX_GAMEPADS];

const int servoPin = 5;
const int dcPin1 = 12;
const int dcPin2 = 14;
const int dcPinEN = 13;

const int pwmChannel = 6;

int servoAngle = 145; // Start at center position (90 degrees)

// Non-blocking timing for smooth servo stepping
unsigned long lastServoMove = 0;
const unsigned long stepInterval = 50; // Move every 50ms while button is held

void onConnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.printf("Connected: %s\n", ctl->getModelName().c_str());
      myControllers[i] = ctl;
      break;
    }
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.println("Controller disconnected.");
      myControllers[i] = nullptr;
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Bluepad32 with Servo...");

  // Allow allocation of all timers for ESP32Servo
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  myServo.setPeriodHertz(50);      // Standard 50Hz servo
  myServo.attach(servoPin, 500, 2400); // Attach pin with standard microsecond pulses
  myServo.write(servoAngle);
  
  pinMode(dcPin1, OUTPUT);
  pinMode(dcPin2, OUTPUT);
  pinMode(dcPinEN, OUTPUT);
  ledcSetup(pwmChannel, 1000, 8);
  ledcAttachPin(dcPinEN, pwmChannel);
  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();
  BP32.enableVirtualDevice(false);
}

void loop() {
  BP32.update();
  ledcWrite(pwmChannel, 255);
  myServo.write(servoAngle);

  for (auto ctl : myControllers) {
    if (ctl && ctl->isConnected() && ctl->hasData()) {
      uint8_t dpad = ctl->dpad();

      // Check non-blocking timer before stepping the servo
      if (millis() - lastServoMove >= stepInterval) {
        if (dpad & DPAD_LEFT) {
          servoAngle -= 10;
          servoAngle = constrain(servoAngle, 100, 165); // Keep within 0 - 180 range
          Serial.printf("D-Pad: Left | New Angle: %d\n", servoAngle);
          lastServoMove = millis();
        } 
        else if (dpad & DPAD_RIGHT) {
          servoAngle += 10;
          servoAngle = constrain(servoAngle, 100, 165); // Keep within 0 - 180 range
          Serial.printf("D-Pad: Right | New Angle: %d\n", servoAngle);
          lastServoMove = millis();
        }
        else if (dpad & DPAD_UP) {
          digitalWrite(dcPin1, 0);
          digitalWrite(dcPin2, 1);
        } 
        else if (dpad & DPAD_DOWN) {
          digitalWrite(dcPin1, 1);
          digitalWrite(dcPin2, 0);
        }
      }

      // Read face buttons
      if (ctl->a()) Serial.println("Button A Pressed!");
      if (ctl->b()) Serial.println("Button B Pressed!");
      if (ctl->x()) {
         Serial.println("Button X Pressed | Centering!");
         servoAngle=145;
      }        
      if (ctl->y()) Serial.println("Button Y Pressed!");
    }
  }

  delay(15);
}