#include "art/object/road.h"

#include "art/image/3d.h"
#include "art/image/polyfill.h"

#include "art/_share.h"

#define TO_3D(x, y, z) to_screen_from_3d(horizon, (struct Point3d){x, y, z})

void road_draw(struct Image* image, int16_t horizon, float xa, float width) {
  float xb = xa + width;
  float y = 0.0f;
  float z_start = FOV * 4;
  float z_end = INFINITY;
  enum Color color = _branches_color;

  struct Point a_start = TO_3D(xa, y, z_start);
  struct Point b_start = TO_3D(xb, y, z_start);

  struct Point a_end = TO_3D(xa, y, z_end);
  struct Point b_end = TO_3D(xb, y, z_end);

  {  // Background
    struct Point points[] = {a_start, b_start, a_end, b_end};
    polyfill(image, points, sizeof(points) / sizeof(struct Point), color, 32,
             _leaves_color);
  }

  // Side gradient
  dithering_array_random();
  const int gradient_step = 5;
  int16_t border_size = 20;
  for (int i = 0; i < gradient_step; i++) {
    int16_t l = -border_size / 2 + border_size / gradient_step * i;
    int16_t r = l + border_size / gradient_step;
    struct Point points_a[3] = {
        TO_3D(xa - l, y, z_start),
        TO_3D(xa - r, y, z_start),
        a_end,
    };
    struct Point points_b[3] = {
        TO_3D(xb + l, y, z_start),
        TO_3D(xb + r, y, z_start),
        b_end,
    };
    int ii = i >= gradient_step / 2 ? gradient_step - i - 1 : i;
    uint8_t threshold = 50 + 70.0 / (gradient_step / 2) * ii;
    // uint8_t threshold = 55 + 115.0 / gradient_step * i;
    polyfill(image, points_a, 3, color, threshold, TRANSPARENT);
    polyfill(image, points_b, 3, color, threshold, TRANSPARENT);
  }
}