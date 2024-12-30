#pragma once
#include "image.h"

void polygon_border(struct Image* image, struct Point* points, uint8_t size,
                    enum Color color, uint8_t thickness);