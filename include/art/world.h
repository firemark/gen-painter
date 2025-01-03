#pragma once
#include <stdbool.h>
#include "art/image/image.h"
#include "art/object/street_light.h"
#include "art/object/tree.h"

#define GRID_CELL_SIZE 10
#define GRID_SIZE_H 20
#define GRID_SIZE_W 50

enum WorldCell { NOT_FILLED, EMPTY, GRASS, ROAD, TREE, HOUSE, LAKE, ROCK };

struct LargeObject {
    bool visible;
    int16_t x_size;
    int16_t y_size;
    struct Point position;
};

struct TreeConfig {
  struct Tree tree;
  int16_t tree_height;
  int16_t count;
};

struct World {
  enum WorldCell grid[GRID_SIZE_H][GRID_SIZE_W];
  bool has_right_light;
  bool has_left_light;
  bool has_left_sea;
  bool has_right_sea;
  enum StreetLighStyle street_light_style;
  struct TreeConfig tree_config;
  struct LargeObject object;
  struct Road {
    int16_t width;
    int16_t x;
  } road;
};

bool world_init(void);
void world_setup(struct World* world);
void world_draw_back(struct Image* image, struct World* world,
                     int16_t horizont);
void world_draw_front(struct Image* image, struct World* world,
                      int16_t horizont);
void world_reset(void);