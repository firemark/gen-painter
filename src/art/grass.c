#include "art/grass.h"
#include "art/random.h"

#include <stdio.h>
#include <stdlib.h>

#include "art/_share.h"

void grass_draw(struct Image *image, int16_t height) {
  int16_t x = 0;
  while (x < IMAGE_WIDTH) {
    int16_t yy = height - 16 + random_int(32);
    int16_t grass_tall = (32 + random_int(32)) / 4;
    x += 10 - 3 + random_int(3);
    int16_t r = 10 - random_int(20);
    struct Point points[4] = {
        {x, yy},
        {x - r / 4, yy - grass_tall * 2},
        {x + r / 2, yy - grass_tall * 3},
        {x + r, yy - grass_tall * 4},
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