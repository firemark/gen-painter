#include "art.h"
#include "image.h"
#include "perlin.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BRANCHES_SIZE (1024 * 2)
#define LEAFES_SIZE 1024
#define GRASS_SIZE (1024 * 2)
#define BEZIER_LINES_HIGH 16
#define BEZIER_LINES_LOW 6

static uint16_t _branches_count;
static uint16_t _leafes_count;
static uint16_t _grass_count;
static uint16_t _rain_density;
static uint16_t _perlin;
static uint8_t _background_size;
static uint8_t _background_shift;

static enum Color _background_color;
static enum Color _leaves_color;
static enum Color _branches_color;

struct LineZ {
  struct Line line;
  uint8_t zdepth;
};

static struct Line *_branches;
static struct LineZ *_grass;
static struct Circle *_leafes;

struct Tree {
  float main_branch_ratio;
  float side_branch_ratio;
  float curvy_ratio;
  uint8_t top_start;
  float top_rot;
  float bottom_rot;
};

static inline float _random(int16_t start, int16_t end) {
  return (start + (int16_t)(art_random() % (end - start))) / 1000.0;
}

static inline int16_t _random_int(int16_t x) { return art_random() % x; }

static inline float _thickness(int16_t w) { return w * w / 2048.0 + 1.0; }
static inline float _lerp(int16_t x0, int16_t x1, float t) {
  return x0 * (1.0 - t) + x1 * t;
}

static void _leaf(struct Point p, uint16_t size) {
  if (_leafes_count >= LEAFES_SIZE) {
    return;
  }

  _leafes[_leafes_count++] = (struct Circle){
      .color = _leaves_color,
      .d = size,
      .p = p,
  };
}

static struct Point _bezier(float t, struct Point b[4]) {
  struct Point p;
  float t_inv = 1 - t;
  float tt = t * t;
  float ttt = tt * t;
  float tt_inv = t_inv * t_inv;
  float ttt_inv = tt_inv * t_inv;
  float t_tt_inv = t * tt_inv;
  float tt_t_inv = tt * t_inv;

  p.x = b[0].x * ttt_inv + 3 * b[1].x * t_tt_inv + 3 * b[2].x * tt_t_inv +
        b[3].x * ttt;
  p.y = b[0].y * ttt_inv + 3 * b[1].y * t_tt_inv + 3 * b[2].y * tt_t_inv +
        b[3].y * ttt;

  return p;
}

