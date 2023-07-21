#pragma once
#include "image.h"

struct Point bezier(float t, struct Point b[4]);

static inline float lerp(int16_t x0, int16_t x1, float t) {
  return x0 * (1.0 - t) + x1 * t;
}