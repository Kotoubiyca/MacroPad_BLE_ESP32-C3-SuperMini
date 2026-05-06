#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>

#include "state.hpp"
#include "actions.hpp"
#include "config.hpp"
#include "debug.hpp"

DNSServer dnsServer;
const byte DNS_PORT = 53;

String htmlEscape(String value) {
  value.replace("&", "&amp;");
  value.replace("<", "&lt;");
  value.replace(">", "&gt;");
  value.replace("\"", "&quot;");
  return value;
}

String sleepOption(unsigned long value, const String& label) {
  String html = "<option value='" + String(value) + "'";

  if (sleepTimeoutMs == value) {
    html += " selected";
  }

  html += ">";
  html += label;
  html += "</option>";

  return html;
}

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
  html += ".hint{font-size:13px;color:#aaa;line-height:1.4}";
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

  html += "<div class='card'>";
  html += "<h3>Sleep mode</h3>";
  html += "<label for='sleepTimeout'>Sleep timeout</label>";
  html += "<select id='sleepTimeout' name='sleepTimeout'>";
  html += sleepOption(0, "Disabled");
  html += sleepOption(120000, "2 minutes");
  html += sleepOption(300000, "5 minutes");
  html += sleepOption(600000, "10 minutes");
  html += sleepOption(900000, "15 minutes");
  html += sleepOption(1800000, "30 minutes");
  html += "</select>";
  html += "<div class='hint'>";
  html += "Recommended: 2 minutes minimum. Use 5 minutes while debugging over USB.";
  html += "</div>";
  html += "</div>";

  for (int b = 0; b < BUTTON_COUNT; b++) {
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

      if (buttonActions[b] == a) {
        html += " selected";
      }

      html += ">";
      html += actionName(a);
      html += "</option>";
    }

    html += "</select>";

    html += "<textarea id='c" + String(b) + "' name='b" + String(b) + "_custom'>";
    html += htmlEscape(buttonCustom[b]);
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
  for (int i = 0; i < BUTTON_COUNT; i++) {
    String actionName = "b" + String(i) + "_action";
    String customName = "b" + String(i) + "_custom";

    if (server.hasArg(actionName)) {
      buttonActions[i] = server.arg(actionName).toInt();
    }

    if (server.hasArg(customName)) {
      buttonCustom[i] = server.arg(customName);
    }
  }

  if (server.hasArg("sleepTimeout")) {
    sleepTimeoutMs = server.arg("sleepTimeout").toInt();
  }

  saveConfig();

  server.send(200, "text/html", "<h2>Saved. Rebooting...</h2>");

  delay(1000);
  ESP.restart();
}

void startConfigMode() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-MacroPad-Config", "12345678");

  IPAddress apIP = WiFi.softAPIP();

  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", HTTP_GET, handleRoot);

  server.on("/generate_204", HTTP_GET, handleRoot);
  server.on("/gen_204", HTTP_GET, handleRoot);
  server.on("/hotspot-detect.html", HTTP_GET, handleRoot);
  server.on("/connecttest.txt", HTTP_GET, handleRoot);
  server.on("/ncsi.txt", HTTP_GET, handleRoot);

  server.on("/save", HTTP_POST, handleSave);

  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  server.begin();

  DBGLN("Config mode started");
  DBGLN("AP: ESP32-MacroPad-Config");
  DBGLN("Password: 12345678");
  DBGLN("Open: http://192.168.4.1");
}

void handleConfigWeb() {
  dnsServer.processNextRequest();
  server.handleClient();
}