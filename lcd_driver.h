/*
 * @file lcd_driver.h
 * @brief The LCD driver header for the BOOSTXL-K350QVG boosterpack
 *
 * This header file contains the LCD driver function
 * prototypes and the definitions required for the
 * BOOSTXL-K350QVG boosterpack. It uses 4-wire SPI to
 * communicate with the LCD, and is capable of drawing
 * pixels, lines and text on the display
 *
 * @date 03-Dec-2019
 * @author Prayag Milan Desai, Rahul Ramaprasad
 */

#ifndef LCD_DRIVER_H_
#define LCD_DRIVER_H_

//The MSP432 header
#include "msp.h"
//Standard header files
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
//The header file containing LCD specific macros
#include "HAL_MSP_EXP432P401R_KITRONIX320X240_SSD2119_SPI.h"
//The header file containing the lcd driver specific macros
#include "kitronix320x240x16_ssd2119_spi.h"
//the font
#include "Lucida_Console_8pts.h"

/* The pin numbers for the GPIOs USED TO COMMNUICATE WITH THE LCD */
//SPI - SCLK
#define CLK BIT5
//SPI - SDA
#define MOSI BIT6
//SPI - SCS
#define LCD_CS BIT0
//LCD - SDC
#define DC BIT6
//LCD - RESET
#define RESET BIT5
//LCD - BACKLIGHT
#define PWM BIT7

/* The macros defining common 16-bit colours for the LCD */
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define BLACK 0x0000
#define WHITE (RED | GREEN | BLUE)


/*
 * @brief Initialize the LCD
 *
 * This function intitializes the LCD using the
 * routine described in the example provided
 * by texas instruments found at
 * http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP-EXP432P401R/latest/index_FDS.html
 *
 * @return void
 */
void lcdInit(void);


/*
 * @brief Send the register comnmand
 *
 * This function selects the SDC line, sends the
 * register address where data is to be written,
 * and deselects the SDC line
 *
 * @param The register command address to be written
 * @return void
 */
void lcdSendCommand(uint8_t command);

/*
 * @brief Send 2 bytes of SPI data
 *
 * This function sends in total 16 bits of
 * data via SPI, MSB first.
 * Uses polling approach
 *
 * @param 16 bits of data to be sent
 * @return void
 */
void spi16bytes(uint16_t in);

/*
 * @brief Send 1 byte of SPI data
 *
 * This function sends in total 8 bits of
 * data via SPI, MSB first.
 * Uses polling approach
 *
 * @param 8 bits of data to be sent
 * @return void
 */
void spi8bytes(uint8_t in);

/*
 * @brief Selects/Deselects CS pin
 *
 * Toggles the SCS GPIO high or low,
 * depending upon the parameter.
 * Active low, selects the LCD
 *
 * @param state of the GPIO pin, 1 = HIGH, 0 = LOW
 * @return void
 */
void selectCS(int state);

/*
 * @brief Selects/Deselects DC pin
 *
 * Toggles the SCD GPIO high or low,
 * depending upon the parameter.
 * Active low, selects whether the input
 * is command or data
 *
 * @param state of the GPIO pin, 1 = HIGH, 0 = LOW
 * @return void
 */
void selectDC(int state);

/*
 * @brief Blocking delay in ms
 *
 * Blocking delay in milliseconds, using a for
 * loop that uses the system clock speed for
 * exact calculations
 *
 * Reference: http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP-EXP432P401R/latest/index_FDS.html
 *
 * @param amount of delay in ms
 * @return void
 */
void delay(uint16_t msec);

/*
 * @brief Sets the cursor mode
 *
 * Sets the cursor mode of the LCD display
 * to auto increment/decrement in horizontal/
 * vertical direction upon GRAM data write
 *
 * @param id1, id0: ID[1:0] Decides the inc/dec direction
 * @param am: Decides the cursor movement in the vertical direction
 * @return void
 */
void setCursorMode(uint8_t id1, uint8_t id0, uint8_t am);

/*
 * @brief Sets a pixel to a perticular color
 *
 * Sets a pixel to a perticular color, requires
 * X-Y position and the color
 *
 * @param posX The X axis position of the pixel to be displayed
 * @param posY The Y axis position of the pixel to be displayed
 * @param color The 16-bit color of the pixel
 * @return void
 */
void drawPixelXY(uint16_t posX, uint16_t posY, uint16_t color);

/*
 * @brief Draw a line in horizontal direction
 *
 * Draws a line in the horizontal direction, given the
 * start X-Y position, color and length
 *
 * @param startPosX The X axis position of the line to be displayed from
 * @param length the length of the line, in pixels
 * @param startPosY The Y axis position of the line to be displayed from
 * @param color The 16-bit color of the pixel
 * @return void
 */
void drawLineX(uint16_t startPosX, uint16_t length, uint16_t startPosY, uint16_t color);

/*
 * @brief Draw a line in vertical direction
 *
 * Draws a line in the vertical direction, given the
 * start X-Y position, color and length
 *
 * @param startPosY The Y axis position of the line to be displayed from
 * @param length the length of the line, in pixels
 * @param startPosX The X axis position of the line to be displayed from
 * @param color The 16-bit color of the pixel
 * @return void
 */
void drawLineY(uint16_t startPosY, uint16_t length, uint16_t startPosX, uint16_t color);

/*
 * @brief Write a character on the LCD
 *
 * Writes a character on the LCD, given the
 * start X-Y position, color and the input character
 *
 * @param posX The X axis position of the pixel to be displayed
 * @param posY The Y axis position of the pixel to be displayed
 * @param color The 16-bit color of the character
 * @param inChar The character ASCII code to be written on the LCD
 * @return void
 */
void writeLetter(uint16_t posX, uint16_t posY, uint16_t color, char inChar);

/*
 * @brief Sets the X-Y position of the cursor
 *
 * Set the X-Y position of the cursor of the LCD
 *
 * @param posX The X axis position of the cursor
 * @param posY The Y axis position of the cursor
 * @return void
 */
void setCursorXY(uint16_t posX, uint16_t posY);

/*
 * @brief Write a string on the LCD
 *
 * Writes a string on the LCD, given the
 * start X-Y position, color and the input character
 *
 * @param posX The X axis position of the pixel to be displayed
 * @param posY The Y axis position of the pixel to be displayed
 * @param color The 16-bit color of the characters
 * @param *inString The pointer to the character array which is to be written on the LCD
 * @return void
 */
void writeString(uint16_t posX, uint16_t posY, uint16_t color, char* inString);



#endif /* LCD_DRIVER_H_ */
