/*****************************************************************************
* | File      	:	EPD_12in48b.h
* | Author      :   Waveshare team
* | Function    :   Electronic paper driver
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2018-11-29
* | Info        :
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
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
#ifndef _EPD_12IN48B_H_
#define _EPD_12IN48B_H_
#include <stdint.h>

#define EPD_SCK_PIN  14
#define EPD_MOSI_PIN 13

#define EPD_M1_CS_PIN 23
#define EPD_S1_CS_PIN 22
#define EPD_M2_CS_PIN 21
#define EPD_S2_CS_PIN 19

#define EPD_M1S1_DC_PIN  25
#define EPD_M2S2_DC_PIN  27

#define EPD_M1S1_RST_PIN 33
#define EPD_M2S2_RST_PIN 26

#define EPD_M1_BUSY_PIN  35
#define EPD_S1_BUSY_PIN  32
#define EPD_M2_BUSY_PIN  18
#define EPD_S2_BUSY_PIN  34

// Display resolution
#define EPD_12in48B_MAX_WIDTH 1304
#define EPD_12in48B_MAX_HEIGHT 984

#define EPD_12in48B_M1_WIDTH 648
#define EPD_12in48B_M1_HEIGHT (EPD_12in48B_MAX_HEIGHT / 2)
#define EPD_12in48B_S1_WIDTH (EPD_12in48B_MAX_WIDTH - 648) // 656
#define EPD_12in48B_S1_HEIGHT (EPD_12in48B_MAX_HEIGHT / 2)
#define EPD_12in48B_M2_WIDTH (EPD_12in48B_MAX_WIDTH - 648) // 656
#define EPD_12in48B_M2_HEIGHT (EPD_12in48B_MAX_HEIGHT / 2)
#define EPD_12in48B_S2_WIDTH 648
#define EPD_12in48B_S2_HEIGHT (EPD_12in48B_MAX_HEIGHT / 2)

uint8_t EPD_12in48B_Init(void);
void EPD_12in48B_Clear(void);

void EPD_M1_SendCommand(uint8_t Reg);
void EPD_M1_SendData(uint8_t Data);
void EPD_S1_SendCommand(uint8_t Reg);
void EPD_S1_SendData(uint8_t Data);
void EPD_M2_SendCommand(uint8_t Reg);
void EPD_M2_SendData(uint8_t Data);
void EPD_S2_SendCommand(uint8_t Reg);
void EPD_S2_SendData(uint8_t Data);

void EPD_12in48B_cmd1S2(void);
void EPD_12in48B_cmd1M2(void);
void EPD_12in48B_cmd1M1(void);
void EPD_12in48B_cmd1S1(void);
void EPD_12in48B_data1S2(uint8_t data);
void EPD_12in48B_data1M2(uint8_t data);
void EPD_12in48B_data1M1(uint8_t data);
void EPD_12in48B_data1S1(uint8_t data);

void EPD_12in48B_cmd2S2(void);
void EPD_12in48B_cmd2M2(void);
void EPD_12in48B_cmd2M1(void);
void EPD_12in48B_cmd2S1(void);
void EPD_12in48B_data2S2(uint8_t data);
void EPD_12in48B_data2M2(uint8_t data);
void EPD_12in48B_data2M1(uint8_t data);
void EPD_12in48B_data2S1(uint8_t data);

void EPD_12in48B_TurnOnDisplay(void);
void EPD_12in48B_Sleep(void);

#endif