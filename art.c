#include "art.h"
#include "image.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define LINES_SIZE 1024
#define CIRCLES_SIZE 1024

static struct Bitmap EXAMPLE = {{
  // clang-format off
  0b11111111, 0b11111111,
  0b10000000, 0b00000001,
  0b10000000, 0b00000001,
  0b10000000, 0b00000001,
  0b10000000, 0b00000001,
  0b10000000, 0b00000001,
  0b10000000, 0b00000001,
  0b10000000, 0b00000001,
  0b10000000, 0b00000001,
  0b10000000, 0b00000001,
  0b10000000, 0b00000001,
  0b10000000, 0b00000001,
  0b10000000, 0b00000001,
  0b10000000, 0b00000001,
  0b10000000, 0b00000001,
  0b11111111, 0b11111111,
  // clang-format on
}};

static uint16_t _lines_count;
static uint16_t _circles_count;
static uint16_t _rain_density;

static enum Color _background_color;
static enum Color _leaves_color;
static enum Color _branches_color;

static struct Line *_lines;
static struct Circle *_circles;

struct Tree {
  float main_branch_ratio;
  float side_branch_ratio;
  uint8_t top_start;
  float top_rot;
  float bottom_rot;
};

static inline float _random(int16_t start, int16_t end) {
  return (start + (int16_t)(art_random() % (end - start))) / 1000.0;
}

static inline int16_t _random_int(int16_t x) {
  return art_random() % x;
}

static void _leaf(uint16_t x, uint16_t y, uint16_t size) {
  if (_circles_count >= CIRCLES_SIZE) {
    return;
  }

  _circles[_circles_count++] = (struct Circle){
      .color = _leaves_color,
      .d = size,
      .x = x,
      .y = y,
  };
}

static void _tree(uint8_t n, struct Tree* tree, uint16_t x, uint16_t y, uint16_t w, float rot) {
  if (n < tree->top_start && _random_int(100) > n * 8) {
    _leaf(x, y, w / 20 + _random_int(5));
  }

  if (n == 0 || _lines_count >= LINES_SIZE) {
    return;
  }

  if (n > tree->top_start && _random_int(100) > 70 + n * 5) {
    return;
  }

  uint16_t nx = x + w * sin(rot);
  uint16_t ny = y - w * cos(rot);

  _lines[_lines_count++] = (struct Line){
      .color = _branches_color,
      .thickness = w * w / 2048 + 1,
      .x0 = x,
      .y0 = y,
      .x1 = nx,
      .y1 = ny,
  };

  float dr0 = _random(200, 1000);
  uint16_t nx0 = x + dr0 * w * sin(rot);
  uint16_t ny0 = y - dr0 * w * cos(rot);

  float dr1 = _random(200, 1000);
  uint16_t nx1 = x + dr1 * w * sin(rot);
  uint16_t ny1 = y - dr1 * w * cos(rot);

  uint16_t nw1 = w * (tree->main_branch_ratio + _random(-125, 125));
  uint16_t nw2 = w * (tree->side_branch_ratio + _random(-125, 125));
  uint16_t nw3 = w * (tree->side_branch_ratio + _random(-125, 125));

  float new_rot = n > tree->top_start ? tree->top_rot : tree->bottom_rot;

  _tree(n - 1, tree, nx, ny, nw1, rot + _random(-125, 125));
  _tree(n - 1, tree, nx0, ny0, nw2, rot + _random(-125, 125) + new_rot);
  _tree(n - 1, tree, nx1, ny1, nw3, rot + _random(-125, 125) - new_rot);
}

static void _grid(struct Image *image) {
  if (_background_color == BLACK) {
    return;
  }
  for (uint16_t y = 0; y < IMAGE_HEIGHT; y += 4) {
    for (uint16_t x = 0; x < IMAGE_WIDTH; x += 4) {
      uint16_t xx = image->x_offset + x + _random_int(4);
      uint16_t yy = image->y_offset + y + _random_int(4);
      struct Line line = {
          .color = _branches_color,
          .thickness = 1,
          .x0 = xx,
          .y0 = yy,
          .x1 = xx,
          .y1 = yy,
      };
      image_draw_line(image, &line);
    }
  }
}

static void _rain(struct Image *image) {
  for (uint16_t i = 0; i < _rain_density; i++) {
    uint16_t x = image->x_offset + _random_int(IMAGE_WIDTH);
    uint16_t y = image->y_offset + _random_int(IMAGE_HEIGHT) - 12;
    struct Line line = {
        .color = _leaves_color,
        .thickness = 1,
        .x0 = x,
        .y0 = y,
        .x1 = x,
        .y1 = y + 12,
    };
    //image_draw_line(image, &line);
    image_paste_bitmap(image, &EXAMPLE, _leaves_color, x, y);
  }
}

static void _random_colors(void)
{
  switch(_random_int(4)) {
    case 0: // Day
      _background_color = WHITE;
      _leaves_color = RED;
      _branches_color = BLACK;
    break;
    case 1: // Night 1
      _background_color = BLACK;
      _leaves_color = WHITE;
      _branches_color = RED;
    break;
    case 2: // Night 2
      _background_color = BLACK;
      _leaves_color = RED;
      _branches_color = WHITE;
    break;
    case 3: // Afternoon
      _background_color = RED;
      _leaves_color = WHITE;
      _branches_color = BLACK;
    break;
  }
}

static void _generate_tree(void){
  uint16_t x = FULL_IMAGE_WIDTH / 2 + _random_int(200) - 100;
  uint16_t y = FULL_IMAGE_HEIGHT - 1;
  uint16_t w = 200 + _random_int(50);
  float rot = _random(-50, 50);

  struct Tree tree = {
    .main_branch_ratio=_random(600, 900),
    .side_branch_ratio=_random(500, 800),
    .top_start=2 + _random_int(2),
    .top_rot=_random(300, 900),
    .bottom_rot=_random(100, 400),
  };
  _tree(6, &tree, x, y, w, rot);
}

static void _reset(void) {
  _lines_count = 0;
  _circles_count = 0;
}

void art_init(void) {
  _lines = malloc(sizeof(struct Line) * LINES_SIZE);
  _circles = malloc(sizeof(struct Circle) * CIRCLES_SIZE);
}

void art_make(void) {
  _reset();
  _random_colors();
  _generate_tree();
  _rain_density = _random_int(512);

  printf("total lines: %d\n", _lines_count);
  printf("total circles: %d\n", _circles_count);
}

void art_draw(struct Image *image) {
  image_clear(image, _background_color);
  _grid(image);
  _rain(image);

  for (uint16_t i = 0; i < _circles_count; i++) {
    for (uint16_t j = 0; j < 3; j++) {
      uint16_t dx = 16 - _random_int(32);
      uint16_t dy = 16 - _random_int(32);
      struct Circle circle = _circles[i];
      circle.x += dx;
      circle.y += dy;
      image_draw_circle(image, &circle);
    }
  }

  for (uint16_t i = 0; i < _lines_count; i++) {
    image_draw_line(image, &_lines[i]);
  }

  for (uint16_t i = 0; i < _circles_count; i++) {
    for (uint16_t j = 0; j < 2; j++) {
      uint16_t dx = 16 - (art_random() % 32);
      uint16_t dy = 16 - (art_random() % 32);
      struct Circle circle = _circles[i];
      circle.x += dx;
      circle.y += dy;
      image_draw_circle(image, &circle);
    }
  }
}
