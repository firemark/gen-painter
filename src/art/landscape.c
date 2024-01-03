#include "art/landscape.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "art/image/3d.h"
#include "art/image/image_adv.h"
#include "art/mountain.h"
#include "art/random.h"

#include "art/_share.h"

static int16_t _y;

int16_t landscape_generate(void) {
  int16_t background_size = 148 + random_int(32);
  _y = IMAGE_HEIGHT - background_size;
  return background_size;
};

static void _draw_mountains(struct Image *image);
static void _draw_terrain(struct Image *image);

void landscape_draw(struct Image *image) {
  _draw_mountains(image);
  _draw_terrain(image);
}

static void _draw_mountains(struct Image *image) {
  int16_t mountain_height = 400 + random_int(200);
  int16_t mountain_width = 1000 + random_int(400);
  int16_t x_center = IMAGE_WIDTH / 2 + random_int(100) - 200;
  if (random_int(32) > 24) {
    draw_mountain(image, x_center + random_int(100) - 300, _y, mountain_width,
                  mountain_height);
    draw_mountain(image, x_center - random_int(100) + 300, _y,
                  mountain_width / 2, mountain_height / 2);
  } else {
    draw_mountain(image, x_center, _y, mountain_width, mountain_height);
  }
}

static void _draw_terrain(struct Image *image) {
  image_draw_rectangle(image, _branches_color, 128, _leaves_color,
                       (struct Point){0, _y},
                       (struct Point){IMAGE_WIDTH, IMAGE_HEIGHT});
}