#pragma once

#include <Arduino.h>
#include "pins.hpp"
#include "debug.hpp"

static unsigned long lastActivityMs = 0;
static bool softSleepMode = false;

const uint32_t ACTIVE_CPU_MHZ = 160;
const uint32_t IDLE_CPU_MHZ = 80;

inline void initSleepTimer() {
  lastActivityMs = millis();
  softSleepMode = false;
  setCpuFrequencyMhz(ACTIVE_CPU_MHZ);
}

inline void markActivity(const char* source = "unknown") {
  lastActivityMs = millis();

  if (softSleepMode) {
    softSleepMode = false;
    setCpuFrequencyMhz(ACTIVE_CPU_MHZ);

    DBGF("[WAKE] Exit soft sleep from ", source);

    delay(30);
  }
}

inline void enterSoftSleep() {
  if (softSleepMode) return;

  softSleepMode = true;
  setCpuFrequencyMhz(IDLE_CPU_MHZ);

  DBGF("Enter soft sleep");
}

inline void handleAutoSleep() {
  unsigned long idle = millis() - lastActivityMs;

  if (!softSleepMode && millis() - lastActivityMs >= sleepTimeoutMs) {
    DBGF("[SLEEP] -> entering soft sleep");

    enterSoftSleep();
  }

  if (softSleepMode) {
    delay(20);
  }
}
