#pragma once
#include "image.h"

uint8_t tree_init(void);
void tree_reset(void);
void tree_generate(void);
void tree_draw_back(struct Image *image);
void tree_draw_front(struct Image *image);
void tree_draw_branches(struct Image *image);