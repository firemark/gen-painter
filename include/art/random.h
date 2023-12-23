#pragma once
#include "image/image.h"
#include <stdint.h>

/// @warning This functions should be implemented in main!
uint32_t art_random();

void random_shuffle_array(void);

/// @brief get next random value from random array and inrecement index.
/// @param index index of random array which will be updated after returning number.
/// @return value from 0 to 32
int8_t random_next(uint8_t *index);

static inline float random_range(int16_t start, int16_t end) {
  return (start + (int16_t)(art_random() % (end - start))) / 1000.0;
}

static inline int16_t random_int(int16_t x) { return art_random() % x; }