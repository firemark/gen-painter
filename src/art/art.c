#include "art/art.h"
#include "art/clouds.h"
#include "art/forecast.h"
#include "art/grass.h"
#include "art/image/3d.h"
#include "art/landscape.h"
#include "art/random.h"
#include "art/road.h"
#include "art/sun.h"
#include "art/tree.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "art/_share.h"

#define GRID_CELL_SIZE 10
#define GRID_SIZE_H 20
#define GRID_SIZE_W 50

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
  _tree_config.count = 1 + random_int(3);
  _tree_config.tree_height = 200 - 30 * _tree_config.count;
}

enum Cell { NOT_FILLED, EMPTY, GRASS, ROAD, TREE };

static struct World {
  enum Cell grid[GRID_SIZE_H][GRID_SIZE_W];
  struct Road {
    int16_t width;
    int16_t x;
  } road;
} _world;

static void _world_setup() {
  uint8_t x, y;
  for (y = 0; y < GRID_SIZE_H; y++) {
    for (x = 0; x < GRID_SIZE_W; x++) {
      _world.grid[y][x] = NOT_FILLED;
    }
  }

  _world.road.width = 4 + random_int(4);
  _world.road.x = 3 + random_int(GRID_SIZE_W - 1 - _world.road.width - 6);
  for (y = 0; y < GRID_SIZE_H; y++) {
    for (x = 0; x <= _world.road.width; x++) {
      _world.grid[y][_world.road.x + x] = ROAD;
    }
  }

  uint8_t tree_count = _tree_config.count;
  while (tree_count > 0) {
    // TODO poisson point process.
    x = random_int(GRID_SIZE_W);
    y = 3 + random_int(GRID_SIZE_H / 2);
    enum Cell *cell = &_world.grid[y][x];
    if (*cell != NOT_FILLED) {
      continue;
    }

    *cell = TREE;
    tree_count--;
  }

  for (y = 0; y < GRID_SIZE_H; y++) {
    for (x = 0; x < GRID_SIZE_W; x++) {
      enum Cell *cell = &_world.grid[y][x];
      if (*cell != NOT_FILLED) {
        continue;
      }

      *cell = GRASS; // random_int(32) > 24 ? EMPTY : GRASS;
    }
  }
}

uint8_t art_init(void) { return tree_init() && clouds_init(); }

void art_make(struct ArtData data) {
  _data = data;
  _reset();
  _random_colors();
  _trees_setup();
  _world_setup();
  _horizont_height = landscape_generate();
}

static void _draw_tree(struct Image *image, struct Point point, float height_ratio) {
  int16_t tree_height = (_tree_config.tree_height + random_int(50)) * height_ratio;
  tree_generate(point, tree_height, &_tree_config.tree);
  tree_draw_back(image);
  tree_draw_branches(image);
  tree_draw_front(image);
  tree_reset();
}

static inline float _g_(float x) { return x * GRID_CELL_SIZE; }
static inline float _x_(float x) { return _g_(x - GRID_SIZE_W / 2); }
static inline float _y_(float y) { return FOV * 5 + _g_(y) * 8; }

void art_draw(struct Image *image) {
  image_clear(image, _background_color);

  sun_draw(image);
  _rain(image);
  _snow(image);
  clouds_draw(image);
  landscape_draw(image);

  int16_t horizont = IMAGE_HEIGHT - _horizont_height;

  road_draw(image, horizont, _x_(_world.road.x), _g_(_world.road.width));

  int16_t x, y;
  for (y = GRID_SIZE_H - 1; y >= 0; y--) {
    for (x = 0; x < GRID_SIZE_W; x++) {
      struct Point3d position = {_x_(x), 0.0f, _y_(y)};
      struct Point point = to_screen_from_3d(horizont, position);
      switch (_world.grid[y][x]) {
      case GRASS: {
        float size = GRID_CELL_SIZE * 50 * 8 / position.z;
        struct Circle c = {.color = _leaves_color, .p = point, .d = size};
        image_draw_circle(image, &c);
        // grass_draw();
      } break;
      case TREE: {
        float height_ratio = GRID_CELL_SIZE * 50 / position.z;
        _draw_tree(image, point, height_ratio);
      }
      break;
      case NOT_FILLED:
      case EMPTY:
      case ROAD:
        break;
      }
    }
  }

  _rain(image);
  _snow(image);

  forecast_draw(image);
}
