#include "image.h"

void image_clear(struct Image *image) {
  for (uint16_t i = 0; i < IMAGE_SIZE; i++) {
    image->buffer[i] = 0;
  }
}

static inline void _image_set_pixel(struct Image *image, enum Color color,
                                    uint8_t x, uint8_t y) {
  uint8_t index = (x >> 2) * IMAGE_WIDTH + y;
  uint8_t *byte = &image->buffer[index];
  switch (x & 0b11) {
  case 0:
    *byte = (*byte & ~0b00000011) | color;
    break;
  case 1:
    *byte = (*byte & ~0b00001100) | (color << 2);
    break;
  case 2:
    *byte = (*byte & ~0b00110000) | (color << 4);
    break;
  case 3:
    *byte = (*byte & ~0b11000000) | (color << 6);
    break;
  }
}

void image_draw_line(struct Image *image, struct Line *line) {
  _image_set_pixel(image, line->color, line->x0, line->y0);
}