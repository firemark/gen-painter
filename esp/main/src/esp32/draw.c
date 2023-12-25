#include "esp32/draw.h"

#include <stdio.h>

#include "art/image/image.h"
#include "art/art.h"
#include "esp32/epd.h"

#include "esp_heap_caps.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void draw_S1(struct Image *image);
static void draw_M1(struct Image *image);
static void draw_S2(struct Imj samage *image);
static void draw_M2(struct Image *image);

static void _yield(void) {
  vTaskDelay(1);
}

static void _art_draw(struct Image *image) {
  art_draw(image);
  _yield();
}

void draw_to_screen(void) {
  _yield();
  struct Image *image = image_create();
  if (!image) {
    printf("Not enough memory to draw! available memory: %u; required: %u\n",
           heap_caps_get_free_size(MALLOC_CAP_DEFAULT), sizeof(struct Image));
    return;
  }

  image->offset.x = 0;
  image->offset.y = 0;
  _art_draw(image);
  draw_S2(image);

  image->offset.x = EPD_12in48B_S2_WIDTH;
  _art_draw(image);
  draw_M2(image);

  image->offset.x = 0;
  image->offset.y = EPD_12in48B_S2_HEIGHT;
  _art_draw(image);
  draw_M1(image);

  image->offset.x = EPD_12in48B_S2_WIDTH;
  _art_draw(image);
  draw_S1(image);

  image_destroy(image);
}

static void draw_S1(struct Image *image) {
  uint16_t x, y;

  // BLACK
  EPD_S1_SendCommand(0x10);
  for (y = 0; y < EPD_12in48B_S1_HEIGHT; y++) {
    for (x = 0; x < EPD_12in48B_S1_WIDTH; x += 8) {
      uint8_t data = 0xFF;
      for (uint16_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == BLACK) {
          data &= ~(1 << (7 - i));
        }
      }
      EPD_S1_SendData(data);
    }
  }
  _yield();

  // RED
  EPD_S1_SendCommand(0x13);
  for (y = 0; y < EPD_12in48B_S1_HEIGHT; y++) {
    for (x = 0; x < EPD_12in48B_S1_WIDTH; x += 8) {
      uint8_t data = 0x00;
      for (uint16_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == RED) {
          data |= 1 << (7 - i);
        }
      }
      EPD_S1_SendData(data);
    }
  }
  _yield();
}

static void draw_M1(struct Image *image) {
  uint16_t x, y;

  // BLACK
  EPD_M1_SendCommand(0x10);
  for (y = 0; y < EPD_12in48B_M1_HEIGHT; y++) {
    for (x = 0; x < EPD_12in48B_M1_WIDTH; x += 8) {
      uint8_t data = 0xFF;
      for (uint16_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == BLACK) {
          data &= ~(1 << (7 - i));
        }
      }
      EPD_M1_SendData(data);
    }
  }
  _yield();

  // RED
  EPD_M1_SendCommand(0x13);
  for (y = 0; y < EPD_12in48B_M1_HEIGHT; y++) {
    for (x = 0; x < EPD_12in48B_M1_WIDTH; x += 8) {
      uint8_t data = 0x00;
      for (uint16_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == RED) {
          data |= 1 << (7 - i);
        }
      }
      EPD_M1_SendData(data);
    }
  }
  _yield();
}

static void draw_S2(struct Image *image) {
  uint16_t x, y;

  // BLACK
  EPD_S2_SendCommand(0x10);
  for (y = 0; y < EPD_12in48B_S2_HEIGHT; y++) {
    for (x = 0; x < EPD_12in48B_S2_WIDTH; x += 8) {
      uint8_t data = 0xFF;
      for (uint16_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == BLACK) {
          data &= ~(1 << (7 - i));
        }
      }
      EPD_S2_SendData(data);
    }
  }
  _yield();

  // RED
  EPD_S2_SendCommand(0x13);
  for (y = 0; y < EPD_12in48B_S2_HEIGHT; y++) {
    for (x = 0; x < EPD_12in48B_S2_WIDTH; x += 8) {
      uint8_t data = 0x00;
      for (uint16_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == RED) {
          data |= 1 << (7 - i);
        }
      }
      EPD_S2_SendData(data);
    }
  }
  _yield();
}

static void draw_M2(struct Image *image) {
  uint16_t x, y;

  // BLACK
  EPD_M2_SendCommand(0x10);
  for (y = 0; y < EPD_12in48B_M2_HEIGHT; y++) {
    for (x = 0; x < EPD_12in48B_M2_WIDTH; x += 8) {
      uint8_t data = 0xFF;
      for (uint16_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == BLACK) {
          data &= ~(1 << (7 - i));
        }
      }
      EPD_M2_SendData(data);
    }
  }
  _yield();

  // RED
  EPD_M2_SendCommand(0x13);
  for (y = 0; y < EPD_12in48B_M2_HEIGHT; y++) {
    for (x = 0; x < EPD_12in48B_M2_WIDTH; x += 8) {
      uint8_t data = 0x00;
      for (uint16_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == RED) {
          data |= 1 << (7 - i);
        }
      }
      EPD_M2_SendData(data);
    }
  }
  _yield();
}