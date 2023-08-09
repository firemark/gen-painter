#include "forecast.h"

#include <stdlib.h>

#include "_share.h"

static int16_t _get_number_width(int16_t number, uint8_t scale, uint8_t zpad);
static int16_t _get_digit_width(uint8_t digit, uint8_t scale);
static int16_t _draw_number(struct Image *image, struct Point p, int16_t number,
                            uint8_t scale, uint8_t zpad);
static int16_t _draw_digit(struct Image *image, struct Point *p, uint8_t digit,
                           uint8_t scale);

static void _draw_background(struct Image *image, int16_t x, int16_t width) {
  image_draw_rectangle(image, _branches_color, 128, (struct Point){x, 0},
                       (struct Point){x + width, 128});
  image_draw_rectangle(image, _background_color, 128, (struct Point){x + 4, 4},
                       (struct Point){x + width - 4, 128 - 4});
}

void forecast_draw(struct Image *image) {
  int16_t shift = 118;
  int16_t width = _get_number_width(0, 4, 4) + _get_digit_width('o', 4);
  int16_t data_width = _get_number_width(0, 2, 5) + _get_digit_width('-', 2);
  int16_t bg_width = shift + width;
  int16_t x = 0;
  for (uint8_t i = 0; i < 4; i++) {
    struct Forecast *forecast = &_data.forecast[i];
    _draw_background(image, x, bg_width);

    {
      struct Point p = {x + shift, 52};
      p.x += _draw_number(image, p, forecast->temperature, 4, 1);
      p.x += _draw_digit(image, &p, 'o', 4);
    }

    {
      struct Point p = {x + shift + (width - data_width) / 2, 8};
      p.x += _draw_number(image, p, forecast->hour, 2, 2);
      p.x += _draw_digit(image, &p, '-', 2);
      p.x += _draw_number(image, p, forecast->minute, 2, 2);
    }
    x += bg_width;
  }
}

static int16_t _draw_number(struct Image *image, struct Point p, int16_t number,
                            uint8_t scale, uint8_t zpad) {
  uint8_t digits[5];
  uint8_t size = 0;
  uint16_t abs_number = abs(number);
  int16_t old_x = p.x;

  if (number < 0) {
    p.x += _draw_digit(image, &p, '-', scale);
    zpad--;
  }

  do {
    digits[size++] = abs_number % 10;
    abs_number /= 10;
  } while (abs_number > 0 && size < 5);

  uint8_t zpad_diff = size > zpad ? 0 : zpad - size;
  for (uint8_t i = 0; i < zpad_diff; i++) {
    p.x += _draw_digit(image, &p, 0, scale);
  }
  for (uint8_t i = 0; i < size; i++) {
    p.x += _draw_digit(image, &p, digits[size - 1 - i], scale);
  }

  return p.x - old_x;
}

static int16_t _get_number_width(int16_t number, uint8_t zpad, uint8_t scale) {
  uint8_t size = 0;
  uint16_t abs_number = abs(number);
  int16_t width = 0;
  if (number < 0) {
    width += _get_digit_width('-', scale);
    zpad--;
  }

  do {
    width += _get_digit_width(abs_number % 10, scale);
    abs_number /= 10;
    size++;
  } while (abs_number > 0 && size < 5);

  uint8_t zpad_diff = size > zpad ? 0 : zpad - size;
  for (uint8_t i = 0; i < zpad_diff; i++) {
    width += _get_digit_width(0, scale);
  }

  return width;
}

static int16_t _get_digit_width(uint8_t digit, uint8_t scale) {
  switch (digit) {
  case 0:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 9:
    return 11 * scale;
  case 1:
    return 8 * scale;
  case 8:
    return 9 * scale;
  case 'o':
  case '-':
  case '+':
  case ' ':
    return 8 * scale;
  case ',':
    return 2 * scale;
  default:
    return 0;
  }
}

