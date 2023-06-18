#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#include "art.h"
#include "epd.h"
#include "image.h"

static void ports_setup(void);
static void SPI_setup(void);
static void draw_to_screen(void);
static void draw_partial_S2(struct Image *image, uint8_t x_chunk,
                            uint8_t y_chunk);

void main(void) {
  ports_setup();
  SPI_setup();

  EPD_12in48B_Init();

  for (;;) {
    EPD_12in48B_Clear();
    art_make();
    draw_to_screen();
    _delay_ms(500);
  }
}

static void ports_setup(void) {
  DDRD = (1 << P_CS_S1) | (1 << P_CS_S2) | (1 << P_CS_M1) | (1 << P_CS_M2) |
         (1 << P_DC_1) | (1 << P_DC_2) | (1 << P_RST_1) | (1 << P_RST_2);
  DDRB = (1 << 5) | (1 << 3) | (1 << 2);
  DDRC = 0;
  PORTC =
      (1 << P_BUSY_S2) | (1 << P_BUSY_M2) | (1 << P_BUSY_S1) | (1 << P_BUSY_M1);
}

static void SPI_setup(void) { SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPI2X); }

// static void draw_old(void) {
//   int x, y;
//   static uint8_t i;
//   // S2 part 648*492
//   EPD_12in48B_cmd1S2();
//   for (y = 0; y < 492; y++)
//     for (x = 0; x < 81; x++) {
//       EPD_12in48B_data1S2(++i);
//     }

//   //   EPD_12in48B_cmd2S2();
//   //   for (y = 0; y < 492; y++)
//   //     for (x = 0; x < 81; x++) {
//   //       EPD_12in48B_data2S2((x + y) % 64 > 32 ? 0xFF : ~i);
//   //     }

//   // M2 part 656*492
//   EPD_12in48B_cmd1M2();
//   for (y = 0; y < 492; y++)
//     for (x = 81; x < 163; x++) {
//       EPD_12in48B_data1M2(++i);
//     }

//   //   EPD_12in48B_cmd2M2();
//   //   for (y = 0; y < 492; y++)
//   //     for (x = 81; x < 163; x++) {
//   //       EPD_12in48B_data2M2((x + y) % 64 > 32 ? 0xFF : ~i);
//   //     }

//   // S1 part 656*492
//   EPD_12in48B_cmd1S1();
//   for (y = 492; y < 984; y++)
//     for (x = 81; x < 163; x++) {
//       EPD_12in48B_data1S1(++i);
//     }

//   //   // S1 part 656*492
//   //   EPD_12in48B_cmd2S1();
//   //   for (y = 492; y < 984; y++)
//   //     for (x = 81; x < 163; x++) {
//   //       EPD_12in48B_data2S1((x + y) % 64 > 32 ? 0xFF : ~i);
//   //     }

//   // M1 part 648*492
//   EPD_12in48B_cmd1M1();
//   for (y = 492; y < 984; y++)
//     for (x = 0; x < 81; x++) {
//       EPD_12in48B_data1M1(++i);
//     }

//   //   EPD_12in48B_cmd2M1();
//   //   for (y = 492; y < 984; y++)
//   //     for (x = 0; x < 81; x++) {
//   //       EPD_12in48B_data2M1((x + y) % 64 > 32 ? 0xFF : ~i);
//   //     }

//   EPD_12in48B_TurnOnDisplay();
// }

static void draw_to_screen(void) {
  static struct Image image;
  uint8_t x_chunk, y_chunk;

  image.x_offset = 0;
  image.y_offset = 0;

  for (y_chunk = 0; y_chunk < 4; y_chunk++) {
    for (x_chunk = 0; x_chunk < 4; x_chunk++) {
      art_draw(&image);
      draw_partial_S2(&image, x_chunk, y_chunk);
      image.x_offset += IMAGE_WIDTH;
    }
    image.y_offset += IMAGE_HEIGHT;
    image.x_offset = 0;
  }
}

static void draw_partial_S2(struct Image *image, uint8_t x_chunk,
                            uint8_t y_chunk) {
  // WIDTH = 648 (81 bytes)
  // HEIGHT = 492
  uint16_t x0 = x_chunk * IMAGE_WIDTH;
  uint16_t y0 = y_chunk * IMAGE_HEIGHT;
  uint16_t x1 = (x_chunk + 1) * IMAGE_WIDTH - 1;
  uint16_t y1 = (y_chunk + 1) * IMAGE_HEIGHT - 1;

  x1 = x1 < 648 ? x1 : (648 - 1);
  y1 = y1 < 492 ? y1 : (492 - 1);

  EPD_12in48B_partialS2(x0, y0, x1, y1);

  uint16_t w = x1 - x0;
  uint16_t h = y1 - y0;
  uint16_t x, y;

  // BLACK
  EPD_12in48B_cmd1S2();
  for (y = 0; y <= h; y++) {
    for (x = 0; x <= w; x += 8) {
      uint8_t data = 0xFF;
      for (uint8_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == BLACK) {
          data &= ~(1 << i);
        }
      }
      EPD_12in48B_data1S2(data);
    }
  }

  // RED
  EPD_12in48B_cmd2S2();
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x += 8) {
      uint8_t data = 0xFF;
      for (uint8_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == RED) {
          data &= ~(1 << i);
        }
      }
      EPD_12in48B_data2S2(data);
    }
  }

  EPD_12in48B_TurnOnDisplay();
}