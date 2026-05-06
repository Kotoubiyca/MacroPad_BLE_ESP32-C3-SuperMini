#pragma once
#include <Arduino.h>
#include "state.hpp"

const char* actionName(uint8_t action) {
  switch (action) {
    case ACT_DISABLED: return "Disabled";
    case ACT_F13: return "F13";
    case ACT_F14: return "F14";
    case ACT_F15: return "F15";
    case ACT_F16: return "F16";
    case ACT_CTRL_C: return "Ctrl + C";
    case ACT_CTRL_V: return "Ctrl + V";
    case ACT_CTRL_Z: return "Ctrl + Z";
    case ACT_CTRL_SHIFT_ESC: return "Ctrl + Shift + Esc";
    case ACT_ALT_TAB: return "Alt + Tab";
    case ACT_WIN_D: return "Win + D";
    case ACT_ENTER: return "Enter";
    case ACT_ESC: return "Esc";
    case ACT_MEDIA_PLAY: return "Play / Pause";
    case ACT_MEDIA_NEXT: return "Next Track";
    case ACT_MEDIA_PREV: return "Previous Track";
    case ACT_CUSTOM: return "Custom text / macro";
    default: return "Unknown";
  }
}

void tapKey(uint8_t key) {
  keyboard.press(key);
  delay(35);
  keyboard.releaseAll();
}

void tapMedia(uint16_t mediaKey) {
  keyboard.press(mediaKey);
  delay(35);
  keyboard.release(mediaKey);
}

void tapCombo(uint8_t key, uint8_t modifiers) {
  keyboard.press(modifiers);
  keyboard.press(key);
  delay(45);
  keyboard.releaseAll();
}

void typeText(String text) {
  for (int i = 0; i < text.length(); i++) {
    keyboard.print(text[i]);
    delay(8);
  }
}

void runMacroLine(String line) {
  line.trim();

  if (line.length() == 0) return;

  if (line.startsWith("TEXT:")) {
    String text = line.substring(5);
    typeText(text);
    return;
  }

  line.toUpperCase();

  if (line.startsWith("DELAY:")) {
    delay(line.substring(6).toInt());
    return;
  }

  if (line == "ENTER") tapKey(KEY_RETURN);
  else if (line == "ESC") tapKey(KEY_ESCAPE);
  else if (line == "TAB") tapKey(KEY_TAB);
  else if (line == "SPACE") tapKey(KEY_SPACE);
  else if (line == "BACKSPACE") tapKey(KEY_BACKSPACE);

  else if (line == "F13") tapKey(KEY_F13);
  else if (line == "F14") tapKey(KEY_F14);
  else if (line == "F15") tapKey(KEY_F15);
  else if (line == "F16") tapKey(KEY_F16);

  else if (line == "CTRL+C") tapCombo(KEY_C, KEY_MOD_LCTRL);
  else if (line == "CTRL+V") tapCombo(KEY_V, KEY_MOD_LCTRL);
  else if (line == "CTRL+Z") tapCombo(KEY_Z, KEY_MOD_LCTRL);
  else if (line == "WIN+D") tapCombo(KEY_D, KEY_MOD_LGUI);
  else if (line == "ALT+TAB") tapCombo(KEY_TAB, KEY_MOD_LALT);

  else if (line == "CTRL+SHIFT+ESC") {
    keyboard.press(KEY_LCTRL);
    keyboard.press(KEY_LSHIFT);
    keyboard.press(KEY_ESCAPE);
    delay(45);
    keyboard.releaseAll();
  }

  else if (line == "MEDIA:PLAY") tapMedia(MEDIA_PLAY_PAUSE);
  else if (line == "MEDIA:MUTE") tapMedia(MEDIA_MUTE);
  else if (line == "MEDIA:NEXT") tapMedia(MEDIA_NEXT_TRACK);
  else if (line == "MEDIA:PREV") tapMedia(MEDIA_PREV_TRACK);
  else if (line == "MEDIA:VOLUP") tapMedia(MEDIA_VOLUME_UP);
  else if (line == "MEDIA:VOLDOWN") tapMedia(MEDIA_VOLUME_DOWN);
}

void runCustom(String macro) {
  macro.replace("\r", "");
  macro.replace(";", "\n");

  int start = 0;

  while (start < macro.length()) {
    int end = macro.indexOf('\n', start);
    if (end == -1) end = macro.length();

    String line = macro.substring(start, end);
    runMacroLine(line);

    delay(60);
    start = end + 1;
  }
}

void runAction(uint8_t action, int buttonIndex) {
  if (!keyboard.isPaired()) return;

  if (action == ACT_CUSTOM) {
    runCustom(buttonCustom[buttonIndex]);
    return;
  }

  switch (action) {
    case ACT_DISABLED:
      break;

    case ACT_F13:
      tapKey(KEY_F13);
      break;

    case ACT_F14:
      tapKey(KEY_F14);
      break;

    case ACT_F15:
      tapKey(KEY_F15);
      break;

    case ACT_F16:
      tapKey(KEY_F16);
      break;

    case ACT_CTRL_C:
      tapCombo(KEY_C, KEY_MOD_LCTRL);
      break;

    case ACT_CTRL_V:
      tapCombo(KEY_V, KEY_MOD_LCTRL);
      break;

    case ACT_CTRL_Z:
      tapCombo(KEY_Z, KEY_MOD_LCTRL);
      break;

    case ACT_CTRL_SHIFT_ESC:
      keyboard.press(KEY_LCTRL);
      keyboard.press(KEY_LSHIFT);
      keyboard.press(KEY_ESCAPE);
      delay(45);
      keyboard.releaseAll();
      break;

    case ACT_ALT_TAB:
      tapCombo(KEY_TAB, KEY_MOD_LALT);
      break;

    case ACT_WIN_D:
      tapCombo(KEY_D, KEY_MOD_LGUI);
      break;

    case ACT_ENTER:
      tapKey(KEY_RETURN);
      break;

    case ACT_ESC:
      tapKey(KEY_ESCAPE);
      break;

    case ACT_MEDIA_PLAY:
      tapMedia(MEDIA_PLAY_PAUSE);
      break;

    case ACT_MEDIA_NEXT:
      tapMedia(MEDIA_NEXT_TRACK);
      break;

    case ACT_MEDIA_PREV:
      tapMedia(MEDIA_PREV_TRACK);
      break;
  }
}

void handleButtons() {
  for (int i = 0; i < BUTTON_COUNT; i++) {
    bool state = digitalRead(BUTTON_PINS[i]);

    if (state != lastButtonState[i] && millis() - lastButtonMs[i] > debounceMs) {
      lastButtonMs[i] = millis();
      lastButtonState[i] = state;

      if (state == LOW) {
        runAction(buttonActions[i], i);
      }
    }
  }
}

void handleEncoder() {
  int clk = digitalRead(ENC_CLK);

  if (clk != lastClk && clk == LOW) {
    int dt = digitalRead(ENC_DT);

    if (keyboard.isPaired()) {
      if (dt != clk) {
        tapMedia(MEDIA_VOLUME_UP);
      } else {
        tapMedia(MEDIA_VOLUME_DOWN);
      }
    }
  }

  lastClk = clk;

  bool sw = digitalRead(ENC_SW);

  if (sw != lastEncSw && millis() - lastEncSwMs > debounceMs) {
    lastEncSwMs = millis();
    lastEncSw = sw;

    if (sw == LOW && keyboard.isPaired()) {
      tapMedia(MEDIA_MUTE);
    }
  }
}