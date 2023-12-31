#include "art/mountain.h"
#include "art/_share.h"
#include "art/image/image_adv.h"
#include "art/random.h"

#include <stdbool.h>
#include <stdlib.h>

struct Peak {
  struct Image *image;
  int16_t x_center;
  int16_t y;
  int16_t width;
  int16_t height;
};

static void _draw_peak(struct Peak *peak, int16_t y_base_left,
                       int16_t y_base_right, bool with_base);
static void _draw_small_peak(struct Peak *peak, int16_t ratio, int16_t shift);

void draw_mountain(struct Image *image, int16_t x_center, int16_t y,
                   int16_t width, int16_t height) {
  int16_t tiny_width = width / 8;
  struct Peak peak = {
      .image = image,
      .x_center = x_center,
      .y = y,
      .width = width,
      .height = height,
  };

  if (random_int(16) > 6) {
    _draw_small_peak(&peak, 2, +tiny_width * 2 + random_int(tiny_width));
  }
  if (random_int(16) > 6) {
    _draw_small_peak(&peak, 2, -tiny_width * 2 - random_int(tiny_width));
  }
  _draw_peak(&peak, y, y, false);
  if (random_int(16) > 6) {
    _draw_small_peak(&peak, 4, +tiny_width + random_int(tiny_width));
  }
  if (random_int(16) > 10) {
    _draw_small_peak(&peak, 4, +tiny_width * 2 + random_int(tiny_width));
  }
  if (random_int(16) > 6) {
    _draw_small_peak(&peak, 4, -tiny_width - random_int(tiny_width));
  }
  if (random_int(16) > 10) {
    _draw_small_peak(&peak, 4, -tiny_width * 2 - random_int(tiny_width));
  }
}

static void _draw_small_peak(struct Peak *peak, int16_t ratio, int16_t shift) {
  int16_t w = peak->width / ratio + random_int_b(peak->width / (ratio * 2));
  int16_t h = peak->height / ratio + random_int_b(peak->height / (ratio * 2));
  int16_t abs_shift = abs(shift);
  if (abs_shift + w / 2 > peak->width / 2 - 32) {
    w = peak->width - 2 * (abs_shift + 32);
  }
  int16_t sign = shift > 0 ? 1 : -1;
  int16_t xa = abs_shift - sign * w / 2;
  int16_t xb = abs_shift + sign * w / 2;
  int16_t h_left = (peak->height * (xa - peak->width / 2) / peak->width) + 20 +
                   random_int(20);
  int16_t h_right = (peak->height * (xb - peak->width / 2) / peak->width) + 20 +
                    random_int(20);
  struct Peak new_peak = {
      .image = peak->image,
      .x_center = peak->x_center + shift,
      .y = peak->y,
      .width = w,
      .height = h,
  };
  _draw_peak(&new_peak, peak->y + h_left, peak->y + h_right, true);
}

struct MountainPoints {
  struct Point *points;
  uint8_t peak_size;
  uint8_t edge_size;
  uint8_t base_size;
  uint8_t size;
};

static struct MountainPoints *
_mountain_points_create(uint8_t peak_size, uint8_t edge_size, bool with_base) {
  uint8_t base_size = with_base ? peak_size / 2 : 0;
  uint8_t size = peak_size + edge_size + base_size;

  struct MountainPoints *points = malloc(sizeof(struct MountainPoints));
  points->points = malloc(sizeof(struct Point) * size);
  points->peak_size = peak_size;
  points->edge_size = edge_size;
  points->base_size = base_size;
  points->size = size;
  return points;
}

static void _mountain_points_destroy(struct MountainPoints *points) {
  free(points->points);
  free(points);
}

static void _polyfill_mountain(struct Image *image,
                               struct MountainPoints *points,
                               uint8_t threshold) {
  polyfill(image, points->points, points->size, _leaves_color, threshold,
           _background_color);
}

static void _draw_mountain_strokes(struct Peak *peak, int16_t count,
                                   struct Point *dark_peak,
                                   struct Point *light_peak, enum Color color) {
  uint8_t i;
  struct Point diff = {(dark_peak->x - light_peak->x),
                       (dark_peak->y - light_peak->y)};
  struct Point vec[2] = {
      {diff.x / 2, diff.y / 2},
      {diff.x / 3, diff.y / 3},
  };

  for (i = 0; i < count; i++) {
    int j;
    int s = -diff.y / 32;
    int16_t rh = random_int(peak->height * 6 / 7);
    int16_t rw = random_int(peak->width * rh / peak->height);
    if (diff.x > 0) {
      rw = -rw;
    }
    struct Point r = {dark_peak->x + rw / 2, dark_peak->y + rh};
    for (j = 0; j < s; j++) {
      struct Point point = {random_int(10) + r.x - diff.y * j / s,
                            random_int(10) + r.y - diff.x * j / s};
      struct Line line = {
          .color = color,
          .thickness = 1,
          .p0 = point,
          .p1.x = point.x - vec[j % 2].x,
          .p1.y = point.y - vec[j % 2].y,
      };
      image_draw_line(peak->image, &line);
    }
  }
}

