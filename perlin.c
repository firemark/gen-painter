#include "perlin.h"

#include <math.h>

// Code from https://en.wikipedia.org/wiki/Perlin_noise
static inline float _interpolate(float a0, float a1, float w) {
  return (a1 - a0) * w + a0;
}

struct Vec {
  float x;
  float y;
};

struct Vec _random_gradient(int16_t ix, int16_t iy) {
  // No precomputed gradients mean this works for any number of grid coordinates
  const uint16_t w = 8 * sizeof(uint16_t);
  const uint16_t s = w / 2; // rotation width
  unsigned a = ix, b = iy;
  a *= 3284157443;
  b ^= a << s | a >> (w - s);
  b *= 1911520717;
  a ^= b << s | b >> (w - s);
  a *= 2048419325;
  float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]
  struct Vec v;
  v.x = cos(random);
  v.y = sin(random);
  return v;
}

// Computes the dot product of the distance and gradient vectors.
static float _dot_grid_gradient(int16_t ix, int16_t iy, float x, float y) {
  // Get gradient from integer coordinates
  struct Vec gradient = _random_gradient(ix, iy);

  // Compute the distance vector
  float dx = x - (float)ix;
  float dy = y - (float)iy;

  // Compute the dot-product
  return (dx * gradient.x + dy * gradient.y);
}

// Compute Perlin noise at coordinates x, y
static float _perlin(float x, float y) {
  // Determine grid cell coordinates
  int16_t x0 = (int16_t)floor(x);
  int16_t x1 = x0 + 1;
  int16_t y0 = (int16_t)floor(y);
  int16_t y1 = y0 + 1;

  // Determine interpolation weights
  // Could also use higher order polynomial/s-curve here
  float sx = x - (float)x0;
  float sy = y - (float)y0;

  // Interpolate between grid point gradients
  float n0, n1, ix0, ix1, value;

  n0 = _dot_grid_gradient(x0, y0, x, y);
  n1 = _dot_grid_gradient(x1, y0, x, y);
  ix0 = _interpolate(n0, n1, sx);

  n0 = _dot_grid_gradient(x0, y1, x, y);
  n1 = _dot_grid_gradient(x1, y1, x, y);
  ix1 = _interpolate(n0, n1, sx);

  value = _interpolate(ix0, ix1, sy);
  return ((value * 0.5) + 0.5);
}

static inline void _image_set_pixel(struct Image *image, enum Color color,
                                    uint16_t x, uint16_t y) {
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

static inline uint8_t _threshold(uint8_t threshold, uint16_t z) {
  switch (threshold / 16) {
  case 0:
    return 0;
  case 1:
    return z % 128 == 0;
  case 2:
    return z % 64 == 0;
  case 3:
    return z % 32 == 0;
  case 4:
    return z % 16 == 0;
  case 5:
    return z % 8 == 0;
  case 6:
    return z % 4 == 0;
  case 7:
    return z % 2 == 0;
  case 8:
    return z % 4 != 0;
  case 9:
    return z % 8 != 0;
  case 10:
    return z % 16 != 0;
  case 11:
    return z % 32 != 0;
  case 12:
    return z % 64 != 0;
  case 13:
    return z % 128 != 0;
  default:
    return 1;
  }
}

void image_perlin(struct Image *image, enum Color color, uint16_t shift, float size) {
  for (int16_t y = 0; y < IMAGE_HEIGHT; y++) {
    for (int16_t x = 0; x < IMAGE_WIDTH; x++) {
      uint16_t z = x + y;
      float xx = (float)(x + image->offset.x + shift);
      float yy = (float)(y + image->offset.y + shift);
      float t1 = _perlin(xx * size, yy * size * 2);
      float t2 = _perlin(xx * size * 4, yy * size * 4) / 4;
      float t3 = _perlin(xx * size * 8, yy * size * 8) / 8;
      uint8_t t = (t1 + t2 + t3) * 0.5 * 0xFF;
      if (_threshold(t, z)) {
        _image_set_pixel(image, color, x, y);
      }
    }
  }
}