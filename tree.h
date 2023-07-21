#pragma once
#include "image.h"

void tree_init(void);
void tree_reset(void);
void tree_generate(void);
void tree_draw_back(struct Image *image);
void tree_draw_front(struct Image *image);
void tree_draw_branches(struct Image *image);