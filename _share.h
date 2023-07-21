#pragma once
#include "image.h"
#include <stdint.h>

extern int8_t _random_background_shifts[64];
extern uint8_t _background_shifts_index;
extern int16_t _temperature;

extern enum Color _background_color;
extern enum Color _leaves_color;
extern enum Color _branches_color;