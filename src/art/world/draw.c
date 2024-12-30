#include "art/world.h"
#include "art/_share.h"
#include "art/image/3d.h"
#include "art/image/polyfill.h"
#include "art/object/grass.h"
#include "art/object/house.h"
#include "art/object/lake.h"
#include "art/object/road.h"
#include "art/object/rock.h"
#include "art/object/street_light.h"
#include "art/object/tree.h"
#include "art/random.h"

static void _draw_tree(struct Image* image, struct TreeConfig* config, int16_t hor, int16_t x, int16_t y);
static void _draw_grass(struct Image* image, int16_t hor, int16_t x, int16_t y);
static void _draw_sea(struct Image* image, int16_t hor, float x_road, int16_t x_border);
static void _draw_road(struct Image* image, int16_t hor, struct Road* road);
static void _draw_rock(struct Image* image, int16_t hor, int16_t x, int16_t y);
static void _draw_street_light(struct Image* image, enum StreetLighStyle style,
                               int16_t hor, int16_t x, int16_t y);
static void _draw_house(struct Image* image, int16_t hor,
                        struct LargeObject* house);
static void _draw_lake(struct Image* image, int16_t hor,
                       struct LargeObject* house);

static inline float _g_(float x) {
  return x * GRID_CELL_SIZE;
}
static inline float _x_(float x) {
  return _g_(x - GRID_SIZE_W / 2);
}
static inline float _y_(float y) {
  return FOV * 5 + _g_(y) * 8;
}

void world_draw_back(struct Image* image, struct World* world,
                     int16_t horizont) {
  if (world->has_left_sea) {
    _draw_sea(image, horizont, world->road.x - 0.5f, 0);
  }
  if (world->has_right_sea) {
    _draw_sea(image, horizont, world->road.x + world->road.width + 0.5f, IMAGE_WIDTH);
  }
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
          _draw_grass(image, horizont, x, y);
          break;
        case TREE:
          _draw_tree(image, &world->tree_config, horizont, x, y);
          break;
        case ROCK:
          _draw_rock(image, horizont, x, y);
          break;
        case HOUSE:
          _draw_house(image, horizont, &world->object);
          break;
        case LAKE:
          _draw_lake(image, horizont, &world->object);
          break;
        default:
          break;
      }
    }
  }
}

static void _draw_tree(struct Image* image, struct TreeConfig* tree_config, int16_t hor, int16_t x, int16_t y) {
  struct Point3d position = {_x_(x), 0.0f, _y_(y)};
  struct Point point = to_screen_from_3d(hor, position);
  float height_ratio = GRID_CELL_SIZE * FOV / position.z;
  int16_t tree_height =
      (tree_config->tree_height + random_int(50)) * height_ratio;
  tree_generate(point, tree_height, &tree_config->tree);
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

static void _draw_rock(struct Image* image, int16_t hor, int16_t x, int16_t y) {

  struct Point point = {_x_(x), _y_(y)};
  float size_factor = 5 + random_int(5);
  float height = 2 + random_int(3);
  rock_draw(image, &point, size_factor, height, hor);
}

static void _draw_sea(struct Image* image, int16_t hor, float x_road, int16_t x_border) {
  float x = _x_(x_road);
  float z[2] = {INFINITY, FOV * 4};
  float y[3] = {-4, -2, 0};
  struct Point a[3];
  struct Point b[3];
  int i;
  for (i=0; i < 3; i++) {
    a[i] = to_screen_from_3d(hor, (struct Point3d){x, y[i], z[0]});
    b[i] = to_screen_from_3d(hor, (struct Point3d){x, y[i], z[1]});
  }
  struct Point points[4] = {
      {x_border, a[0].y},
      {x_border, b[0].y},
      b[0],
      a[0],
  };
  polyfill_mirror(image, points, 4, hor);
  for (i=0; i < 3; i++) { // draw stairs;
    struct Line line = {
      .p0=a[i],
      .p1=b[i],
      .color=_branches_color,
      .thickness=2,
    };
    image_draw_line(image, &line);
  }
}

static void _draw_road(struct Image* image, int16_t hor, struct Road* road) {
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
  struct Point2d p0 = {_x_(house->position.x), _y_(house->position.y)};
  struct Point2d p1 = {_x_(house->position.x + house->x_size),
                       _y_(house->position.y + house->y_size)};
  house_draw(image, hor, p0, p1, 40.0);
}

static void _draw_lake(struct Image* image, int16_t hor,
                       struct LargeObject* lake) {
  struct Point2d p0 = {_x_(lake->position.x), _y_(lake->position.y)};
  struct Point2d p1 = {_x_(lake->position.x + lake->x_size),
                       _y_(lake->position.y + lake->y_size)};
  lake_draw(image, hor, p0, p1);
}