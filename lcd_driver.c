/*
 * @file lcd_driver.c
 * @brief The LCD driver for the BOOSTXL-K350QVG boosterpack
 *
 * This source file contains the LCD driver for the
 * BOOSTXL-K350QVG boosterpack. It uses 4-wire SPI to
 * communicate with the LCD, and is capable og drawing
 * pixels, lines and text on the display
 *
 * PLEASE SEE THE CORRESPONDING HEADER FILES FOR DETAILED
 * FUNCTION DESCRIPTIONS
 *
 * @date 03-Dec-2019
 * @author Prayag Milan Desai, Rahul Ramaprasad
 */


#include "lcd_driver.h"


//Set the ID[1:0] bits and the AM bit to configure the cursor mode
void setCursorMode(uint8_t id1, uint8_t id0, uint8_t am)
{
    //create the curcor config bits
    uint16_t temp = ((id1 << 2) | (id0 << 1) | am) << 3;
    //Combine them with the default cursor config
    temp |= 0x6800;
    //Send the register address and the data created
    lcdSendCommand(SSD2119_ENTRY_MODE_REG);
    spi16bytes(temp);
}

//Delay for a perticular number of milliseconds
//Taken from the example LCD driver found at
//http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP-EXP432P401R/latest/index_FDS.html
void delay(uint16_t msec)
{
    //loop counter
    uint32_t i = 0;
    //Calculate time taken from clock speed and machine cycles
    uint32_t time = (msec / 1000) * (SYSTEM_CLOCK_SPEED / 15);

    //Run a blank loop
    for(i = 0; i < time; i++)
    {
        ;
    }
}

//Send the command register address to the LCD
void lcdSendCommand(uint8_t command)
{
    //Select command line
    selectDC(0);
    //send the command
    spi8bytes(command);
    //Deselect command line
    selectDC(1);
}

//Send 16 byes of SPI data
void spi16bytes(uint16_t in)
{
    //Load the MSB
    EUSCI_B0->TXBUF = (uint8_t)(in >> 8);
    while (!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));

    //Load the LSB
    EUSCI_B0->TXBUF = (uint8_t)(in & 0xFF);
    while (!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));

}

//Send 8 bytes of SPI data
void spi8bytes(uint8_t in)
{
    //load the 8 bytes
    EUSCI_B0->TXBUF = in;
    while (!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));
}

//Select/Deselect the LCD
void selectCS(int state)
{
    //Set or clear the SCS line
    if(state)
        P5->OUT |= LCD_CS;
    else
        P5->OUT &= ~LCD_CS;
}

//Select/Deselect the LCD command line
void selectDC(int state)
{
    //Set or clear the SDC line
    if(state)
        P4->OUT |= DC;
    else
        P4->OUT &= ~DC;
}

//Set the position of the cursor in X-Y axis
void setCursorXY(uint16_t posX, uint16_t posY)
{
    //Send the X position
    lcdSendCommand(SSD2119_X_RAM_ADDR_REG);
    spi16bytes(posX);
    //Send the Y position
    lcdSendCommand(SSD2119_Y_RAM_ADDR_REG);
    spi16bytes(posY);
}

//Dray a pixel on the LCD
void drawPixelXY(uint16_t posX, uint16_t posY, uint16_t color)
{
    //Select the LCD
    selectCS(0);
    //Set the cursor
    setCursorXY(posX, posY);
    //Write the pixel data
    lcdSendCommand(SSD2119_RAM_DATA_REG);
    spi16bytes(color);
    //Deselect the LCD
    selectCS(1);
}

