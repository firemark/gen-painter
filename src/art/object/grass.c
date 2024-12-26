#include "art/object/grass.h"
#include "art/random.h"

#include "art/_share.h"

static inline void _draw_blade(struct Image* image, struct Point* p,
                               int16_t tall) {
  int16_t r = random_int_b(tall);
  struct Point points[4] = {
      {p->x, p->y},
      {p->x - r / 3, p->y - tall * 2},
      {p->x + r / 3, p->y - tall * 3},
      {p->x + r, p->y - tall * 4},
  };
  for (uint8_t i = 0; i < 3; i++) {
    struct Line line = {
        .color = _leaves_color,
        .thickness = i > 1 ? 2 - i : 1,
        .p0 = points[i],
        .p1 = points[i + 1],
    };
    image_draw_line(image, &line);
  }
}

void grass_draw(struct Image* image, int16_t xa, int16_t xb, int16_t y,
                int16_t tall) {
  struct Point p = {xa, y};
  int16_t x = xa;
  while (p.x < xb) {
    p.x += 4 + random_int_b(3);
    p.y = y + random_int_b(16);
    _draw_blade(image, &p, tall);
  }
}

void grass_draw_vec(struct Image* image, struct Point* p0, struct Point* p1,
                    uint16_t count, int16_t tall) {
  int i;
  float dx = (float)(p1->x - p0->x) / count;
  float dy = (float)(p1->y - p0->y) / count;
  for (i = 0; i < count; i++) {
    struct Point p = {p0->x + (int16_t)(i * dx) + random_int_b(3),
                      p0->y + (int16_t)(i * dy) + random_int_b(3)};
    _draw_blade(image, &p, tall);
  }
}