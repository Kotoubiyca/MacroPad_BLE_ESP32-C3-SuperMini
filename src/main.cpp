#include <Arduino.h>
#include <HijelHID_BLEKeyboard.h>

HijelHID_BLEKeyboard bleKeyboard("ESP32 MediaPad C3", "DIY", 50);

#define ENC_CLK 6
#define ENC_DT  7
#define ENC_SW  8

int lastCLK = HIGH;
bool lastSwState = HIGH;

void sendMediaKey(uint16_t key) {
  bleKeyboard.press(key);
  delay(30);
  bleKeyboard.release(key);
  delay(30);
}

void setup() {
  delay(1500);
  Serial.begin(115200);

  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);

  lastCLK = digitalRead(ENC_CLK);

  bleKeyboard.begin();
  Serial.println("BLE started");
}

void loop() {
  if (!bleKeyboard.isConnected()) {
    delay(100);
    return;
  }

  int currentCLK = digitalRead(ENC_CLK);

  if (currentCLK != lastCLK) {
    if (digitalRead(ENC_DT) != currentCLK) {
      Serial.println("Volume up");
      sendMediaKey(MEDIA_VOLUME_UP);
    } else {
      Serial.println("Volume down");
      sendMediaKey(MEDIA_VOLUME_DOWN);
    }

    delay(8);
  }

  lastCLK = currentCLK;

  bool swState = digitalRead(ENC_SW);

  if (lastSwState == HIGH && swState == LOW) {
    Serial.println("Mute");
    sendMediaKey(MEDIA_MUTE);
    delay(250);
  }

  lastSwState = swState;
}