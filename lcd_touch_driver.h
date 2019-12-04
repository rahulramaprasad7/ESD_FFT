/*
 * lcd_touch_driver.h
 *
 *  Created on: 03-Dec-2019
 *      Author: prayag
 */

#ifndef LCD_TOUCH_DRIVER_H_
#define LCD_TOUCH_DRIVER_H_

#include "msp.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

/* X+ definitions. ---> PIn 24 on LaunchPad ---> P4.0, A13 on MSp432*/

#define TOUCH_X_PLUS_PORT           P4
#define TOUCH_X_PLUS_PIN            BIT0
#define TOUCH_X_PLUS_ADC_CHANNEL    13

/* Y+ definitions. ----> PIn 23 on LaunchPad ---> P6.1, A14 on MSP432*/
#define TOUCH_Y_PLUS_PORT             P6
#define TOUCH_Y_PLUS_PIN              BIT1
#define TOUCH_Y_PLUS_ADC_CHANNEL      14

/* X- definitions. --> Pin 31 on LaunchPad --> P3.7 on MSP432*/
#define TOUCH_X_MINUS_PORT      P3
#define TOUCH_X_MINUS_PIN       BIT7

/* Y- definitions. --> Pin 11 on LaunchPad  --> P3.6 on MSP432*/
#define TOUCH_Y_MINUS_PORT      P3
#define TOUCH_Y_MINUS_PIN       BIT6

/* Threshold for detecting if there is a touch. */
#define TOUCH_THRESHOLD 16000


uint16_t getTouchX(void);
uint16_t getTouchY(void);


#endif /* LCD_TOUCH_DRIVER_H_ */
