//*****************************************************************************
//
// HAL_MSP_EXP430F5529LP_KITRONIX320X240_SSD2119_SPI.h - Prototypes for the
//           KITRONIX320X240_SSD2119 LCD display driver.
//
//
//                 MSP430F5529                 BOOSTXL-K350QVG-S1
//                -----------------              ------------
//               |     P1.6/UCB0SIMO|---------> |LCD_SDI     |
//            /|\|                  |           |            |
//             | |      P1.5/UCB0CLK|---------> |LCD_SCL     |
//             --|RST               |           |            |
//               |              P5.0|---------> |LCD_SCS     |
//               |              P4.6|---------> |LCD_SDC     |
//               |              P3.5|---------> |LCD_RESET   |
//               |        P2.7/TA2.2|---------> |LCD_PWM     |
//               |                  |           |            |
//               |                  |            ------------
//                ------------------
//****************************************************************************


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


int main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW |             // Stop watchdog timer
            WDT_A_CTL_HOLD;


    //Set as outputs, inputs and SPI function of the GPIO
    P1->DIR |= BIT5 | BIT6;
    //    P1->DIR &= ~BIT5;
    P1->SEL0 |= BIT5 | BIT6;

    P5->DIR |= BIT0;
    P5->OUT &= ~BIT0;

    P4->DIR |= BIT6;
    P4->OUT &= ~BIT6;

    P3->DIR |= BIT5;
    P3->OUT &= ~BIT5;

    P2->DIR |= BIT7;
    P2->OUT |= BIT7;


    //    EUSCI_B_SPI_MSB_FIRST,                             // MSB First
    //            EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT,         // Phase
    //            EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW,         // Low polarity
    //            EUSCI_B_SPI_3PIN                                   // 3Wire SPI Mode


    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST; // Put eUSCI state machine in reset
    EUSCI_B0->CTLW0 = EUSCI_B_CTLW0_SWRST | // Remain eUSCI state machine in reset
            EUSCI_B_CTLW0_MST |             // Set as SPI master
            EUSCI_B_CTLW0_SYNC |            // Set as synchronous mode
            EUSCI_B_CTLW0_CKPH |
            EUSCI_B_CTLW0_MSB;              // MSB first

    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SSEL__SMCLK; // SMCLK
    EUSCI_B0->BRW = 0x01;                   // /2,fBitClock = fBRCLK/(UCBRx+1).
    EUSCI_B0->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;// Initialize USCI state machine
    EUSCI_B0->IE |= EUSCI_B_IE_TXIE;        // Enable USCI_B0 TX interrupt


    selectDC(1);
    selectCS(1);

    delay(1);

    P3->OUT |= BIT5;

    delay(1);


    //    delay(10);

    // Enable global interrupt
    __enable_irq();

    // Enable eUSCI_B0 interrupt in NVIC module
    //    NVIC->ISER[0] = 1 << ((EUSCIB0_IRQn) & 31);

    //Infinite loop
    lcdInit();

//    int i = 0;
//    int fontno = 1;
//
//    int startFont = fontno * 22;
//    int endFont = (fontno * 22) + 21;

//    for(i = startFont; i <= endFont; i++){
//        printf("i: %d | value: 0x%X\n", i, lucidaConsole_8ptBitmaps[i]);
//    }


//    writeLetter(50, 50, GREEN, 'O');
    writeString(0, 0, RED, "The quick brown fox jumped over the lazy dog.");
    writeString(0, 12, RED, "!@#$%^&*()_+}{~,./';\|/");
    writeString(0, 24, RED, "Voltage (V) ->");

    while(1){

    }
}



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

void drawLineX(uint16_t startPosX, uint16_t length, uint16_t startPosY, uint16_t color)
{

    lcdSendCommand(SSD2119_X_RAM_ADDR_REG);
    spi16bytes(startPosX);
    lcdSendCommand(SSD2119_Y_RAM_ADDR_REG);
    spi16bytes(startPosY);

    setCursorMode(1, 1, 0);

    uint16_t i;
    lcdSendCommand(SSD2119_RAM_DATA_REG);
    for(i = 0; i < length; i++)
        spi16bytes(color);
}

void drawLineY(uint16_t startPosY, uint16_t length, uint16_t startPosX, uint16_t color)
{
    lcdSendCommand(SSD2119_X_RAM_ADDR_REG);
    spi16bytes(startPosX);
    lcdSendCommand(SSD2119_Y_RAM_ADDR_REG);
    spi16bytes(startPosY);

    setCursorMode(1, 1, 1);

    uint16_t i;
    lcdSendCommand(SSD2119_RAM_DATA_REG);
    for(i = 0; i < length; i++)
        spi16bytes(color);
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

    if(inChar < '!' || inChar > '~')
        return;

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


    drawLineX(0, 5, 0, BLUE);
    drawLineX(10, 10, 10, RED);
    drawLineX(20, 15, 20, GREEN);
    drawLineX(30, 20, 30, BLUE);

    drawLineY(50, 5, 0, BLUE);
    drawLineY(60, 10, 10, RED);
    drawLineY(70, 15, 20, GREEN);
    drawLineY(80, 20, 30, BLUE);



    //
    // Deselect the LCD for SPI comunication.
    //
    selectCS(1);

}
