#include "art/object/street_light.h"
#include "art/_share.h"
#include "art/image/image_adv.h"

static void _draw_classic(struct Image *image, struct Point head,
                          struct Line line_stick, int16_t head_height);
static void _draw_sphere(struct Image *image, struct Point head,
                         struct Line line_stick, int16_t head_height);

void street_light_draw(struct Image *image, enum StreetLighStyle style,
                       struct Point point, int16_t head_height,
                       int16_t height) {
  struct Point head = {point.x, point.y - height};

  struct Line line_stick = {
      .thickness = height / 30,
      .color = _branches_color == WHITE ? _background_color : _branches_color,
  };

  struct Line line_stick_bottom = {
      .thickness = line_stick.thickness * 2,
      .color = line_stick.color,
  };

  struct Line line_head = {
      .thickness = line_stick.thickness / 8,
      .color = line_stick.color,
  };

  int16_t sb = height / 3;
  int16_t hb = head_height / 3;
  int16_t hd = head_height / 5;

  { // Base bottom
    line_stick.p0 = (struct Point){point.x - hb, point.y};
    line_stick.p1 = (struct Point){point.x + hb, point.y};
    image_draw_line(image, &line_stick);
    struct Circle circle = {
        .color = line_stick.color,
        .d = line_stick.thickness / 2,
    };
    circle.p = line_stick.p0;
    image_draw_circle(image, &circle);
    circle.p = line_stick.p1;
    image_draw_circle(image, &circle);
  }

  { // Stick bottom
    line_stick_bottom.p0 = point;
    line_stick_bottom.p1 = (struct Point){point.x, point.y - sb};
    image_draw_line(image, &line_stick_bottom);
  }

  { // Base
    line_stick.p0 = (struct Point){point.x - hd, point.y - sb};
    line_stick.p1 = (struct Point){point.x + hd, point.y - sb};
    image_draw_line(image, &line_stick);

    struct Circle circle = {
        .color = line_stick.color,
        .d = line_stick.thickness / 2,
    };
    circle.p = line_stick.p0;
    image_draw_circle(image, &circle);
    circle.p = line_stick.p1;
    image_draw_circle(image, &circle);
  }

  { // Stick
    line_stick.p0 = point;
    line_stick.p1 = head;
    image_draw_line(image, &line_stick);
  }

  switch (style) {
  case STREET_LIGHT_CLASSIC:
    _draw_classic(image, head, line_stick, head_height);
    break;
  case STREET_LIGHT_SPHERE:
    _draw_sphere(image, head, line_stick, head_height);
    break;
  }
}

static void _draw_classic(struct Image *image, struct Point head,
                          struct Line line_stick, int16_t head_height) {
  int16_t ha = head_height / 2;
  int16_t hb = head_height / 3;
  int16_t hc = head_height - hb;
  int16_t hd = head_height / 5;

  struct Line line_head = {
      .thickness = line_stick.thickness / 8,
      .color = line_stick.color,
  };

  { // Glass
    struct Point points[] = {
        {head.x - hb, head.y},
        {head.x + hb, head.y},
        {head.x + ha, head.y - hc},
        {head.x - ha, head.y - hc},
    };
    polyfill(image, points, sizeof(points) / sizeof(struct Point), WHITE, 0xFF,
             BLACK);
  }

  { // Top
    struct Point points[] = {
        {head.x - ha, head.y - hc},
        {head.x, head.y - head_height},
        {head.x + ha, head.y - hc},
    };
    polyfill(image, points, sizeof(points) / sizeof(struct Point),
             line_head.color, 0xFF, BLACK);
  }

  { // Circles!
    struct Circle circle = {
        .color = line_stick.color,
        .d = hd / 2,
    };
    circle.p = (struct Point){head.x, head.y - head_height};
    image_draw_circle(image, &circle);
    circle.d = hd / 3;
    circle.p = (struct Point){head.x - ha, head.y - hc};
    image_draw_circle(image, &circle);
    circle.p = (struct Point){head.x + ha, head.y - hc};
    image_draw_circle(image, &circle);
  }

  { // Bottom head
    line_stick.p0 = (struct Point){head.x - hb, head.y};
    line_stick.p1 = (struct Point){head.x + hb, head.y};
    image_draw_line(image, &line_stick);
  }

  { // Top head
    line_stick.p0 = (struct Point){head.x - ha, head.y - hc};
    line_stick.p1 = (struct Point){head.x + ha, head.y - hc};
    image_draw_line(image, &line_stick);
  }

  if (head_height < 12) {
    // To small to draw details.
    return;
  }

  int i;
  for (i = 0; i <= 3; i++) {
    line_head.p0 = (struct Point){head.x - hb + hb * 2 * i / 3, head.y};
    line_head.p1 = (struct Point){head.x - ha + ha * 2 * i / 3, head.y - hc};
    image_draw_line(image, &line_head);
  }
}

static void _draw_sphere(struct Image *image, struct Point head,
                         struct Line line_stick, int16_t head_height) {
  int16_t ha = head_height / 2;
  int16_t hb = head_height / 3;
  { // Glass
    struct Circle circle = {.color = WHITE,
                            .d = ha,
                            .p = (struct Point){head.x, head.y - ha}};
    image_draw_circle(image, &circle);
  }

  { // Bottom head
    line_stick.p0 = (struct Point){head.x - hb, head.y};
    line_stick.p1 = (struct Point){head.x + hb, head.y};
    image_draw_line(image, &line_stick);
    struct Circle circle = {
        .color = line_stick.color,
        .d = line_stick.thickness / 2,
    };
    circle.p = line_stick.p0;
    image_draw_circle(image, &circle);
    circle.p = line_stick.p1;
    image_draw_circle(image, &circle);
  }
}