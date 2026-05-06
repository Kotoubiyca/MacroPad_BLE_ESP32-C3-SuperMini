#pragma once
#include <Arduino.h>
#include "state.hpp"

const unsigned long DEFAULT_SLEEP_TIMEOUT_MS = 120000; // 2 min

void loadConfig() {
  prefs.begin("macropad", true);

  for (int i = 0; i < BUTTON_COUNT; i++) {
    buttonActions[i] = prefs.getUChar(("b" + String(i) + "a").c_str(), buttonActions[i]);
    buttonCustom[i] = prefs.getString(("b" + String(i) + "c").c_str(), buttonCustom[i]);
  }

  sleepTimeoutMs = prefs.getULong("sleepMs", DEFAULT_SLEEP_TIMEOUT_MS);

  prefs.end();
}

void saveConfig() {
  prefs.begin("macropad", false);

  for (int i = 0; i < BUTTON_COUNT; i++) {
    prefs.putUChar(("b" + String(i) + "a").c_str(), buttonActions[i]);
    prefs.putString(("b" + String(i) + "c").c_str(), buttonCustom[i]);
  }

  prefs.putULong("sleepMs", sleepTimeoutMs);

  prefs.end();
}