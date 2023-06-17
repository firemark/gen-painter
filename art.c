#include "art.h"

static uint8_t _lines_count;
static struct Line _lines[8];

void art_make() {
  _lines_count = 8;
  for (uint8_t i = 0; i < _lines_count; i++) {
    int16_t s = 16 * i;
    _lines[i] = (struct Line){.color = i % 2 ? RED : BLACK,
                              .x0 = 0 + s,
                              .y0 = 128 + s,
                              .x1 = 256 + s,
                              .y1 = 0 + s};
  }
}

void art_draw(struct Image *image) {
  image_clear(image);
  for (uint8_t i = 0; i < _lines_count; i++) {
    image_draw_line(image, &_lines[i]);
  }
}