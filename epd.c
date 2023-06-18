/*****************************************************************************
* | File      	:	EPD_12in48b.c
* | Author      :   Waveshare team
* | Function    :   Electronic paper driver
* | Info	 :
*----------------
* |	This version:   V1.1
* | Date	 :   2022-09-13
* | Info	 :
*----------------
* Version	: V1.1
* Date		: 2022-09-13
* Log		: Added support for V2
*----------------
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "epd.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

static void EPD_Reset(void);
static void EPD_M1_SendCommand(uint8_t Reg);
static void EPD_M1_SendData(uint8_t Data);
static void EPD_S1_SendCommand(uint8_t Reg);
static void EPD_S1_SendData(uint8_t Data);
static void EPD_M2_SendCommand(uint8_t Reg);
static void EPD_M2_SendData(uint8_t Data);
static void EPD_S2_SendCommand(uint8_t Reg);
static void EPD_S2_SendData(uint8_t Data);
static void EPD_M1M2_SendCommand(uint8_t Reg);
static void EPD_M1M2_SendData(uint8_t Data);
static void EPD_M1S1M2S2_SendCommand(uint8_t Reg);
static void EPD_M1S1M2S2_SendData(uint8_t Data);
static void EPD_M1_ReadBusy(void);
static void EPD_M2_ReadBusy(void);
static void EPD_S1_ReadBusy(void);
static void EPD_S2_ReadBusy(void);
// static void EPD_M1_ReadTemperature(void);
static void EPD_SetLut(void);

#define EPD_VERSION 1

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
uint8_t EPD_12in48B_Init(void) {
  PORTD |= (1 << P_CS_S1) | (1 << P_CS_S2) | (1 << P_CS_M1) | (1 << P_CS_M2) |
           (1 << P_DC_1) | (1 << P_DC_2);
  EPD_Reset();

#if EPD_VERSION == 1
  // panel setting
  EPD_M1_SendCommand(0x00);
  EPD_M1_SendData(0x2f); // KW-3f   KWR-2F	BWROTP 0f	BWOTP 1f
  EPD_S1_SendCommand(0x00);
  EPD_S1_SendData(0x2f);
  EPD_M2_SendCommand(0x00);
  EPD_M2_SendData(0x23);
  EPD_S2_SendCommand(0x00);
  EPD_S2_SendData(0x23);

  // POWER SETTING
  EPD_M1_SendCommand(0x01);
  EPD_M1_SendData(0x07);
  EPD_M1_SendData(0x17); // VGH=20V,VGL=-20V
  EPD_M1_SendData(0x3F); // VDH=15V
  EPD_M1_SendData(0x3F); // VDL=-15V
  EPD_M1_SendData(0x0d);
  EPD_M2_SendCommand(0x01);
  EPD_M2_SendData(0x07);
  EPD_M2_SendData(0x17); // VGH=20V,VGL=-20V
  EPD_M2_SendData(0x3F); // VDH=15V
  EPD_M2_SendData(0x3F); // VDL=-15V
  EPD_M2_SendData(0x0d);

  // booster soft start
  EPD_M1_SendCommand(0x06);
  EPD_M1_SendData(0x17); // A
  EPD_M1_SendData(0x17); // B
  EPD_M1_SendData(0x39); // C
  EPD_M1_SendData(0x17);
  EPD_M2_SendCommand(0x06);
  EPD_M2_SendData(0x17);
  EPD_M2_SendData(0x17);
  EPD_M2_SendData(0x39);
  EPD_M2_SendData(0x17);

  // resolution setting
  EPD_M1_SendCommand(0x61);
  EPD_M1_SendData(0x02);
  EPD_M1_SendData(0x88); // source 648
  EPD_M1_SendData(0x01); // gate 492
  EPD_M1_SendData(0xEC);
  EPD_S1_SendCommand(0x61);
  EPD_S1_SendData(0x02);
  EPD_S1_SendData(0x90); // source 656
  EPD_S1_SendData(0x01); // gate 492
  EPD_S1_SendData(0xEC);
  EPD_M2_SendCommand(0x61);
  EPD_M2_SendData(0x02);
  EPD_M2_SendData(0x90); // source 656
  EPD_M2_SendData(0x01); // gate 492
  EPD_M2_SendData(0xEC);
  EPD_S2_SendCommand(0x61);
  EPD_S2_SendData(0x02);
  EPD_S2_SendData(0x88); // source 648
  EPD_S2_SendData(0x01); // gate 492
  EPD_S2_SendData(0xEC);

  EPD_M1S1M2S2_SendCommand(0x15); // DUSPI
  EPD_M1S1M2S2_SendData(0x20);

  EPD_M1S1M2S2_SendCommand(0x30); // PLL
  EPD_M1S1M2S2_SendData(0x08);

  EPD_M1S1M2S2_SendCommand(0x50); // Vcom and data interval setting
  EPD_M1S1M2S2_SendData(0x31);
  EPD_M1S1M2S2_SendData(0x07);

  EPD_M1S1M2S2_SendCommand(0x60); // TCON
  EPD_M1S1M2S2_SendData(0x22);

  EPD_M1_SendCommand(0xE0); // POWER SETTING
  EPD_M1_SendData(0x01);
  EPD_M2_SendCommand(0xE0); // POWER SETTING
  EPD_M2_SendData(0x01);

  EPD_M1S1M2S2_SendCommand(0xE3);
  EPD_M1S1M2S2_SendData(0x00);

  EPD_M1_SendCommand(0x82);
  EPD_M1_SendData(0x1c);
  EPD_M2_SendCommand(0x82);
  EPD_M2_SendData(0x1c);

  EPD_SetLut();
  // EPD_M1_ReadTemperature();
#elif EPD_VERSION == 2
  // panel setting for Clear
  // EPD_M1_SendCommand(0x00);
  // EPD_M1_SendData(0x07);	//KW-3f   KWR-2F	BWROTP 0f	BWOTP 1f
  // EPD_S1_SendCommand(0x00);
  // EPD_S1_SendData(0x07);
  // EPD_M2_SendCommand(0x00);
  // EPD_M2_SendData(0x07);
  // EPD_S2_SendCommand(0x00);
  // EPD_S2_SendData(0x07);

  // panel setting for Display
  EPD_M1_SendCommand(0x00);
  EPD_M1_SendData(0x0f); // KW-3f   KWR-2F	BWROTP 0f	BWOTP 1f
  EPD_S1_SendCommand(0x00);
  EPD_S1_SendData(0x0f);
  EPD_M2_SendCommand(0x00);
  EPD_M2_SendData(0x03);
  EPD_S2_SendCommand(0x00);
  EPD_S2_SendData(0x03);

  // booster soft start
  EPD_M1_SendCommand(0x06);
  EPD_M1_SendData(0x17); // A
  EPD_M1_SendData(0x17); // B
  EPD_M1_SendData(0x39); // C
  EPD_M1_SendData(0x17);
  EPD_M2_SendCommand(0x06);
  EPD_M2_SendData(0x17);
  EPD_M2_SendData(0x17);
  EPD_M2_SendData(0x39);
  EPD_M2_SendData(0x17);

  // resolution setting
  EPD_M1_SendCommand(0x61);
  EPD_M1_SendData(0x02);
  EPD_M1_SendData(0x88); // source 648
  EPD_M1_SendData(0x01); // gate 492
  EPD_M1_SendData(0xEC);
  EPD_S1_SendCommand(0x61);
  EPD_S1_SendData(0x02);
  EPD_S1_SendData(0x90); // source 656
  EPD_S1_SendData(0x01); // gate 492
  EPD_S1_SendData(0xEC);
  EPD_M2_SendCommand(0x61);
  EPD_M2_SendData(0x02);
  EPD_M2_SendData(0x90); // source 656
  EPD_M2_SendData(0x01); // gate 492
  EPD_M2_SendData(0xEC);
  EPD_S2_SendCommand(0x61);
  EPD_S2_SendData(0x02);
  EPD_S2_SendData(0x88); // source 648
  EPD_S2_SendData(0x01); // gate 492
  EPD_S2_SendData(0xEC);

  EPD_M1S1M2S2_SendCommand(0x15); // DUSPI
  EPD_M1S1M2S2_SendData(0x20);

  EPD_M1S1M2S2_SendCommand(0x50); // Vcom and data interval setting
  EPD_M1S1M2S2_SendData(0x11);
  EPD_M1S1M2S2_SendData(0x07);

  EPD_M1S1M2S2_SendCommand(0x60); // TCON
  EPD_M1S1M2S2_SendData(0x22);

  EPD_M1S1M2S2_SendCommand(0xE3);
  EPD_M1S1M2S2_SendData(0x00);

//   EPD_M1_ReadTemperature();
#else
#error Wrong version choice.
#endif
  return 0;
}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void EPD_12in48B_Clear(void) {
  uint16_t y, x;

  // Partial Out command
  EPD_M1_SendCommand(0x92);
  EPD_S1_SendCommand(0x92);
  EPD_M2_SendCommand(0x92);
  EPD_S2_SendCommand(0x92);

  // M1 part 648*492
  EPD_M1_SendCommand(0x10);
  for (y = 492; y < 984; y++) {
    for (x = 0; x < 81; x++) {
      EPD_M1_SendData(0xff);
    }
  }
  EPD_M1_SendCommand(0x13);
  for (y = 492; y < 984; y++) {
    for (x = 0; x < 81; x++) {
      EPD_M1_SendData(0x00);
    }
  }

  // S1 part 656*492
  EPD_S1_SendCommand(0x10);
  for (y = 492; y < 984; y++) {
    for (x = 81; x < 163; x++) {
      EPD_S1_SendData(0xff);
    }
  }
  EPD_S1_SendCommand(0x13);
  for (y = 492; y < 984; y++) {
    for (x = 81; x < 163; x++) {
      EPD_S1_SendData(0x00);
    }
  }

  // M2 part 656*492
  EPD_M2_SendCommand(0x10);
  for (y = 0; y < 492; y++) {
    for (x = 81; x < 163; x++) {
      EPD_M2_SendData(0xff);
    }
  }
  EPD_M2_SendCommand(0x13);
  for (y = 0; y < 492; y++) {
    for (x = 81; x < 163; x++) {
      EPD_M2_SendData(0x00);
    }
  }

  // S2 part 648*492
  EPD_S2_SendCommand(0x10);
  for (y = 0; y < 492; y++) {
    for (x = 0; x < 81; x++) {
      EPD_S2_SendData(0xff);
    }
  }
  EPD_S2_SendCommand(0x13);
  for (y = 0; y < 492; y++) {
    for (x = 0; x < 81; x++) {
      EPD_S2_SendData(0x00);
    }
  }

  // // Turn On Display
  EPD_12in48B_TurnOnDisplay();
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD_12in48B_cmd1S2(void) { EPD_S2_SendCommand(0x10); }
void EPD_12in48B_cmd1M2(void) { EPD_M2_SendCommand(0x10); }
void EPD_12in48B_cmd1M1(void) { EPD_M1_SendCommand(0x10); }
void EPD_12in48B_cmd1S1(void) { EPD_S1_SendCommand(0x10); }
void EPD_12in48B_data1S2(uint8_t data) { EPD_S2_SendData(data); }
void EPD_12in48B_data1M2(uint8_t data) { EPD_M2_SendData(data); }
void EPD_12in48B_data1M1(uint8_t data) { EPD_M1_SendData(data); }
void EPD_12in48B_data1S1(uint8_t data) { EPD_S1_SendData(data); }

void EPD_12in48B_cmd2S2(void) { EPD_S2_SendCommand(0x13); }
void EPD_12in48B_cmd2M2(void) { EPD_M2_SendCommand(0x13); }
void EPD_12in48B_cmd2M1(void) { EPD_M1_SendCommand(0x13); }
void EPD_12in48B_cmd2S1(void) { EPD_S1_SendCommand(0x13); }
void EPD_12in48B_data2S2(uint8_t data) { EPD_S2_SendData(~data); }
void EPD_12in48B_data2M2(uint8_t data) { EPD_M2_SendData(~data); }
void EPD_12in48B_data2M1(uint8_t data) { EPD_M1_SendData(~data); }
void EPD_12in48B_data2S1(uint8_t data) { EPD_S1_SendData(~data); }

void EPD_12in48B_partialS2(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) { 
  // Partial window command
  EPD_S2_SendCommand(0x90); 

  // HRST
  EPD_S2_SendData(x0 >> 8);
  EPD_S2_SendData(x0 & ~0b111);

  // HRED
  EPD_S2_SendData(x1 >> 8);
  EPD_S2_SendData(x1 | 0b111);

  // VRST
  EPD_S2_SendData(y0 >> 8);
  EPD_S2_SendData(y0);

  // VRED
  EPD_S2_SendData(y1 >> 8);
  EPD_S2_SendData(y1);

  // PT_SCAN
  EPD_S2_SendData(0b1);

  // Partial In command
  EPD_S2_SendCommand(0x91); 
}

/******************************************************************************
function :	Turn On Display
parameter:
******************************************************************************/
void EPD_12in48B_TurnOnDisplay(void) {
  EPD_M1M2_SendCommand(0x04); // power on
  _delay_ms(300);
  EPD_M1S1M2S2_SendCommand(0x12); // Display Refresh

  EPD_M1_ReadBusy();
  EPD_S1_ReadBusy();
  EPD_M2_ReadBusy();
  EPD_S2_ReadBusy();
}

