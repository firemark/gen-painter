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
  int16_t x_center = IMAGE_WIDTH / 2 + random_int(100) - 200;
  if (random_int(32) > 24) {
    _draw_mountain(image, x_center + random_int(100) - 300, mountain_width,
                   mountain_height);
    _draw_mountain(image, x_center - random_int(100) + 300, mountain_width / 2,
                   mountain_height / 2);
  } else {
    _draw_mountain(image, x_center, mountain_width,
                   mountain_height);
  }
  _draw_terrain(image);
}

static void _polyfill_mountain(struct Image *image, struct Point *points,
                               uint8_t size, uint8_t threshold) {
  polyfill(image, points, size, _leaves_color, threshold, _background_color);
}

static void _draw_mountain(struct Image *image, int16_t x_center, int16_t width,
                           int16_t height) {
  uint8_t i;
  uint8_t peak_points_size = 12 + random_int(4);
  uint8_t edge_points_size = 6 + random_int(4);
  uint8_t peak_index = peak_points_size / 2 + random_int(2) - 1;

  uint8_t dark_points_size = peak_index + edge_points_size;
  uint8_t light_points_size =
      peak_points_size - peak_index + edge_points_size + 1;
  struct Point *dark_points = malloc(sizeof(struct Point) * dark_points_size);
  struct Point *light_points = malloc(sizeof(struct Point) * light_points_size);

  int16_t span_height_up = height / (peak_index - 1);
  int16_t span_height_down = height / (peak_points_size - peak_index + 1);
  int16_t x_span = width / peak_points_size;

  int16_t x = x_center - width / 2;
  int16_t y = _y;

  x = x_center - width / 2;
  y = _y;
  dark_points[0] = (struct Point){x, _y};
  for (i = 1; i < peak_index; i++) {
    x += x_span;
    y -= span_height_up;
    dark_points[i] =
        (struct Point){x + random_int(32) - 64, y + random_int(32) - 64};
  }

  light_points[0] = dark_points[peak_index - 1];
  int16_t span_height_edge = (_y - y) / edge_points_size;
  for (i = 0; i < edge_points_size; i++) {
    y += span_height_edge;
    struct Point point = {x + random_int(64) - 128, y + random_int(32)};
    dark_points[peak_index + i] = point;
    light_points[1 + i] = point;
  }
  dark_points[dark_points_size - 1].y = _y;
  light_points[edge_points_size].y = _y;

  x = x_center + width / 2;
  y = _y;
  light_points[edge_points_size + 1] = (struct Point){x, _y};
  for (i = edge_points_size + 2; i < light_points_size; i++) {
    x -= x_span;
    y -= span_height_down;
    light_points[i] =
        (struct Point){x + random_int(32) - 64, y + random_int(32) - 64};
  }

  _polyfill_mountain(image, dark_points, dark_points_size, 96u);
  _polyfill_mountain(image, light_points, light_points_size, 128u);
  free(dark_points);
  free(light_points);
}

static void _draw_terrain(struct Image *image) {
  image_draw_rectangle(image, _branches_color, 96u, _leaves_color,
                       (struct Point){0, _y},
                       (struct Point){IMAGE_WIDTH, IMAGE_HEIGHT});
}