#include "art/image/border.h"

void polygon_border(struct Image* image, struct Point* points, uint8_t size,
                    enum Color color, uint8_t thickness) {
  struct Line line = {
      .color = color,
      .thickness = thickness,
  };

  if (size < 2) {
    return;
  }

  for (uint8_t i = 0; i < size - 1; i++) {
    line.p0 = points[i];
    line.p1 = points[i + 1];
    image_draw_line(image, &line);
  }

  line.p0 = points[size - 1];
  line.p1 = points[0];
  image_draw_line(image, &line);
}