#include <Arduino.h>
#include <Bluepad32.h>

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

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
  Serial.println("Starting Bluepad32...");

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();
  BP32.enableVirtualDevice(false);
}

void loop() {
  BP32.update();

  for (auto ctl : myControllers) {
    if (ctl && ctl->isConnected() && ctl->hasData()) {
      if (ctl->a()) Serial.println("Button A Pressed!");
      if (ctl->b()) Serial.println("Button B Pressed!");

      static unsigned long lastPrint = 0;
      if (millis() - lastPrint > 500) {
        Serial.printf("LX: %d | LY: %d\n", ctl->axisX(), ctl->axisY());
        lastPrint = millis();
      }
    }
  }

  delay(15);
}