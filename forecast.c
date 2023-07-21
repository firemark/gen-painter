#include "forecast.h"

#include <stdlib.h>

#include "_share.h"

static int16_t _get_number_width(int16_t number, uint8_t zpad);
static int16_t _get_digit_width(uint8_t digit);
static int16_t _draw_number(struct Image *image, struct Point p, int16_t number,
                            uint8_t zpad);
static int16_t _draw_digit(struct Image *image, struct Point *p, uint8_t digit);

static void _draw_background(struct Image *image, int16_t width) {
  image_draw_rectangle(image, _branches_color, 128, (struct Point){0, 0},
                       (struct Point){width, 120});
  image_draw_rectangle(image, _background_color, 128, (struct Point){4, 4},
                       (struct Point){width - 4, 120 - 4});
}

void forecast_draw(struct Image *image) {
  _draw_background(image, 32 + _get_number_width(_temperature, 5) + _get_digit_width('o'));
  struct Point p = {32, 32};
  p.x += _draw_number(image, p, _temperature, 1);
  p.x += _draw_digit(image, &p, 'o');
}

static int16_t _draw_number(struct Image *image, struct Point p, int16_t number,
                            uint8_t zpad) {
  uint8_t digits[5];
  uint8_t size = 0;
  uint16_t abs_number = abs(number);
  int16_t old_x = p.x;

  if (number < 0) {
    p.x += _draw_digit(image, &p, '-');
    zpad--;
  }

  do {
    digits[size++] = abs_number % 10;
    abs_number /= 10;
  } while (abs_number > 0 && size < 5);

  uint8_t zpad_diff = size > zpad ? 0 : zpad - size;
  for (uint8_t i = 0; i < zpad_diff; i++) {
    p.x += _draw_digit(image, &p, 0);
  }
  for (uint8_t i = 0; i < size; i++) {
    p.x += _draw_digit(image, &p, digits[size - 1 - i]);
  }

  return p.x - old_x;
}

static int16_t _get_number_width(int16_t number, uint8_t zpad) {
  uint8_t size = 0;
  uint16_t abs_number = abs(number);
  int16_t width = 0;
  if (number < 0) {
    width += _get_digit_width('-');
    zpad--;
  }

  do {
    width += _get_digit_width(abs_number % 10);
    abs_number /= 10;
    size++;
  } while (abs_number > 0 && size < 5);

  uint8_t zpad_diff = size > zpad ? 0 : zpad - size;
  for (uint8_t i = 0; i < zpad_diff; i++) {
    width += _get_digit_width(0);
  }

  return width;
}

static int16_t _get_digit_width(uint8_t digit) {
  switch (digit) {
  case 0:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 9:
    return 44;
  case 1:
    return 30;
  case 8:
    return 36;
  case 'o':
  case '-':
  case '+':
  case ' ':
    return 32;
  case ',':
    return 8;
  default:
    return 0;
  }
}

