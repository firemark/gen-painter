#include "art/image/image_adv.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Point bezier(float t, struct Point b[4]) {
  struct Point p;
  float t_inv = 1 - t;
  float tt = t * t;
  float ttt = tt * t;
  float tt_inv = t_inv * t_inv;
  float ttt_inv = tt_inv * t_inv;
  float t_tt_inv = t * tt_inv;
  float tt_t_inv = tt * t_inv;

  p.x = b[0].x * ttt_inv + 3 * b[1].x * t_tt_inv + 3 * b[2].x * tt_t_inv +
        b[3].x * ttt;
  p.y = b[0].y * ttt_inv + 3 * b[1].y * t_tt_inv + 3 * b[2].y * tt_t_inv +
        b[3].y * ttt;

  return p;
}

struct Edge {
  int16_t y_max;
  int16_t y_min;
  float x;
  float slope;
};

static int _sort_edge(const void *aa, const void *bb) {
  const struct Edge *a = aa;
  const struct Edge *b = bb;
  if (a->x == b->x) {
    return 0;
  }
  if (a->x > b->x) {
    return 1;
  }
  return -1;
}


void polyfill(struct Image *image, struct Point *points, uint8_t size,
              enum Color color, uint8_t threshold, enum Color bg_color) {
  uint8_t i;
  struct Edge *edges = malloc(sizeof(struct Edge) * size);
  struct Edge **active_edges = malloc(sizeof(struct Edge *) * size);

  // Fill edges.
  for (i = 0; i < size; i++) {
    struct Point *a = &points[i];
    struct Point *b = &points[i < size - 1 ? i + 1 : 0];
    struct Edge *edge = &edges[i];
    int16_t y_max;
    int16_t y_min;

    if (a->y < b->y) {
      // Swap
      struct Point *c = a;
      a = b;
      b = c;
    }

    edge->x = a->x;
    edge->y_max = a->y;
    edge->y_min = b->y;

    float dx = b->x - a->x;
    float dy = b->y - a->y;
    edge->slope = dy != 0.0 ? dx / dy : NAN;
  }

  // Sort edges array.
  qsort(edges, size, sizeof(struct Edge), _sort_edge);

  //   for (i = 0; i < size; i++) {
  //     struct Edge *e = &edges[i];
  //     printf("%3d) ymax: %5d; ymin: %5d; x: %+9.2f\n", //
  //            i, e->y_max, e->y_min, e->x);
  //   }

  // Find min/max.
  int16_t y_max = 0;
  int16_t y_min = IMAGE_HEIGHT;
  for (i = 0; i < size; i++) {
    struct Edge *edge = &edges[i];
    if (edge->y_max > y_max) {
      y_max = edge->y_max;
    }
    if (edge->y_min < y_min) {
      y_min = edge->y_min;
    }
  }

  // Fill polygon.
  int16_t y;
  for (y = y_max; y > y_min; y--) {
    // Fill active edges.
    uint8_t active_edges_count = 0;
    for (i = 0; i < size; i++) {
      // TODO find optimization to not
      // finding active edges on each line.
      struct Edge *edge = &edges[i];
      if (y > edge->y_max || y <= edge->y_min || isnan(edge->slope)) {
        continue;
      }
      active_edges[active_edges_count] = edge;
      active_edges_count++;
    }

//     printf("Total active edges: %d; y: %5d\n", active_edges_count, y);
//     for (i = 0; i < active_edges_count; i++) {
//       struct Edge *e = active_edges[i];
//       printf("%3d) ymax: %5d; ymin: %5d; x: %+9.2f\n", //
//              i, e->y_max, e->y_min, e->x);
//     }
//     printf("---\n\n");

    if (active_edges_count < 2) {
      continue; // Not enough edges.
    }

    // Fill line
    int16_t xa;
    int16_t xb;
    for (i = 0; i < active_edges_count; i++) {
      struct Edge *edge = active_edges[i];
      if (i % 2 == 0) {
        xa = edge->x;
      } else {
        xb = edge->x;
        image_draw_hline(image, y, xa, xb, color, threshold, bg_color);
      }
      edge->x -= edge->slope;
    }
  }

  free(active_edges);
  free(edges);
}