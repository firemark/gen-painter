#include "art/image/3d.h"
#include "art/image/image_adv.h"
#include "art/object/grass.h"
#include "art/random.h"

#include <stdio.h>
#include <stdlib.h>

#include "art/_share.h"

#define TO_2D(x, y, z) to_screen_from_3d(hor, (struct Point3d){x, y, z})
#define DRAW_POLY4(a, b, c, d, threshold) \
    { \
      struct Point points[4] = {a, b, c, d}; \
      polyfill(image, points, 4, _branches_color, threshold, _leaves_color); \
    }

#define DRAW_POLY3(a, b, c, threshold) \
    { \
      struct Point points[3] = {a, b, c}; \
      polyfill(image, points, 3, _branches_color, threshold, _leaves_color); \
    }

void house_draw(struct Image *image, int16_t hor, struct Point3d p0,
                struct Point3d p1, int16_t h) {
  struct Point fl0 = TO_2D(p0.x, p0.y, p0.z);
  struct Point bl0 = TO_2D(p0.x, p0.y, p1.z);
  struct Point fr0 = TO_2D(p1.x, p0.y, p0.z);
  struct Point br0 = TO_2D(p1.x, p0.y, p1.z);

  struct Point fl1 = TO_2D(p0.x, p0.y + h, p0.z);
  struct Point bl1 = TO_2D(p0.x, p0.y + h, p1.z);
  struct Point fr1 = TO_2D(p1.x, p0.y + h, p0.z);
  struct Point br1 = TO_2D(p1.x, p0.y + h, p1.z);

  float mid_z = (p0.z + p1.z) / 2.0;
  struct Point l2 = TO_2D(p0.x, p0.y + 2 * h, mid_z);
  struct Point r2 = TO_2D(p1.x, p0.y + 2 * h, mid_z);

  // {
  //   struct Point points[4] = {dl0, dl1, dr1, dr0};
  //   polyfill(image, points, 4, _branches_color, 128, _leaves_color);
  // }

  {
    DRAW_POLY3(bl1, l2, fl1, 196);
    DRAW_POLY3(br1, r2, fr1, 196);
    DRAW_POLY4(fl1, l2, r2, fr1, 64);
  }

  {
    DRAW_POLY4(fl0, fl1, bl1, bl0, 196);
    DRAW_POLY4(fr0, fr1, br1, br0, 196);
    DRAW_POLY4(fl0, fl1, fr1, fr0, 128);
  }

  {
    struct Line line_up = {
      .color=_branches_color,
      .thickness=2,
      .p0=l2,
      .p1=r2,
    };
    struct Line line_down = {
      .color=_branches_color,
      .thickness=2,
      .p0=fl1,
      .p1=fr1,
    };
    image_draw_line(image, &line_up);
    image_draw_line(image, &line_down);
  }

  for(int16_t x = p0.x; x <= p1.x; x += 10) {
    struct Line line = {
      .color=_branches_color,
      .thickness=2,
      .p0=TO_2D(x, p0.y + 2 * h, mid_z),
      .p1=TO_2D(x, p0.y + 1 * h, p0.z),
    };
    image_draw_line(image, &line);
  }

}