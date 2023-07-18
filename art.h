#pragma once
#include "image.h"
#include <stdint.h>

void art_init(void);
void art_make(int16_t temperature, uint16_t rain_density);
void art_draw(struct Image *image);

// This functions should be implemented in main!
uint32_t art_random();