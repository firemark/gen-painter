#include "art/object/lake.h"

#include <stdlib.h>
#include <math.h>

#include "art/image/image_adv.h"
#include "art/object/grass.h"
#include "art/random.h"

#include "art/_share.h"

#define TO_2D(x, y) to_screen_from_3d(hor, (struct Point3d){x, 0, y})
#define POINTS_COUNT 20
#define NOISE_SIZE 32

void lake_draw(struct Image* image, int16_t hor, struct Point2d p0,
               struct Point2d p1) {
  struct Point points[POINTS_COUNT];
  float a = (float)(p1.x - p0.x) * 0.5f;
  float b = (float)(p1.y - p0.y) * 0.5f;
  float cx = (float)(p1.x + p0.x) * 0.5f;
  float cy = (float)(p1.y + p0.y) * 0.5f;

  int i;
  for (i = 0; i < POINTS_COUNT; i++) {
    float t = 2.f * M_PI * ((float)i / (float)POINTS_COUNT);
    int16_t x = (a - random_int(NOISE_SIZE)) * cos(t) + cx;
    int16_t y = (b - random_int(NOISE_SIZE)) * sin(t) + cy;
    points[i] = TO_2D(x, y);
  }

  polyfill_mirror(image, points, POINTS_COUNT, hor);
  polyfill(image, points, POINTS_COUNT, _background_color, 32, TRANSPARENT);

  for (i = 0; i < POINTS_COUNT; i++) {
    struct Point *a = &points[i];
    struct Point *b = &points[(i + 1) % POINTS_COUNT];
    int16_t count = (abs(a->x - b->x) + abs(a->y - b->y));
    grass_draw_vec(image, a, b, count / 5, 8);
    grass_draw_vec(image, a, b, count / 2, 3);
  }
}