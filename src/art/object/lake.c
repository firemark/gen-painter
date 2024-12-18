#include "art/image/3d.h"
#include "art/image/image_adv.h"
#include "art/random.h"

#include "art/_share.h"

#define TO_2D(x, y, z) to_screen_from_3d(hor, (struct Point3d){x, y, z})

void lake_draw(struct Image* image, int16_t hor, struct Point3d p0,
               struct Point3d p1, int16_t h) {
  struct Point ld = TO_2D(p0.x, 0, p0.z);
  struct Point rd = TO_2D(p1.x, 0, p0.z);
  struct Point lu = TO_2D(p0.x, 0, p1.z);
  struct Point ru = TO_2D(p1.x, 0, p1.z);

  struct Point points[] = {lu, ld, rd, ru};
  polyfill_mirror(image, points, 4, hor);
}