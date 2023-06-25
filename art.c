#include "art.h"
#include "eink-esp32/image.h"
#include "image.h"

#include <math.h>
#include <stdio.h>

#define LINES_SIZE 1024

static uint16_t _lines_count;
static uint16_t _rain_density;
static struct Line _lines[LINES_SIZE];

static void _tree(uint8_t n, uint16_t x, uint16_t y, uint16_t w, float rot) {
  if (n == 0 || _lines_count >= LINES_SIZE) {
    return;
  }

  if (n > 3 && (art_random() % 100) > 70 + n * 5) {
    return;
  }

  uint16_t nx = x + w * sin(rot);
  uint16_t ny = y - w * cos(rot);
  float rw = (20 - (int8_t)(art_random() % 40)) / 320.0;
  uint16_t nw = w * 3 / 4 + rw;

  _lines[_lines_count++] = (struct Line){
      .color = BLACK,
      .thickness = w * w / 2048 + 1,
      .x0 = x,
      .y0 = y,
      .x1 = nx,
      .y1 = ny,
  };

  float r0 = (20 - (int8_t)(art_random() % 40)) / 320.0;
  float r1 = (20 - (int8_t)(art_random() % 40)) / 160.0;
  float r2 = (20 - (int8_t)(art_random() % 40)) / 160.0;

  _tree(n - 1, nx, ny, nw * 1.1, rot + r0);
  _tree(n - 1, nx, ny, nw, rot + r1 + 0.5);
  _tree(n - 1, nx, ny, nw, rot + r2 - 0.5);
}

static void _rain(struct Image *image) {
  for(uint16_t i = 0; i < _rain_density; i++) {
    uint16_t x = image->x_offset + art_random() % IMAGE_WIDTH;
    uint16_t y = image->y_offset + (art_random() % IMAGE_HEIGHT) - 12;
    struct Line line = {
      .color = RED,
      .thickness = 1,
      .x0 = x,
      .y0 = y,
      .x1 = x,
      .y1 = y + 12,
    };
    image_draw_line(image, &line);
  }
}

void art_make(void) {
  _lines_count = 0;
  uint16_t x = FULL_IMAGE_WIDTH / 2;
  uint16_t y = FULL_IMAGE_HEIGHT - 1;
  _tree(6, x, y, 250, 0);
  _tree(5, x - 300, y, 100 + (art_random() % 40), 0);
  _tree(5, x + 300, y, 100 + (art_random() % 40), 0);

  _rain_density = art_random() % 512;

  printf("total lines: %d\n", _lines_count);
}

void art_draw(struct Image *image) {
  image_clear(image);
  _rain(image);
  for (uint16_t i = 0; i < _lines_count; i++) {
    image_draw_line(image, &_lines[i]);
  }
}