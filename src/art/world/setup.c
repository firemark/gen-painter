#include "art/world.h"

#include "art/random.h"

static void _setup_init(struct World* world);
static void _setup_road(struct World* world);
static void _setup_sea(struct World* world);
static void _setup_trees(struct World* world);
static void _setup_grass(struct World* world);
static void _setup_large_object(struct World* world);

void world_setup(struct World* world) {
  _setup_init(world);
  _setup_road(world);
  _setup_sea(world);
  _setup_large_object(world);
  _setup_trees(world);
  _setup_grass(world);

  world->has_left_light = random_int(10) > 4;
  world->has_right_light = random_int(10) > 4;
  world->street_light_style = random_int(2);
}

static void _setup_init(struct World* world) {
  uint8_t x, y;
  for (y = 0; y < GRID_SIZE_H; y++) {
    for (x = 0; x < GRID_SIZE_W; x++) {
      world->grid[y][x] = NOT_FILLED;
    }
  }
}

static void _setup_road(struct World* world) {
  uint8_t x, y;
  world->road.width = 4 + random_int(4);
  world->road.x =
      GRID_SIZE_W / 4 + random_int(GRID_SIZE_W / 2 - 1 - world->road.width);
  for (y = 0; y < GRID_SIZE_H; y++) {
    for (x = 0; x <= world->road.width; x++) {
      world->grid[y][world->road.x + x] = ROAD;
    }
  }
}

static void _setup_sea(struct World* world) {
  world->has_left_sea = random_int(24) > 20;
  world->has_right_sea = random_int(24) > 20;

  if (world->has_left_sea) {
    int x, y;
    for (x = 0; x < world->road.x; x++) {
      for (y = 0; y < GRID_SIZE_H; y++) {
        world->grid[y][x] = EMPTY;
      }
    }
  }
  if (world->has_right_sea) {
    int x, y;
    for (x = world->road.x; x < GRID_SIZE_W; x++) {
      for (y = 0; y < GRID_SIZE_H; y++) {
        world->grid[y][x] = EMPTY;
      }
    }
  }
}

static void _setup_trees(struct World* world) {
  world->tree_config.tree = (struct Tree){
      .main_branch_ratio = random_range(600, 900),
      .side_branch_ratio = random_range(500, 800),
      .curvy_ratio = random_range(0, 1000),
      .top_start = 2 + random_int(2),
      .top_rot = random_range(300, 900),
      .bottom_rot = random_range(100, 400),
  };
  world->tree_config.count = 1 + random_int(7);
  int16_t dec = 30 * world->tree_config.count;
  world->tree_config.tree_height = 200 - (dec > 100 ? 100 : dec);

  uint8_t x, y;
  uint8_t tree_count = world->tree_config.count;
  uint8_t tries_count = 50;
  while (tree_count > 0 && tries_count > 0) {
    tries_count--;
    // TODO poisson point process.
    x = GRID_SIZE_W / 4 + random_int(GRID_SIZE_W / 2);
    y = 3 + random_int(GRID_SIZE_H / 2);
    enum WorldCell* cell = &world->grid[y][x];
    if (*cell != NOT_FILLED) {
      continue;
    }

    *cell = TREE;
    tree_count--;
  }
}

static void _setup_grass(struct World* world) {
  uint8_t x, y;
  for (y = 0; y < GRID_SIZE_H; y++) {
    for (x = 0; x < GRID_SIZE_W; x++) {
      enum WorldCell* cell = &world->grid[y][x];
      if (*cell != NOT_FILLED) {
        continue;
      }

      switch (random_int(24)) {
        case 0 ... 14:
          *cell = GRASS;
          break;
        case 15:
          *cell = ROCK;
          break;
        default:
          *cell = EMPTY;
      }
    }
  }
}

static void _setup_large_object(struct World* world) {
  if (random_int(32) < 4) {
    // Don't draw.
    world->object.visible = false;
    return;
  }

  enum WorldCell cell;
  int16_t x_min_size;
  int16_t y_min_size;
  int16_t y_start;
  switch (random_int(2)) {
    case 0: {
      x_min_size = 8;
      y_min_size = 8;
      y_start = 8;
      cell = HOUSE;
      break;
    }
    case 1: {
      x_min_size = 14;
      y_min_size = 10;
      y_start = 4;
      cell = LAKE;
      break;
    }
  }

  int16_t x_size;
  int16_t y_size;
  struct Point p;
  uint8_t i = random_int(2);
  uint8_t tries_count;
  for (tries_count = 50; tries_count > 0; tries_count--) {
    y_size = y_min_size + random_int_b(2);
    x_size = x_min_size + random_int_b(2);
    if (i++ % 2) {  //
      // Right side
      if (world->has_right_sea) {
        continue;
      }
      p.x = world->road.x + world->road.width + 2;
    } else {
      // Left side
      if (world->has_left_sea) {
        continue;
      }
      p.x = world->road.x - 2 - x_size;
    }
    p.y = y_start + random_int_b(4);
    if (p.x >= 0 && p.y >= 0 && p.x + x_size < GRID_SIZE_W &&
        p.y + y_size < GRID_SIZE_H) {
      break;
    }
  }

  if (tries_count == 0) {
    // Fail to find place.
    world->object.visible = false;
    return;
  }

  world->object.position = p;
  world->object.x_size = x_size;
  world->object.y_size = y_size;
  world->object.visible = true;
  for (uint8_t y = 0; y < y_size; y++) {
    for (uint8_t x = 0; x < x_size; x++) {
      world->grid[p.y + y][p.x + x] = EMPTY;
    }
  }

  world->grid[p.y][p.x] = cell;
}