//Draw a horizontal line
void drawLineX(uint16_t startPosX, uint16_t length, uint16_t startPosY, uint16_t color)
{
    //Select the LCD
    selectCS(0);

    //Set the X and Y cursor
    lcdSendCommand(SSD2119_X_RAM_ADDR_REG);
    spi16bytes(startPosX);
    lcdSendCommand(SSD2119_Y_RAM_ADDR_REG);
    spi16bytes(startPosY);

    //Config the cursor to auto increment horizontally
    setCursorMode(1, 1, 0);

    //Send the pixel data till the length of the line is reached
    uint16_t i;
    lcdSendCommand(SSD2119_RAM_DATA_REG);
    for(i = 0; i < length; i++)
        spi16bytes(color);

    //Deselect the LCD
    selectCS(1);
}

//Draw a vertical line
void drawLineY(uint16_t startPosY, uint16_t length, uint16_t startPosX, uint16_t color)
{
    //Select the LCD
    selectCS(0);

    //Set the X and Y cursor
    lcdSendCommand(SSD2119_X_RAM_ADDR_REG);
    spi16bytes(startPosX);
    lcdSendCommand(SSD2119_Y_RAM_ADDR_REG);
    spi16bytes(startPosY);

    //Config the cursor to auto increment vertically
    setCursorMode(1, 1, 1);

    //Send the pixel data till the length of the line is reached
    uint16_t i;
    lcdSendCommand(SSD2119_RAM_DATA_REG);
    for(i = 0; i < length; i++)
        spi16bytes(color);

    //Deselect the LCD
    selectCS(0);
}

//Write a string of characters on the LCD
void writeString(uint16_t posX, uint16_t posY, uint16_t color, char *inString)
{
    //If the string still exists, write it to the LCD
    while(*inString){
        writeLetter(posX, posY, color, *inString++);
        //Move X to the right by the character width
        posX += 7;
    }
}

//Write a single letter on the LCD
void writeLetter(uint16_t posX, uint16_t posY, uint16_t color, char inChar)
{
    //Select the LCD
    selectCS(0);

    //Return if invalid character entered
    if(inChar < ' ' || inChar > '~')
        return;

    //Offset the input if spce is detected
    if(inChar == ' ')
        inChar = '~' + 1;

    //Offset character to index 0
    inChar -= '!';

    //Loop counters
    int i = 0;
    int j = 0;
    //Start and end index of the array
    int startFont = inChar * 11;
    int endFont = (inChar * 11) + 10;

    //Cursor mode set
    setCursorMode(1, 1, 0);
    setCursorXY(posX, posY);


    //Send command
    for(i = startFont; i <= endFont; i++){
        lcdSendCommand(SSD2119_RAM_DATA_REG);
        //Write X axis
        for(j = 7; j >= 0; j--){
            //Write Y axis
            uint16_t temp = ((lucidaConsole_8ptBitmaps_narrow[i] >> j) & 0x01);
            if(temp){
                spi16bytes(color);
            } else {
                spi16bytes(BLACK);
            }
        }

        //Set the new cursor position
        setCursorXY(posX, ++posY);

    }
    //Deselect the LCD
    selectCS(1);
}


// SPI interrupt service routine
void EUSCIB0_IRQHandler(void)
{

    if (EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG)
    {
    }
}


