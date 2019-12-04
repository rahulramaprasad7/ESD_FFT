/*
 * lcd_touch_driver.c
 *
 *  Created on: 03-Dec-2019
 *      Author: prayag
 */

#include "lcd_touch_driver.h"

//Read X: read Y1, X2 GND, X1 VCC, Y2 Tristate
//Read Y: read X1, Y2 GND, Y1 VCC, X2 Tristate


//void adc_init()
//{
//    P5->SEL1 |= BIT4;                       // Configure P5.4 for ADC
//    P5->SEL0 |= BIT4;
//
//    // Enable global interrupt
//    __enable_irq();
//
//    // Enable ADC interrupt in NVIC module
//    NVIC->ISER[0] = 1 << ((ADC14_IRQn) & 31);
//
//    // Sampling time, S&H=16, ADC14 on
//    ADC14->CTL0 = ADC14_CTL0_SHT0_2 | ADC14_CTL0_SHP | ADC14_CTL0_ON;
//    ADC14->CTL1 = ADC14_CTL1_RES_3;         // Use sampling timer, 12-bit conversion results
//
//    ADC14->MCTL[0] |= ADC14_MCTLN_INCH_1;   // A1 ADC input select; Vref=AVCC
//    ADC14->IER0 |= ADC14_IER0_IE0;          // Enable ADC conv complete interrupt
//    in_data[i].real = (3.3 * ((float)ADC14->MEM[0]) / 16383) - 1.65;  //Store the ADC sample
//}
bool xready = false;
bool yready = false;
volatile long i = 0;
uint16_t getTouchX(void)
{
    //X2 GND
    TOUCH_X_MINUS_PORT->SEL0 &= ~TOUCH_X_MINUS_PIN; //GPIO MODE
    TOUCH_X_MINUS_PORT->SEL1 &= ~TOUCH_X_MINUS_PIN; //GPIO MODE
    TOUCH_X_MINUS_PORT->DIR |= TOUCH_X_MINUS_PIN;   //OUTPUT
    TOUCH_X_MINUS_PORT->OUT &= ~TOUCH_X_MINUS_PIN;  //GND

    //X1 VCC
    TOUCH_X_PLUS_PORT->SEL0 &= ~TOUCH_X_PLUS_PIN;   //GPIO MODE
    TOUCH_X_PLUS_PORT->SEL1 &= ~TOUCH_X_PLUS_PIN;   //GPIO MODE
    TOUCH_X_PLUS_PORT->DIR |= TOUCH_X_PLUS_PIN;     //OUTPUT
    TOUCH_X_PLUS_PORT->OUT |= TOUCH_X_PLUS_PIN;     //VCC

    //Y2 TRISTATE
    TOUCH_Y_MINUS_PORT->SEL0 &= ~TOUCH_Y_MINUS_PIN; //GPIO MODE
    TOUCH_Y_MINUS_PORT->SEL1 &= ~TOUCH_Y_MINUS_PIN; //GPIO MODE
    TOUCH_Y_MINUS_PORT->DIR &= ~TOUCH_Y_MINUS_PIN;  //INPUT

    //ADC input Y1
    TOUCH_Y_PLUS_PORT->DIR &= ~TOUCH_Y_PLUS_PIN;

    TOUCH_Y_PLUS_PORT->SEL1 |= TOUCH_Y_PLUS_PIN;                             //ADC mode
    TOUCH_Y_PLUS_PORT->SEL0 |= TOUCH_Y_PLUS_PIN;                              //ADC mode


    //    if(!yready)
    if(!xready){
        //            printf("X\n");
        NVIC->ISER[0] = 1 << ((ADC14_IRQn) & 31);
        xready = true;
//
//        // Sampling time, S&H=16, ADC14 on
//        ADC14->CTL0 = ADC14_CTL0_SHT0_2 | ADC14_CTL0_SHP | ADC14_CTL0_ON | ADC14_CTL0_CONSEQ_3;
//        //        ADC14->CTL1 = ADC14_CTL1_RES_2;         // Use sampling timer, 12-bit conversion results
//
//        ADC14->MCTL[0] |= ADC14_MCTLN_INCH_14;   // A1 ADC input select; Vref=AVCC
//        ADC14->IER0 |= ADC14_IER0_IE0;          // Enable ADC conv complete interrupt
//        ADC14->CTL0 |= ADC14_CTL0_ENC | ADC14_CTL0_SC;
    }


    //    // Turn on ADC14, extend sampling time to avoid overflow of results
    //    ADC14->CTL0 = ADC14_CTL0_ON |
    //            ADC14_CTL0_MSC |
    //            ADC14_CTL0_SHT0__192 |
    //            ADC14_CTL0_SHP |
    //            ADC14_CTL0_CONSEQ_0;
    //    ADC14->CTL0 &= ~ADC14_CTL0_ENC;
    //
    ////    ADC14->MCTL[2] = 0;
    //    ADC14->MCTL[14] = TOUCH_Y_PLUS_ADC_CHANNEL;    // ref+=AVcc, channel = A1
    //    ADC14->CTL1 |= ADC14_MCTLN_INCH_14 << ADC14_CTL1_CSTARTADD_OFS;
    //
    //    ADC14->MEM[14] = 0;;
    //    ADC14->IER0 = ADC14_IER0_IE14;           // Enable ADC14IFG.3
    //
    //    ADC14->CTL0 |= ADC14_CTL0_ENC | ADC14_CTL0_SC;
    //
    //
    //    //    delay(10);
    //
    //    uint16_t result = ADC14->MEM[14];
    //    ADC14->CTL0 &= ~ADC14_CTL0_ENC;
    //
    //
    //    printf("X: %d\n", result);

    uint32_t result = 0;
    if(result <= 3000){
        return 0;
    } else
        return result;
}

