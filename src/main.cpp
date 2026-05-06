#include <Arduino.h>

#include "pins.hpp"
#include "state.hpp"
#include "actions.hpp"
#include "config.hpp"
#include "web.hpp"

void setup() {
  Serial.begin(115200);

  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);

  for (int i = 0; i < BUTTON_COUNT; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }

  delay(300);

  loadConfig();

  if (digitalRead(ENC_SW) == LOW) {
    configMode = true;
    startConfigMode();
    return;
  }

  keyboard.setLogLevel(HIDLogLevel::Normal);
  keyboard.begin();

  lastClk = digitalRead(ENC_CLK);

  Serial.println("Normal BLE mode started");
}

void loop() {
  if (configMode) {
    handleConfigWeb();
    delay(2);
    return;
  }

  handleEncoder();
  handleButtons();
}