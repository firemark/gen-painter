#include "art/world.h"
#include "art/_share.h"
#include "art/image/3d.h"
#include "art/object/grass.h"
#include "art/object/house.h"
#include "art/object/lake.h"
#include "art/object/road.h"
#include "art/object/street_light.h"
#include "art/object/tree.h"
#include "art/random.h"

static struct TreeConfig {
  struct Tree tree;
  int16_t tree_height;
  int16_t count;
} _tree_config;

bool world_init(void) {
  return tree_init();
}
void world_reset(void) {
  tree_reset();
}

static void _setup_init(struct World* world);
static void _setup_road(struct World* world);
static void _setup_trees(struct World* world);
static void _setup_grass(struct World* world);
static void _setup_large_object(struct World* world);

void world_setup(struct World* world) {
  _setup_init(world);
  _setup_road(world);
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

static void _setup_trees(struct World* world) {
  _tree_config.tree = (struct Tree){
      .main_branch_ratio = random_range(600, 900),
      .side_branch_ratio = random_range(500, 800),
      .curvy_ratio = random_range(0, 1000),
      .top_start = 2 + random_int(2),
      .top_rot = random_range(300, 900),
      .bottom_rot = random_range(100, 400),
  };
  _tree_config.count = 1 + random_int(7);
  int16_t dec = 30 * _tree_config.count;
  _tree_config.tree_height = 200 - (dec > 100 ? 100 : dec);

  uint8_t x, y;
  uint8_t tree_count = _tree_config.count;
  while (tree_count > 0) {
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

      *cell = random_int(32) > 14 ? EMPTY : GRASS;
    }
  }
}

