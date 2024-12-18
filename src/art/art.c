#include "art/art.h"
#include "art/_share.h"
#include "art/background/clouds.h"
#include "art/background/landscape.h"
#include "art/background/landscape_fog.h"
#include "art/background/sun.h"
#include "art/forecast.h"
#include "art/random.h"
#include "art/world.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct ArtData _data;
enum Color _background_color;
enum Color _leaves_color;
enum Color _branches_color;

static void _rain(struct Image* image) {
  uint16_t density = _data.rain_density < 2000 ? _data.rain_density : 2000;
  for (uint16_t i = 0; i < density; i++) {
    struct Point p0 = {random_int(IMAGE_WIDTH), random_int(IMAGE_HEIGHT)};
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

static void _snow(struct Image* image) {
  uint16_t density = _data.snow_density < 2000 ? _data.snow_density : 2000;
  for (uint16_t i = 0; i < density; i++) {
    struct Point p = {random_int(IMAGE_WIDTH), random_int(IMAGE_HEIGHT)};
    for (int8_t j = -1; j <= 1; j++) {
      struct Line line = {
          .p0 = {p.x + 4 * j, p.y},
          .p1 = {p.x - 4 * j, p.y + 8},
          .thickness = 1,
          .color = _branches_color,
      };
      image_draw_line(image, &line);
    }
    {
      struct Line line = {
          .p0 = {p.x + 4, p.y + 4},
          .p1 = {p.x - 4, p.y + 4},
          .thickness = 1,
          .color = _branches_color,
      };
      image_draw_line(image, &line);
    }
  }
}

static void _random_colors(void) {
  switch (random_int(4)) {
    case 0:  // Day
      _background_color = WHITE;
      _leaves_color = RED;
      _branches_color = BLACK;
      break;
    case 1:  // Night 1
      _background_color = BLACK;
      _leaves_color = WHITE;
      _branches_color = RED;
      break;
    case 2:  // Night 2
      _background_color = BLACK;
      _leaves_color = RED;
      _branches_color = WHITE;
      break;
    case 3:  // Afternoon
      _background_color = RED;
      _leaves_color = WHITE;
      _branches_color = BLACK;
      break;
  }
}

static void _reset(void) {
  world_reset();
  random_shuffle_array();
  dithering_array_random();
}

int16_t _horizont_height;
static struct World _world;

uint8_t art_init(void) {
  return world_init() && clouds_init() && dithering_array_init();
}

void art_make(struct ArtData data) {
  _data = data;
  _reset();
  _random_colors();
  world_setup(&_world);
  _horizont_height = landscape_generate();
}

void art_draw(struct Image* image) {
  if (_background_color != WHITE) {
    image_clear(image, _background_color);
  } else {
    image_draw_rectangle(image, WHITE, 192, BLACK, (struct Point){0, 0},
                         (struct Point){IMAGE_WIDTH, IMAGE_HEIGHT});
  }
  int16_t horizont = IMAGE_HEIGHT - _horizont_height;

  sun_draw(image);
  _rain(image);
  _snow(image);
  clouds_draw(image);
  landscape_draw(image);
  world_draw_back(image, &_world, horizont);
  draw_landscape_fog(image, horizont, IMAGE_WIDTH);
  world_draw_front(image, &_world, horizont);
  _rain(image);
  _snow(image);

  forecast_draw(image);
}
