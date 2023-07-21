#include "clouds.h"
#include "random.h"

#include <stdlib.h>

#include "_share.h"

#define CLOUDS_SIZE 64

struct Cloud {
  struct Point point;
  uint8_t width;
  uint8_t height;
};

static uint16_t _clouds_count;

static struct Cloud *_clouds;

static void _draw_background_cloud_fancy(struct Image *image,
                                         struct Cloud *cloud, uint8_t treshold,
                                         int16_t x_shift, int16_t y_shift,
                                         uint8_t size);

static void _draw_background_cloud(struct Image *image, struct Cloud *cloud,
                                   uint8_t step);

void clouds_init(void) { _clouds = malloc(sizeof(struct Cloud) * CLOUDS_SIZE); }

void clouds_reset(void) { _clouds_count = 0; }

void clouds_generate(void) {
  _clouds_count = random_int(8);
  for (uint8_t i = 0; i < _clouds_count; i++) {
    _clouds[i] = (struct Cloud){
        .point =
            {
                .x = random_int(FULL_IMAGE_WIDTH),
                .y = random_int(500),
            },
        .width = 8 + random_int(8),
        .height = 4 + random_int(3),
    };
  }
}

void clouds_draw(struct Image *image) {
  for (uint8_t i = 0; i < _clouds_count; i++) {
    _draw_background_cloud(image, &_clouds[i], 0);
  }
}

static void _draw_background_cloud_bar(struct Image *image, int16_t x_start,
                                       int16_t x_end, int16_t y, uint8_t width,
                                       uint8_t size, uint8_t threshold) {
  int16_t x = x_start;
  while (x < x_end) {
    int8_t r0 = _random_background_shifts[_background_shifts_index] / 2;
    int8_t r1 = _random_background_shifts[_background_shifts_index + 1] / 2;
    _background_shifts_index =
        (_background_shifts_index + 2) % sizeof(_random_background_shifts);
    x += size / 2 + r0;
    struct Point point = {x, y + r1};
    struct Circle circle = {
        .p = point,
        .d = size,
        .color = WHITE,
    };

    image_draw_circle_threshold(image, &circle, threshold,
                                _background_color == WHITE ? BLACK
                                                           : _background_color);
  }
}

static void _draw_background_cloud_fancy(struct Image *image,
                                         struct Cloud *cloud, uint8_t treshold,
                                         int16_t x_shift, int16_t y_shift,
                                         uint8_t size) {
  const uint8_t mult = 24;
  uint8_t span = (cloud->width - cloud->height) / 4;
  uint8_t _i = 0;
  int16_t y = 0;
  int16_t y_end = (cloud->height) * mult;
  while (y < y_end) {
    uint8_t i = (cloud->height * y) / y_end;
    int16_t x = span * i * mult;
    int16_t x_end = (cloud->width - span * i) * mult;
    while (x < x_end) {
      uint8_t r1 = _random_background_shifts[_i] + 1;
      _i = (_i + 1) % sizeof(_random_background_shifts);
      uint8_t r2 = _random_background_shifts[_i] / 2;
      _i = (_i + 1) % sizeof(_random_background_shifts);
      uint8_t r3 = _random_background_shifts[_i] / 2;
      _i = (_i + 1) % sizeof(_random_background_shifts);
      x += r1;

      struct Point point = {
          .x = cloud->point.x + x + x_shift,
          .y = cloud->point.y - y - y_shift + r3,
      };
      struct Circle circle = {
          .p = point,
          .d = size + r2,
          .color = WHITE,
      };
      image_draw_circle_threshold(
          image, &circle, treshold,
          _background_color == WHITE ? BLACK : _background_color);
    }

    y += _random_background_shifts[_i] + 1;
    _i = (_i + 1) % sizeof(_random_background_shifts);
  }
}

static void _draw_background_cloud(struct Image *image, struct Cloud *cloud,
                                   uint8_t step) {
  const uint8_t mult = 24;
  if (step == 0) {
    uint8_t span = (cloud->width - cloud->height) / 4;
    for (uint8_t i = 0; i < cloud->height; i++) {
      int16_t y = cloud->point.y - i * mult;
      int16_t x_start = cloud->point.x + span * i * mult;
      int16_t x_end = cloud->point.x + (cloud->width - span * i) * mult;
      _draw_background_cloud_bar(image, x_start, x_end, y, 16, 12, 128 - 16);
    }
  } else {
    int8_t r0 = _random_background_shifts[_background_shifts_index];
    _background_shifts_index =
        (_background_shifts_index + 1) % sizeof(_random_background_shifts);
    if (step > 2) {
      return;
    }
  }

  _draw_background_cloud_fancy(image, cloud, 128 - 16, -4, -4, 8);
  _draw_background_cloud_fancy(image, cloud, 128 + 64, 2, 2, 6);
  _draw_background_cloud_fancy(image, cloud, 128, 0, 0, 2);
  _draw_background_cloud_fancy(image, cloud, 128, -2, -2, 2);

  // TODO: UGLY! fix this
  int8_t r0x = 16 - _random_background_shifts[_background_shifts_index];
  _background_shifts_index =
      (_background_shifts_index + 1) % sizeof(_random_background_shifts);
  int8_t r0y = 16 - _random_background_shifts[_background_shifts_index];
  _background_shifts_index =
      (_background_shifts_index + 1) % sizeof(_random_background_shifts);
  int8_t r1x = 16 - _random_background_shifts[_background_shifts_index];
  _background_shifts_index =
      (_background_shifts_index + 1) % sizeof(_random_background_shifts);
  int8_t r1y = 16 - _random_background_shifts[_background_shifts_index];
  _background_shifts_index =
      (_background_shifts_index + 1) % sizeof(_random_background_shifts);

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