#pragma once
#include "image.h"

void clouds_init(void);
void clouds_reset(void);
void clouds_generate(void);
void clouds_draw(struct Image *image);