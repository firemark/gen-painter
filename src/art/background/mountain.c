#include "art/background/mountain.h"
#include "art/image/3d.h"
#include "art/image/image_adv.h"
#include "art/random.h"
#include "art/random_point.h"

#include "art/_share.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#define WIDTH 100

struct MountainPointPath {
  struct Point3d point;
  struct MountainPointPath *down;
};

struct MountainPointTree {
  struct MountainPointTree *right;
  struct MountainPointPath path;
};

static void _create_mountain_path(struct MountainPointPath *ptr) {
  struct Point3d point = ptr->point;
  while (point.y > 0.0f) {
    point.y -= WIDTH / 8 + random_int_b(WIDTH / 16);
    if (point.y < 0.0f) {
      point.y = 0.0f;
    }
    point.z += WIDTH / 8 + random_int(WIDTH / 16);
    ptr->down = malloc(sizeof(struct MountainPointPath));
    ptr = ptr->down;
    ptr->point = point;
    ptr->point.x += random_int_b(WIDTH / 4);
  }
  ptr->down = NULL;
}

static void _smooth_path(struct MountainPointPath *up) {
  while (up) {
    struct MountainPointPath *mid = up->down;
    if (!mid) {
      return;
    }
    struct MountainPointPath *down = mid->down;
    if (!down) {
      return;
    }
#define CALC(v) (0.25 * up->point.v + 0.5 * mid->point.v + 0.25 * down->point.v)
    mid->point = (struct Point3d){
        .x = CALC(x),
        .y = CALC(y),
        .z = CALC(z),
    };
    up = mid;
  }
}

static struct MountainPointTree *_create_mountain(int16_t width,
                                                  int16_t height) {
  struct MountainPointTree *tree = malloc(sizeof(struct MountainPointTree));
  struct MountainPointTree *ptr = tree;
  struct Point bezier_points[4] = {
      // {z, y},
      {random_int(WIDTH * 2), 0},
      {random_int(WIDTH * 2), height - random_int(WIDTH / 2)},
      {random_int(WIDTH * 2), height - random_int(WIDTH / 2)},
      {random_int(WIDTH * 2), 0},
  };
  for (int16_t x = 0; x < width; x += WIDTH + random_int_b(WIDTH / 4)) {
    struct Point bezier_point = bezier((float)x / (float)width, bezier_points);
    ptr->path.point = (struct Point3d){
        .x = x - width / 2,
        .y = bezier_point.y + random_int_b(WIDTH / 4),
        .z = bezier_point.x + random_int_b(WIDTH / 2),
    };
    _create_mountain_path(&ptr->path);
    _smooth_path(&ptr->path);
    ptr->right = malloc(sizeof(struct MountainPointTree));
    ptr = ptr->right;
  }

  // Last point.
  {
    struct Point bezier_point = bezier(1.0f, bezier_points);
    ptr->path.point = (struct Point3d){
        .x = width / 2,
        .y = 0.0f,
        .z = bezier_point.x,
    };
    ptr->path.down = NULL;
    ptr->right = NULL;
  }

  return tree;
}

static void _free_mountain(struct MountainPointTree *tree) {
  while (tree) {
    struct MountainPointPath *path = &tree->path;
    while (path) {
      struct MountainPointPath *prev = path;
      path = path->down;
      free(path);
    }
    struct MountainPointTree *prev = tree;
    tree = tree->right;
    free(prev);
  }
}

static inline void _draw_poly(struct Image *image, int16_t x_center, int16_t y,
                              struct Point3d *points) {
  struct Point3d normal = compute_normal_from_triangle(points);
  float vshadow = normal.x * 0.5 + normal.z + 0.1;
  uint8_t shadow = vshadow > 0.0f ? (vshadow > 1.0f ? 196 : 196 * vshadow) : 0;
#define TO_POINT(p) ((struct Point){p.x + x_center, y - p.y})
  struct Point points_2d[3] = {
      TO_POINT(points[0]),
      TO_POINT(points[1]),
      TO_POINT(points[2]),
  };
#undef TO_POINT
  polyfill(image, points_2d, 3, _leaves_color, shadow, _background_color);
}

static void _draw_path(struct Image *image, int16_t x_center, int16_t y,
                       struct MountainPointPath *left,
                       struct MountainPointPath *right) {
  bool left_side = true;
  for (;;) {
    bool both = left->down && right->down;
    if (both ? left_side : !!left->down) {
      struct Point3d points[3] = {left->point, left->down->point, right->point};
      _draw_poly(image, x_center, y, points);
      left = left->down;
    } else if (right->down) {
      struct Point3d points[3] = {left->point, right->down->point,
                                  right->point};
      _draw_poly(image, x_center, y, points);
      right = right->down;
    } else {
      break;
    }

    left_side = !left_side;
  }
}

static void _draw(struct Image *image, int16_t x_center, int16_t y,
                  struct MountainPointTree *tree) {
  while (tree->right) {
    _draw_path(image, x_center, y, &tree->path, &tree->right->path);
    tree = tree->right;
  }
}

void draw_mountain(struct Image *image, int16_t x_center, int16_t y,
                   int16_t width, int16_t height) {
  struct MountainPointTree *tree = _create_mountain(width, height);
  _draw(image, x_center, y, tree);
  _free_mountain(tree);
}