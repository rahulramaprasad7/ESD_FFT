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

//    writeLetter(50, 50, GREEN, 'O');
    writeString(0, 0, WHITE, "The quick brown fox jumped over the lazy dog.");
    writeString(0, 12, WHITE, "!@#$%^&*()_+}{~,./';\\|/");
    writeString(0, 24, WHITE, "Voltage (V) ->");


    ADC14->IER0 |= ADC14_IER0_IE14;          // Enable ADC conv complete interrupt

    while(1){
        getTouchX();
//        printf("ADC: %d\n", getTouchX());
    }
}


