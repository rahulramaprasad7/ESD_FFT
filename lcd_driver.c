/*
 * lcd_driver.c
 *
 *  Created on: 03-Dec-2019
 *      Author: prayag
 */


#include "lcd_driver.h"



void setCursorMode(uint8_t id1, uint8_t id0, uint8_t am)
{
    uint16_t temp = ((id1 << 2) | (id0 << 1) | am) << 3;
    temp |= 0x6800;
    lcdSendCommand(SSD2119_ENTRY_MODE_REG);
    spi16bytes(temp);
}

void delay(uint16_t msec)
{
    uint32_t i = 0;
    uint32_t time = (msec / 1000) * (SYSTEM_CLOCK_SPEED / 15);

    for(i = 0; i < time; i++)
    {
        ;
    }
}

void lcdSendCommand(uint8_t command)
{
    selectDC(0);
    spi8bytes(command);
    selectDC(1);
}

void spi16bytes(uint16_t in)
{
    //    selectCS(1);
    EUSCI_B0->TXBUF = (uint8_t)(in >> 8);
    while (!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));

    EUSCI_B0->TXBUF = (uint8_t)(in & 0xFF);
    while (!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));

}

void spi8bytes(uint8_t in)
{
    //    selectCS(1);
    EUSCI_B0->TXBUF = in;
    while (!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));
}

void selectCS(int state)
{
    if(state)
        P5->OUT |= LCD_CS;
    else
        P5->OUT &= ~LCD_CS;
}

void selectDC(int state)
{
    if(state)
        P4->OUT |= DC;
    else
        P4->OUT &= ~DC;
}

void setCursorXY(uint16_t posX, uint16_t posY)
{
    lcdSendCommand(SSD2119_X_RAM_ADDR_REG);
    spi16bytes(posX);
    lcdSendCommand(SSD2119_Y_RAM_ADDR_REG);
    spi16bytes(posY);
}

void drawPixelXY(uint16_t posX, uint16_t posY, uint16_t color)
{
    selectCS(0);
    setCursorXY(posX, posY);
    lcdSendCommand(SSD2119_RAM_DATA_REG);
    spi16bytes(color);
    selectCS(1);
}

void drawLineX(uint16_t startPosX, uint16_t length, uint16_t startPosY, uint16_t color)
{
    selectCS(0);

    lcdSendCommand(SSD2119_X_RAM_ADDR_REG);
    spi16bytes(startPosX);
    lcdSendCommand(SSD2119_Y_RAM_ADDR_REG);
    spi16bytes(startPosY);

    setCursorMode(1, 1, 0);

    uint16_t i;
    lcdSendCommand(SSD2119_RAM_DATA_REG);
    for(i = 0; i < length; i++)
        spi16bytes(color);

    selectCS(1);
}

void drawLineY(uint16_t startPosY, uint16_t length, uint16_t startPosX, uint16_t color)
{
    selectCS(0);

    lcdSendCommand(SSD2119_X_RAM_ADDR_REG);
    spi16bytes(startPosX);
    lcdSendCommand(SSD2119_Y_RAM_ADDR_REG);
    spi16bytes(startPosY);

    setCursorMode(1, 1, 1);

    uint16_t i;
    lcdSendCommand(SSD2119_RAM_DATA_REG);
    for(i = 0; i < length; i++)
        spi16bytes(color);

    selectCS(0);
}

void writeString(uint16_t posX, uint16_t posY, uint16_t color, char *inString)
{
    printf("\n");

    while(*inString){
        writeLetter(posX, posY, color, *inString++);
        posX += 7;
    }
}

//int putstr (char *s)
//{
//    int i = 0;
//    while (*s){
//        putchar(*s++);
//        i++;
//    }
//    return i+1;
//}

void writeLetter(uint16_t posX, uint16_t posY, uint16_t color, char inChar)
{
    selectCS(0);

    if(inChar < ' ' || inChar > '~')
        return;

    if(inChar == ' ')
            inChar = '~' + 1;

    inChar -= '!';
//    printf("Entered Character: %d\n", inChar);

    int i = 0;
    int j = 0;
    int startFont = inChar * 11;
    int endFont = (inChar * 11) + 10;

//    printf("Start: %d End: %d\n", startFont, endFont);

    setCursorMode(1, 1, 0);
    setCursorXY(posX, posY);


    for(i = startFont; i <= endFont; i++){
//        printf("IN: 0x%X ", lucidaConsole_8ptBitmaps[i]);
        lcdSendCommand(SSD2119_RAM_DATA_REG);

        for(j = 7; j >= 0; j--){
            uint16_t temp = ((lucidaConsole_8ptBitmaps_narrow[i] >> j) & 0x01);
            if(temp){
//                printf("1");
                spi16bytes(color);
            } else {
//                printf("0");
                spi16bytes(BLACK);
            }
        }
//        printf("\n");

//        if((i + 1) % 2 == 0 && i != 0){
            setCursorXY(posX, ++posY);
//        }

    }
    selectCS(1);

}


// SPI interrupt service routine
void EUSCIB0_IRQHandler(void)
{

    if (EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG)
    {
        //send MSB
        //        EUSCI_B0->TXBUF = MSB;
        //        while (!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));

    }
}



void lcdInit(void)
{
    while (!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));

    uint32_t ulCount;
    //    volatile uint32_t i;

    //    HAL_LCD_initLCD();

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
        //        spi16bytes(0xF800);    // Red
//        delay(1);
        //        spi16bytes(0x07E0);    // Green
        //        spi16bytes(0x001F);    // Blue
    }


//    drawLineX(0, 5, 0, BLUE);
//    drawLineX(10, 10, 10, RED);
//    drawLineX(20, 15, 20, GREEN);
//    drawLineX(30, 20, 30, BLUE);
//
//    drawLineY(50, 5, 0, BLUE);
//    drawLineY(60, 10, 10, RED);
//    drawLineY(70, 15, 20, GREEN);
//    drawLineY(80, 20, 30, BLUE);



    //
    // Deselect the LCD for SPI comunication.
    //
    selectCS(1);

}
