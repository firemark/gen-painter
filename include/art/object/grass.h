#pragma once
#include "art/image/image.h"

void grass_draw(struct Image* image, int16_t xa, int16_t xb, int16_t y,
                int16_t tall);

void grass_draw_vec(struct Image* image, struct Point* p0, struct Point* p1,
                    uint16_t count, int16_t tall);