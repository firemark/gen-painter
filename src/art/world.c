#include "art/world.h"
#include "art/image/3d.h"
#include "art/random.h"

#include "art/_share.h"
#include "art/grass.h"
#include "art/road.h"
#include "art/tree.h"

static struct TreeConfig {
  struct Tree tree;
  int16_t tree_height;
  int16_t count;
} _tree_config;

bool world_init(void) { return tree_init(); }
void world_reset(void) { tree_reset(); }

static void _setup_init(struct World *world);
static void _setup_road(struct World *world);
static void _setup_trees(struct World *world);
static void _setup_grass(struct World *world);

void world_setup(struct World *world) {
  _setup_init(world);
  _setup_road(world);
  _setup_trees(world);
  _setup_grass(world);
}

static void _setup_init(struct World *world) {
  uint8_t x, y;
  for (y = 0; y < GRID_SIZE_H; y++) {
    for (x = 0; x < GRID_SIZE_W; x++) {
      world->grid[y][x] = NOT_FILLED;
    }
  }
}

static void _setup_road(struct World *world) {
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

static void _setup_trees(struct World *world) {
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

  uint8_t x, y;
  uint8_t tree_count = _tree_config.count;
  while (tree_count > 0) {
    // TODO poisson point process.
    x = GRID_SIZE_W / 4 + random_int(GRID_SIZE_W / 2);
    y = 3 + random_int(GRID_SIZE_H / 2);
    enum WorldCell *cell = &world->grid[y][x];
    if (*cell != NOT_FILLED) {
      continue;
    }

    *cell = TREE;
    tree_count--;
  }
}

static void _setup_grass(struct World *world) {
  uint8_t x, y;
  for (y = 0; y < GRID_SIZE_H; y++) {
    for (x = 0; x < GRID_SIZE_W; x++) {
      enum WorldCell *cell = &world->grid[y][x];
      if (*cell != NOT_FILLED) {
        continue;
      }

      *cell = random_int(32) > 30 ? EMPTY : GRASS;
    }
  }
}

static void _draw_tree(struct Image *image, int16_t hor, int16_t x, int16_t y);
static void _draw_grass(struct Image *image, int16_t hor, int16_t x, int16_t y);

static inline float _g_(float x) { return x * GRID_CELL_SIZE; }
static inline float _x_(float x) { return _g_(x - GRID_SIZE_W / 2); }
static inline float _y_(float y) { return FOV * 5 + _g_(y) * 8; }

#define INVOKE_CB(cb)                                                          \
  {                                                                            \
    cb(image, horizont, x, y);                                                 \
    break;                                                                     \
  }

void world_draw(struct Image *image, struct World *world, int16_t horizont) {
  road_draw(image, horizont, _x_(world->road.x), _g_(world->road.width));

  int16_t x, y;
  for (y = GRID_SIZE_H - 1; y >= 0; y--) {
    for (x = 0; x < GRID_SIZE_W; x++) {
      enum WorldCell cell = world->grid[y][x];
      switch (cell) {
      case GRASS:
        INVOKE_CB(_draw_grass);
      case TREE:
        INVOKE_CB(_draw_tree);
      case NOT_FILLED:
      case EMPTY:
      case ROAD:
        break;
      }
    }
  }
}

static void _draw_tree(struct Image *image, int16_t hor, int16_t x, int16_t y) {
  struct Point3d position = {_x_(x), 0.0f, _y_(y)};
  struct Point point = to_screen_from_3d(hor, position);
  float height_ratio = GRID_CELL_SIZE * 50 / position.z;
  int16_t tree_height =
      (_tree_config.tree_height + random_int(50)) * height_ratio;
  tree_generate(point, tree_height, &_tree_config.tree);
  tree_draw_back(image);
  tree_draw_branches(image);
  tree_draw_front(image);
  tree_reset();
}

static void _draw_grass(struct Image *image, int16_t hor, int16_t x,
                        int16_t y) {
  struct Point3d position_a = {_x_(x - 1), 0.0f, _y_(y)};
  struct Point3d position_b = {_x_(x), 0.0f, _y_(y)};
  struct Point point_a = to_screen_from_3d(hor, position_a);
  struct Point point_b = to_screen_from_3d(hor, position_b);
  float size_factor = 200 + random_int(100);
  float size = GRID_CELL_SIZE * size_factor / position_a.z;
  grass_draw(image, point_a.x, point_b.x, point_a.y, size);
}