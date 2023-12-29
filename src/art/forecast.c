#include "art/forecast.h"

#include <stdint.h>
#include <stdlib.h>

#include "art/art_data.h"
#include "art/image/image.h"

#include "art/_share.h"

static int16_t _get_number_width(int16_t number, uint8_t scale, uint8_t zpad);
static int16_t _get_digit_width(uint8_t digit, uint8_t scale);
static int16_t _draw_number(struct Image *image, struct Point p, int16_t number,
                            uint8_t scale, uint8_t zpad);
static int16_t _draw_digit(struct Image *image, struct Point *p, uint8_t digit,
                           uint8_t scale);
static void _draw_icon(struct Image *image, struct Point *p,
                       enum WeatherType weather, uint8_t hour);

static void _draw_background(struct Image *image, int16_t x, int16_t width) {
  image_draw_rectangle(image, _branches_color, 128 + 32, TRANSPARENT,
                       (struct Point){x, 0}, (struct Point){x + width, 128});
  image_draw_rectangle(image, _background_color, 128 + 32, TRANSPARENT,
                       (struct Point){x + 4, 4},
                       (struct Point){x + width - 4, 128 - 4});
}

void forecast_draw(struct Image *image) {
  int16_t shift = 162;
  int16_t width = _get_number_width(0, 4, 3) + _get_digit_width('o', 4);
  int16_t data_width = _get_number_width(0, 2, 5) + _get_digit_width('-', 2);
  int16_t bg_width = shift + width;
  int16_t x = 0;
  for (uint8_t i = 0; i < FORECAST_SIZE; i++) {
    struct Forecast *forecast = &_data.forecast[i];
    _draw_background(image, x, bg_width);

    {
      struct Point p = {x, 0};
      _draw_icon(image, &p, forecast->type, forecast->hour);
    }

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
      .p.x = p->x + points[0].x,
      .p.y = p->y + points[0].y,
      .d = thickness / 2,
      .color = _branches_color,
  };
  image_draw_circle(image, &cirlce);
  return _get_digit_width(digit, s);
}

static void _draw_sun_moon(struct Image *image, struct Point p, uint8_t d,
                           uint8_t hour) {
  struct Circle circle = {
      .p.x = p.x + d,
      .p.y = p.y + d,
      .d = d,
      .color = _leaves_color,
  };
  image_draw_circle(image, &circle);

  if (hour >= 20 || hour < 6) {
    image_draw_circle(image, &circle);

    circle.color = _background_color;
    circle.p.x += d / 2;
    image_draw_circle(image, &circle);
  }
}

static void _draw_cloud(struct Image *image, struct Point p, uint8_t s,
                        uint8_t t) {
  struct Point p0 = {p.x + 1 * s, p.y + 2 * s};
  struct Point p1 = {p.x + 3 * s, p.y + 4 * s};

  struct Circle down_left = {
      .p.x = p.x + 1 * s,
      .p.y = p.y + 3 * s,
      .color = _branches_color,
      .d = s,
  };
  struct Circle down_right = {
      .p.x = p.x + 3 * s,
      .p.y = p.y + 3 * s,
      .color = _branches_color,
      .d = s,
  };

  struct Circle up_left = {
      .p.x = p.x + 3 * s / 2,
      .p.y = p.y + 5 * s / 2,
      .color = _branches_color,
      .d = s,
  };
  struct Circle up_right = {
      .p.x = p.x + 2 * s,
      .p.y = p.y + 9 * s / 4,
      .color = _branches_color,
      .d = s,
  };

  image_draw_rectangle(image, _branches_color, t, TRANSPARENT, p0, p1);
  image_draw_circle_threshold(image, &down_left, t, _background_color);
  image_draw_circle_threshold(image, &down_right, t, _background_color);
  image_draw_circle_threshold(image, &up_left, t, _background_color);
  image_draw_circle_threshold(image, &up_right, t, _background_color);
}

static void _draw_x(struct Image *image, struct Point *p) {
  struct Line line_left = {
      .p0 = {p->x + 12, p->y + 12},
      .p1 = {p->x + 120, p->y + 120},
      .thickness = 8,
      .color = _leaves_color,
  };
  struct Line line_right = {
      .p0 = {p->x + 120, p->y + 12},
      .p1 = {p->x + 12, p->y + 120},
      .thickness = 8,
      .color = _leaves_color,
  };
  image_draw_line(image, &line_left);
  image_draw_line(image, &line_right);
}

static void _draw_bolt(struct Image *image, struct Point p) {
  struct Point points[4] = {
      {.x = p.x, .y = p.y + 0},
      {.x = p.x - 10, .y = p.y + 20},
      {.x = p.x + 10, .y = p.y + 20},
      {.x = p.x - 10, .y = p.y + 48},
  };

  for (uint8_t i = 0; i < 3; i++) {
    uint8_t d = 6 - 2 * i;
    struct Line line = {
        .p0 = points[i + 0],
        .p1 = points[i + 1],
        .thickness = d,
        .color = _leaves_color,
    };

    struct Circle circle = {
        .p = points[i],
        .d = d / 2,
        .color = _leaves_color,
    };

    image_draw_line(image, &line);
    image_draw_circle(image, &circle);
  }
}

