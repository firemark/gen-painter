#include "art/background/landscape_fog.h"

#include "art/_share.h"

static const int16_t HEIGHT_UP = 2;
static const int16_t HEIGHT_DOWN = 4;

void draw_landscape_fog(struct Image* image, int16_t y, int16_t width) {
  for (uint8_t i = 0; i < 16; i++) {
    int16_t height = HEIGHT_UP * i;
    struct Point p0 = {0, y - height - HEIGHT_UP};
    struct Point p1 = {width, y - height};
    image_draw_rectangle(image, _leaves_color, (15 - i) * 16, TRANSPARENT, p0,
                         p1);
  }
  for (uint8_t i = 0; i < 10; i++) {
    int16_t height = HEIGHT_DOWN * i;
    struct Point p0 = {0, y + height + HEIGHT_DOWN};
    struct Point p1 = {width, y + height};
    image_draw_rectangle(image, _leaves_color, (9 - i) * 10, TRANSPARENT, p0,
                         p1);
  }
}
