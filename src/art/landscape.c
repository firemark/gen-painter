#include "art/landscape.h"

#include <stdbool.h>
#include <stdint.h>
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
static void _draw_peak(struct Image *image, int16_t x_center, int16_t width,
                       int16_t height, int16_t y_base_left,
                       int16_t y_base_right, bool with_base);
static void _draw_small_peak(struct Image *image, int16_t x_center,
                             int16_t width, int16_t height, int16_t ratio,
                             int16_t shift);
static void _draw_terrain(struct Image *image);

void landscape_draw(struct Image *image) {
  int16_t mountain_height = 400 + random_int(200);
  int16_t mountain_width = 1000 + random_int(400);
  int16_t x_center = IMAGE_WIDTH / 2 + random_int(100) - 200;
  if (random_int(32) > 24) {
    _draw_mountain(image, x_center + random_int(100) - 300, mountain_width,
                   mountain_height);
    _draw_mountain(image, x_center - random_int(100) + 300, mountain_width / 2,
                   mountain_height / 2);
  } else {
    _draw_mountain(image, x_center, mountain_width, mountain_height);
  }
  _draw_terrain(image);
}

static void _draw_mountain(struct Image *image, int16_t x_center, int16_t width,
                           int16_t height) {
  int16_t tiny_width = width / 8;
  if (random_int(16) > 6) {
    _draw_small_peak(image, x_center, width, height, 2,
                     +tiny_width * 2 + random_int(tiny_width));
  }
  if (random_int(16) > 6) {
    _draw_small_peak(image, x_center, width, height, 2,
                     -tiny_width * 2 - random_int(tiny_width));
  }
  _draw_peak(image, x_center, width, height, _y, _y, false);
  if (random_int(16) > 6) {
    _draw_small_peak(image, x_center, width, height, 4,
                     +tiny_width + random_int(tiny_width));
  }
  if (random_int(16) > 10) {
    _draw_small_peak(image, x_center, width, height, 4,
                     +tiny_width * 2 + random_int(tiny_width));
  }
  if (random_int(16) > 6) {
    _draw_small_peak(image, x_center, width, height, 4,
                     -tiny_width - random_int(tiny_width));
  }
  if (random_int(16) > 10) {
    _draw_small_peak(image, x_center, width, height, 4,
                     -tiny_width * 2 - random_int(tiny_width));
  }
}

static void _draw_small_peak(struct Image *image, int16_t x_center,
                             int16_t width, int16_t height, int16_t ratio,
                             int16_t shift) {

  int16_t w = width / ratio + random_int(width / (ratio * 2));
  int16_t h = height / ratio + random_int(height / (ratio * 2));
  int16_t abs_shift = abs(shift);
  if (abs_shift + w / 2 > width / 2 - 32) {
    w = width - 2 * (abs_shift + 32);
  }
  int16_t sign = shift > 0 ? 1 : -1;
  int16_t xa = abs_shift - sign * w / 2;
  int16_t xb = abs_shift + sign * w / 2;
  int16_t h_left = (height * (xa - width / 2) / width) + 20 + random_int(20);
  int16_t h_right = (height * (xb - width / 2) / width) + 20 + random_int(20);
  _draw_peak(image, x_center + shift, w, h, _y + h_left, _y + h_right, true);
}

static void _polyfill_mountain(struct Image *image, struct Point *points,
                               uint8_t size, uint8_t threshold) {
  polyfill(image, points, size, _leaves_color, threshold, _background_color);
}