void EPD_12in48B_refreshS2(void) {
  EPD_M1M2_SendCommand(0x04); // power on
  _delay_ms(300);
  EPD_S2_SendCommand(0x12); // Display Refresh
  EPD_S2_ReadBusy();
}

/******************************************************************************
function :	Enter sleep mode
parameter:
******************************************************************************/
void EPD_12in48B_Sleep(void) {
  EPD_M1S1M2S2_SendCommand(0X02); // power off
  _delay_ms(300);

  EPD_M1S1M2S2_SendCommand(0X07); // deep sleep
  EPD_M1S1M2S2_SendData(0xA5);
  _delay_ms(300);
}

/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
static void EPD_Reset(void) {
  PORTD |= (1 << P_RST_1) | (1 << P_RST_2);
  _delay_ms(200);
  PORTD &= ~((1 << P_RST_1) | (1 << P_RST_2));
  _delay_ms(10);
  PORTD |= (1 << P_RST_1) | (1 << P_RST_2);
  _delay_ms(200);
}

/******************************************************************************
function :	send command and data(M1\M2\S1\S2\M1S1\M1S1M2S2)
parameter:
    Reg : Command register
or:
    Data : Write data
******************************************************************************/

#define SPI_WRITE(data)                                                        \
  {                                                                            \
    SPDR = data;                                                               \
    while (!(SPSR & (1 << SPIF)))                                              \
      ;                                                                        \
  }

