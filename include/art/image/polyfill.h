#pragma once
#include "image.h"

void polyfill(struct Image* image, struct Point* points, uint8_t size,
              enum Color color, uint8_t threshold, enum Color bg_color);

void polyfill_mirror(struct Image* image, struct Point* points, uint8_t size,
                     int horizont);