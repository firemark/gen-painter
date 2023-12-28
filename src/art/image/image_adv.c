#include "art/image/image_adv.h"
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
  int16_t x_current;
  float slope;
};

static int _sort_edge(const void *aa, const void *bb) {
  const struct Edge *a = aa;
  const struct Edge *b = bb;
  if (a->y_max == b->y_max) {
    return 0;
  }
  if (a->y_max < b->y_max) {
    return 1;
  }
  return -1;
}

void polyfill(struct Point *points, uint8_t size, enum Color color) {
  uint8_t i;
  struct Edge *edges = malloc(sizeof(struct Edge) * size);
  struct Edge *active_edges = malloc(sizeof(struct Edge *) * size);
  memset(active_edges, 0, sizeof(struct Edge *) * size);

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
    edge->slope = (float)(a->x - b->x) / (float)(a->y - b->y);
    edge->x_current = a->x;
    edge->y_max = a->y;
    edge->y_min = b->y;
  }

  // Sort edges array.
  qsort(edges, size, sizeof(struct Edge), _sort_edge);

//   for (i = 0; i < size; i++) {
//     struct Edge *e = &edges[i];
//     printf("%3d) ymax: %5d; ymin: %5d; x: %5d; slope: %+8.2f\n", //
//            i, e->y_max, e->y_min, e->x_current, e->slope);
//   }

  free(active_edges);
  free(edges);
}