#pragma once

#if __has_include("local_pins.hpp")
  #include "local_pins.hpp"
#else
  #define ENC_CLK 6
  #define ENC_DT 7
  #define ENC_SW 8

  #define BUTTON_COUNT 4

  const int BUTTON_PINS[BUTTON_COUNT] = {0, 1, 2, 3};
#endif