static void _setup_large_object(struct World* world) {
  // if (random_int(32) < 4) {
  //   // Don't draw.
  //   world->object.visible = false;
  //   return;
  // }

  enum WorldCell cell;
  int16_t x_min_size;
  int16_t y_min_size;
  int16_t y_start;
  switch (1 /*random_int(2)*/) {
    case 0: {
      x_min_size = 8;
      y_min_size = 8;
      y_start = 8;
      cell = HOUSE;
      break;
    }
    case 1: {
      x_min_size = 14;
      y_min_size = 14;
      y_start = 4;
      cell = LAKE;
      break;
    }
  }

  int16_t x_size;
  int16_t y_size;
  struct Point p;
  uint8_t i = random_int(2);
  for (;;) {
    y_size = y_min_size + random_int_b(2);
    x_size = x_min_size + random_int_b(2);
    if (i++ % 2) {
      p.x = world->road.x + world->road.width + 2;
    } else {
      p.x = world->road.x - 2 - x_size;
    }
    p.y = 4 + random_int_b(4);
    if (p.x >= 0 && p.y >= 0 && p.x + x_size < GRID_SIZE_W &&
        p.y + y_size < GRID_SIZE_H) {
      break;
    }
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

  cell = LAKE;
  world->grid[p.y][p.x] = cell;
}

static void _draw_tree(struct Image* image, int16_t hor, int16_t x, int16_t y);
static void _draw_grass(struct Image* image, int16_t hor, int16_t x, int16_t y);
static void _draw_road(struct Image* image, int16_t hor, struct Road* road);
static void _draw_street_light(struct Image* image, enum StreetLighStyle style,
                               int16_t hor, int16_t x, int16_t y);
static void _draw_house(struct Image* image, int16_t hor,
                        struct LargeObject* house);
static void _draw_lake(struct Image* image, int16_t hor,
                       struct LargeObject* house);

#define INVOKE_CB(cb)          \
  {                            \
    cb(image, horizont, x, y); \
    break;                     \
  }

void world_draw_back(struct Image* image, struct World* world,
                     int16_t horizont) {
  _draw_road(image, horizont, &world->road);
}

void world_draw_front(struct Image* image, struct World* world,
                      int16_t horizont) {
  int16_t x, y;
  for (y = GRID_SIZE_H - 1; y >= 0; y--) {
    if (y % 6 == 0) {
      if (world->has_left_light) {
        int16_t xx = world->road.x;
        _draw_street_light(image, world->street_light_style, horizont, xx, y);
      }
      if (world->has_right_light) {
        int16_t xx = world->road.x + world->road.width;
        _draw_street_light(image, world->street_light_style, horizont, xx, y);
      }
    }
    for (x = 0; x < GRID_SIZE_W; x++) {
      enum WorldCell cell = world->grid[y][x];
      switch (cell) {
        case GRASS:
          INVOKE_CB(_draw_grass);
        case TREE:
          INVOKE_CB(_draw_tree);
        case HOUSE: {
          _draw_house(image, horizont, &world->object);
          break;
        }
        case LAKE: {
          _draw_lake(image, horizont, &world->object);
          break;
        }
        case NOT_FILLED:
        case EMPTY:
        case ROAD:
          break;
      }
    }
  }
}

static inline float _g_(float x) {
  return x * GRID_CELL_SIZE;
}
static inline float _x_(float x) {
  return _g_(x - GRID_SIZE_W / 2);
}
static inline float _y_(float y) {
  return FOV * 5 + _g_(y) * 8;
}

static void _draw_tree(struct Image* image, int16_t hor, int16_t x, int16_t y) {
  struct Point3d position = {_x_(x), 0.0f, _y_(y)};
  struct Point point = to_screen_from_3d(hor, position);
  float height_ratio = GRID_CELL_SIZE * FOV / position.z;
  int16_t tree_height =
      (_tree_config.tree_height + random_int(50)) * height_ratio;
  tree_generate(point, tree_height, &_tree_config.tree);
  tree_draw_back(image);
  tree_draw_branches(image);
  tree_draw_front(image);
  tree_reset();
}

static void _draw_grass(struct Image* image, int16_t hor, int16_t x,
                        int16_t y) {
  struct Point3d position_a = {_x_(x - 1), 0.0f, _y_(y)};
  struct Point3d position_b = {_x_(x), 0.0f, _y_(y)};
  struct Point point_a = to_screen_from_3d(hor, position_a);
  struct Point point_b = to_screen_from_3d(hor, position_b);
  float size_factor = 100 + random_int(100);
  float size = _g_(size_factor) / position_a.z;
  grass_draw(image, point_a.x, point_b.x, point_a.y, size);
}

static void _draw_road(struct Image* image, int16_t hor, struct Road* road) {
  float x = _x_(road->x);
  float w = _g_(road->width);
  road_draw(image, hor, _x_(road->x), _g_(road->width));
}

static void _draw_street_light(struct Image* image, enum StreetLighStyle style,
                               int16_t hor, int16_t x, int16_t y) {
  float xx = _x_(x);
  float yy = _y_(y);
  struct Point3d position = {xx, 0.0f, yy};
  struct Point point = to_screen_from_3d(hor, position);
  float height = _g_(6000.0f) / position.z;
  street_light_draw(image, style, point, height / 4, height);
}

static void _draw_house(struct Image* image, int16_t hor,
                        struct LargeObject* house) {
  struct Point3d p0 = {_x_(house->position.x), 0.0f, _y_(house->position.y)};
  struct Point3d p1 = {_x_(house->position.x + house->x_size), 0.0f,
                       _y_(house->position.y + house->y_size)};
  house_draw(image, hor, p0, p1, 40.0);
}

static void _draw_lake(struct Image* image, int16_t hor,
                       struct LargeObject* lake) {
  struct Point3d p0 = {_x_(lake->position.x), 0.0f, _y_(lake->position.y)};
  struct Point3d p1 = {_x_(lake->position.x + lake->x_size), 0.0f,
                       _y_(lake->position.y + lake->y_size)};
  lake_draw(image, hor, p0, p1);
}
