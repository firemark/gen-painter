#include "art/grass.h"
#include "art/random.h"

#include <stdio.h>
#include <stdlib.h>

#include "art/_share.h"

void grass_draw(struct Image *image, int16_t height) {
  int16_t x = 0;
  while (x < IMAGE_WIDTH) {
    int16_t yy = height - 16 + random_int(32);
    x += 10 - 3 + random_int(3);
    int16_t r = 5 - random_int(10);
    struct Point points[4] = {
        {x, yy},
        {x - r / 4, yy - 10 + random_int(5)},
        {x + r / 2, yy - 20 + random_int(5)},
        {x + r, yy - 30 + random_int(5)},
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