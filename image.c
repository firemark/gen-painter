#include "image.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define TO_FIXED_POINT(x) ((x) << 8)
#define FROM_FIXED_POINT(x) ((x) >> 8)

#define ABORT_IF_OUTSIDE(start, end, length)                                   \
  {                                                                            \
    if (end < 0 || start > (length)) {                                         \
      return;                                                                  \
    }                                                                          \
  }

#define CLIP(start, end, length)                                               \
  {                                                                            \
    ABORT_IF_OUTSIDE(start, end, length)                                       \
    start = start > 0 ? start : 0;                                             \
    end = end < length ? end : (length);                                       \
  }

#define CLIP_FINAL(x, length)                                                  \
  {                                                                            \
    if (x < 0) {                                                               \
      x = 0;                                                                   \
    } else if (x > length) {                                                   \
      x = length;                                                              \
    }                                                                          \
  }

void image_clear(struct Image *image) {
  for (uint16_t i = 0; i < IMAGE_SIZE; i++) {
    image->buffer[i] = 0;
  }
}

static inline void _image_set_pixel(struct Image *image, enum Color color,
                                    uint8_t x, uint8_t y) {
  uint16_t index = (x >> 2) * IMAGE_WIDTH + y;
  if (index >= IMAGE_SIZE) {
    return;
  }
  uint8_t *byte = &image->buffer[index];
  switch (x & 0b11) {
  case 0:
    *byte = (*byte & 0b11111100) | color;
    break;
  case 1:
    *byte = (*byte & 0b11110011) | (color << 2);
    break;
  case 2:
    *byte = (*byte & 0b11001111) | (color << 4);
    break;
  case 3:
    *byte = (*byte & 0b00111111) | (color << 6);
    break;
  }
}

static inline void _image_draw_xline(struct Image *image, enum Color color,
                                     uint8_t y, uint8_t x0, uint8_t x1) {
  for (uint8_t x = x0; x <= x1; x++) {
    _image_set_pixel(image, color, x, y);
  }
}

static inline void _image_draw_yline(struct Image *image, enum Color color,
                                     uint8_t x, uint8_t y0, uint8_t y1) {
  for (uint8_t y = y0; y <= y1; y++) {
    _image_set_pixel(image, color, x, y);
  }
}

static inline void _image_draw_line_low(struct Image *image, enum Color color,
                                        uint8_t x0, uint8_t x1, uint8_t y0,
                                        uint8_t y1) {
  // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
  int16_t dx = x1 - x0;
  int16_t dy = y1 - y0;

  uint8_t yi = 1;
  if (dy < 0) {
    yi = -1;
    dy = -dy;
  }

  int16_t d = 2 * dy - dx;
  for (uint8_t x = x0, y = y0; x <= x1; x++) {
    _image_set_pixel(image, color, x, y);
    if (d > 0) {
      y += yi;
      d -= 2 * dx;
    }
    d += 2 * dy;
  }
}

static inline void _image_draw_line_high(struct Image *image, enum Color color,
                                         uint8_t x0, uint8_t x1, uint8_t y0,
                                         uint8_t y1) {
  // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
  int16_t dy = y1 - y0;
  int16_t dx = x1 - x0;

  uint8_t xi = 1;
  if (dx < 0) {
    xi = -1;
    dx = -dx;
  }

  int16_t d = 2 * dx - dy;
  for (uint8_t y = y0, x = x0; y <= y1; y++) {
    _image_set_pixel(image, color, x, y);
    if (d > 0) {
      x += xi;
      d -= 2 * dy;
    }
    d += 2 * dx;
  }
}

static inline void _image_draw_line(struct Image *image, enum Color color,
                                    uint8_t x0, uint8_t x1, uint8_t y0,
                                    uint8_t y1) {
  // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
  uint8_t abs_dy = y1 > y0 ? y1 - y0 : y0 - y1;
  uint8_t abs_dx = x1 > x0 ? x1 - x0 : x0 - x1;
  if (abs_dy < abs_dx) {
    if (x0 > x1) {
      _image_draw_line_low(image, color, x1, x0, y1, y0);
    } else {
      _image_draw_line_low(image, color, x0, x1, y0, y1);
    }
  } else {
    if (y0 > y1) {
      _image_draw_line_high(image, color, x1, x0, y1, y0);
    } else {
      _image_draw_line_high(image, color, x0, x1, y0, y1);
    }
  }
}

