#pragma once
#include <stdint.h>

#define IMAGE_WIDTH 1304
#define IMAGE_HEIGHT 984
#define BUFFER_CHUNK_COUNT 4
#define IMAGE_HEIGHT_CHUNK (IMAGE_HEIGHT / BUFFER_CHUNK_COUNT)
#define IMAGE_SIZE (IMAGE_HEIGHT * (IMAGE_WIDTH >> 2))
#define IMAGE_SIZE_CHUNK (IMAGE_HEIGHT_CHUNK * (IMAGE_WIDTH >> 2))

#define BITMAP_WIDTH 16
#define BITMAP_HEIGHT 16
#define BITMAP_SIZE (BITMAP_HEIGHT * (BITMAP_WIDTH >> 3))

enum Color { WHITE = 0b00, BLACK = 0b01, RED = 0b10, TRANSPARENT = 0b11 };

struct Point {
  int16_t x;
  int16_t y;
};

struct Line {
  enum Color color;
  uint8_t thickness;
  struct Point p0;
  struct Point p1;
};

struct Circle {
  enum Color color;
  struct Point p;
  uint8_t d;
};

struct Image {
  // Because ESP32 doesn't have virtual addresses,
  // It's required to make smaller chunks in different parts of memory.
  uint8_t* buffer[BUFFER_CHUNK_COUNT];
};

struct Bitmap {
  uint8_t buffer[BITMAP_SIZE];
};

struct Image* image_create();
void image_destroy(struct Image* image);
void image_clear(struct Image* image, enum Color color);
void image_draw_hline(struct Image* image, int16_t y, int16_t x0, int16_t x1,
                      enum Color color, uint8_t threshold, enum Color bg_color);
void image_draw_hline_mirror(struct Image* image, int16_t y, int16_t x0,
                             int16_t x1, int horizont);
void image_draw_line(struct Image* image, struct Line* line);
void image_draw_circle(struct Image* image, struct Circle* circle);
void image_paste_bitmap(struct Image* image, struct Bitmap* bitmap,
                        enum Color color, struct Point p);
void image_draw_rectangle(struct Image* image, enum Color color,
                          uint8_t threshold, enum Color bg_color,
                          struct Point p0, struct Point p1);
void image_draw_circle_threshold(struct Image* image, struct Circle* circle,
                                 uint8_t threshold, enum Color background);
char dithering_array_init(void);
void dithering_array_random(void);

static inline enum Color image_get(struct Image* image, uint16_t x,
                                   uint16_t y) {
  uint8_t chunk = y / IMAGE_HEIGHT_CHUNK;
  uint32_t index =
      (x >> 2) + (y - chunk * IMAGE_HEIGHT_CHUNK) * (IMAGE_WIDTH >> 2);
  uint8_t byte = image->buffer[chunk][index];

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
