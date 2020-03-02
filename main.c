/*
 * @file main.c
 * @brief Real-Time FFT implementation on DSP
 *
 * This source file contains the real-time implementation
 * of FFT using the CMSIS DSP libraray rfft() fucntion
 * along with DMA for transfering data from the ADC to 
 * the DSP.
 *
 * @date 14-Dec-2019
 * @author Prayag Milan Desai, Rahul Ramaprasad
 */
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "HAL_MSP_EXP432P401R_KITRONIX320X240_SSD2119_SPI.h"
#include "kitronix320x240x16_ssd2119_spi.h"
#include "Lucida_Console_8pts.h"
#include "lcd_driver.h"
#include "lcd_touch_driver.h"
#include <stdio.h>
#include <arm_math.h>
#include <arm_const_structs.h>

//Number of points for FFT
#define TEST_LENGTH_SAMPLES 512
#define SAMPLE_LENGTH 512

/* ------------------------------------------------------------------
 * Global variables for FFT Bin Example
 * ------------------------------------------------------------------- */
uint32_t fftSize = SAMPLE_LENGTH;
uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;
volatile arm_status status;

#define NFFT 512


#define SMCLK_FREQUENCY     48000000
#define SAMPLE_FREQUENCY    8000

/* DMA Control Table */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN(MSP_EXP432P401RLP_DMAControlTable, 1024)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=1024
#elif defined(__GNUC__)
__attribute__ ((aligned (1024)))
#elif defined(__CC_ARM)
__align(1024)
#endif
static DMA_ControlTable MSP_EXP432P401RLP_DMAControlTable[32];

/* FFT data/processing buffers*/
float hann[SAMPLE_LENGTH];
int16_t data_array1[SAMPLE_LENGTH];
int16_t data_array2[SAMPLE_LENGTH];
int16_t data_input[SAMPLE_LENGTH * 2];
int16_t data_output[SAMPLE_LENGTH];

volatile int switch_data = 0;

uint32_t color = 0;

/* Timer_A PWM Configuration Parameter */
Timer_A_PWMConfig pwmConfig =
{
 TIMER_A_CLOCKSOURCE_SMCLK,
 TIMER_A_CLOCKSOURCE_DIVIDER_1,
 (SMCLK_FREQUENCY / SAMPLE_FREQUENCY),
 TIMER_A_CAPTURECOMPARE_REGISTER_1,
 TIMER_A_OUTPUTMODE_SET_RESET,
 (SMCLK_FREQUENCY / SAMPLE_FREQUENCY) / 2
};