static uint8_t _fill_mountain_peak(struct MountainPoints *points, uint8_t index,
                                   int16_t x, int16_t y, int16_t span_x,
                                   int16_t span_y, struct Point *rand) {
  uint8_t i;
  points->points[index++] = (struct Point){x, y};
  for (i = 1; i < points->peak_size; i++) {
    x += span_x;
    y += span_y;
    points->points[index++] =
        (struct Point){x + random_int_b(rand->x), y + random_int_b(rand->y)};
  }
  return index;
}

static uint8_t _fill_mountain_base(struct MountainPoints *points, uint8_t index,
                                   int16_t x, int16_t y, int16_t y_target,
                                   int16_t span_x, struct Point *rand) {
  if (points->base_size == 0) {
    return index;
  }

  uint8_t i;
  int span_y = (y_target - y) / points->base_size;
  for (i = 0; i < points->base_size; i++) {
    x += span_x;
    y += span_y;
    struct Point point = {x + random_int_b(rand->x * 2),
                          y + random_int_b(rand->y * 8)};
    points->points[index++] = point;
  }
  return index;
}

static void _draw_peak(struct Peak *peak, int16_t y_base_left,
                       int16_t y_base_right, bool with_base) {
  uint8_t i;
  int16_t y_base_mid = (y_base_left + y_base_right) / 2;
  uint8_t peak_points_size = 32 + random_int(8);
  uint8_t edge_points_size = 12 + random_int(8);
  uint8_t peak_index = peak_points_size / 2 + random_int(8) - 4;
  with_base = false;

  struct MountainPoints *dark =
      _mountain_points_create(peak_index, edge_points_size, with_base);
  struct MountainPoints *light = _mountain_points_create(
      peak_points_size - peak_index, edge_points_size + 1, with_base);

  int16_t span_height_up =
      (peak->height + y_base_mid - y_base_right) / (dark->peak_size - 1);
  int16_t span_height_down =
      (peak->height + y_base_mid - y_base_left) / (light->peak_size + 1);
  int16_t span_x = peak->width / peak_points_size;
  struct Point rand = {peak->width / 100, peak->height / 100};

  uint8_t dark_index = 0;
  uint8_t light_index = 0;

  // Generate left side (darker) of mountain
  // from the bottom left to the center peak.
  dark_index =
      _fill_mountain_peak(dark, dark_index, peak->x_center - peak->width / 2,
                          y_base_left, span_x, -span_height_up, &rand);

  // Generate edge of mountain for left and right side.
  // from the center peak to the center bottom.
  struct Point edge = dark->points[dark_index - 1];
  light->points[light_index++] = edge;
  int16_t span_height_edge = (y_base_mid - edge.y) / edge_points_size;
  int16_t span_x_edge = random_int(rand.x * 2);
  for (i = 0; i < edge_points_size; i++) {
    edge.x += span_x_edge;
    edge.y += span_height_edge;
    int16_t r = rand.x * 2 + (i - edge_points_size / 2);
    if (r > 0) {
      struct Point point = {edge.x + random_int_b(r), edge.y + random_int_b(r)};
      dark->points[dark_index++] = point;
      light->points[light_index++] = point;
    } else {
      dark->points[dark_index++] = edge;
      light->points[light_index++] = edge;
    }
  }
  dark->points[dark_index - 1].y = y_base_mid;
  light->points[light_index - 1].y = y_base_mid;

  // Generate base of the left side
  // from the bottom center to the bottom left.
  dark_index = _fill_mountain_base(dark, dark_index, edge.x, y_base_mid,
                                   y_base_left, -span_x * 2, &rand);

  // Generate base of the right side
  // from the bottom center to the bottom right.
  light_index = _fill_mountain_base(light, light_index, edge.x, y_base_mid,
                                    y_base_right, +span_x * 2, &rand);

  // Generate right side (lighter) of mountain
  // from the bottom right to the center peak.
  light_index =
      _fill_mountain_peak(light, light_index, peak->x_center + peak->width / 2,
                          y_base_right, -span_x, -span_height_down, &rand);

  uint8_t t0;
  uint8_t t1;
  if (_background_color == WHITE) {
    t0 = 128u;
    t1 = 96u;
  } else {
    t0 = 96u;
    t1 = 128u;
  }

  struct Point *dark_peak = &dark->points[dark->peak_size - 1];
  struct Point *light_peak = &light->points[light->size - 4];
  _polyfill_mountain(peak->image, light, t1);
  _draw_mountain_strokes(peak, edge_points_size * 4, dark_peak, light_peak,
                         _background_color == WHITE ? _leaves_color
                                                    : _background_color);
  _polyfill_mountain(peak->image, dark, t0);
  dark_peak = &dark->points[dark->peak_size - 4];
  light_peak = &dark->points[dark->peak_size - 2];
  _draw_mountain_strokes(peak, edge_points_size * 4, light_peak, dark_peak,
                         _background_color == WHITE ? _background_color
                                                    : _leaves_color);

  _mountain_points_destroy(dark);
  _mountain_points_destroy(light);
}