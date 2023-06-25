#include <stdlib.h>

#include "DEV_Config.h"
#include "utility/EPD_12in48b.h"

extern "C" {
#include "art.h"
}

int Version = 1;

static void draw_to_screen(void);
static void draw_S1(struct Image *image);
static void draw_M1(struct Image *image);
static void draw_S2(struct Image *image);
static void draw_M2(struct Image *image);

uint32_t art_random() {
  return esp_random();
}

void setup() {
  DEV_ModuleInit();

  printf("Init \r\n");
  DEV_TestLED();
  EPD_12in48B_Init();

  printf("Clear \r\n");
  EPD_12in48B_Clear();
  printf("NewImage \r\n");

  art_init();
  art_make();
  draw_to_screen();

  EPD_12in48B_TurnOnDisplay();
  DEV_Delay_ms(10000);

  // clear for Long-term preservation
  printf("clear... \r\n");
  EPD_12in48B_Clear();
  printf("sleep... \r\n");
  EPD_12in48B_Sleep();
}

void loop() {
  //
}

static void draw_to_screen(void) {
  static struct Image image;

  image.x_offset = 0;
  image.y_offset = 0;
  art_draw(&image);
  draw_S2(&image);

  image.x_offset = EPD_12in48B_S2_WIDTH;
  art_draw(&image);
  draw_M2(&image);

  image.x_offset = 0;
  image.y_offset = EPD_12in48B_S2_HEIGHT;
  art_draw(&image);
  draw_M1(&image);

  image.x_offset = EPD_12in48B_S2_WIDTH;
  art_draw(&image);
  draw_S1(&image);
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
}
