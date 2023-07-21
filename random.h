#pragma once
#include "image.h"
#include <stdint.h>

// This functions should be implemented in main!
uint32_t art_random();

static inline float random_range(int16_t start, int16_t end) {
  return (start + (int16_t)(art_random() % (end - start))) / 1000.0;
}

static inline int16_t random_int(int16_t x) { return art_random() % x; }