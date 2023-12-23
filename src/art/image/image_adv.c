#include "art/image/image_adv.h"

struct Point bezier(float t, struct Point b[4]) {
  struct Point p;
  float t_inv = 1 - t;
  float tt = t * t;
  float ttt = tt * t;
  float tt_inv = t_inv * t_inv;
  float ttt_inv = tt_inv * t_inv;
  float t_tt_inv = t * tt_inv;
  float tt_t_inv = tt * t_inv;

  p.x = b[0].x * ttt_inv + 3 * b[1].x * t_tt_inv + 3 * b[2].x * tt_t_inv +
        b[3].x * ttt;
  p.y = b[0].y * ttt_inv + 3 * b[1].y * t_tt_inv + 3 * b[2].y * tt_t_inv +
        b[3].y * ttt;

  return p;
}