void image_draw_line(struct Image *image, struct Line *line) {
  int16_t x0 = line->x0 - image->x_offset;
  int16_t x1 = line->x1 - image->x_offset;
  int16_t y0 = line->y0 - image->y_offset;
  int16_t y1 = line->y1 - image->y_offset;

  int16_t dx = x1 - x0;
  int16_t dy = y1 - y0;

  if (dx == 0 && dy == 0) {
    ABORT_IF_OUTSIDE(x0, x0, IMAGE_WIDTH - 1);
    ABORT_IF_OUTSIDE(y0, y0, IMAGE_HEIGHT - 1);
    _image_set_pixel(image, line->color, x0, y0);
  } else if (dx == 0) {
    ABORT_IF_OUTSIDE(x0, x0, IMAGE_WIDTH - 1);
    if (dy > 0) { // y1 > y0
      CLIP(y0, y1, IMAGE_HEIGHT - 1);
      _image_draw_yline(image, line->color, x0, y0, y1);
    } else { // y0 > y1
      CLIP(y1, y0, IMAGE_HEIGHT - 1);
      _image_draw_yline(image, line->color, x0, y1, y0);
    }
  } else if (dy == 0) {
    ABORT_IF_OUTSIDE(y0, y0, IMAGE_WIDTH - 1);
    if (dx > 0) { // x1 > x0
      CLIP(x0, x1, IMAGE_WIDTH - 1);
      _image_draw_xline(image, line->color, y0, x0, x1);
    } else { // x0 > x1
      CLIP(x1, x0, IMAGE_WIDTH - 1);
      _image_draw_xline(image, line->color, y0, x1, x0);
    }
  } else {
    // https://en.wikipedia.org/wiki/Liang–Barsky_algorithm
    int32_t u_min = 0;
    int32_t u_max = TO_FIXED_POINT(1);

    int32_t u1 = TO_FIXED_POINT(x0) / -dx;
    int32_t u2 = TO_FIXED_POINT(IMAGE_WIDTH - 1 - x0) / +dx;
    if (dx > 0) {
      u_min = u1 > u_min ? u1 : u_min;
      u_max = u2 < u_max ? u2 : u_max;
    } else {
      u_min = u2 > u_min ? u2 : u_min;
      u_max = u1 < u_max ? u1 : u_max;
    }

    int32_t u3 = TO_FIXED_POINT(y0) / -dy;
    int32_t u4 = TO_FIXED_POINT(IMAGE_HEIGHT - 1 - y0) / +dy;
    if (dy > 0) {
      u_min = u3 > u_min ? u3 : u_min;
      u_max = u4 < u_max ? u4 : u_max;
    } else {
      u_min = u4 > u_min ? u4 : u_min;
      u_max = u3 < u_max ? u3 : u_max;
    }

    if (u_min > u_max) {
      return; // outside of clip window
    }

    int16_t xn0 = x0 + (int16_t)FROM_FIXED_POINT(dx * u_min);
    int16_t yn0 = y0 + (int16_t)FROM_FIXED_POINT(dy * u_min);
    int16_t xn1 = x0 + (int16_t)FROM_FIXED_POINT(dx * u_max);
    int16_t yn1 = y0 + (int16_t)FROM_FIXED_POINT(dy * u_max);

    // Round errors clip
    CLIP_FINAL(xn0, IMAGE_WIDTH - 1);
    CLIP_FINAL(xn1, IMAGE_WIDTH - 1);
    CLIP_FINAL(yn0, IMAGE_HEIGHT - 1);
    CLIP_FINAL(yn1, IMAGE_HEIGHT - 1);

    // printf("u_max=%+0.4f; u_min=%+0.4f; "
    //        "pn0=%+4d,%+4d; pn1=%+4d,%+4d; "
    //        "d=%+4d,%+4d; "
    //        "offset=%+4d,%+4d\n",
    //        u_max / 256.0, u_min / 256.0, xn0, yn0, xn1, yn1, dx, dy,
    //        image->x_offset, image->y_offset);

    _image_draw_line(image, line->color, xn0, xn1, yn0, yn1);
  }
}