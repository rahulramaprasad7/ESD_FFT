/*
 * lcd_driver.h
 *
 *  Created on: 03-Dec-2019
 *      Author: prayag
 */

#ifndef LCD_DRIVER_H_
#define LCD_DRIVER_H_

#include "msp.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "HAL_MSP_EXP432P401R_KITRONIX320X240_SSD2119_SPI.h"
#include "kitronix320x240x16_ssd2119_spi.h"
#include "Lucida_Console_8pts.h"


#define CLK BIT5
#define MOSI BIT6
#define LCD_CS BIT0
#define DC BIT6
#define RESET BIT5
#define PWM BIT7

#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define BLACK 0x0000
#define WHITE (RED | GREEN | BLUE)



void lcdInit(void);
void lcdSendCommand(uint8_t command);
void spi16bytes(uint16_t in);
void spi8bytes(uint8_t in);
void selectCS(int state);
void selectDC(int state);
void delay(uint16_t msec);
void setCursorMode(uint8_t id1, uint8_t id0, uint8_t am);
void drawLineX(uint16_t startPosX, uint16_t length, uint16_t startPosY, uint16_t color);
void drawLineY(uint16_t startPosY, uint16_t length, uint16_t startPosX, uint16_t color);
void writeLetter(uint16_t posX, uint16_t posY, uint16_t color, char inChar);
void setCursorXY(uint16_t posX, uint16_t posY);
void writeString(uint16_t posX, uint16_t posY, uint16_t color, char* inString);



#endif /* LCD_DRIVER_H_ */
