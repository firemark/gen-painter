#include "art/grass.h"
#include "art/random.h"

#include <stdio.h>
#include <stdlib.h>

#include "art/_share.h"

#define GRASS_SIZE (1024 * 2)

struct LineZ {
  struct Line line;
  uint8_t zdepth;
};

static uint16_t _grass_count;
static struct LineZ *_grass;

static void _grass_generate(int16_t y);

uint8_t grass_init(void) {
  _grass = malloc(sizeof(struct LineZ) * GRASS_SIZE);
  return _grass != NULL;
};

void grass_reset(void) { _grass_count = 0; }

void grass_generate(void) {
  _grass_generate(IMAGE_HEIGHT);
  _grass_generate(IMAGE_HEIGHT - 20);
  _grass_generate(IMAGE_HEIGHT - 40);
  _grass_generate(IMAGE_HEIGHT - 60);
  printf("total grass: %d\n", _grass_count);
}

static void _grass_generate(int16_t y) {

  int16_t x = 0;
  while (x < IMAGE_WIDTH && _grass_count < GRASS_SIZE - 4) {
    int16_t yy = y - 16 + random_int(32);
    x += 10 - 3 + random_int(6);
    int16_t r = -40 + random_int(80);
    struct Point points[4] = {
        {x, yy},
        {x - 5 - r / 4, yy - 20 + random_int(5)},
        {x - 5 + r / 2, yy - 40 + random_int(5)},
        {x - 5 + r, yy - 50 + random_int(5)},
    };
    for (uint8_t i = 0; i < 3; i++) {
      _grass[_grass_count++] = (struct LineZ){
          .line =
              (struct Line){
                  .color = _leaves_color,
                  .thickness = 6 - 2 * i,
                  .p0 = points[i],
                  .p1 = points[i + 1],
              },
          .zdepth = random_int(255),
      };
    }
  }
}

void grass_draw_back(struct Image *image) {
  for (uint16_t i = 0; i < _grass_count; i++) {
    if (_grass[i].zdepth < 128) {
      image_draw_line(image, &_grass[i].line);
    }
  }
}

void grass_draw_front(struct Image *image) {
  for (uint16_t i = 0; i < _grass_count; i++) {
    if (_grass[i].zdepth > 128) {
      image_draw_line(image, &_grass[i].line);
    }
  }
}