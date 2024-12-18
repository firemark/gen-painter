#include "art/image/3d.h"
#include "art/image/image_adv.h"
#include "art/object/grass.h"
#include "art/random.h"

#include <stdio.h>
#include <stdlib.h>

#include "art/_share.h"

#define TO_2D(x, y, z) to_screen_from_3d(hor, (struct Point3d){x, y, z})
#define DRAW_POLY4(a, b, c, d, threshold)                        \
  {                                                              \
    struct Point points[4] = {a, b, c, d};                       \
    polyfill(image, points, 4, BLACK, threshold, _leaves_color); \
  }

#define DRAW_POLY4V(a, b, c, d, threshold)                       \
  {                                                              \
    struct Point points[4] = {a, b, c, d};                       \
    polyfill(image, points, 4, _leaves_color, threshold, BLACK); \
  }

#define DRAW_POLY3(a, b, c, threshold)                           \
  {                                                              \
    struct Point points[3] = {a, b, c};                          \
    polyfill(image, points, 3, BLACK, threshold, _leaves_color); \
  }

static float _r() {
  return (float)random_int(16) / 256.0;
}

static void _draw_grid_roof(struct Image* image, int16_t hor, int16_t x0,
                            int16_t x1, int16_t h0, int16_t h1, int16_t z0,
                            int16_t z1) {
  const int16_t dx = x1 - x0;
  const int16_t dz = z1 - z0;
  const int16_t dh = h1 - h0;
  const double r = 0.0625 / 2;

  for (float i = 0.0; i <= 1.0 - r; i += r) {
    int16_t z = z0 + dz * i;
    int16_t h = h0 + dh * i;
    float j = -_r() / 2;

    while (j <= 1.0 - _r() / 2) {
      float rr = _r();
      int16_t x = x0 + dx * j;
      struct Line line_a = {
          .color = BLACK,
          .thickness = 2,
          .p0 = TO_2D(x, h + random_int_b(1), z),
          .p1 = TO_2D(x + dx * rr, h, z),
      };
      image_draw_line(image, &line_a);

      struct Line line = {
          .color = BLACK,
          .thickness = 1,
          .p0 = TO_2D(x, h + random_int_b(1), z),
          .p1 = TO_2D(x, h + dh * rr, z + dz * rr),
      };
      image_draw_line(image, &line);

      j += rr;
    }
  }

  struct Line line = {
      .color = BLACK,
      .thickness = 2,
      .p0 = TO_2D(x0, h1, z1),
      .p1 = TO_2D(x1, h1, z1),
  };
  image_draw_line(image, &line);
}

static void _draw_grid_h(struct Image* image, int16_t hor, int16_t x0,
                         int16_t x1, int16_t h0, int16_t h1, int16_t z0,
                         int16_t z1) {
  const int16_t dx = x1 - x0;
  const int16_t dz = z1 - z0;
  const int16_t dh = h1 - h0;
  const double r = 0.125;

  for (float i = 0.0; i <= 1.0 - r; i += r) {
    int16_t z = z0 + dz * i;
    int16_t h = h0 + dh * i;
    struct Line line = {
        .color = BLACK,
        .thickness = 2,
        .p0 = TO_2D(x0, h, z),
        .p1 = TO_2D(x1, h, z),
    };
    image_draw_line(image, &line);
  }

  struct Line line = {
      .color = BLACK,
      .thickness = 2,
      .p0 = TO_2D(x0, h1, z1),
      .p1 = TO_2D(x1, h1, z1),
  };
  image_draw_line(image, &line);
}

static void _draw_grid_v(struct Image* image, int16_t hor, int16_t x0,
                         int16_t x1, int16_t h0, int16_t h1, int16_t z0,
                         int16_t z1) {
  const int16_t dx = x1 - x0;
  const int16_t dz = z1 - z0;
  const int16_t dh = h1 - h0;
  const double r = 0.125;

  for (float i = 0.0; i <= 1.0 - r; i += r) {
    int16_t x = x0 + dx * i;
    int16_t h = h0 + dh * i;
    struct Line line = {
        .color = BLACK,
        .thickness = 2,
        .p0 = TO_2D(x, h, z0),
        .p1 = TO_2D(x, h, z1),
    };
    image_draw_line(image, &line);
  }

  struct Line line = {
      .color = BLACK,
      .thickness = 2,
      .p0 = TO_2D(x1, h1, z0),
      .p1 = TO_2D(x1, h1, z1),
  };
  image_draw_line(image, &line);
}