void ADC14_IRQHandler(void) {
    ++i;
    if(yready){
        yready = false;
        printf("X:\t %d\t%d\n",ADC14->MEM[1],i);
    }

    if(xready){
        xready = false;
        printf("Y: %d\t\t%d\n",ADC14->MEM[0],i);
    }
    NVIC->ICER[0] = 1 << ((ADC14_IRQn) & 31);
//    ADC14->CTL0 &= ~ADC14_CTL0_ON;
}


//Read Y: read X1, Y2 GND, Y1 VCC, X2 Tristate
uint16_t getTouchY(void)
{
    //Y2 GND
    TOUCH_Y_MINUS_PORT->SEL0 &= ~TOUCH_Y_MINUS_PIN; //GPIO MODE
    TOUCH_Y_MINUS_PORT->SEL1 &= ~TOUCH_Y_MINUS_PIN; //GPIO MODE
    TOUCH_Y_MINUS_PORT->DIR |= TOUCH_Y_MINUS_PIN;   //OUTPUT
    TOUCH_Y_MINUS_PORT->OUT &= ~TOUCH_Y_MINUS_PIN;  //GND

    //Y1 VCC
    TOUCH_Y_PLUS_PORT->SEL0 &= ~TOUCH_Y_PLUS_PIN;   //GPIO MODE
    TOUCH_Y_PLUS_PORT->SEL1 &= ~TOUCH_Y_PLUS_PIN;   //GPIO MODE
    TOUCH_Y_PLUS_PORT->DIR |= TOUCH_Y_PLUS_PIN;     //OUTPUT
    TOUCH_Y_PLUS_PORT->OUT |= TOUCH_Y_PLUS_PIN;     //VCC

    //X2 TRISTATE
    TOUCH_X_MINUS_PORT->SEL0 &= ~TOUCH_X_MINUS_PIN; //GPIO MODE
    TOUCH_X_MINUS_PORT->SEL1 &= ~TOUCH_X_MINUS_PIN; //GPIO MODE
    TOUCH_X_MINUS_PORT->DIR &= ~TOUCH_X_MINUS_PIN;  //INPUT

    //ADC input X1
    TOUCH_X_PLUS_PORT->DIR &= ~TOUCH_X_PLUS_PIN;
    TOUCH_X_PLUS_PORT->SEL1 |= TOUCH_X_PLUS_PIN;                             //ADC mode
    TOUCH_X_PLUS_PORT->SEL0 |= TOUCH_X_PLUS_PIN;                              //ADC mode

    //    // Turn on ADC14, extend sampling time to avoid overflow of results
    //    ADC14->CTL0 = ADC14_CTL0_ON |
    //            ADC14_CTL0_MSC |
    //            ADC14_CTL0_SHT0__192 |
    //            ADC14_CTL0_SHP |
    //            ADC14_CTL0_CONSEQ_1;
    //
    //    ADC14->CTL0 &= ~ADC14_CTL0_ENC;
    //    ADC14->MCTL[1] = 0;
    //    ADC14->MCTL[2] = TOUCH_X_PLUS_ADC_CHANNEL;    // ref+=AVcc, channel = A1
    //
    //    ADC14->IER0 = ADC14_IER0_IE2;           // Enable ADC14IFG.3
    //
    //    ADC14->CTL0 |= ADC14_CTL0_ENC | ADC14_CTL0_SC;
    //
    //    uint16_t result = ADC14->MEM[2];
    //    ADC14->CTL0 &= ~ADC14_CTL0_ENC;
    //
    //    printf("Y: %d\n", result);

    if(!yready){
        //        printf("Y\n");
        NVIC->ISER[0] = 1 << ((ADC14_IRQn) & 31);
        yready = true;
//
//        // Sampling time, S&H=16, ADC14 on
//        ADC14->CTL0 = ADC14_CTL0_SHT0_2 | ADC14_CTL0_SHP | ADC14_CTL0_ON | ADC14_CTL0_CONSEQ_3;
//        //        ADC14->CTL1 = ADC14_CTL1_RES_2;         // Use sampling timer, 12-bit conversion results
//
//        ADC14->MCTL[0] |= ADC14_MCTLN_INCH_13;   // A1 ADC input select; Vref=AVCC
//        ADC14->IER0 |= ADC14_IER0_IE0;          // Enable ADC conv complete interrupt
//        ADC14->CTL0 |= ADC14_CTL0_ENC | ADC14_CTL0_SC;
    }

    uint16_t result = 0;

    if(result <= 2300){
        return 0;
    } else
        return result;
}
