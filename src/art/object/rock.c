#include "art/object/rock.h"
#include "art/image/3d.h"
#include "art/image/image_adv.h"
#include "art/random.h"

#include "art/_share.h"

#define TO_2D(x, y, z) to_screen_from_3d(hor, (struct Point3d){x, y, z})

#define R random_int_b(size / 4)
#define RR random_int(height / 4)
void rock_draw(struct Image* image, struct Point* p, int16_t size,
               int16_t height, int16_t hor) {
  int16_t h = size / 4;
  int16_t hh = size / 2;
  struct Point lf0 = TO_2D(p->x - hh + R, 0, p->y + hh + R);
  struct Point rf0 = TO_2D(p->x + hh + R, 0, p->y + hh + R);
  struct Point rb0 = TO_2D(p->x + hh + R, 0, p->y - hh + R);
  struct Point lb0 = TO_2D(p->x - hh + R, 0, p->y - hh + R);

  struct Point lf1 = TO_2D(p->x - h + R, height + RR - height / 2, p->y + h + R);
  struct Point rf1 = TO_2D(p->x + h + R, height + RR - height / 2, p->y + h + R);
  struct Point rb1 = TO_2D(p->x + h + R, height + RR, p->y - h + R);
  struct Point lb1 = TO_2D(p->x - h + R, height + RR, p->y - h + R);

  struct Point points_top[4] = {lf1, rf1, rb1, lb1};
  struct Point points_left[4] = {lf0, lf1, lb1, lb0};
  struct Point points_right[4] = {rf0, rf1, rb1, rb0};
  struct Point points_front[4] = {lf0, lf1, rf1, rf0};

  polyfill(image, points_right, 4, BLACK, 180, _leaves_color);
  polyfill(image, points_left, 4, BLACK, 200, _leaves_color);
  polyfill(image, points_top, 4, BLACK, 20, _leaves_color);
  polyfill(image, points_front, 4, BLACK, 80, _leaves_color);
  polygon_border(image, points_left, 4, BLACK, 1);
  polygon_border(image, points_right, 4, BLACK, 1);
  polygon_border(image, points_top, 4, BLACK, 1);
}
