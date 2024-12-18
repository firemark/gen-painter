#pragma once
#include "image.h"

struct Point bezier(float t, struct Point b[4]);
void polyfill(struct Image* image, struct Point* points, uint8_t size,
              enum Color color, uint8_t threshold, enum Color bg_color);

void polyfill_mirror(struct Image* image, struct Point* points, uint8_t size,
                     int horizont);

void polygon_border(struct Image* image, struct Point* points, uint8_t size,
                    enum Color color, uint8_t thickness);

static inline float lerp(int16_t x0, int16_t x1, float t) {
  return x0 * (1.0 - t) + x1 * t;
}