static void _draw_tear(struct Image *image, struct Point p) {
  struct Circle circle = {
      .p = p,
      .d = 2,
      .color = _leaves_color,
  };

  for (uint8_t i = 0; i <= 6; i++) {
    circle.d = 1 + i;
    circle.p.y += circle.d / 2 + 1;
    image_draw_circle(image, &circle);
  }
}

static void _draw_snowflake(struct Image *image, struct Point p) {
  for (int8_t i = -1; i <= 1; i++) {
    struct Line line = {
        .p0 = {p.x + 8 * i, p.y},
        .p1 = {p.x - 8 * i, p.y + 16},
        .thickness = 2,
        .color = _leaves_color,
    };
    image_draw_line(image, &line);
  }
  {
    struct Line line = {
        .p0 = {p.x + 8, p.y + 8},
        .p1 = {p.x - 8, p.y + 8},
        .thickness = 2,
        .color = _leaves_color,
    };
    image_draw_line(image, &line);
  }
}

static void _draw_many_clouds(struct Image *image, struct Point *p) {
  _draw_cloud(image, (struct Point){p->x + 18, p->y - 20}, 20, 255);
  _draw_cloud(image, (struct Point){p->x + 32, p->y - 5}, 20, 128 - 16);
}

static void _draw_icon(struct Image *image, struct Point *p,
                       enum WeatherType weather, uint8_t hour) {
  switch (weather) {
  case WEATHER_THUNDERSTORM:
    _draw_bolt(image, (struct Point){p->x + 64, p->y + 72});
    _draw_many_clouds(image, p);
    break;
  case WEATHER_DRIZZLE:
    _draw_cloud(image, (struct Point){p->x + 24, p->y - 18}, 20, 128 - 32);
    for (uint8_t i = 0; i < 5; i++) {
      struct Circle circle = {
          .p = {p->x + 32 + 16 * i, p->y + 72},
          .d = 2,
          .color = _leaves_color,
      };
      image_draw_circle_threshold(image, &circle, 128 - 16, _background_color);

      circle.p.y += 8;
      circle.p.x += 8;
      image_draw_circle_threshold(image, &circle, 128 - 16, _background_color);

      circle.p.y += 8;
      circle.p.x -= 8;
      image_draw_circle_threshold(image, &circle, 128 - 16, _background_color);

      circle.p.y += 8;
      circle.p.x += 8;
      image_draw_circle_threshold(image, &circle, 128 - 16, _background_color);
    }
    break;
  case WEATHER_RAIN:
    _draw_many_clouds(image, p);
    for (uint8_t i = 0; i < 3; i++) {
      _draw_tear(image, (struct Point){p->x + 32 + i * 32, p->y + 80});
    }
    break;
  case WEATHER_SHOWER_RAIN:
    _draw_many_clouds(image, p);
    for (uint8_t i = 0; i < 3; i++) {
      _draw_tear(image, (struct Point){p->x + 32 + i * 32, p->y + 80});
      _draw_tear(image, (struct Point){p->x + 48 + i * 32, p->y + 90});
    }
    break;
  case WEATHER_SNOW:
    _draw_many_clouds(image, p);
    for (uint8_t i = 0; i < 3; i++) {
      _draw_snowflake(image, (struct Point){p->x + 32 + i * 32, p->y + 80});
    }
    break;
  case WEATHER_SHOWER_SNOW:
    _draw_many_clouds(image, p);
    for (uint8_t i = 0; i < 3; i++) {
      _draw_snowflake(image, (struct Point){p->x + 32 + i * 32, p->y + 80});
      _draw_snowflake(image, (struct Point){p->x + 48 + i * 32, p->y + 100});
    }
    break;
  case WEATHER_FOG:
    image_draw_rectangle(image, _branches_color, 128 - 64, TRANSPARENT,
                         (struct Point){p->x + 12, p->y + 12},
                         (struct Point){p->x + 120, p->y + 100});
    _draw_cloud(image, (struct Point){p->x + 12, p->y + 10}, 27, 128 - 32);
    break;
  case WEATHER_CLEAR:
    _draw_sun_moon(image, (struct Point){p->x + 12, p->y + 10}, 52, hour);
    break;
  case WEATHER_FEW_CLOUDS:
    _draw_sun_moon(image, (struct Point){p->x + 48, p->y + 10}, 48, hour);
    _draw_cloud(image, (struct Point){p->x + 12, p->y}, 27, 255);
    break;
  case WEATHER_MANY_CLOUDS:
    _draw_cloud(image, (struct Point){p->x + 8, p->y - 20}, 20, 128 - 32);
    _draw_cloud(image, (struct Point){p->x + 12, p->y - 24}, 27, 255);
    _draw_cloud(image, (struct Point){p->x + 32, p->y + 28}, 20, 128 - 16);
    break;
  default:
    _draw_x(image, p);
    break;
  }
}