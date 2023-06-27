#include "image.h"
#include "eink-esp32/image.h"
#include <stdint.h>

#define TO_FIXED_POINT(x) ((x) << 16)
#define FROM_FIXED_POINT(x) ((x) >> 16)

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
    } else if (x > (length)) {                                                 \
      x = (length);                                                            \
    }                                                                          \
  }

void image_clear(struct Image *image, enum Color color) {
  uint8_t byte = color | (color << 2) | (color << 4) | (color << 6);
  for (uint32_t i = 0; i < IMAGE_SIZE; i++) {
    image->buffer[i] = byte;
  }
}

static inline void _image_set_pixel(struct Image *image, enum Color color,
                                    uint16_t x, uint16_t y) {
  if (x >= IMAGE_WIDTH || y >= IMAGE_HEIGHT) {
    return;
  }
  uint32_t index = (x >> 2) + y * (IMAGE_WIDTH >> 2);
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
                                     uint8_t thickness, uint16_t y, uint16_t x0,
                                     uint16_t x1) {
  uint16_t thickness_half = thickness / 2;
  uint16_t y0 = y > thickness_half ? y - thickness_half : 0;
  uint16_t y1 = y + thickness_half;
  y1 = y1 < (IMAGE_HEIGHT - 1) ? y1 : IMAGE_HEIGHT - 1;
  for (uint16_t yn = y0; yn <= y1; yn++) {
    for (uint16_t xn = x0; xn <= x1; xn++) {
      _image_set_pixel(image, color, xn, yn);
    }
  }
}

static inline void _image_draw_yline(struct Image *image, enum Color color,
                                     uint8_t thickness, uint16_t x, uint16_t y0,
                                     uint16_t y1) {
  uint16_t thickness_half = thickness / 2;
  uint16_t x0 = x > thickness_half ? x - thickness_half : 0;
  uint16_t x1 = x + thickness_half;
  x1 = x1 < (IMAGE_WIDTH - 1) ? x1 : IMAGE_WIDTH - 1;
  for (uint16_t xn = x0; xn <= x1; xn++) {
    for (uint16_t yn = y0; yn <= y1; yn++) {
      _image_set_pixel(image, color, xn, yn);
    }
  }
}

static inline void _image_draw_line_low_simple(struct Image *image,
                                               enum Color color, uint16_t y,
                                               uint16_t x0, uint16_t x1,
                                               int16_t i, int16_t dx,
                                               int16_t dy) {
  int16_t d = 2 * dy - dx;
  for (uint16_t x = x0; x <= x1; x++) {
    _image_set_pixel(image, color, x, y);
    if (d > 0) {
      y += i;
      d += 2 * (dy - dx);
    } else {
      d += 2 * dy;
    }
  }
}

static inline void _image_draw_line_high_simple(struct Image *image,
                                                enum Color color, uint16_t x,
                                                uint16_t y0, uint16_t y1,
                                                int16_t i, int16_t dx,
                                                int16_t dy) {
  int16_t d = 2 * dx - dy;
  for (uint16_t y = y0; y <= y1; y++) {
    _image_set_pixel(image, color, x, y);
    if (d > 0) {
      x += i;
      d += 2 * (dx - dy);
    } else {
      d += 2 * dx;
    }
  }
}

static inline void _image_draw_line_low(struct Image *image, enum Color color,
                                        uint8_t thickness, uint16_t x0,
                                        uint16_t x1, uint16_t y0, uint16_t y1) {
  // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
  int16_t dx = x1 - x0;
  int16_t dy = y1 - y0;

  int16_t i = 1;
  if (dy < 0) {
    i = -1;
    dy = -dy;
  }

  uint16_t half = thickness / 2;
  int16_t nd = 2 * dy - dx;

  for (int16_t t = 0, dnx = 0; t <= half; t++) {
    uint16_t nx0_l = x0 > dnx ? x0 - dnx : 0;
    uint16_t nx1_l = x1 > dnx ? x1 - dnx : 0;
    uint16_t nx0_r = x0 + dnx;
    uint16_t nx1_r = x1 + dnx;
    _image_draw_line_low_simple(image, color, y0 + t, nx0_l, nx1_l, i, dx, dy);
    _image_draw_line_low_simple(image, color, y0 - t, nx0_r, nx1_r, i, dx, dy);
    // TODO resolve problem with edges

    if (nd < 0) {
      _image_draw_line_low_simple(image, color, y0 - t - 1, nx0_l, nx1_l, i, dx,
                                  dy);
      _image_draw_line_low_simple(image, color, y0 + t + 1, nx0_r, nx1_r, i, dx,
                                  dy);
      dx += i;
      nd += 2 * dx;
    } else {
      nd += 2 * (dy - dx);
    }
  }
}

static inline void _image_draw_line_high(struct Image *image, enum Color color,
                                         uint8_t thickness, uint16_t x0,
                                         uint16_t x1, uint16_t y0,
                                         uint16_t y1) {
  // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
  int16_t dy = y1 - y0;
  int16_t dx = x1 - x0;

  int16_t i = 1;
  if (dx < 0) {
    i = -1;
    dx = -dx;
  }

  uint16_t half = thickness / 2;
  int16_t nd = 2 * dx - dy;

  for (int16_t t = 0, dny = 0; t <= half; t++) {
    uint16_t ny0_l = y0 > dny ? y0 - dny : 0;
    uint16_t ny1_l = y1 > dny ? y1 - dny : 0;
    uint16_t ny0_r = y0 + dny;
    uint16_t ny1_r = y1 + dny;
    _image_draw_line_high_simple(image, color, x0 + t, ny0_l, ny1_l, i, dx, dy);
    _image_draw_line_high_simple(image, color, x0 - t, ny0_r, ny1_r, i, dx, dy);
    // TODO resolve problem with edges

    if (nd < 0) {
      _image_draw_line_high_simple(image, color, x0 + 1 + t, ny0_l, ny1_l, i, dx,
                                   dy);
      _image_draw_line_high_simple(image, color, x0 - 1 - t, ny0_r, ny1_r, i, dx,
                                   dy);
      dny += i;
      nd += 2 * dy;
    } else {
      nd += 2 * (dx - dy);
    }
  }
}