static void _tree(uint8_t n, struct Tree *tree, struct Point p, int16_t w,
                  float rot) {
  if (n < tree->top_start && _random_int(100) > n * 8) {
    _leaf(p, w / 20 + _random_int(5));
  }

  uint16_t chunks = w > 100 ? BEZIER_LINES_HIGH : BEZIER_LINES_LOW;
  if (n == 0 || _branches_count >= BRANCHES_SIZE - chunks) {
    return;
  }

  if (n > tree->top_start && _random_int(100) > 70 + n * 5) {
    return;
  }

  struct Point points[4];
  points[0] = p;

  float _sin = sin(rot);
  float _cos = cos(rot);

  float r1 = _random(200, 500);
  float r2 = r1 + _random(200, 500);

  float dx1 = w * r1;
  float dy1 = tree->curvy_ratio * w * _random(-1000, +1000);
  points[1] = (struct Point){
      p.x + dx1 * _sin - dy1 * _cos,
      p.y - dx1 * _cos - dy1 * _sin,
  };

  float dx2 = w * r2;
  float dy2 = tree->curvy_ratio * w * _random(-1000, +1000);
  points[2] = (struct Point){
      p.x + dx2 * _sin - dy2 * _cos,
      p.y - dx2 * _cos - dy2 * _sin,
  };

  points[3] = (struct Point){
      p.x + w * _sin,
      p.y - w * _cos,
  };

  struct Point old_p = p;

  int16_t new_w = w * (tree->main_branch_ratio + _random(-125, 125));

  for (uint8_t i = 1; i < chunks - 1; i++) {
    float t = (float)i / chunks;
    struct Point new_p = _bezier(t, points);
    float part_w = _lerp(w, new_w, t);
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

  _tree(n - 1, tree, points[3], new_w, rot + _random(-125, 125));

  float new_rot = n > tree->top_start ? tree->top_rot : tree->bottom_rot;

  float split_right = _random(200, 800);
  struct Point side_right = _bezier(split_right, points);
  float w_right = _lerp(w, new_w, split_right) * tree->side_branch_ratio;
  _tree(n - 1, tree, side_right, w_right, rot + _random(-125, 125) + new_rot);

  float split_left = _random(200, 800);
  struct Point side_left = _bezier(split_left, points);
  float w_left = _lerp(w, new_w, split_left) * tree->side_branch_ratio;
  _tree(n - 1, tree, side_left, w_left, rot + _random(-125, 125) - new_rot);
}

// static void _rain(struct Image *image) {
//   for (uint16_t i = 0; i < _rain_density; i++) {
//     struct Point p = {image->offset.x + _random_int(IMAGE_WIDTH),
//                       image->offset.y + _random_int(IMAGE_HEIGHT)};
//     image_paste_bitmap(image, &EXAMPLE, _leaves_color, p);
//   }
// }

static void _random_colors(void) {
  switch (_random_int(4)) {
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

static void _generate_grass(int16_t y) {
  int16_t x = 0;
  while (x < FULL_IMAGE_WIDTH && _grass_count < GRASS_SIZE - 4) {
    x += 10 - 3 + _random_int(6);
    int16_t r = -40 + _random_int(80);
    struct Point points[4] = {
        {x, y},
        {x - 5 - r / 4, y - 20 + _random_int(5)},
        {x - 5 + r / 2, y - 40 + _random_int(5)},
        {x - 5 + r, y - 50 + _random_int(5)},
    };
    for (uint8_t i = 0; i < 3; i++) {
      _grass[_grass_count++] = (struct LineZ){
          .line =
              (struct Line){
                  .color = _leaves_color,
                  .thickness = 6 - 2 * i,
                  .p0 = points[i],
                  .p1 = points[i + 1],
              },
          .zdepth = _random_int(255),
      };
    }
  }
}

static void _generate_tree(void) {
  struct Point p = {
      FULL_IMAGE_WIDTH / 2 + _random_int(200) - 100,
      FULL_IMAGE_HEIGHT - 60,
  };
  uint16_t w = 200 + _random_int(50);
  float rot = _random(-50, 50);

  struct Tree tree = {
      .main_branch_ratio = _random(600, 900),
      .side_branch_ratio = _random(500, 800),
      .curvy_ratio = _random(0, 1000),
      .top_start = 2 + _random_int(2),
      .top_rot = _random(300, 900),
      .bottom_rot = _random(100, 400),
  };
  _tree(6, &tree, p, w, rot);
}

static void _reset(void) {
  _branches_count = 0;
  _leafes_count = 0;
  _grass_count = 0;
}

static void _draw_background(struct Image *image) {
  for(uint8_t i = 8; i < 16; i++) {
    int16_t y = FULL_IMAGE_HEIGHT - 1 - (i - 8) * _background_size + _background_shift;
    struct Point p0 = {0, y - _background_size + 1};
    struct Point p1 = {FULL_IMAGE_WIDTH - 1, y};
    image_draw_rectangle(image, _branches_color, 255 - i * 16, p0, p1);
  }
}

void art_init(void) {
  _branches = malloc(sizeof(struct Line) * BRANCHES_SIZE);
  _grass = malloc(sizeof(struct LineZ) * GRASS_SIZE);
  _leafes = malloc(sizeof(struct Circle) * LEAFES_SIZE);
}

void art_make(void) {
  _reset();
  _random_colors();
  _generate_tree();
  _generate_grass(FULL_IMAGE_HEIGHT);
  _generate_grass(FULL_IMAGE_HEIGHT - 20);
  _generate_grass(FULL_IMAGE_HEIGHT - 40);
  _generate_grass(FULL_IMAGE_HEIGHT - 60);
  _rain_density = _random_int(512);
  _background_size = 32 + _random_int(32);
  _background_shift = _random_int(32);
  _perlin = _random_int(0xFF00);

  printf("total branches: %d\n", _branches_count);
  printf("total leafes: %d\n", _leafes_count);
  printf("total grass: %d\n", _grass_count);
}


void art_draw(struct Image *image) {
  image_clear(image, _background_color);
  // _grid(image);
  // _rain(image);
  // image_perlin(image, _branches_color, _perlin, 0.005);
  _draw_background(image);

  for (uint16_t i = 0; i < _leafes_count; i++) {
    for (uint16_t j = 0; j < 3; j++) {
      uint16_t dx = 16 - _random_int(32);
      uint16_t dy = 16 - _random_int(32);
      struct Circle circle = _leafes[i];
      circle.p.x += dx;
      circle.p.y += dy;
      image_draw_circle(image, &circle);
    }
  }

  for (uint16_t i = 0; i < _grass_count; i++) {
    if (_grass[i].zdepth < 128) {
      image_draw_line(image, &_grass[i].line);
    }
  }

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

  for (uint16_t i = 0; i < _grass_count; i++) {
    if (_grass[i].zdepth > 128) {
      image_draw_line(image, &_grass[i].line);
    }
  }

  for (uint16_t i = 0; i < _leafes_count; i++) {
    for (uint16_t j = 0; j < 2; j++) {
      uint16_t dx = 16 - (art_random() % 32);
      uint16_t dy = 16 - (art_random() % 32);
      struct Circle circle = _leafes[i];
      circle.p.x += dx;
      circle.p.y += dy;
      image_draw_circle(image, &circle);
    }
  }
}
