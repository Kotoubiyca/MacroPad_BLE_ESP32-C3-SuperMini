#pragma once

// 0 = serial print logs on, 1 = serial print logs off
#define DEBUG_ENABLED 1

#if DEBUG_ENABLED
  #define DBG(x) Serial.print(x)
  #define DBGLN(x) Serial.println(x)
  #define DBGF(...) Serial.printf(__VA_ARGS__)
#else
  #define DBG(x)
  #define DBGLN(x)
  #define DBGF(...)
#endif