#define SEND_CMD(Reg, CS, DC)                                                  \
  {                                                                            \
    PORTD &= ~((1 << CS) | (1 << DC));                                         \
    SPI_WRITE(Reg);                                                            \
    PORTD |= (1 << CS) | (1 << DC);                                            \
  }

#define SEND_DATA(Data, CS, DC)                                                \
  {                                                                            \
    PORTD = (PORTD & ~(1 << CS)) | (1 << DC);                                  \
    SPI_WRITE(Data);                                                           \
    PORTD |= 1 << CS;                                                          \
  }

static void EPD_M1_SendCommand(uint8_t Reg) { SEND_CMD(Reg, P_CS_M1, P_DC_1); }
static void EPD_M1_SendData(uint8_t Data) { SEND_DATA(Data, P_CS_M1, P_DC_1); }
static void EPD_S1_SendCommand(uint8_t Reg) { SEND_CMD(Reg, P_CS_S1, P_DC_1); }
static void EPD_S1_SendData(uint8_t Data) { SEND_DATA(Data, P_CS_S1, P_DC_1); }
static void EPD_M2_SendCommand(uint8_t Reg) { SEND_CMD(Reg, P_CS_M2, P_DC_2); }
static void EPD_M2_SendData(uint8_t Data) { SEND_DATA(Data, P_CS_M2, P_DC_2); }
static void EPD_S2_SendCommand(uint8_t Reg) { SEND_CMD(Reg, P_CS_S2, P_DC_2); }
static void EPD_S2_SendData(uint8_t Data) { SEND_DATA(Data, P_CS_S2, P_DC_2); }

