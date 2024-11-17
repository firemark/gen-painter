#include "art/object/grass.h"
#include "art/random.h"

#include <stdio.h>
#include <stdlib.h>

#include "art/_share.h"

void grass_draw(struct Image *image, int16_t xa, int16_t xb, int16_t y, int16_t tall) {
  int16_t x = xa;
  while (x < xb) {
    int16_t yy = y + random_int(32) - 16;
    x += 4 + random_int(6) - 3;
    int16_t r = random_int(30) - 15;
    struct Point points[4] = {
        {x, yy},
        {x - r / 3, yy - tall * 2},
        {x + r / 3, yy - tall * 3},
        {x + r, yy - tall * 4},
    };
    for (uint8_t i = 0; i < 3; i++) {
      struct Line line = {
          .color = _leaves_color,
          .thickness = 3 - i,
          .p0 = points[i],
          .p1 = points[i + 1],
      };
      image_draw_line(image, &line);
    }
  }
}