int main(void)
{
    /* Halting WDT and disabling master interrupts */
    MAP_WDT_A_holdTimer();
    MAP_Interrupt_disableMaster();

    /* Set the core voltage level to VCORE1 */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

    /* Set 2 flash wait states for Flash bank 0 and 1*/
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Initializes Clock System */
    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);


    // Initialize Hann Window
    int n;
    for(n = 0; n < SAMPLE_LENGTH; n++)
    {
        hann[n] = 0.5f - 0.5f * cosf((2 * PI * n) / (SAMPLE_LENGTH - 1));
    }

    /* Configuring Timer_A to have a period of approximately 500ms and
     * an initial duty cycle of 10% of that (3200 ticks)  */
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);

    /* Initializing ADC (MCLK/1/1) */
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, 0);

    ADC14_setSampleHoldTrigger(ADC_TRIGGER_SOURCE1, false);

    /* Configuring GPIOs (4.3 A10) */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN4,
                                               GPIO_TERTIARY_MODULE_FUNCTION);

    /* Configuring ADC Memory */
    ADC14_configureSingleSampleMode(ADC_MEM0, true);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                    ADC_INPUT_A1, false);

    /* Set ADC result format to signed binary */
    ADC14_setResultFormat(ADC_SIGNED_BINARY);

    /* Configuring DMA module */
    DMA_enableModule();
    DMA_setControlBase(MSP_EXP432P401RLP_DMAControlTable);

    DMA_disableChannelAttribute(DMA_CH7_ADC14,
                                UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                UDMA_ATTR_HIGH_PRIORITY |
                                UDMA_ATTR_REQMASK);




    /* Setting Control Indexes. In this case we will set the source of the
     * DMA transfer to ADC14 Memory 0
     *  and the destination to the
     * destination data array. */
    MAP_DMA_setChannelControl(
            UDMA_PRI_SELECT | DMA_CH7_ADC14,
            UDMA_SIZE_16 | UDMA_SRC_INC_NONE |
            UDMA_DST_INC_16 | UDMA_ARB_1);
    MAP_DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH7_ADC14,
                               UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
                               data_array1, SAMPLE_LENGTH);

    MAP_DMA_setChannelControl(
            UDMA_ALT_SELECT | DMA_CH7_ADC14,
            UDMA_SIZE_16 | UDMA_SRC_INC_NONE |
            UDMA_DST_INC_16 | UDMA_ARB_1);
    MAP_DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH7_ADC14,
                               UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
                               data_array2, SAMPLE_LENGTH);

    /* Assigning/Enabling Interrupts */
    MAP_DMA_assignInterrupt(DMA_INT1, 7);
    MAP_Interrupt_enableInterrupt(INT_DMA_INT1);
    MAP_DMA_assignChannel(DMA_CH7_ADC14);
    MAP_DMA_clearInterruptFlag(7);
    MAP_Interrupt_enableMaster();

    /* Now that the DMA is primed and setup, enabling the channels. The ADC14
     * hardware should take over and transfer/receive all bytes */
    MAP_DMA_enableChannel(7);
    MAP_ADC14_enableConversion();

    P1->DIR |= BIT5 | BIT6;
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

    lcdInit();


    SCB->SCR &= ~SCB_SCR_SLEEPONEXIT_Msk;   // Wake up on exit from ISR

    //(320x240 resolution)
    //    selectCS(0);
    drawLineX(20, 300, 220, GREEN);
    drawLineY(0, 220, 20, GREEN);

    //LCD UI
    //X-axis labels
    writeString(20,480 , WHITE, "0");
    writeString(84, 480, WHITE, "Fs/4");
    writeString(148, 480, WHITE, "Fs/2");
    writeString(212, 480, WHITE, "3Fs/4");
    writeString(276, 480, WHITE, "Fs");

    //Y-axis labels
    writeString(0, 460, WHITE, "20");
    writeString(0, 440, WHITE, "40");
    writeString(0, 420, WHITE, "60");
    writeString(0, 400, WHITE, "80");
    writeString(0, 380, WHITE, "100");
    writeString(0, 360, WHITE, "120");
    writeString(0, 340, WHITE, "140");
    writeString(0, 320, WHITE, "160");
    writeString(0, 300, WHITE, "180");
    writeString(0, 280, WHITE, "200");
    writeString(0, 260, WHITE, "220");
    while(1)
    {
        int i = 0;
        MAP_PCM_gotoLPM0();


        /* Computer real FFT using the completed data buffer */
        if(switch_data & 1)
        {
            for(i = 0; i < 512; i++)
            {
                data_array1[i] = (int16_t)(hann[i] * data_array1[i]);
            }
            arm_rfft_instance_q15 instance;
            status = arm_rfft_init_q15(&instance, fftSize, ifftFlag,
                                       doBitReverse);

            arm_rfft_q15(&instance, data_array1, data_input);
        }
        else
        {
            for(i = 0; i < 512; i++)
            {
                data_array2[i] = (int16_t)(hann[i] * data_array2[i]);
            }
            arm_rfft_instance_q15 instance;
            status = arm_rfft_init_q15(&instance, fftSize, ifftFlag,
                                       doBitReverse);

            arm_rfft_q15(&instance, data_array2, data_input);
        }

        /* Calculate magnitude of FFT complex output */
        for(i = 0; i < 1024; i += 2)
        {
            data_output[i / 2]  = (int32_t)(sqrtf((data_input[i] * data_input[i]) + (data_input[i + 1] * data_input[i + 1])));
            if ( data_output[i / 2] > 200)
                data_output[i / 2] /= 100;
        }

        q15_t maxValue;
        uint32_t maxIndex = 0;

        arm_max_q15(data_output, fftSize, &maxValue, &maxIndex);

        //Color mapping based on highest frequency element
        if(maxIndex <= 64)
        {
            color = ((uint32_t)(0xFF * (maxIndex / 64.0f)) << 8) + 0xFF;
        }
        else if(maxIndex <= 128)
        {
            color =
                    (0xFF - (uint32_t)(0xFF * ((maxIndex - 64) / 64.0f))) + 0xFF00;
        }
        else if(maxIndex <= 192)
        {
            color =
                    ((uint32_t)(0xFF * ((maxIndex - 128) / 64.0f)) << 16) + 0xFF00;
        }
        else
        {
            color =
                    ((0xFF -
                            (uint32_t)(0xFF *
                                    ((maxIndex - 192) / 64.0f))) << 8) + 0xFF0000;
        }
        //Clearing previous output on LCD before printing new output
        for(i = 0; i < 320; i++)
        {
            drawLineY(239, 218, i + 21, BLACK);
        }
        //Printing FFT output on the LCD
        for(i = 0; i < 256; i += 2)
        {
            int x = min(200, (int)((data_output[i] + data_output[i + 1])));
            drawLineY(239 - ( 20 + x), x, i + 20 , color);
        }
    }
}

/* Completion interrupt for ADC14 MEM0 */
void DMA_INT1_IRQHandler(void)
{
    /* Switch between primary and alternate bufferes with DMA's PingPong mode */
    if(DMA_getChannelAttribute(7) & UDMA_ATTR_ALTSELECT)
    {
        DMA_setChannelControl(
                UDMA_PRI_SELECT | DMA_CH7_ADC14,
                UDMA_SIZE_16 | UDMA_SRC_INC_NONE |
                UDMA_DST_INC_16 | UDMA_ARB_1);
        DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH7_ADC14,
                               UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
                               data_array1, SAMPLE_LENGTH);
        switch_data = 1;
    }
    else
    {
        DMA_setChannelControl(
                UDMA_ALT_SELECT | DMA_CH7_ADC14,
                UDMA_SIZE_16 | UDMA_SRC_INC_NONE |
                UDMA_DST_INC_16 | UDMA_ARB_1);
        DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH7_ADC14,
                               UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
                               data_array2, SAMPLE_LENGTH);
        switch_data = 0;
    }
}