static void _draw_peak(struct Image *image, int16_t x_center, int16_t width,
                       int16_t height, int16_t y_base_left,
                       int16_t y_base_right, bool with_base) {
  uint8_t i;
  int16_t y_base_mid = (y_base_left + y_base_right) / 2;
  uint8_t peak_points_size = 32 + random_int(8);
  uint8_t edge_points_size = 12 + random_int(8);
  uint8_t peak_index = peak_points_size / 2 + random_int(8) - 4;

  uint8_t dark_peak_points_size = peak_index;
  uint8_t dark_base_points_size = with_base ? dark_peak_points_size / 2 : 0;
  uint8_t dark_points_size =
      dark_peak_points_size + edge_points_size + dark_base_points_size;
  struct Point *dark_points = malloc(sizeof(struct Point) * dark_points_size);

  uint8_t light_peak_points_size = peak_points_size - peak_index;
  uint8_t light_base_points_size = with_base ? light_peak_points_size / 2 : 0;
  uint8_t light_points_size =
      light_peak_points_size + edge_points_size + light_base_points_size + 1;
  struct Point *light_points = malloc(sizeof(struct Point) * light_points_size);

  int16_t span_height_up =
      (height + y_base_mid - y_base_right) / (dark_peak_points_size - 1);
  int16_t span_height_down =
      (height + y_base_mid - y_base_left) / (light_peak_points_size + 1);
  int16_t span_x = width / peak_points_size;
  int16_t x_rand = width / 100;
  int16_t y_rand = height / 100;

  uint8_t dark_index = 0;
  uint8_t light_index = 0;

  int16_t x;
  int16_t y;

  // Generate left side (darker) of mountain
  // from the bottom left to the center peak.
  x = x_center - width / 2;
  y = y_base_left;
  dark_points[dark_index++] = (struct Point){x, y};
  for (i = 1; i < dark_peak_points_size; i++) {
    x += span_x;
    y -= span_height_up;
    dark_points[dark_index++] =
        (struct Point){x + random_int_b(x_rand), y + random_int_b(y_rand)};
  }

  // Generate edge of mountain for left and right side.
  // from the center peak to the center bottom.
  light_points[light_index++] = dark_points[dark_index - 1];
  int16_t span_height_edge = (y_base_mid - y) / edge_points_size;
  int16_t span_x_edge = random_int(x_rand * 2);
  for (i = 0; i < edge_points_size; i++) {
    x += span_x_edge;
    y += span_height_edge;
    int16_t rand = x_rand * 2 + (i - edge_points_size / 2);
    struct Point point = {x + (rand > 0 ? random_int_b(rand) : 0),
                          y + (rand > 0 ? random_int_b(rand) : 0)};
    dark_points[dark_index++] = point;
    light_points[light_index++] = point;
  }
  dark_points[dark_index - 1].y = y_base_mid;
  light_points[light_index - 1].y = y_base_mid;

  // Generate base of the left side
  // from the bottom center to the bottom left.
  int base_dark_index = dark_index;
  if (dark_base_points_size > 0) {
    int x_dark_base = x;
    int span_left_base = (y_base_left - y_base_mid) / dark_base_points_size;
    y = y_base_mid;
    for (i = 0; i < dark_base_points_size; i++) {
      x_dark_base -= span_x * 2;
      y += span_left_base;
      struct Point point = {x_dark_base + +random_int_b(x_rand * 2),
                            y + random_int_b(y_rand * 8)};
      dark_points[dark_index++] = point;
    }
  }

  // Generate base of the right side
  // from the bottom center to the bottom right.
  int base_light_index = light_index;
  if (light_base_points_size > 0) {
    int x_light_base = x;
    int span_right_base = (y_base_right - y_base_mid) / light_base_points_size;
    y = y_base_mid;
    for (i = 0; i < light_base_points_size; i++) {
      x_light_base += span_x * 2;
      y += span_right_base;
      struct Point point = {x_light_base + random_int_b(x_rand * 2),
                            y + random_int_b(y_rand * 2) * 8};
      light_points[light_index++] = point;
    }
  }

  // Generate right side (lighter) of mountain
  // from the bottom right to the center peak.
  x = x_center + width / 2;
  y = y_base_right;
  light_points[light_index++] = (struct Point){x, y};
  for (i = 1; i < light_peak_points_size; i++) {
    x -= span_x;
    y -= span_height_down;
    light_points[light_index++] =
        (struct Point){x + random_int_b(x_rand), y + random_int_b(y_rand)};
  }

  uint8_t t0 = 96u;
  uint8_t t1 = 128u;

  if (_background_color == WHITE) {
    t0 = 128u;
    t1 = 96u;
  }

  _polyfill_mountain(image, dark_points, dark_points_size, t0);
  _polyfill_mountain(image, light_points, light_points_size, t1);

  struct Point *dark_peak = &dark_points[dark_peak_points_size - 1];
  struct Point *light_peak = &light_points[light_points_size - 4];
  struct Point vec1 = {(dark_peak->x - light_peak->x) / 2, (dark_peak->y - light_peak->y) / 2};
  struct Point vec2 = {(dark_peak->x - light_peak->x) / 3, (dark_peak->y - light_peak->y) / 3};

  for(i = 1; i < edge_points_size; i++) {
    struct Point a = dark_points[dark_peak_points_size + i - 2];
    struct Point b = dark_points[dark_peak_points_size + i - 1];
    struct Point d = {b.x - a.x, b.y - a.y};
    int j;
    int s = d.y / 12;
    for (j=0; j < s; j++) {
      struct Point point = {a.x + d.x * j / s, a.y + d.y * j / s};
      struct Line line = {
        .color=_background_color == WHITE ? _leaves_color : _background_color,
        .thickness=1,
        .p0=point,
        .p1.x=point.x - (j % 2 ? vec1.x : vec2.x),
        .p1.y=point.y - (j % 2 ? vec1.y : vec2.y),
      };
      image_draw_line(image, &line);
    }
  }

  free(dark_points);
  free(light_points);
}

static void _draw_terrain(struct Image *image) {
  image_draw_rectangle(image, _branches_color, 96u, _leaves_color,
                       (struct Point){0, _y},
                       (struct Point){IMAGE_WIDTH, IMAGE_HEIGHT});
}