#include "art/random.h"
#include <stdint.h>

static int8_t _array[64];

void random_shuffle_array(void) {
  for (uint8_t i = 0; i < sizeof(_array); i++) {
    _array[i] = random_int(32);
  }
}

int8_t random_next(uint8_t *index) {
  *index = (*index + 1) % sizeof(_array);
  return _array[*index];
}