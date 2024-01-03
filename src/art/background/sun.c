#include "art/background/sun.h"
#include <math.h>

#include "art/_share.h"

static void _draw_sun(struct Image *image);
static void _draw_moon(struct Image *image);

void sun_draw(struct Image *image) {
  uint8_t hour = _data.minute / 60;
  if (hour < 6) {
    return;
  } else if (hour < 18) {
    _draw_sun(image);
  } else if (hour < 24) {
    _draw_moon(image);
  }
}

static inline float angle_to_x(float angle) {
  return IMAGE_WIDTH * (0.8 * sin(angle / 2.0) + 0.1);
}

static inline float angle_to_y(float angle) {
  return IMAGE_HEIGHT - IMAGE_HEIGHT * (0.5 * sin(angle) + 0.2);
}

static inline uint8_t angle_to_threshold(float angle) {
  return 128.0 * sin(angle) + 64.0;
}

static inline float minute_to_angle(uint16_t minute) {
  return minute * M_PI / (12.0 * 60.0);
}

static void _draw_sun(struct Image *image) {
  float angle = minute_to_angle(_data.minute - 6 * 60);
  int16_t x = angle_to_x(angle);
  int16_t y = angle_to_y(angle);
  struct Circle circle = {
      .p = {x, y},
      .d = 64,
      .color = _leaves_color,
  };
  image_draw_circle_threshold(image, &circle, angle_to_threshold(angle),
                              _background_color);
}

static void _draw_moon(struct Image *image) {
  float angle = 2.0 * minute_to_angle(_data.minute - 18 * 60);
  int16_t x = angle_to_x(angle);
  int16_t y = angle_to_y(angle);
  struct Circle circle = {
      .p = {x, y},
      .d = 64,
      .color = _leaves_color,
  };
  image_draw_circle_threshold(image, &circle, angle_to_threshold(angle),
                              _background_color);
  circle.p.x += 32;
  image_draw_circle_threshold(image, &circle, 0, _background_color);
}