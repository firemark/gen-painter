#include "art/tree.h"
#include "art/image/image_adv.h"
#include "art/random.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "art/_share.h"

#define LEAFES_SIZE 1024
#define BRANCHES_SIZE (1024 * 2)
#define BEZIER_LINES_HIGH 16
#define BEZIER_LINES_LOW 6

static uint16_t _branches_count;
static uint16_t _leaves_count;

static struct Line *_branches;
static struct Circle *_leaves;

static void _tree(uint8_t n, struct Tree *tree, struct Point p, int16_t w,
                  float rot);

uint8_t tree_init(void) {
  _branches = malloc(sizeof(struct Line) * BRANCHES_SIZE);
  _leaves = malloc(sizeof(struct Circle) * LEAFES_SIZE);
  return _branches != NULL && _leaves != NULL;
}

void tree_reset(void) {
  _branches_count = 0;
  _leaves_count = 0;
}

void tree_generate(struct Point p, uint16_t height, struct Tree *tree) {
  float rot = random_range(-50, 50);

  _tree(6, tree, p, height, rot);
}

void tree_draw_back(struct Image *image) {
  for (uint16_t i = 0; i < _leaves_count; i++) {
    for (uint16_t j = 0; j < 3; j++) {
      uint16_t dx = 16 - random_int(32);
      uint16_t dy = 16 - random_int(32);
      struct Circle circle = _leaves[i];
      circle.p.x += dx;
      circle.p.y += dy;
      image_draw_circle(image, &circle);
    }
  }
}

void tree_draw_front(struct Image *image) {
  for (uint16_t i = 0; i < _leaves_count; i++) {
    for (uint16_t j = 0; j < 2; j++) {
      uint16_t dx = 16 - random_int(32);
      uint16_t dy = 16 - random_int(32);
      struct Circle circle = _leaves[i];
      circle.p.x += dx;
      circle.p.y += dy;
      image_draw_circle_threshold(image, &circle, 128, TRANSPARENT);
    }
  }
}

void tree_draw_branches(struct Image *image) {
  for (uint16_t i = 0; i < _branches_count; i++) {
    struct Line *line = &_branches[i];
    image_draw_line(image, line);
    if (line->thickness > 5) {
      struct Circle circle = {
          .color = line->color,
          .d = line->thickness / 2,
          .p = line->p1,
      };
      image_draw_circle(image, &circle);
    }
  }
}

static inline float _thickness(int16_t w) { return w * w / 2048.0 + 1.0; }

static void _leaf(struct Point p, uint16_t size) {
  if (_leaves_count >= LEAFES_SIZE) {
    return;
  }

  _leaves[_leaves_count++] = (struct Circle){
      .color = _leaves_color,
      .d = size,
      .p = p,
  };
}

static void _tree(uint8_t n, struct Tree *tree, struct Point p, int16_t w,
                  float rot) {
  if (n < tree->top_start && random_int(100) > n * 8) {
    _leaf(p, w / 20 + random_int(5));
  }

  uint16_t chunks = w > 100 ? BEZIER_LINES_HIGH : BEZIER_LINES_LOW;
  if (n == 0 || _branches_count >= BRANCHES_SIZE - chunks) {
    return;
  }

  if (n > tree->top_start && random_int(100) > 70 + n * 5) {
    return;
  }

  struct Point points[4];
  points[0] = p;

  float _sin = sin(rot);
  float _cos = cos(rot);

  float r1 = random_range(200, 500);
  float r2 = r1 + random_range(200, 500);

  float dx1 = w * r1;
  float dy1 = tree->curvy_ratio * w * random_range(-1000, +1000);
  points[1] = (struct Point){
      p.x + dx1 * _sin - dy1 * _cos,
      p.y - dx1 * _cos - dy1 * _sin,
  };

  float dx2 = w * r2;
  float dy2 = tree->curvy_ratio * w * random_range(-1000, +1000);
  points[2] = (struct Point){
      p.x + dx2 * _sin - dy2 * _cos,
      p.y - dx2 * _cos - dy2 * _sin,
  };

  points[3] = (struct Point){
      p.x + w * _sin,
      p.y - w * _cos,
  };

  struct Point old_p = p;

  int16_t new_w = w * (tree->main_branch_ratio + random_range(-125, 125));

  for (uint8_t i = 1; i < chunks - 1; i++) {
    float t = (float)i / chunks;
    struct Point new_p = bezier(t, points);
    float part_w = lerp(w, new_w, t);
    _branches[_branches_count++] = (struct Line){
        .color = _branches_color,
        .thickness = _thickness(part_w),
        .p0 = old_p,
        .p1 = new_p,
    };
    old_p = new_p;
  }

  _branches[_branches_count++] = (struct Line){
      .color = _branches_color,
      .thickness = _thickness(new_w),
      .p0 = old_p,
      .p1 = points[3],
  };

  _tree(n - 1, tree, points[3], new_w, rot + random_range(-125, 125));

  float new_rot = n > tree->top_start ? tree->top_rot : tree->bottom_rot;

  float split_right = random_range(200, 800);
  struct Point side_right = bezier(split_right, points);
  float w_right = lerp(w, new_w, split_right) * tree->side_branch_ratio;
  _tree(n - 1, tree, side_right, w_right,
        rot + random_range(-125, 125) + new_rot);

  float split_left = random_range(200, 800);
  struct Point side_left = bezier(split_left, points);
  float w_left = lerp(w, new_w, split_left) * tree->side_branch_ratio;
  _tree(n - 1, tree, side_left, w_left,
        rot + random_range(-125, 125) - new_rot);
}