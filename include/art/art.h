#pragma once
#include <stdint.h>
#include "art_data.h"
#include "image/image.h"

uint8_t art_init(void);
void art_make(struct ArtData data);
void art_draw(struct Image* image);

// This functions should be implemented in main!
uint32_t art_random();