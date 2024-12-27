#include "art/object/rock.h"

#include <math.h>
#include <stdlib.h>

#include "art/image/3d.h"
#include "art/image/image_adv.h"
#include "art/random.h"

#include "art/_share.h"

#define TO_2D(x, y, z) to_screen_from_3d(hor, (struct Point3d){x, y, z})
#define MAX_POINTS_COUNT 8
#define R random_int_b(size / (2 * edges))
#define RR random_int(height / 2)

#if MAX_POINTS_COUNT < 5
#error Too small!
#endif

void rock_draw(struct Image* image, struct Point* p, int16_t size,
               int16_t height, int16_t hor) {
  int edges = random_int(MAX_POINTS_COUNT - 5) + 5;
  int16_t radius_top = size / 3;
  int16_t radius_base = size / 2;
  struct Point base = TO_2D(p->x, 0, p->y);

  struct Point *points_top = malloc(sizeof(struct Point) * edges);
  struct Point *points_base = malloc(sizeof(struct Point) * edges);

  int i;
  float t = 2.f * M_PI * (float)random_int(256) / 1024.f;
  float dt = 2.f * M_PI * 1.f / (float)edges;
  for (i = 0; i < edges; i++) {
    int16_t x, y;

    x = radius_top * cos(t) + p->x + R;
    y = radius_top * sin(t) + p->y + R;
    int16_t h = height - height / 3 * sin(t) + RR;
    points_top[i] = TO_2D(x, h, y);

    x = radius_base * cos(t) + p->x + R;
    y = radius_base * sin(t) + p->y + R;
    points_base[i] = TO_2D(x, 0, y);

    t += dt;
  }

  for (i = 0; i < edges; i++) {
    int next_i = (i + 1) % edges;
    struct Point points[4] = {
      points_top[i],
      points_top[next_i],
      points_base[next_i],
      points_base[i],
    };
    if (points[0].x < points[1].x) {
      continue;
    }
    uint8_t threshold = points[0].x > base.x ? 80 : 180;
    polyfill(image, points, 4, BLACK, threshold, _leaves_color);
    polygon_border(image, points, 4, BLACK, 1);
  }

  polyfill(image, points_top, edges, BLACK, 32, _leaves_color);
  polygon_border(image, points_top, edges, BLACK, 1);

  free(points_top);
  free(points_base);
}
