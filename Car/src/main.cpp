#include <Arduino.h>
#include <Bluepad32.h>
#include <ESP32Servo.h>

Servo myServo;
ControllerPtr myControllers[BP32_MAX_GAMEPADS];

//Pin Definitions
const int servoPin = 27;
const int IN1 = 26;
const int IN2 = 25;
const int IN3 = 33;
const int IN4 = 32;

//Stepper Motor Variables
unsigned long lastStepTime = 0;
int stepNumber = 0;
int loopForward = 0; 

//Constants for Motion Range
const int servoMin = 100;
const int servoMax = 165;
const int servoCenter = (servoMin+servoMax)/2;
const unsigned long stepInterval = 25;
int servoAngle = 145;

// Non-blocking timing for smooth servo stepping
unsigned long lastServoMove = 0;

//Stepper Motor Patterns
const int sequence[4][4] = {
  {1, 0, 0, 1},
  {1, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 1}
};

//Move Stepper Motor
void stepForward(int direction) {
  if (micros() - lastStepTime >= stepInterval) {
    lastStepTime = micros();

    // Write current step pattern to motor pins
    digitalWrite(IN1, sequence[stepNumber][0]);
    digitalWrite(IN2, sequence[stepNumber][1]);
    digitalWrite(IN3, sequence[stepNumber][2]);
    digitalWrite(IN4, sequence[stepNumber][3]);

    // Advance step pointer forward
    stepNumber=stepNumber+direction;

    // Loop sequence back to 0 after reaching step 3
    if (stepNumber < 0) {
      stepNumber = 3;
    }
    if (stepNumber > 3) {
      stepNumber = 0;
    }
    if (direction == 0) {
      digitalWrite(IN1, 0);
      digitalWrite(IN2, 0);
      digitalWrite(IN3, 0);
      digitalWrite(IN4, 0);
    }
  }
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

  //Bluetooth Controller Setup
  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();
  BP32.enableVirtualDevice(false);

  //Stepper Motor Outputs
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}



void loop() {
  //Bluetooth Connection
  BP32.update();
  
  if (loopForward==1) {
    stepForward(1);
  } else if (loopForward==-1) {
    stepForward(-1);
  } else {
    stepForward(0);
  }

  //Writing DC Motor EN
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
          loopForward=1;
          Serial.println("D-Pad: UP  | Forward");
        } 
        else if (dpad & DPAD_DOWN) {
          //Bottom D-Pad Pressed : Begin Movement Reverse
          loopForward=-1;
          Serial.println("D-Pad: DOWN  | Reverse");
        }
      }

      // Face Buttons
      if (ctl->x()) { // X Button Pressed : Center Robot
         Serial.println("Button X Pressed | Centering!");
         servoAngle=145;
      }
      else if (ctl->y()) { // Y Button Pressed : Stop Movement
         Serial.println("Button Y Pressed | Stopping!");
         loopForward=0;
      }     
    }
  }

  delay(5);
}