static inline void _image_draw_line(struct Image *image, enum Color color,
                                    uint8_t thickness, uint16_t x0, uint16_t x1,
                                    uint16_t y0, uint16_t y1) {
  // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
  uint16_t abs_dy = y1 > y0 ? y1 - y0 : y0 - y1;
  uint16_t abs_dx = x1 > x0 ? x1 - x0 : x0 - x1;
  if (abs_dy < abs_dx) {
    if (x0 > x1) {
      _image_draw_line_low(image, color, thickness, x1, x0, y1, y0);
    } else {
      _image_draw_line_low(image, color, thickness, x0, x1, y0, y1);
    }
  } else {
    if (y0 > y1) {
      _image_draw_line_high(image, color, thickness, x1, x0, y1, y0);
    } else {
      _image_draw_line_high(image, color, thickness, x0, x1, y0, y1);
    }
  }
}

void image_draw_line(struct Image *image, struct Line *line) {
  int16_t x0 = line->p0.x - image->offset.x;
  int16_t x1 = line->p1.x - image->offset.x;
  int16_t y0 = line->p0.y - image->offset.y;
  int16_t y1 = line->p1.y - image->offset.y;

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
      _image_draw_yline(image, line->color, line->thickness, x0, y0, y1);
    } else { // y0 > y1
      CLIP(y1, y0, IMAGE_HEIGHT - 1);
      _image_draw_yline(image, line->color, line->thickness, x0, y1, y0);
    }
  } else if (dy == 0) {
    ABORT_IF_OUTSIDE(y0, y0, IMAGE_WIDTH - 1);
    if (dx > 0) { // x1 > x0
      CLIP(x0, x1, IMAGE_WIDTH - 1);
      _image_draw_xline(image, line->color, line->thickness, y0, x0, x1);
    } else { // x0 > x1
      CLIP(x1, x0, IMAGE_WIDTH - 1);
      _image_draw_xline(image, line->color, line->thickness, y0, x1, x0);
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
    //        image->offset.x, image->offset.y);

    _image_draw_line(image, line->color, line->thickness, xn0, xn1, yn0, yn1);
  }
}

void image_draw_circle(struct Image *image, struct Circle *circle) {
  int16_t t1 = circle->d / 16, t2;
  int16_t x0 = circle->p.x - image->offset.x;
  int16_t y0 = circle->p.y - image->offset.y;

  int16_t x = circle->d;
  int16_t y = 0;
  while (x >= y) {
    int16_t x_min = x0 - x;
    int16_t x_max = x0 + x;
    int16_t y_min = x0 - y;
    int16_t y_max = x0 + y;

    CLIP_FINAL(x_min, IMAGE_WIDTH - 1);
    CLIP_FINAL(x_max, IMAGE_WIDTH - 1);
    CLIP_FINAL(y_min, IMAGE_WIDTH - 1);
    CLIP_FINAL(y_max, IMAGE_WIDTH - 1);

    if (x_max > x_min) {
      _image_draw_xline(image, circle->color, 1, y0 + y, x_min, x_max);
      _image_draw_xline(image, circle->color, 1, y0 - y, x_min, x_max);
    }

    if (y_max > y_min) {
      _image_draw_xline(image, circle->color, 1, y0 + x, y_min, y_max);
      _image_draw_xline(image, circle->color, 1, y0 - x, y_min, y_max);
    }

    y++;
    t1 = t1 + y;
    t2 = t1 - x;
    if (t2 >= 0) {
      t1 = t2;
      x--;
    }
  }
}

void image_paste_bitmap(struct Image *image, struct Bitmap *bitmap,
                        enum Color color, struct Point p) {
  int16_t x0 = p.x - BITMAP_WIDTH / 2 - image->offset.x;
  int16_t x1 = x0 + BITMAP_WIDTH - 1;
  int16_t y0 = p.y - BITMAP_HEIGHT / 2 - image->offset.y;
  int16_t y1 = y0 + BITMAP_HEIGHT - 1;

  CLIP_FINAL(x0, IMAGE_WIDTH - 1);
  CLIP_FINAL(x1, IMAGE_WIDTH - 1);
  CLIP_FINAL(y0, IMAGE_HEIGHT - 1);
  CLIP_FINAL(y1, IMAGE_HEIGHT - 1);

  uint8_t w = x1 - x0;
  uint8_t h = y1 - y0;

  for (uint16_t by = 0; by <= h; by++) {
    for (uint16_t bx = 0; bx <= w; bx++) {
      uint16_t index = by * (BITMAP_WIDTH >> 3) + (bx >> 3);
      if ((bitmap->buffer[index] << (bx & 0b111)) & 0b10000000) {
        _image_set_pixel(image, color, x0 + bx, y0 + by);
      }
    }
  }
}