#include "art/landscape.h"
#include "art/image/image_adv.h"
#include "art/random.h"

#include "art/_share.h"

static uint8_t _background_size;
static uint8_t _background_shift;
static uint8_t _i;

static void _draw_background_bar(struct Image *image, int16_t y,
                                 uint8_t threshold);

void landscape_generate(void) {
  _background_size = 48 + random_int(32);
  _background_shift = random_int(16);
};

void landscape_draw(struct Image *image) {
  int16_t y = IMAGE_HEIGHT - 1 - _background_size + _background_shift;
  _i = 0;
  struct Point points[4] = {
      {0, 160},
      {IMAGE_WIDTH * 1 / 4, 200},
      {IMAGE_WIDTH * 1 / 2, 300},
      {IMAGE_WIDTH * 3 / 4, 100},
  };
  polyfill(points, 4, _branches_color);
  _draw_background_bar(image, y - _background_size * 2, 96);
  _draw_background_bar(image, y - _background_size, 112);
  _draw_background_bar(image, y - _background_size / 2, 128);
  _draw_background_bar(image, y, 160);
}

static void _draw_background_bar(struct Image *image, int16_t y,
                                 uint8_t threshold) {
  int16_t x = 0;
  while (x < IMAGE_WIDTH) {
    x += _background_size / 2 + random_next(&_i);
    struct Point point = {x, y + random_next(&_i)};
    struct Circle circle = {
        .p = point,
        .d = _background_size,
        .color = _branches_color,
    };
    image_draw_circle_threshold(image, &circle, threshold, _background_color);
  }
}