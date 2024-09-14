#include "art/background/landscape_fog.h"
#include "art/image/image_adv.h"

#include "art/_share.h"

#include <stdbool.h>
#include <stdlib.h>

static const int16_t HEIGHT_UP = 4;
static const int16_t HEIGHT_DOWN = 2;
static const int16_t HEIGHT_SKY = 4;

void draw_landscape_fog(struct Image *image, int16_t y, int16_t width) {
  for (uint8_t i = 0; i < 16; i++) {
    int16_t height = HEIGHT_UP * i;
    struct Point p0 = {0, y - height - HEIGHT_UP};
    struct Point p1 = {width, y - height};
    image_draw_rectangle(image, _leaves_color, (15 - i) * 16, TRANSPARENT, p0, p1);
  }
  for (uint8_t i = 0; i < 16; i++) {
    int16_t height = HEIGHT_DOWN * i;
    struct Point p0 = {0, y + height + HEIGHT_DOWN};
    struct Point p1 = {width, y + height};
    image_draw_rectangle(image, _leaves_color, (15 - i) * 16, TRANSPARENT, p0, p1);
  }
}
