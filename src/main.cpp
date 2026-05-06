#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HijelHID_BLEKeyboard.h>
#include <DNSServer.h>

DNSServer dnsServer;
const byte DNS_PORT = 53;

// ===== PINS =====
#define ENC_CLK 6
#define ENC_DT  7
#define ENC_SW  8

const uint8_t BUTTON_PINS[4] = {0, 1, 2, 3}; // GPIO

// ===== BLE =====
HijelHID_BLEKeyboard keyboard("ESP32C3 MacroPad", "DIY", 100);

// ===== WEB CONFIG =====
WebServer server(80);
Preferences prefs;

bool configMode = false;

// ===== BUTTON ACTIONS =====
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

uint8_t buttonActions[4] = {
  ACT_F13,
  ACT_F14,
  ACT_F15,
  ACT_F16
};

String buttonCustom[4] = {
  "TEXT:Hello",
  "CTRL+C",
  "MEDIA:MUTE",
  "CTRL+L\nTEXT:https://google.com\nENTER"
};

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

// ===== BUTTON STATE =====
bool lastButtonState[4] = {HIGH, HIGH, HIGH, HIGH};
unsigned long lastButtonMs[4] = {0, 0, 0, 0};
const unsigned long debounceMs = 35;

// ===== ENCODER STATE =====
int lastClk = HIGH;
bool lastEncSw = HIGH;
unsigned long lastEncSwMs = 0;

// ===== CONFIG =====
void loadConfig() {
  prefs.begin("macropad", true);

  for (int i = 0; i < 4; i++) {
    buttonActions[i] = prefs.getUChar(("b" + String(i) + "a").c_str(), buttonActions[i]);
    buttonCustom[i] = prefs.getString(("b" + String(i) + "c").c_str(), buttonCustom[i]);
  }

  prefs.end();
}

void saveConfig() {
  prefs.begin("macropad", false);

  for (int i = 0; i < 4; i++) {
    prefs.putUChar(("b" + String(i) + "a").c_str(), buttonActions[i]);
    prefs.putString(("b" + String(i) + "c").c_str(), buttonCustom[i]);
  }

  prefs.end();
}

// ===== HID HELPERS =====
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
  line.toUpperCase();

  if (line.length() == 0) return;

  if (line.startsWith("TEXT:")) {
    String text = line.substring(5);
    typeText(text);
    return;
  }

  if (line.startsWith("DELAY:")) {
    int ms = line.substring(6).toInt();
    delay(ms);
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
      delay(40);
      keyboard.releaseAll();
      break;

    case ACT_ALT_TAB:
      keyboard.press(KEY_LALT);
      keyboard.press(KEY_TAB);
      delay(40);
      keyboard.releaseAll();
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

// ===== ENCODER =====
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

// ===== BUTTONS =====
void handleButtons() {
  for (int i = 0; i < 4; i++) {
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

// ===== WEB UI =====
String buildPage() {
  String html;

  html += "<!doctype html><html><head>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>ESP32 MacroPad</title>";
  html += "<style>";
  html += "body{font-family:Arial;margin:20px;background:#111;color:#eee}";
  html += "select,textarea,button{font-size:16px;padding:8px;margin:8px 0;width:100%;box-sizing:border-box}";
  html += "textarea{height:110px;background:#000;color:#0f0;border:1px solid #555}";
  html += ".card{background:#222;padding:16px;border-radius:12px;margin-bottom:12px}";
  html += ".hint{font-size:13px;color:#aaa}";
  html += "</style>";

  html += "<script>";
  html += "function toggleCustom(i){";
  html += "var s=document.getElementById('a'+i);";
  html += "var t=document.getElementById('c'+i);";
  html += "t.style.display=(s.value=='99')?'block':'none';";
  html += "}";
  html += "window.onload=function(){for(let i=0;i<4;i++)toggleCustom(i);};";
  html += "</script>";

  html += "</head><body>";
  html += "<h2>ESP32-C3 MacroPad Config</h2>";
  html += "<form method='POST' action='/save'>";

  for (int b = 0; b < 4; b++) {
    html += "<div class='card'>";
    html += "<h3>Button " + String(b + 1) + "</h3>";

    html += "<select id='a" + String(b) + "' name='b" + String(b) + "_action' onchange='toggleCustom(" + String(b) + ")'>";

    uint8_t actions[] = {
      ACT_DISABLED,
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
      ACT_CUSTOM
    };

    for (uint8_t i = 0; i < sizeof(actions); i++) {
      uint8_t a = actions[i];
      html += "<option value='" + String(a) + "'";
      if (buttonActions[b] == a) html += " selected";
      html += ">";
      html += actionName(a);
      html += "</option>";
    }

    html += "</select>";

    html += "<textarea id='c" + String(b) + "' name='b" + String(b) + "_custom'>";
    html += buttonCustom[b];
    html += "</textarea>";

    html += "<div class='hint'>";
    html += "Examples:<br>";
    html += "TEXT:Hello<br>";
    html += "CTRL+L<br>";
    html += "TEXT:https://google.com<br>";
    html += "ENTER<br>";
    html += "DELAY:500<br>";
    html += "MEDIA:MUTE";
    html += "</div>";

    html += "</div>";
  }

  html += "<button type='submit'>Save and reboot</button>";
  html += "</form>";
  html += "<p class='hint'>Encoder is fixed: volume / mute.</p>";
  html += "</body></html>";

  return html;
}

void handleRoot() {
  server.send(200, "text/html", buildPage());
}

void handleSave() {
  for (int i = 0; i < 4; i++) {
    String actionName = "b" + String(i) + "_action";
    String customName = "b" + String(i) + "_custom";

    if (server.hasArg(actionName)) {
      buttonActions[i] = server.arg(actionName).toInt();
    }

    if (server.hasArg(customName)) {
      buttonCustom[i] = server.arg(customName);
    }
  }

  saveConfig();

  server.send(200, "text/html",
       "<h2>Saved. Rebooting...</h2><p>You can close this page.</p>");

  delay(1000);
  ESP.restart();
}

void startConfigMode() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-MacroPad-Config", "12345678");

  IPAddress apIP = WiFi.softAPIP();

  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", HTTP_GET, handleRoot);

  // Captive portal redirects
  server.on("/generate_204", HTTP_GET, handleRoot);       // Android
  server.on("/gen_204", HTTP_GET, handleRoot);            // Android/Chrome
  server.on("/hotspot-detect.html", HTTP_GET, handleRoot);// iOS/macOS
  server.on("/connecttest.txt", HTTP_GET, handleRoot);    // Windows
  server.on("/ncsi.txt", HTTP_GET, handleRoot);           // Windows

  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  server.on("/save", HTTP_POST, handleSave);

  server.begin();
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);

  for (int i = 0; i < 4; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }

  delay(300);

  loadConfig();

  // Config mode: зажать кнопку энкодера при включении
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

// ===== LOOP =====
void loop() {
  if (configMode) {
    dnsServer.processNextRequest();
    server.handleClient();
    delay(2);
    return;
  }

  handleEncoder();
  handleButtons();
}