#pragma once
#include <math.h>
#include "image.h"

#define FOV 50.0f

struct Point3d {
  float x;
  float y;
  float z;
};

struct Point2d {
  float x;
  float y;
};

static inline struct Point to_screen_from_3d(int16_t horizon,
                                             struct Point3d p3d) {
  float ratio = isinf(p3d.z) ? 0.0f : FOV / p3d.z;
  return (struct Point){
      .x = IMAGE_WIDTH / 2 + ratio * p3d.x * FOV,
      .y = horizon + ratio * (horizon - p3d.y * FOV),
  };
}

static inline struct Point3d compute_normal_from_triangle(
    struct Point3d* points) {
  // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
#define SUB(a, b) \
  { a.x - b.x, a.y - b.y, a.z - b.z }
  struct Point3d u = SUB(points[1], points[0]);
  struct Point3d v = SUB(points[2], points[0]);
#undef SUB
  struct Point3d n = {
      .x = u.y * v.z - u.z * v.y,
      .y = u.z * v.x - u.x * v.z,
      .z = u.x * v.y - u.y * v.x,
  };
  struct Point3d absn = {
      .x = fabs(n.x),
      .y = fabs(n.y),
      .z = fabs(n.z),
  };
  float max_abs = n.x;
  if (max_abs < n.y) {
    max_abs = n.y;
  }
  if (max_abs < n.z) {
    max_abs = n.z;
  }
  n.x /= max_abs;
  n.y /= max_abs;
  n.z /= max_abs;
  return n;
}

static inline float compute_angle(struct Point3d a, struct Point3d b) {
  // https://www.cuemath.com/geometry/angle-between-vectors/
  float an = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
  float bn = sqrt(b.x * b.x + b.y * b.y + b.z * b.z);
  float dot = a.x * b.x + a.y + b.y + a.z * b.z;
  float cos = dot / (an * bn);
  return fmod(acos(cos) + M_2_PI, M_2_PI);
}