static void _draw_door_v(struct Image* image, int16_t hor, int16_t x, int16_t h,
                         int16_t z) {
  const int16_t h_bg_size = 80;

  z -= h_bg_size / 2;

  {  // Background
    struct Point points[4] = {
        TO_2D(x, 0, z),
        TO_2D(x, h, z),
        TO_2D(x, h, z + h_bg_size),
        TO_2D(x, 0, z + h_bg_size),
    };
    polyfill(image, points, 4, BLACK, 160, _leaves_color);
    polygon_border(image, points, 4, BLACK, 1);
  }

  for (int16_t zz = 0; zz < h_bg_size; zz += 20) {
    struct Line line = {
        .color = BLACK,
        .thickness = 1,
        .p0 = TO_2D(x, 0, z + zz),
        .p1 = TO_2D(x, h, z + zz),
    };
    image_draw_line(image, &line);
  }

  {  // Knob
    int16_t hh = h * 0.5;
    struct Point points[4] = {
        TO_2D(x, hh, z),
        TO_2D(x, h, z),
        TO_2D(x, h, z + 10),
        TO_2D(x, hh, z + 10),
    };
    polyfill(image, points, 4, BLACK, 220, _leaves_color);
  }
}

static void _draw_window_h(struct Image* image, int16_t hor, int16_t x,
                           int16_t h, int16_t z) {
  const int16_t size = 12;
  const int16_t h_count = 2 + random_int(2);
  const int16_t v_count = 1 + random_int(2);
  const int16_t border = 1;
  const int16_t out_border = 1;
  const int16_t h_window_size = size / h_count;
  const int16_t v_window_size = size / v_count;
  const int16_t h_in_bg_size = size + border * (h_count + 1);
  const int16_t v_in_bg_size = size + border * (v_count + 1);
  const int16_t h_bg_size = h_in_bg_size + out_border * 2;
  const int16_t v_bg_size = v_in_bg_size + out_border * 2;

  x -= h_bg_size / 2;
  h -= v_bg_size / 2;

  {  // Background
    struct Point points[4] = {
        TO_2D(x, h, z),
        TO_2D(x, h + v_bg_size, z),
        TO_2D(x + h_bg_size, h + v_bg_size, z),
        TO_2D(x + h_bg_size, h, z),
    };
    polyfill(image, points, 4, BLACK, 48, _leaves_color);
    polygon_border(image, points, 4, BLACK, 1);
  }

  {  // In border
    int16_t xx = x + out_border;
    int16_t hh = h + out_border;
    struct Point points[4] = {
        TO_2D(xx, hh, z),
        TO_2D(xx, hh + v_in_bg_size, z),
        TO_2D(xx + h_in_bg_size, hh + v_in_bg_size, z),
        TO_2D(xx + h_in_bg_size, hh, z),
    };
    polyfill(image, points, 4, BLACK, 72, _leaves_color);
    polygon_border(image, points, 4, BLACK, 2);
  }

  // Holes
  for (uint8_t i = 0; i < h_count; i++) {
    for (uint8_t j = 0; j < v_count; j++) {
      int16_t xx = x + (h_window_size + border) * i + border + out_border;
      int16_t hh = h + (v_window_size + border) * j + border + out_border;
      struct Point points[4] = {
          TO_2D(xx, hh, z),
          TO_2D(xx, hh + v_window_size, z),
          TO_2D(xx + h_window_size, hh + v_window_size, z),
          TO_2D(xx + h_window_size, hh, z),
      };
      polyfill(image, points, 4, _leaves_color, 8, BLACK);
    }
  }
}

static void _draw_triangle_grid(struct Image* image, int16_t hor, int16_t x,
                                int16_t h0, int16_t h1, int16_t z0, int16_t z1,
                                int16_t z2) {
  const int16_t dz0 = z1 - z0;
  const int16_t dz1 = z2 - z1;
  const int16_t dh = h1 - h0;
  const double r = 0.125;

  for (float i = 0.0; i <= 1.0 - r; i += r) {
    int16_t h = h0 + dh * i;
    int16_t z = z0 + dz0 * i;
    struct Line line = {
        .color = BLACK,
        .thickness = 2,
        .p0 = TO_2D(x, h0, z),
        .p1 = TO_2D(x, h, z),
    };
    image_draw_line(image, &line);
  }

  for (float i = 0.0; i <= 1.0 - r; i += r) {
    int16_t h = h1 - dh * i;
    int16_t z = z1 + dz1 * i;
    struct Line line = {
        .color = BLACK,
        .thickness = 2,
        .p0 = TO_2D(x, h0, z),
        .p1 = TO_2D(x, h, z),
    };
    image_draw_line(image, &line);
  }
}

