#pragma once
#include "art/image/image.h"

struct Tree {
  float main_branch_ratio;
  float side_branch_ratio;
  float curvy_ratio;
  uint8_t top_start;
  float top_rot;
  float bottom_rot;
};

uint8_t tree_init(void);
void tree_reset(void);
void tree_generate(struct Point p, uint16_t height, struct Tree *tree);
void tree_draw_back(struct Image *image);
void tree_draw_front(struct Image *image);
void tree_draw_branches(struct Image *image);