static void EPD_M1M2_SendCommand(uint8_t Reg) {
  PORTD &= ~((1 << P_CS_M1) | (1 << P_CS_M2) | (1 << P_DC_1) | (1 << P_DC_2));
  SPI_WRITE(Reg);
  PORTD |= (1 << P_CS_M1) | (1 << P_CS_M2) | (1 << P_DC_1) | (1 << P_DC_2);
}
static void EPD_M1M2_SendData(uint8_t Data) {
  PORTD &= ~((1 << P_CS_M1) | (1 << P_CS_M2));
  PORTD |= (1 << P_DC_1) | (1 << P_DC_2);
  SPI_WRITE(Data);
  PORTD |= (1 << P_CS_M1) | (1 << P_CS_M2);
}

static void EPD_M1S1M2S2_SendCommand(uint8_t Reg) {
  PORTD &= ~((1 << P_CS_S1) | (1 << P_CS_S2) | (1 << P_CS_M1) | (1 << P_CS_M2) |
             (1 << P_DC_1) | (1 << P_DC_2));
  SPI_WRITE(Reg);
  PORTD |= (1 << P_CS_S1) | (1 << P_CS_S2) | (1 << P_CS_M1) | (1 << P_CS_M2) |
           (1 << P_DC_1) | (1 << P_DC_2);
}
static void EPD_M1S1M2S2_SendData(uint8_t Data) {
  PORTD &= ~((1 << P_CS_S1) | (1 << P_CS_S2) | (1 << P_CS_M1) | (1 << P_CS_M2));
  PORTD |= (1 << P_DC_1) | (1 << P_DC_2);
  SPI_WRITE(Data);
  PORTD |= (1 << P_CS_S1) | (1 << P_CS_S2) | (1 << P_CS_M1) | (1 << P_CS_M2);
}

