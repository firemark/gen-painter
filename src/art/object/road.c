#include "art/object/road.h"
#include "art/image/3d.h"
#include "art/image/image_adv.h"
#include "art/_share.h"

#define TO_3D(x, y, z) to_screen_from_3d(horizon, (struct Point3d){x, y, z})

void road_draw(struct Image *image, int16_t horizon, float xa,
               float width) {
  float xb = xa + width;
  float y = 0.0f;
  float z_start = FOV * 4;
  float z_end = INFINITY;
  float z_lane_end = FOV * 100;
  enum Color color = _branches_color;

  struct Point a_start = TO_3D(xa, y, z_start);
  struct Point b_start = TO_3D(xb, y, z_start);

  struct Point a_end = TO_3D(xa, y, z_end);
  struct Point b_end = TO_3D(xb, y, z_end);

  {
    struct Line line_a = {color, 3, a_start, a_end};
    struct Line line_b = {color, 3, b_start, b_end};
    image_draw_line(image, &line_a);
    image_draw_line(image, &line_b);
  }

  {
    struct Point points[] = {a_start, b_start, a_end, b_end};
    polyfill(image, points, sizeof(points) / sizeof(struct Point),
             _branches_color, 48, _leaves_color);
  }

  float z;
  float x_center = xa + width / 2;
  for (z = z_start; z < z_lane_end; z += 100.0f) {
    struct Point a0 = TO_3D(x_center, y, z);
    struct Point a1 = TO_3D(x_center, y, z + 50.0f);

    struct Line line_a = {color, 1000 / z, a0, a1};
    image_draw_line(image, &line_a);
  }
}