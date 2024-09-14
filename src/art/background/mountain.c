#include "art/background/mountain.h"
#include "art/image/3d.h"
#include "art/image/image_adv.h"
#include "art/random.h"
#include "art/random_point.h"

#include "art/_share.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#define MOUNTAIN_BASE_POINTS 12

struct MountainPointTree {
  struct Point3d point;
  struct MountainPointTree *right;
  struct MountainPointTree *down;
};

static void _create_mountain_path(struct MountainPointTree *ptr) {
  struct Point3d point = ptr->point;
  while (point.y > 0.0f) {
    point.y -= 16.0f + random_int_b(4);
    if (point.y < 0.0f) {
      point.y = 0.0f;
    }
    point.z += 20.0f + random_int(8);
    ptr->down = malloc(sizeof(struct MountainPointTree));
    ptr = ptr->down;
    ptr->point = point;
    ptr->point.x += random_int_b(32);
    ptr->right = NULL;
  }
  ptr->down = NULL;
}

static void _smooth_path(struct MountainPointTree *up)
{
  while(up) {
    struct MountainPointTree *mid = up->down;
    if (!mid) {
      return;
    }
    struct MountainPointTree *down = mid->down;
    if (!down) {
      return;
    }
    #define CALC(v) (0.25 * up->point.v + 0.5 * mid->point.v + 0.25 * down->point.v)
    mid->point = (struct Point3d) {
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
  float shift = width / MOUNTAIN_BASE_POINTS;
  for (uint8_t i = 0; i < MOUNTAIN_BASE_POINTS; i++) {
    ptr->point = (struct Point3d){
        .x = (float)i * shift + random_int_b(shift / 4) - width / 2,
        .y = height * sin((float)(i + 1) / MOUNTAIN_BASE_POINTS * M_PI) +
             random_int_b(shift / 2),
        .z = random_int(shift),
    };
    ptr->right = i < MOUNTAIN_BASE_POINTS - 1
                     ? malloc(sizeof(struct MountainPointTree))
                     : NULL;
    _create_mountain_path(ptr);
    _smooth_path(ptr);
    ptr = ptr->right;
  }
  return tree;
}

static void _free_mountain(struct MountainPointTree *tree) {
  while (tree) {
    if (tree->right) {
      _free_mountain(tree->right);
    }
    struct MountainPointTree *prev = tree;
    tree = tree->down;
    free(prev);
  }
}

static void _draw_path(struct Image *image, int16_t x_center, int16_t y,
                       struct MountainPointTree *left,
                       struct MountainPointTree *right) {
  bool left_side = true;
  while (left->down || right->down) {
    bool both = left->down && right->down;
    struct Point3d points[3];
    if (both ? left_side : !!left->down) {
      points[0] = left->point;
      points[1] = left->down->point;
      points[2] = right->point;
      left = left->down;
    } else if (right->down) {
      points[0] = left->point;
      points[1] = right->down->point;
      points[2] = right->point;
      right = right->down;
    }

    static struct Point3d sun_direction = {0.0, 1.0, 1.0};
    struct Point3d normal = compute_normal_from_triangle(points);
    float vshadow = normal.x * 0.5 + normal.z + 0.1;
    uint8_t shadow = vshadow > 0.0f ? (vshadow > 1.0f ? 128 : 128 * vshadow) : 0;
#define TO_POINT(p) ((struct Point){p.x + x_center, y - p.y})
    struct Point points_2d[3] = {
      TO_POINT(points[0]),
      TO_POINT(points[1]),
      TO_POINT(points[2]),
    };
#undef TO_POINT
    polyfill(image, points_2d, 3, _leaves_color, 64 + shadow, _background_color);
    left_side = !left_side;
  }
}

static void _draw(struct Image *image, int16_t x_center, int16_t y,
                  struct MountainPointTree *tree) {
  while (tree->right) {
    _draw_path(image, x_center, y, tree, tree->right);
    tree = tree->right;
  }
}

void draw_mountain(struct Image *image, int16_t x_center, int16_t y,
                   int16_t width, int16_t height) {
  struct MountainPointTree *tree = _create_mountain(width, height);
  _draw(image, x_center, y, tree);
  _free_mountain(tree);
}