/******************************************************************************
function :	Wait until the busy_pin goes LOW(M1\M2\S1\S2)
parameter:
******************************************************************************/
static void EPD_M1_ReadBusy(void) {
  uint8_t busy;
  do {
    EPD_M1_SendCommand(0x71);
    busy = !(PINC & (1 << P_BUSY_M1));
  } while (busy);
  _delay_ms(200);
}
static void EPD_M2_ReadBusy(void) {
  uint8_t busy;
  do {
    EPD_M2_SendCommand(0x71);
    busy = !(PINC & (1 << P_BUSY_M2));
  } while (busy);
  _delay_ms(200);
}
static void EPD_S1_ReadBusy(void) {
  uint8_t busy;
  do {
    EPD_S1_SendCommand(0x71);
    busy = !(PINC & (1 << P_BUSY_S1));
  } while (busy);
  _delay_ms(200);
}
static void EPD_S2_ReadBusy(void) {
  uint8_t busy;
  do {
    EPD_S2_SendCommand(0x71);
    busy = !(PINC & (1 << P_BUSY_S2));
  } while (busy);
  _delay_ms(200);
}

/******************************************************************************
function :	ReadTemperature
parameter:
******************************************************************************/
// static void EPD_M1_ReadTemperature(void) {
//   EPD_M1_SendCommand(0x40);
//   EPD_M1_ReadBusy();
//   _delay_ms(300);

//   DEV_Digital_Write(EPD_M1_CS_PIN, 0);
//   DEV_Digital_Write(EPD_S1_CS_PIN, 1);
//   DEV_Digital_Write(EPD_M2_CS_PIN, 1);
//   DEV_Digital_Write(EPD_S2_CS_PIN, 1);

//   DEV_Digital_Write(EPD_M1S1_DC_PIN, 1);
//   DEV_Delay_us(5);

//   uint8_t temp;
//   temp = DEV_SPI_ReadByte(0x00);
//   DEV_Digital_Write(EPD_M1_CS_PIN, 1);
//   // printf("Read Temperature Reg:%d\r\n", temp);

//   EPD_M1S1M2S2_SendCommand(0xe0); // Cascade setting
//   EPD_M1S1M2S2_SendData(0x03);
//   EPD_M1S1M2S2_SendCommand(0xe5); // Force temperature
//   EPD_M1S1M2S2_SendData(temp);
// }

