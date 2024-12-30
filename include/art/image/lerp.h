#pragma once
#include <stdint.h>

static inline float lerp(int16_t x0, int16_t x1, float t) {
  return x0 * (1.0 - t) + x1 * t;
}