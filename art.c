#include "art.h"
#include "forecast.h"
#include "random.h"
#include "tree.h"
#include "clouds.h"
#include "grass.h"
#include "landscape.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "_share.h"

static uint16_t _rain_density;

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
  random_shuffle_array();
}

void art_init(void) {
  tree_init();
  clouds_init();
  grass_init();
}

void art_make(int16_t temperature, uint16_t rain_density) {
  _reset();
  _random_colors();
  tree_generate();
  grass_generate();
  clouds_generate();
  landscape_generate();
  _rain_density = rain_density;
  _temperature = temperature;
}

void art_draw(struct Image *image) {
  image_clear(image, _background_color);

  _rain(image);
  clouds_draw(image);
  landscape_draw(image);

  tree_draw_back(image);
  grass_draw_back(image);
  tree_draw_branches(image);
  grass_draw_front(image);
  tree_draw_front(image);

  forecast_draw(image);
}