static int16_t _draw_digit(struct Image *image, struct Point *p, uint8_t digit,
                           uint8_t s) {
  struct Point points[12];
  uint8_t points_size = 0;

  switch (digit) {
  case 0:
    points_size = 5;
    points[0] = (struct Point){.x = 0, .y = 0};
    points[1] = (struct Point){.x = 0, .y = 16 * s};
    points[2] = (struct Point){.x = 8 * s, .y = 16 * s};
    points[3] = (struct Point){.x = 8 * s, .y = 0};
    points[4] = (struct Point){.x = 0, .y = 0};
    break;
  case 1:
    points_size = 3;
    points[0] = (struct Point){.x = 4 * s, .y = 16 * s};
    points[1] = (struct Point){.x = 4 * s, .y = 0};
    points[2] = (struct Point){.x = 0, .y = 4 * s};
    break;
  case 2:
    points_size = 5;
    points[0] = (struct Point){.x = 8 * s, .y = 16 * s};
    points[1] = (struct Point){.x = 0, .y = 16 * s};
    points[2] = (struct Point){.x = 6 * s, .y = 4 * s};
    points[3] = (struct Point){.x = 3 * s, .y = 0};
    points[4] = (struct Point){.x = 0, .y = 4 * s};
    break;
  case 3:
    points_size = 5;
    points[0] = (struct Point){.x = 0, .y = 0};
    points[1] = (struct Point){.x = 8 * s, .y = 0};
    points[2] = (struct Point){.x = 4 * s, .y = 8 * s};
    points[3] = (struct Point){.x = 8 * s, .y = 16 * s};
    points[4] = (struct Point){.x = 0, .y = 16 * s};
    break;
  case 4:
    points_size = 4;
    points[0] = (struct Point){.x = 6 * s, .y = 16 * s};
    points[1] = (struct Point){.x = 6 * s, .y = 0};
    points[2] = (struct Point){.x = 0, .y = 8 * s};
    points[3] = (struct Point){.x = 8 * s, .y = 8 * s};
    break;
  case 5:
    points_size = 6;
    points[0] = (struct Point){.x = 8 * s, .y = 0};
    points[1] = (struct Point){.x = 0, .y = 0};
    points[2] = (struct Point){.x = 0, .y = 8 * s};
    points[3] = (struct Point){.x = 8 * s, .y = 8 * s};
    points[4] = (struct Point){.x = 8 * s, .y = 16 * s};
    points[5] = (struct Point){.x = 0, .y = 16 * s};
    break;
  case 6:
    points_size = 6;
    points[0] = (struct Point){.x = 8 * s, .y = 0};
    points[1] = (struct Point){.x = 0, .y = 0};
    points[2] = (struct Point){.x = 0, .y = 16 * s};
    points[3] = (struct Point){.x = 8 * s, .y = 16 * s};
    points[4] = (struct Point){.x = 8 * s, .y = 8 * s};
    points[5] = (struct Point){.x = 0, .y = 8 * s};
    break;
  case 7:
    points_size = 6;
    points[0] = (struct Point){.x = 0, .y = 0};
    points[1] = (struct Point){.x = 8 * s, .y = 0};
    points[2] = (struct Point){.x = 0, .y = 16 * s};
    points[3] = (struct Point){.x = 4 * s, .y = 8 * s};
    points[4] = (struct Point){.x = 0, .y = 8 * s};
    points[5] = (struct Point){.x = 8 * s, .y = 8 * s};
    break;
  case 8:
    points_size = 11;
    points[0] = (struct Point){.x = 3 * s, .y = 0};
    points[1] = (struct Point){.x = 6 * s, .y = 2 * s};
    points[2] = (struct Point){.x = 6 * s, .y = 6 * s};
    points[3] = (struct Point){.x = 0, .y = 10 * s};
    points[4] = (struct Point){.x = 0, .y = 14 * s};
    points[5] = (struct Point){.x = 3 * s, .y = 16 * s};
    points[6] = (struct Point){.x = 6 * s, .y = 14 * s};
    points[7] = (struct Point){.x = 6 * s, .y = 10 * s};
    points[8] = (struct Point){.x = 0, .y = 6 * s};
    points[9] = (struct Point){.x = 0, .y = 2 * s};
    points[10] = (struct Point){.x = 3 * s, .y = 0};
    break;
  case 9:
    points_size = 6;
    points[0] = (struct Point){.x = 0, .y = 16 * s};
    points[1] = (struct Point){.x = 8 * s, .y = 16 * s};
    points[2] = (struct Point){.x = 8 * s, .y = 0};
    points[3] = (struct Point){.x = 0, .y = 0};
    points[4] = (struct Point){.x = 0, .y = 8 * s};
    points[5] = (struct Point){.x = 8 * s, .y = 8 * s};
    break;
  case 'o':
    points_size = 5;
    points[0] = (struct Point){.x = 0, .y = 0};
    points[1] = (struct Point){.x = 0, .y = 4 * s};
    points[2] = (struct Point){.x = 4 * s, .y = 4 * s};
    points[3] = (struct Point){.x = 4 * s, .y = 0};
    points[4] = (struct Point){.x = 0, .y = 0};
    break;
  case '-':
    points_size = 2;
    points[0] = (struct Point){.x = 0, .y = 8 * s};
    points[1] = (struct Point){.x = 4 * s, .y = 8 * s};
    break;
  case '+':
    points_size = 5;
    points[0] = (struct Point){.x = 0, .y = 8 * s};
    points[1] = (struct Point){.x = 4 * s, .y = 8 * s};
    points[2] = (struct Point){.x = 2 * s, .y = 8 * s};
    points[3] = (struct Point){.x = 2 * s, .y = 6 * s};
    points[4] = (struct Point){.x = 2 * s, .y = 10 * s};
    break;
  case ',':
    points_size = 2;
    points[0] = (struct Point){.x = s, .y = 60 - s};
    points[1] = (struct Point){.x = s, .y = 64 + s};
    break;
  default:
    return _get_digit_width(digit, s);
  }

  uint8_t thickness = s + 2;

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
  return _get_digit_width(digit, s);
}