//Initialize the LCD
//Complete routine referenced from
//http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP-EXP432P401R/latest/index_FDS.html
void lcdInit(void)
{
    while (!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));

    //loop counter
    uint32_t ulCount;

    //
    // Select the LCD for SPI comunication.
    //
    selectCS(0);

    //
    // Enter sleep mode (if we are not already there).
    //
    lcdSendCommand(SSD2119_SLEEP_MODE_1_REG);
    spi16bytes(0x0001);

    //
    // Set initial power parameters.
    //
    lcdSendCommand(SSD2119_PWR_CTRL_5_REG);
    spi16bytes(0x00B2);
    lcdSendCommand(SSD2119_VCOM_OTP_1_REG);
    spi16bytes(0x0006);

    //
    // Start the oscillator.
    //
    lcdSendCommand(SSD2119_OSC_START_REG);
    spi16bytes(0x0001);

    //
    // Set pixel format and basic display orientation (scanning direction).
    //
    lcdSendCommand(SSD2119_OUTPUT_CTRL_REG);
    spi16bytes(0x30EF);
    lcdSendCommand(SSD2119_LCD_DRIVE_AC_CTRL_REG);
    spi16bytes(0x0600);

    //
    // Exit sleep mode.
    //
    lcdSendCommand(SSD2119_SLEEP_MODE_1_REG);
    spi16bytes(0x0000);

    //
    // Delay 30mS
    //
    delay(30);

    //
    // Configure pixel color format and MCU interface parameters.
    //
    //    lcdSendCommand(SSD2119_ENTRY_MODE_REG);
    //    spi16bytes(ENTRY_MODE_DEFAULT);
    setCursorMode(1,1,0);

    //
    // Set analog parameters.
    //
    lcdSendCommand(SSD2119_SLEEP_MODE_2_REG);
    spi16bytes(0x0999);
    lcdSendCommand(SSD2119_ANALOG_SET_REG);
    spi16bytes(0x3800);

    //
    // Enable the display.
    //
    lcdSendCommand(SSD2119_DISPLAY_CTRL_REG);
    spi16bytes(0x0033);

    //
    // Set VCIX2 voltage to 6.1V.
    //
    lcdSendCommand(SSD2119_PWR_CTRL_2_REG);
    spi16bytes(0x0005);

    //
    // Configure gamma correction.
    //
    lcdSendCommand(SSD2119_GAMMA_CTRL_1_REG);
    spi16bytes(0x0000);
    lcdSendCommand(SSD2119_GAMMA_CTRL_2_REG);
    spi16bytes(0x0303);
    lcdSendCommand(SSD2119_GAMMA_CTRL_3_REG);
    spi16bytes(0x0407);
    lcdSendCommand(SSD2119_GAMMA_CTRL_4_REG);
    spi16bytes(0x0301);
    lcdSendCommand(SSD2119_GAMMA_CTRL_5_REG);
    spi16bytes(0x0301);
    lcdSendCommand(SSD2119_GAMMA_CTRL_6_REG);
    spi16bytes(0x0403);
    lcdSendCommand(SSD2119_GAMMA_CTRL_7_REG);
    spi16bytes(0x0707);
    lcdSendCommand(SSD2119_GAMMA_CTRL_8_REG);
    spi16bytes(0x0400);
    lcdSendCommand(SSD2119_GAMMA_CTRL_9_REG);
    spi16bytes(0x0a00);
    lcdSendCommand(SSD2119_GAMMA_CTRL_10_REG);
    spi16bytes(0x1000);

    //
    // Configure Vlcd63 and VCOMl.
    //
    lcdSendCommand(SSD2119_PWR_CTRL_3_REG);
    spi16bytes(0x000A);
    lcdSendCommand(SSD2119_PWR_CTRL_4_REG);
    spi16bytes(0x2E00);

    //
    // Set the display size and ensure that the GRAM window is set to allow
    // access to the full display buffer.
    //
    lcdSendCommand(SSD2119_V_RAM_POS_REG);
    spi16bytes((uint16_t)(LCD_VERTICAL_MAX - 1) << 8);
    lcdSendCommand(SSD2119_H_RAM_START_REG);
    spi16bytes(0x0000);
    lcdSendCommand(SSD2119_H_RAM_END_REG);
    spi16bytes(LCD_HORIZONTAL_MAX - 1);
    lcdSendCommand(SSD2119_X_RAM_ADDR_REG);
    spi16bytes(0x00);
    lcdSendCommand(SSD2119_Y_RAM_ADDR_REG);
    spi16bytes(0x00);

    //
    // Clear the contents of the display buffer.
    //
    lcdSendCommand(SSD2119_RAM_DATA_REG);
    for(ulCount = 0; ulCount < 76800; ulCount++)
    {
        spi16bytes(0x0000);    // Black
    }

    //deselect the LCD
    selectCS(1);

}
