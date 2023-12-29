#include "art/landscape.h"

#include <stdlib.h>

#include "art/image/image_adv.h"
#include "art/random.h"

#include "art/_share.h"

static int16_t _y;

int16_t landscape_generate(void) {
  int16_t background_size = 148 + random_int(32);
  _y = IMAGE_HEIGHT - background_size;
  return background_size;
};

static void _draw_mountain(struct Image *image, int16_t x_center, int16_t width,
                           int16_t height);
static void _draw_terrain(struct Image *image);

void landscape_draw(struct Image *image) {
  int16_t mountain_height = 400 + random_int(200);
  int16_t mountain_width = 800 + random_int(400);
  int16_t x_center = IMAGE_WIDTH / 2;
  _draw_mountain(image, x_center + random_int(100) - 100, mountain_width,
                 mountain_height);
  _draw_mountain(image, x_center + random_int(100) + 300, mountain_width,
                 mountain_height / 2);
  _draw_terrain(image);
}

static void _draw_mountain(struct Image *image, int16_t x_center, int16_t width,
                           int16_t height) {
  uint8_t i;
  uint8_t points_size = 12 + random_int(4);
  uint8_t peak_index = points_size / 2 + random_int(2) - 1;
  struct Point *points = malloc(sizeof(struct Point) * points_size);

  int16_t span_height_up = height / peak_index;
  int16_t span_height_down = height / (points_size - peak_index);
  int16_t x_span = width / points_size;

  int16_t x = x_center - width / 2;
  int16_t y = _y;

  points[0] = (struct Point){x, _y};
  for (i = 1; i < peak_index; i++) {
    x += x_span;
    y -= span_height_up;
    points[i] =
        (struct Point){x + random_int(32) - 64, y + random_int(32) - 64};
  }
  for (i = peak_index; i < points_size - 1; i++) {
    x += x_span;
    y += span_height_down;
    points[i] =
        (struct Point){x + random_int(32) - 64, y + random_int(32) - 64};
  }
  points[points_size - 1] = (struct Point){x_center + width / 2, _y};

  polyfill(image, points, points_size, _leaves_color, 128u, _background_color);
  free(points);
}

static void _draw_terrain(struct Image *image) {
  image_draw_rectangle(image, _branches_color, 96u, _leaves_color,
                       (struct Point){0, _y},
                       (struct Point){IMAGE_WIDTH, IMAGE_HEIGHT});
}