static int16_t _draw_digit(struct Image *image, struct Point *p,
                           uint8_t digit) {
  struct Point points[12];
  uint8_t points_size = 0;

  switch (digit) {
  case 0:
    points_size = 5;
    points[0] = (struct Point){.x = 0, .y = 0};
    points[1] = (struct Point){.x = 0, .y = 64};
    points[2] = (struct Point){.x = 32, .y = 64};
    points[3] = (struct Point){.x = 32, .y = 0};
    points[4] = (struct Point){.x = 0, .y = 0};
    break;
  case 1:
    points_size = 3;
    points[0] = (struct Point){.x = 16, .y = 64};
    points[1] = (struct Point){.x = 16, .y = 0};
    points[2] = (struct Point){.x = 0, .y = 16};
    break;
  case 2:
    points_size = 5;
    points[0] = (struct Point){.x = 32, .y = 64};
    points[1] = (struct Point){.x = 0, .y = 64};
    points[2] = (struct Point){.x = 24, .y = 16};
    points[3] = (struct Point){.x = 12, .y = 0};
    points[4] = (struct Point){.x = 0, .y = 16};
    break;
  case 3:
    points_size = 5;
    points[0] = (struct Point){.x = 0, .y = 0};
    points[1] = (struct Point){.x = 32, .y = 0};
    points[2] = (struct Point){.x = 16, .y = 32};
    points[3] = (struct Point){.x = 32, .y = 64};
    points[4] = (struct Point){.x = 0, .y = 64};
    break;
  case 4:
    points_size = 4;
    points[0] = (struct Point){.x = 24, .y = 64};
    points[1] = (struct Point){.x = 24, .y = 0};
    points[2] = (struct Point){.x = 0, .y = 32};
    points[3] = (struct Point){.x = 32, .y = 32};
    break;
  case 5:
    points_size = 6;
    points[0] = (struct Point){.x = 32, .y = 0};
    points[1] = (struct Point){.x = 0, .y = 0};
    points[2] = (struct Point){.x = 0, .y = 32};
    points[3] = (struct Point){.x = 32, .y = 32};
    points[4] = (struct Point){.x = 32, .y = 64};
    points[5] = (struct Point){.x = 0, .y = 64};
    break;
  case 6:
    points_size = 6;
    points[0] = (struct Point){.x = 32, .y = 0};
    points[1] = (struct Point){.x = 0, .y = 0};
    points[2] = (struct Point){.x = 0, .y = 64};
    points[3] = (struct Point){.x = 32, .y = 64};
    points[4] = (struct Point){.x = 32, .y = 32};
    points[5] = (struct Point){.x = 0, .y = 32};
    break;
  case 7:
    points_size = 6;
    points[0] = (struct Point){.x = 0, .y = 0};
    points[1] = (struct Point){.x = 32, .y = 0};
    points[2] = (struct Point){.x = 0, .y = 64};
    points[3] = (struct Point){.x = 16, .y = 32};
    points[4] = (struct Point){.x = 0, .y = 32};
    points[5] = (struct Point){.x = 32, .y = 32};
    break;
  case 8:
    points_size = 11;
    points[0] = (struct Point){.x = 12, .y = 0};
    points[1] = (struct Point){.x = 24, .y = 8};
    points[2] = (struct Point){.x = 24, .y = 24};
    points[3] = (struct Point){.x = 0, .y = 40};
    points[4] = (struct Point){.x = 0, .y = 56};
    points[5] = (struct Point){.x = 12, .y = 64};
    points[6] = (struct Point){.x = 24, .y = 56};
    points[7] = (struct Point){.x = 24, .y = 40};
    points[8] = (struct Point){.x = 0, .y = 24};
    points[9] = (struct Point){.x = 0, .y = 8};
    points[10] = (struct Point){.x = 12, .y = 0};
    break;
  case 9:
    points_size = 6;
    points[0] = (struct Point){.x = 0, .y = 64};
    points[1] = (struct Point){.x = 32, .y = 64};
    points[2] = (struct Point){.x = 32, .y = 0};
    points[3] = (struct Point){.x = 0, .y = 0};
    points[4] = (struct Point){.x = 0, .y = 32};
    points[5] = (struct Point){.x = 32, .y = 32};
    break;
  case 'o':
    points_size = 5;
    points[0] = (struct Point){.x = 0, .y = 0};
    points[1] = (struct Point){.x = 0, .y = 16};
    points[2] = (struct Point){.x = 16, .y = 16};
    points[3] = (struct Point){.x = 16, .y = 0};
    points[4] = (struct Point){.x = 0, .y = 0};
    break;
  case '-':
    points_size = 2;
    points[0] = (struct Point){.x = 0, .y = 32};
    points[1] = (struct Point){.x = 16, .y = 32};
    break;
  case '+':
    points_size = 5;
    points[0] = (struct Point){.x = 0, .y = 32};
    points[1] = (struct Point){.x = 16, .y = 32};
    points[2] = (struct Point){.x = 8, .y = 32};
    points[3] = (struct Point){.x = 8, .y = 24};
    points[4] = (struct Point){.x = 8, .y = 40};
    break;
  case ',':
    points_size = 2;
    points[0] = (struct Point){.x = 4, .y = 60};
    points[1] = (struct Point){.x = 4, .y = 68};
    break;
  default:
    return _get_digit_width(digit);
  }

  uint8_t thickness = 6;

  for (uint8_t i = 1; i < points_size; i++) {
    struct Point *a = &points[i - 1];
    struct Point *b = &points[i];
    struct Line line = {
        .p0 = {.x = p->x + a->x, .y = p->y + a->y},
        .p1 = {.x = p->x + b->x, .y = p->y + b->y},
        .thickness = thickness,
        .color = _branches_color,
    };
    struct Circle cirlce = {
        .p = {.x = p->x + b->x, .y = p->y + b->y},
        .d = thickness / 2,
        .color = _branches_color,
    };
    image_draw_line(image, &line);
    image_draw_circle(image, &cirlce);
  }

  struct Circle cirlce = {
      .p = {.x = p->x + points[0].x, .y = p->y + points[0].y},
      .d = thickness / 2,
      .color = _branches_color,
  };
  image_draw_circle(image, &cirlce);
  return _get_digit_width(digit);
}