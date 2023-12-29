#include "art/landscape.h"
#include "art/image/image_adv.h"
#include "art/random.h"

#include "art/_share.h"

static int16_t _background_size;
static uint8_t _i;

static void _draw_background_bar(struct Image *image, int16_t y,
                                 uint8_t threshold);

int16_t landscape_generate(void) {
  _background_size = 148 + random_int(32);
  return _background_size;
};

void landscape_draw(struct Image *image) {
  int16_t y = IMAGE_HEIGHT - _background_size;
  _i = 0;
  struct Point points[5] = {
      {0, y},
      {IMAGE_WIDTH * 1 / 4, y - 200},
      {IMAGE_WIDTH * 1 / 2, y - 300},
      {IMAGE_WIDTH * 3 / 4, y - 100},
      {IMAGE_WIDTH, y},
  };
  polyfill(image, points, 5, _leaves_color, 128u, _background_color);
  image_draw_rectangle(image, _branches_color, 96u, _leaves_color,
                       (struct Point){0, y},
                       (struct Point){IMAGE_WIDTH, IMAGE_HEIGHT});
}