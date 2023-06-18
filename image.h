#pragma once
#include <stdint.h>

#define IMAGE_WIDTH 48
#define IMAGE_HEIGHT 48
#define IMAGE_SIZE (IMAGE_HEIGHT * (IMAGE_WIDTH >> 2))

enum Color { WHITE = 0b00, BLACK = 0b01, RED = 0b10 };

struct Line {
  enum Color color;
  int16_t x0;
  int16_t y0;
  int16_t x1;
  int16_t y1;
};

struct Image {
  int16_t x_offset;
  int16_t y_offset;
  uint8_t buffer[IMAGE_SIZE];
};

void image_clear(struct Image *image);
void image_draw_line(struct Image *image, struct Line *line);
static inline enum Color image_get(struct Image *image, uint8_t x, uint8_t y) {
  uint16_t index = (x >> 2) * IMAGE_WIDTH + y;
  uint8_t byte = image->buffer[index];
  switch (x & 0b11) {
  case 0:
    return (byte & 0b00000011);
  case 1:
    return (byte & 0b00001100) >> 2;
  case 2:
    return (byte & 0b00110000) >> 4;
  case 3:
    return (byte & 0b11000000) >> 6;
  default:
    return WHITE;
  }
}