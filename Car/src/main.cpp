#include <Arduino.h>
#include <Bluepad32.h>
#include <ESP32Servo.h>

Servo myServo;
ControllerPtr myControllers[BP32_MAX_GAMEPADS];


//Pin Definitions
const int servoPin = 5;
const int dcPin1 = 12;
const int dcPin2 = 14;
const int dcPinEN = 13;
const int ENpwmChannel = 6;

//Constants for Motion Range
const int servoMin = 100;
const int servoMax = 165;
const int servoCenter = (servoMin+servoMax)/2;
const unsigned long stepInterval = 50; // Move every 50ms while button is held
int servoAngle = 145; // Start at center position (90 degrees)

// Non-blocking timing for smooth servo stepping
unsigned long lastServoMove = 0;

//Motor Movement
void setMotorForward(bool fwd) {
  digitalWrite(dcPin1, fwd ? 0 : 1);
  digitalWrite(dcPin2, fwd ? 0 : 0);
}

//Servo Movement
void moveServo(int angle, String direction ) {
  servoAngle += angle;
  servoAngle = constrain(servoAngle, servoMin, servoMax); // Keep within 0 - 180 range
  Serial.printf("D-Pad: %s | New Angle: %d\n", direction.c_str(), servoAngle);
  lastServoMove = millis();
}

//Controller Connections
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
  //Being Serial Monitor
  Serial.begin(115200);
  Serial.println("Starting Bluepad32 with Servo...");

  // Allow allocation of timer for ESP32Servo
  ESP32PWM::allocateTimer(0);

  //Servo PWM
  myServo.setPeriodHertz(50);
  myServo.attach(servoPin, 500, 2400);
  myServo.write(servoAngle);
  
  //DC Motor Pin Modes
  pinMode(dcPin1, OUTPUT);
  pinMode(dcPin2, OUTPUT);
  pinMode(dcPinEN, OUTPUT);

  //DC Motor EN PWM
  ledcSetup(ENpwmChannel, 1000, 8);
  ledcAttachPin(dcPinEN, ENpwmChannel);

  //Bluetooth Controller Setup
  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();
  BP32.enableVirtualDevice(false);
}

void loop() {
  //Bluetooth Connection
  BP32.update();
  
  //Writing DC Motor EN
  ledcWrite(ENpwmChannel, 255);
  myServo.write(servoAngle);

  //Connecting to Controller
  for (auto ctl : myControllers) {
    if (ctl && ctl->isConnected() && ctl->hasData()) {
      uint8_t dpad = ctl->dpad();

      // Check non-blocking timer before stepping the servo
      if (millis() - lastServoMove >= stepInterval) {
        //D-Pad Buttons
        if (dpad & DPAD_LEFT) {
          //Left D-Pad Pressed : Turn 10 Degrees Left
          moveServo(10, "Left");
        } 
        else if (dpad & DPAD_RIGHT) {
          //Right D-Pad Pressed : Turn 10 Degrees Right
          moveServo(-10, "Right");
        }
        else if (dpad & DPAD_UP) {
          //Top D-Pad Pressed : Begin Movement Forward
          setMotorForward(true);
        } 
        else if (dpad & DPAD_DOWN) {
          //Bottom D-Pad Pressed : Begin Movement Reverse
          setMotorForward(false);
        }
      }

      // Face Buttons
      if (ctl->x()) { // X Button Pressed : Center Robot
         Serial.println("Button X Pressed | Centering!");
         servoAngle=145;
      }        
    }
  }

  delay(15);
}