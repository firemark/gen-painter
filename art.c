#include "art.h"

static uint8_t _lines_count;
static struct Line _lines[16];

void art_make(void) {
  _lines_count = 16;
  for (uint8_t i = 0; i < _lines_count; i++) {
    int16_t s = 32 * i;
    _lines[i] = (struct Line){.color = i % 2 ? RED : BLACK,
                              .thickness=i + 1,
                              .x0 = 0 + s,
                              .y0 = 0 + s,
                              .x1 = 512 + s,
                              .y1 = 0 + s};
  }
}

void art_draw(struct Image *image) {
  image_clear(image);
  for (uint8_t i = 0; i < _lines_count; i++) {
    image_draw_line(image, &_lines[i]);
  }
}