void house_draw(struct Image* image, int16_t hor, struct Point3d p0,
                struct Point3d p1, int16_t h) {
  int16_t hh[] = {0, 0.8 * h, 1 * h, 3 * h, 0.6 * h};
  int16_t dxx = p1.x - p0.x;
  int16_t dzz = p1.z - p0.z;
  int16_t mid_x = (p0.x + p1.x) / 2.0;
  int16_t mid_z = (p0.z + p1.z) / 2.0;
  int16_t dx = 20;
  int16_t dz = 0.2 * dzz;

  struct Point fl0 = TO_2D(p0.x, hh[0], p0.z);
  struct Point bl0 = TO_2D(p0.x, hh[0], p1.z);
  struct Point fr0 = TO_2D(p1.x, hh[0], p0.z);
  struct Point br0 = TO_2D(p1.x, hh[0], p1.z);

  struct Point fl1 = TO_2D(p0.x, hh[2], p0.z);
  struct Point bl1 = TO_2D(p0.x, hh[2], p1.z);
  struct Point fr1 = TO_2D(p1.x, hh[2], p0.z);
  struct Point br1 = TO_2D(p1.x, hh[2], p1.z);

  struct Point fl1x = TO_2D(p0.x, hh[4], p0.z);
  struct Point fr1x = TO_2D(p1.x, hh[4], p0.z);

  struct Point l2 = TO_2D(p0.x, hh[3], mid_z);
  struct Point r2 = TO_2D(p1.x, hh[3], mid_z);

  int16_t fpz = p0.z - dz;
  int16_t bpz = p1.z + dz;
  struct Point l2r = TO_2D(p0.x - dx, hh[3], mid_z);
  struct Point r2r = TO_2D(p1.x + dx, hh[3], mid_z);
  struct Point fl1r = TO_2D(p0.x - dx, hh[1], fpz);
  struct Point fr1r = TO_2D(p1.x + dx, hh[1], fpz);
  struct Point bl1r = TO_2D(p0.x - dx, hh[1], bpz);
  struct Point br1r = TO_2D(p1.x + dx, hh[1], bpz);

  {  // Back roof
    DRAW_POLY4(bl1r, l2r, r2r, br1r, 218);
    _draw_grid_roof(image, hor,            //
                    p0.x - dx, p1.x + dx,  // x
                    hh[1], hh[3],          // h
                    bpz, mid_z);           // z
  }

  {  // Triangles
    DRAW_POLY3(bl1, l2, fl1, 196);
    DRAW_POLY3(br1, r2, fr1, 196);

    _draw_triangle_grid(image, hor,          //
                        p0.x,                // x
                        hh[2], hh[3],        // h
                        p0.z, mid_z, p1.z);  // z

    _draw_triangle_grid(image, hor,          //
                        p1.x,                // x
                        hh[2], hh[3],        // h
                        p0.z, mid_z, p1.z);  // z
  }

  {  // Sides
    DRAW_POLY4(fl0, fl1, bl1, bl0, 200);
    DRAW_POLY4(fr0, fr1, br1, br0, 200);

    _draw_grid_v(image, hor,    //
                 p0.x, p0.x,    // x
                 hh[0], hh[2],  // h
                 p0.z, p1.z);   // z

    _draw_grid_v(image, hor,    //
                 p1.x, p1.x,    // x
                 hh[0], hh[2],  // h
                 p1.z, p0.z);   // z

    _draw_door_v(image, hor, p0.x, h, mid_z);
    _draw_door_v(image, hor, p1.x, h, mid_z);
  }

  {  // Front
    DRAW_POLY4(fl0, fl1, fr1, fr0, 148);
    DRAW_POLY4(fl1, fl1x, fr1x, fr1, 200);  // Shadow

    _draw_grid_h(image, hor,    //
                 p0.x, p1.x,    // x
                 hh[0], hh[2],  // h
                 p0.z, p0.z);   // z

    int16_t xx0 = p0.x + 0.25 * dxx;
    int16_t xx1 = p0.x + 0.75 * dxx;
    _draw_window_h(image, hor, xx0, 0.5 * h, p0.z);
    _draw_window_h(image, hor, xx1, 0.5 * h, p0.z);
  }

  {  // Front roof
    DRAW_POLY4(fl1r, l2r, r2r, fr1r, 100);
    _draw_grid_roof(image, hor,            //
                    p0.x - dx, p1.x + dx,  // x
                    hh[1], hh[3],          // h
                    fpz, mid_z);           // z
  }
}