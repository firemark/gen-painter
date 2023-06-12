#include <avr/io.h>
#include <util/delay.h>

#include "epd.h"

static void ports_setup(void);
static void SPI_setup(void);
static void draw(void);

void main(void) {
  ports_setup();
  SPI_setup();

  EPD_12in48B_Init();
  EPD_12in48B_Clear();

  for (;;) {
    draw();
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

static void SPI_setup(void) {
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPI2X);
}

static void draw(void) {
  int x, y;
  static uint8_t i;
  // S2 part 648*492
  EPD_12in48B_cmd1S2();
  for (y = 0; y < 492; y++)
    for (x = 0; x < 81; x++) {
      EPD_12in48B_data1S2(++i);
    }

//   EPD_12in48B_cmd2S2();
//   for (y = 0; y < 492; y++)
//     for (x = 0; x < 81; x++) {
//       EPD_12in48B_data2S2((x + y) % 64 > 32 ? 0xFF : ~i);
//     }

  // M2 part 656*492
  EPD_12in48B_cmd1M2();
  for (y = 0; y < 492; y++)
    for (x = 81; x < 163; x++) {
      EPD_12in48B_data1M2(++i);
    }

//   EPD_12in48B_cmd2M2();
//   for (y = 0; y < 492; y++)
//     for (x = 81; x < 163; x++) {
//       EPD_12in48B_data2M2((x + y) % 64 > 32 ? 0xFF : ~i);
//     }

  // S1 part 656*492
  EPD_12in48B_cmd1S1();
  for (y = 492; y < 984; y++)
    for (x = 81; x < 163; x++) {
      EPD_12in48B_data1S1(++i);
    }

//   // S1 part 656*492
//   EPD_12in48B_cmd2S1();
//   for (y = 492; y < 984; y++)
//     for (x = 81; x < 163; x++) {
//       EPD_12in48B_data2S1((x + y) % 64 > 32 ? 0xFF : ~i);
//     }

  // M1 part 648*492
  EPD_12in48B_cmd1M1();
  for (y = 492; y < 984; y++)
    for (x = 0; x < 81; x++) {
      EPD_12in48B_data1M1(++i);
    }

//   EPD_12in48B_cmd2M1();
//   for (y = 492; y < 984; y++)
//     for (x = 0; x < 81; x++) {
//       EPD_12in48B_data2M1((x + y) % 64 > 32 ? 0xFF : ~i);
//     }

  EPD_12in48B_TurnOnDisplay();
}