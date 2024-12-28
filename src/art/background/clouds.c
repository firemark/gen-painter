#include "art/background/clouds.h"
#include "art/random.h"

#include <stdint.h>
#include <stdlib.h>

#include "art/_share.h"

#define CLOUDS_SIZE 64

struct Cloud {
  struct Point point;
  uint8_t width;
  uint8_t height;
};

static uint8_t _i;
static enum Color _bg_color;

static void _draw_background_cloud_fancy(struct Image* image,
                                         struct Cloud* cloud, uint8_t treshold,
                                         int16_t x_shift, int16_t y_shift,
                                         uint8_t size, enum Color bg_color);

static void _draw_background_cloud(struct Image* image, struct Cloud* cloud,
                                   uint8_t step);

uint8_t clouds_init(void) {
  _bg_color = _background_color == WHITE ? BLACK : _background_color;
  return 1;
}

void clouds_draw(struct Image* image) {
  uint8_t i;
  uint8_t clouds_count = _data.clouds_count;
  for (i = 0; i < clouds_count; i++) {
    struct Cloud cloud = {
        .point.x = random_int(IMAGE_WIDTH),
        .point.y = random_int(IMAGE_HEIGHT - 100),
        .width = 8 + random_int(8),
        .height = 4 + random_int(3),
    };

    _draw_background_cloud(image, &cloud, 0);
  }
}

static void _draw_background_cloud_bar(struct Image* image, int16_t x_start,
                                       int16_t x_end, int16_t y,
                                       uint8_t size, uint8_t threshold) {
  int16_t x = x_start;
  while (x < x_end) {
    x += size / 2 + random_next(&_i) / 2;
    struct Point point = {x, y + random_next(&_i) / 2};
    struct Circle circle = {
        .p = point,
        .d = size,
        .color = WHITE,
    };

    image_draw_circle_threshold(image, &circle, threshold, _bg_color);
  }
}

static void _draw_background_cloud_fancy(struct Image* image,
                                         struct Cloud* cloud, uint8_t treshold,
                                         int16_t x_shift, int16_t y_shift,
                                         uint8_t size, enum Color bg_color) {
  const uint8_t mult = 24;
  uint8_t span = (cloud->width - cloud->height) / 4;
  uint8_t _j = 0;
  int16_t y = 0;
  int16_t y_end = (cloud->height) * mult;
  while (y < y_end) {
    uint8_t i = (cloud->height * y) / y_end;
    int16_t x = span * i * mult;
    int16_t x_end = (cloud->width - span * i) * mult;
    while (x < x_end) {
      x += random_next(&_j) + 1;

      struct Point point = {
          .x = cloud->point.x + x + x_shift,
          .y = cloud->point.y - y - y_shift + random_next(&_j) / 2,
      };
      struct Circle circle = {
          .p = point,
          .d = size + random_next(&_j) / 2,
          .color = WHITE,
      };
      image_draw_circle_threshold(image, &circle, treshold, bg_color);
    }

    y += random_next(&_j) + 1;
  }
}

static void _draw_background_cloud(struct Image* image, struct Cloud* cloud,
                                   uint8_t step) {
  const uint8_t mult = 24;
  if (step == 0) {
    uint8_t span = (cloud->width - cloud->height) / 4;
    for (uint8_t i = 0; i < cloud->height; i++) {
      int16_t y = cloud->point.y - i * mult;
      int16_t x_start = cloud->point.x + span * i * mult;
      int16_t x_end = cloud->point.x + (cloud->width - span * i) * mult;
      _draw_background_cloud_bar(image, x_start, x_end, y, 16,
                                 128 - random_int(16));
    }
  } else {
    if (step > 2) {
      return;
    }
  }

  _draw_background_cloud_fancy(image, cloud, 32, 0, 0, 12, TRANSPARENT);
  _draw_background_cloud_fancy(image, cloud, 128 - 48 - random_int(16), 0, 0, 8,
                               _bg_color);
  _draw_background_cloud_fancy(image, cloud, 128, 2, 2, 6, _bg_color);
  _draw_background_cloud_fancy(image, cloud, 128 - 16 - random_int(4), 0, 0, 2,
                               _bg_color);
  _draw_background_cloud_fancy(image, cloud, 128 - 32 - random_int(4), -2, -2, 2,
                               _bg_color);

  int8_t r0x = 16 - random_next(&_i);
  int8_t r0y = 16 - random_next(&_i);
  int8_t r1x = 16 - random_next(&_i);
  int8_t r1y = 16 - random_next(&_i);

  struct Cloud cloud0 = {
      .point = {.x = cloud->width * 12 + cloud->point.x + r0x * 2,
                .y = cloud->height * 12 + cloud->point.y + r0y * 2},
      .width = cloud->width / 2,
      .height = cloud->height / 2,
  };

  struct Cloud cloud1 = {
      .point = {.x = cloud->width * 12 + cloud->point.x + r1x * 2,
                .y = cloud->height * 12 + cloud->point.y + r1y * 2},
      .width = cloud->width / 2,
      .height = cloud->height / 2,
  };

  _draw_background_cloud(image, &cloud0, step + 1);
  _draw_background_cloud(image, &cloud1, step + 1);
}