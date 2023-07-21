#include "art.h"
#include "forecast.h"
#include "random.h"
#include "tree.h"
#include "clouds.h"
#include "grass.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "_share.h"

static uint16_t _rain_density;

static uint8_t _background_size;
static uint8_t _background_shift;

int8_t _random_background_shifts[64];
uint8_t _background_shifts_index;
int16_t _temperature;

enum Color _background_color;
enum Color _leaves_color;
enum Color _branches_color;

static void _rain(struct Image *image) {
  for (uint16_t i = 0; i < _rain_density; i++) {
    struct Point p0 = {image->offset.x + random_int(IMAGE_WIDTH),
                       image->offset.y + random_int(IMAGE_HEIGHT)};
    struct Point p1 = {p0.x - 8, p0.y + 8};
    struct Line line = {
        .p0 = p0,
        .p1 = p1,
        .thickness = 1,
        .color = _branches_color,
    };
    image_draw_line(image, &line);
  }
}

static void _random_colors(void) {
  switch (random_int(4)) {
  case 0: // Day
    _background_color = WHITE;
    _leaves_color = RED;
    _branches_color = BLACK;
    break;
  case 1: // Night 1
    _background_color = BLACK;
    _leaves_color = WHITE;
    _branches_color = RED;
    break;
  case 2: // Night 2
    _background_color = BLACK;
    _leaves_color = RED;
    _branches_color = WHITE;
    break;
  case 3: // Afternoon
    _background_color = RED;
    _leaves_color = WHITE;
    _branches_color = BLACK;
    break;
  }
}

static void _reset(void) {
  tree_reset();
  grass_reset();
  clouds_reset();

  for (uint8_t i = 0; i < sizeof(_random_background_shifts); i++) {
    _random_background_shifts[i] = random_int(32);
  }
}

void art_init(void) {
  tree_init();
  clouds_init();
  grass_init();
}

static void _draw_background_bar(struct Image *image, int16_t y,
                                 uint8_t threshold) {
  int16_t x = 0;
  while (x < FULL_IMAGE_WIDTH) {
    int8_t r0 = _random_background_shifts[_background_shifts_index];
    int8_t r1 = _random_background_shifts[_background_shifts_index + 1];
    _background_shifts_index =
        (_background_shifts_index + 2) % sizeof(_random_background_shifts);
    x += _background_size / 2 + r0;
    struct Point point = {x, y + r1};
    struct Circle circle = {
        .p = point,
        .d = _background_size,
        .color = _branches_color,
    };
    image_draw_circle_threshold(image, &circle, threshold, _background_color);
  }
}


static void _draw_background(struct Image *image) {
  int16_t y = FULL_IMAGE_HEIGHT - 1 - _background_size + _background_shift;
  _background_shifts_index = 0;

  clouds_draw(image);

  _draw_background_bar(image, y - _background_size * 2, 96);
  _draw_background_bar(image, y - _background_size, 112);
  _draw_background_bar(image, y - _background_size / 2, 128);
  _draw_background_bar(image, y, 160);
}

void art_make(int16_t temperature, uint16_t rain_density) {
  _reset();
  _random_colors();
  tree_generate();
  grass_generate();
  clouds_generate();
  _rain_density = rain_density;
  _background_size = 48 + random_int(32);
  _background_shift = random_int(16);
  _temperature = temperature;
}

void art_draw(struct Image *image) {
  image_clear(image, _background_color);
  _rain(image);
  _draw_background(image);

  tree_draw_back(image);
  grass_draw_back(image);
  tree_draw_branches(image);
  grass_draw_front(image);
  tree_draw_front(image);

  forecast_draw(image);
}
