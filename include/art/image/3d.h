#pragma once
#include "image.h"
#include <math.h>

#define FOV 50.0f

struct Point3d {
  float x;
  float y;
  float z;
};

static inline struct Point to_screen_from_3d(int16_t horizon,
                                             struct Point3d p3d) {
  float ratio = isinf(p3d.z) ? 0.0f : FOV / p3d.z;
  return (struct Point){
      .x = IMAGE_WIDTH / 4 + ratio * p3d.x,
      .y = horizon + ratio * (-p3d.y + horizon),
  };
}