#pragma once
#include <Arduino.h>

#define ENC_CLK 6
#define ENC_DT  7
#define ENC_SW  8

const uint8_t BUTTON_COUNT = 4;
const uint8_t BUTTON_PINS[BUTTON_COUNT] = {0, 1, 2, 3};