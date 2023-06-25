#pragma once
#include <stdint.h>

#define IMAGE_WIDTH 656
#define IMAGE_HEIGHT 492
#define IMAGE_SIZE (IMAGE_HEIGHT * (IMAGE_WIDTH >> 2))

#define FULL_IMAGE_WIDTH 1304
#define FULL_IMAGE_HEIGHT 984

#define BITMAP_WIDTH 16
#define BITMAP_HEIGHT 16
#define BITMAP_SIZE (BITMAP_HEIGHT * (BITMAP_WIDTH >> 3))

enum Color { WHITE = 0b00, BLACK = 0b01, RED = 0b10 };

struct Line {
  enum Color color;
  uint8_t thickness;
  int16_t x0;
  int16_t y0;
  int16_t x1;
  int16_t y1;
};

struct Circle {
  enum Color color;
  uint8_t d;
  int16_t x;
  int16_t y;
};

struct Image {
  int16_t x_offset;
  int16_t y_offset;
  uint8_t buffer[IMAGE_SIZE];
};

struct Bitmap {
  uint8_t buffer[BITMAP_SIZE];
};

void image_clear(struct Image *image, enum Color color);
void image_draw_line(struct Image *image, struct Line *line);
void image_draw_circle(struct Image *image, struct Circle *circle);
void image_paste_bitmap(struct Image *image, struct Bitmap *bitmap,
                        enum Color color, int16_t x, int16_t y);

static inline enum Color image_get(struct Image *image, uint16_t x,
                                   uint16_t y) {
  uint32_t index = (x >> 2) + y * (IMAGE_WIDTH >> 2);
  uint8_t byte = image->buffer[index];
  switch (x & 0b11) {
  case 0:
    return (enum Color)(byte & 0b00000011);
  case 1:
    return (enum Color)((byte & 0b00001100) >> 2);
  case 2:
    return (enum Color)((byte & 0b00110000) >> 4);
  case 3:
    return (enum Color)((byte & 0b11000000) >> 6);
  default:
    return WHITE;
  }
}