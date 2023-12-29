#include "art/art.h"
#include "art/clouds.h"
#include "art/forecast.h"
#include "art/grass.h"
#include "art/landscape.h"
#include "art/random.h"
#include "art/sun.h"
#include "art/tree.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "art/_share.h"

struct ArtData _data;
enum Color _background_color;
enum Color _leaves_color;
enum Color _branches_color;

static void _rain(struct Image *image) {
  uint16_t density = _data.rain_density < 2000 ? _data.rain_density : 2000;
  for (uint16_t i = 0; i < density; i++) {
    struct Point p0 = {random_int(IMAGE_WIDTH), random_int(IMAGE_HEIGHT)};
    struct Point p1 = {p0.x - 8, p0.y + 8};
    struct Line line = {
        .p0 = p0,
        .p1 = p1,
        .thickness = 1,
        .color = _branches_color,
    };
    image_draw_line(image, &line);
  }
}

static void _snow(struct Image *image) {
  uint16_t density = _data.snow_density < 2000 ? _data.snow_density : 2000;
  for (uint16_t i = 0; i < density; i++) {
    struct Point p = {random_int(IMAGE_WIDTH), random_int(IMAGE_HEIGHT)};
    for (int8_t j = -1; j <= 1; j++) {
      struct Line line = {
          .p0 = {p.x + 4 * j, p.y},
          .p1 = {p.x - 4 * j, p.y + 8},
          .thickness = 1,
          .color = _branches_color,
      };
      image_draw_line(image, &line);
    }
    {
      struct Line line = {
          .p0 = {p.x + 4, p.y + 4},
          .p1 = {p.x - 4, p.y + 4},
          .thickness = 1,
          .color = _branches_color,
      };
      image_draw_line(image, &line);
    }
  }
}

static void _random_colors(void) {
  switch (random_int(4)) {
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

static void _reset(void) {
  tree_reset();
  random_shuffle_array();
}

static int16_t _horizont_height;
static struct TreeConfig {
  struct Tree tree;
  int16_t tree_height;
  int16_t count;
  int16_t x_span;
  int16_t x;
} _tree_config;

static void _trees_setup() {
  _tree_config.tree = (struct Tree){
      .main_branch_ratio = random_range(600, 900),
      .side_branch_ratio = random_range(500, 800),
      .curvy_ratio = random_range(0, 1000),
      .top_start = 2 + random_int(2),
      .top_rot = random_range(300, 900),
      .bottom_rot = random_range(100, 400),
  };
  _tree_config.tree_height = 100;
  _tree_config.count = 1 + random_int(3);
  _tree_config.x_span = 300 + random_int(200);
  _tree_config.x = IMAGE_WIDTH / 2 - random_int(200);
  _tree_config.x -= _tree_config.x_span * (1 + _tree_config.count / 2);
}

uint8_t art_init(void) { return tree_init() && clouds_init(); }

void art_make(struct ArtData data) {
  _data = data;
  _reset();
  _random_colors();
  _trees_setup();
  _horizont_height = landscape_generate();
}

static void _draw_tree(struct Image *image, int y) {
  _tree_config.x += _tree_config.x_span;
  _tree_config.count--;
  struct Point point = {_tree_config.x, y};
  int16_t tree_height = _tree_config.tree_height + random_int(50);
  tree_generate(point, tree_height, &_tree_config.tree);
  tree_draw_back(image);
  tree_draw_branches(image);
  tree_draw_front(image);
  tree_reset();
}

void art_draw(struct Image *image) {
  image_clear(image, _background_color);

  sun_draw(image);
  _rain(image);
  _snow(image);
  clouds_draw(image);
  landscape_draw(image);

  int16_t y_min = IMAGE_HEIGHT - _horizont_height + 20;
  int16_t y_max = IMAGE_HEIGHT + 20;
  int16_t y;
  for (y = y_min; y <= y_max; y += 20) {
    grass_draw(image, y);
    if (random_int(32) > 20 && _tree_config.count > 0) {
      _draw_tree(image, y);
    }
  }

  _rain(image);
  _snow(image);

  forecast_draw(image);
}
