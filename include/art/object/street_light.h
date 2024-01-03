#pragma once
#include "art/image/3d.h"
#include "art/image/image.h"

enum StreetLighStyle { STREET_LIGHT_CLASSIC = 0, STREET_LIGHT_SPHERE = 1 };

void street_light_draw(struct Image *image, enum StreetLighStyle style,
                       struct Point point, int16_t head_height, int16_t height);