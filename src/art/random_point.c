#include "art/random_point.h"
#include <stdint.h>
#include "art/random.h"

static inline struct Point _compute_point(struct Point start, struct Point a,
                                          struct Point b, float ua, float ub) {
  float a_dx = a.x - start.x;
  float a_dy = a.y - start.y;
  float b_dx = b.x - start.x;
  float b_dy = b.y - start.y;

  struct Point result;
  result.x = start.x + (int16_t)(ua * a_dx) + (int16_t)(ub * b_dx);
  result.y = start.y + (int16_t)(ua * a_dy) + (int16_t)(ub * b_dy);
  return result;
}

struct Point random_parallelogram(struct Point start, struct Point a,
                                  struct Point b) {
  float ua = (float)random_int(INT16_MAX) / INT16_MAX;
  float ub = (float)random_int(INT16_MAX) / INT16_MAX;
  return _compute_point(start, a, b, ua, ub);
}

struct Point random_triangle(struct Point start, struct Point a,
                             struct Point b) {
  float ua = (float)random_int(INT16_MAX) / INT16_MAX;
  float ub = (float)random_int(INT16_MAX) / INT16_MAX;
  if (ua + ub > 1) {
    ua = 1.0f - ua;
    ub = 1.0f - ub;
  }
  return _compute_point(start, a, b, ua, ub);
}