const unsigned char lut_vcom1[] PROGMEM = {
    0x00, 0x10, 0x10, 0x01, 0x08, 0x01, 0x00, 0x06, 0x01, 0x06, 0x01, 0x05,
    0x00, 0x08, 0x01, 0x08, 0x01, 0x06, 0x00, 0x06, 0x01, 0x06, 0x01, 0x05,
    0x00, 0x05, 0x01, 0x1E, 0x0F, 0x06, 0x00, 0x05, 0x01, 0x1E, 0x0F, 0x01,
    0x00, 0x04, 0x05, 0x08, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char lut_ww1[] PROGMEM = {
    0x91, 0x10, 0x10, 0x01, 0x08, 0x01, 0x04, 0x06, 0x01, 0x06, 0x01, 0x05,
    0x84, 0x08, 0x01, 0x08, 0x01, 0x06, 0x80, 0x06, 0x01, 0x06, 0x01, 0x05,
    0x00, 0x05, 0x01, 0x1E, 0x0F, 0x06, 0x00, 0x05, 0x01, 0x1E, 0x0F, 0x01,
    0x08, 0x04, 0x05, 0x08, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char lut_bw1[] PROGMEM = {
    0xA8, 0x10, 0x10, 0x01, 0x08, 0x01, 0x84, 0x06, 0x01, 0x06, 0x01, 0x05,
    0x84, 0x08, 0x01, 0x08, 0x01, 0x06, 0x86, 0x06, 0x01, 0x06, 0x01, 0x05,
    0x8C, 0x05, 0x01, 0x1E, 0x0F, 0x06, 0x8C, 0x05, 0x01, 0x1E, 0x0F, 0x01,
    0xF0, 0x04, 0x05, 0x08, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char lut_wb1[] PROGMEM = {
    0x91, 0x10, 0x10, 0x01, 0x08, 0x01, 0x04, 0x06, 0x01, 0x06, 0x01, 0x05,
    0x84, 0x08, 0x01, 0x08, 0x01, 0x06, 0x80, 0x06, 0x01, 0x06, 0x01, 0x05,
    0x00, 0x05, 0x01, 0x1E, 0x0F, 0x06, 0x00, 0x05, 0x01, 0x1E, 0x0F, 0x01,
    0x08, 0x04, 0x05, 0x08, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char lut_bb1[] PROGMEM = {
    0x92, 0x10, 0x10, 0x01, 0x08, 0x01, 0x80, 0x06, 0x01, 0x06, 0x01, 0x05,
    0x84, 0x08, 0x01, 0x08, 0x01, 0x06, 0x04, 0x06, 0x01, 0x06, 0x01, 0x05,
    0x00, 0x05, 0x01, 0x1E, 0x0F, 0x06, 0x00, 0x05, 0x01, 0x1E, 0x0F, 0x01,
    0x01, 0x04, 0x05, 0x08, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/******************************************************************************
function :	ReadTemperature
parameter:
******************************************************************************/
static void EPD_SetLut(void) {
  uint16_t count;

  EPD_M1S1M2S2_SendCommand(0x20); // vcom
  for (count = 0; count < 60; count++) {
    EPD_M1S1M2S2_SendData(pgm_read_byte(&lut_vcom1[count]));
  }

  EPD_M1S1M2S2_SendCommand(0x21); // red not use
  for (count = 0; count < 60; count++) {
    EPD_M1S1M2S2_SendData(pgm_read_byte(&lut_ww1[count]));
  }

  EPD_M1S1M2S2_SendCommand(0x22); // bw r
  for (count = 0; count < 60; count++) {
    EPD_M1S1M2S2_SendData(pgm_read_byte(&lut_bw1[count])); // bw=r
  }

  EPD_M1S1M2S2_SendCommand(0x23); // wb w
  for (count = 0; count < 60; count++) {
    EPD_M1S1M2S2_SendData(pgm_read_byte(&lut_wb1[count])); // wb=w
  }

  EPD_M1S1M2S2_SendCommand(0x24); // bb b
  for (count = 0; count < 60; count++) {
    EPD_M1S1M2S2_SendData(pgm_read_byte(&lut_bb1[count])); // bb=b
  }

  EPD_M1S1M2S2_SendCommand(0x25); // bb b
  for (count = 0; count < 60; count++) {
    EPD_M1S1M2S2_SendData(pgm_read_byte(&lut_ww1[count])); // bb=b
  }
}