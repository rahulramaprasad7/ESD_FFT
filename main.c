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
#include "lcd_driver.h"
#include "lcd_touch_driver.h"
#include <math.h>

uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max);

#define arraySize 900

uint16_t testArray[8192] = {0};

int main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW |             // Stop watchdog timer
            WDT_A_CTL_HOLD;

    uint16_t i = 0;
    for(i = 0; i < arraySize - 1; i++)
        testArray[i] = 5;

    for(i = 0; i < arraySize - 1; i++)
        testArray[i] = (uint16_t)((double)100 + (sin(M_PI * ((double)i/100)) * (double)100));


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

    // Enable global interrupt
    __enable_irq();

    // Enable eUSCI_B0 interrupt in NVIC module
    //    NVIC->ISER[0] = 1 << ((EUSCIB0_IRQn) & 31);

    //Infinite loop
    lcdInit();

    //    writeLetter(50, 50, GREEN, 'O');
    //    writeString(0, 0, WHITE, "The quick brown fox jumped over the lazy dog.");
    //    writeString(0, 12, WHITE, "!@#$%^&*()_+}{~,./';\\|/");
    //    writeString(0, 24, WHITE, "Voltage (V) ->");


    ADC14->CTL0 = ADC14_CTL0_ON |
            ADC14_CTL0_MSC |
            ADC14_CTL0_SHT0__192 |
            ADC14_CTL0_SHP |
            ADC14_CTL0_CONSEQ_3;

    ADC14->MCTL[0] = ADC14_MCTLN_INCH_13;    // ref+=AVcc, channel = A0
    ADC14->MCTL[1] = ADC14_MCTLN_INCH_14 | ADC14_MCTLN_EOS;    // ref+=AVcc, channel = A1


    ADC14->IER0 = ADC14_IER0_IE1;           // Enable ADC14IFG.3

    SCB->SCR &= ~SCB_SCR_SLEEPONEXIT_Msk;   // Wake up on exit from ISR

    //(320x240 resolution)
//    selectCS(0);
    drawLineX(20, 300, 220, GREEN);
    drawLineY(0, 220, 20, GREEN);
//    selectCS(1);

    //    uint16_t displayValues[300];
    int noPts = (int)arraySize/300;
    for(i = 0; i < 300; i++){
        uint16_t temp = testArray[i * noPts];
//        drawPixelXY(20 + i, 239 - (20 + temp), WHITE);
//        selectCS(0);
        drawLineY(239 - (20 + temp), temp, 20 + i, WHITE);
//        selectCS(1);
    }

    while(1){
        ADC14->CTL0 |= ADC14_CTL0_ENC |
                ADC14_CTL0_SC;

        uint16_t inX = getTouchX();
        delay(10);
        uint16_t inY = getTouchY();

        //        if(touchX < 14000 && touchY < 14000)
        //            if(touchX >= 8000 && touchX <= 10000 && touchY >= 8000 && touchY <= 10000)
        //                writeString(0, 50, RED, "TOUCH DETECTED    ");
        //            else
        //                writeString(0, 50, RED, "TOUCH NOT DETECTED");
    }
}

uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

