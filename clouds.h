#pragma once
#include "image.h"

uint8_t clouds_init(void);
void clouds_generate(void);
void clouds_draw(struct Image *image);