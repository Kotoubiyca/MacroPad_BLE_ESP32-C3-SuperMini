#pragma once
#include <Arduino.h>
#include <HijelHID_BLEKeyboard.h>
#include <WebServer.h>
#include <Preferences.h>
#include "pins.hpp"

HijelHID_BLEKeyboard keyboard("ESP32C3 MacroPad", "DIY", 100);
WebServer server(80);
Preferences prefs;

bool configMode = false;

enum ActionId : uint8_t {
  ACT_DISABLED = 0,
  ACT_F13,
  ACT_F14,
  ACT_F15,
  ACT_F16,
  ACT_CTRL_C,
  ACT_CTRL_V,
  ACT_CTRL_Z,
  ACT_CTRL_SHIFT_ESC,
  ACT_ALT_TAB,
  ACT_WIN_D,
  ACT_ENTER,
  ACT_ESC,
  ACT_MEDIA_PLAY,
  ACT_MEDIA_NEXT,
  ACT_MEDIA_PREV,
  ACT_CUSTOM = 99
};

uint8_t buttonActions[BUTTON_COUNT] = {
  ACT_F13,
  ACT_F14,
  ACT_F15,
  ACT_F16
};

String buttonCustom[BUTTON_COUNT] = {
  "TEXT:Hello",
  "CTRL+C",
  "MEDIA:MUTE",
  "CTRL+L\nTEXT:https://google.com\nENTER"
};

bool lastButtonState[BUTTON_COUNT] = {HIGH, HIGH, HIGH, HIGH};
unsigned long lastButtonMs[BUTTON_COUNT] = {0, 0, 0, 0};

const unsigned long debounceMs = 35;

int lastClk = HIGH;
bool lastEncSw = HIGH;
unsigned long lastEncSwMs = 0;