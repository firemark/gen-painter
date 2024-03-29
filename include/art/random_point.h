#pragma once
#include "art/image/image.h"

/// @brief Randoms point in parallelogram area.
struct Point random_parallelogram(struct Point start, struct Point a,
                                  struct Point b);

/// @brief Randoms point in triangle area.
struct Point random_triangle(struct Point start, struct Point a,
                             struct Point b);