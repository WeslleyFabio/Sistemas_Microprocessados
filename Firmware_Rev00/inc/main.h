/**
  ******************************************************************************
  * @file    UART/UART_Printf/Inc/UART_Printf_main.h
  * @author  RF Application Team
  * @brief   Header for UART_Printf_main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics. 
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the 
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rf_driver_hal.h"
#include "rf_driver_ll_rtc.h"
#include "utils.h"
#include "adcSetup.h"
#include "log_system.h"

/* Private includes ----------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>

#include "rf_driver_hal_dma.h"
#include "rf_driver_hal_i2c.h"
#include "rf_driver_hal_spi.h"
#include "rf_driver_hal_uart.h"
#include "rf_driver_hal_gpio.h"
#include "rf_driver_hal_tim.h"
#include "rf_driver_hal_tim_ex.h"
#include "rf_driver_ll_gpio.h"
#include "rf_driver_hal_adc.h"
//#include "stsafea_types.h"

#define PB1_CHANNEL			(ADC_CH_VINP1_TO_SINGLE_POSITIVE_INPUT) //for PB1
#define PB2_CHANNEL			(ADC_CH_VINM0_TO_SINGLE_NEGATIVE_INPUT) //for PB2
#define PB3_CHANNEL			(ADC_CH_VINP0_TO_SINGLE_POSITIVE_INPUT) //for PB3
#define PA15_CHANNEL		(ADC_CH_VINP2_TO_SINGLE_POSITIVE_INPUT) //for PA15

#define FLASH_USER_START_ADDR   (FLASH_END_ADDR - FLASH_PAGE_SIZE - 0xF)        /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     (FLASH_END_ADDR - 0xF)                          /* End @ of user Flash area */

#define TIMx                            TIM1
#define EnableClock_TIMx()              __HAL_RCC_TIM1_CLK_ENABLE()
#define DisableClock_TIMx()             __HAL_RCC_TIM1_CLK_DISABLE()

  /**TIM1 GPIO Configuration
  PA4 / AF4   ------> TIM1_CH1
  */
#define TIMx_CH1_PIN                    GPIO_PIN_4
#define TIMx_CH1_AF                     GPIO_AF4_TIM1
#define TIMx_CH1_PORT                   GPIOA

  /**TIM1 GPIO Configuration
  PA5 / AF4   ------> TIM1_CH2
  */
#define TIMx_CH2_PIN                    GPIO_PIN_5
#define TIMx_CH2_AF                     GPIO_AF4_TIM1
#define TIMx_CH2_PORT                   GPIOA

  /**TIM1 GPIO Configuration
  PB2/AF3     ------> TIM1_CH3
  */
#define TIMx_CH3_PIN                    GPIO_PIN_2
#define TIMx_CH3_AF                     GPIO_AF3_TIM1
#define TIMx_CH3_PORT                   GPIOB

  /**TIM1 GPIO Configuration
  PA1/AF4     ------> TIM1_CH4
  */
#define TIMx_CH4_PIN                    GPIO_PIN_1
#define TIMx_CH4_AF                     GPIO_AF4_TIM1
#define TIMx_CH4_PORT                   GPIOA

  /**TIM1 GPIO Configuration
  PB14/AF4    ------> TIM1_CH5
  */
#define TIMx_CH5_PIN                    GPIO_PIN_14
#define TIMx_CH5_AF                     GPIO_AF4_TIM1
#define TIMx_CH5_PORT                   GPIOB

  /**TIM1 GPIO Configuration
  PA11/AF4    ------> TIM1_CH6
  */
#define TIMx_CH6_PIN                    GPIO_PIN_11
#define TIMx_CH6_AF                     GPIO_AF4_TIM1
#define TIMx_CH6_PORT                   GPIOA


#define TIMx_BKIN_PIN                                   LL_GPIO_PIN_14
#define TIMx_BKIN_AF                                    LL_GPIO_AF_4
#define TIMx_BKIN_PORT                                  GPIOA

/* Compute the prescaler value to have TIM1 counter clock equal to 1000000 Hz */
#define PRESCALER_VALUE     (uint32_t)((HAL_TIM_GetPeriphClock(TIMx) / 1000000) - 1)

/* -----------------------------------------------------------------------
TIM1 Configuration: generate 6 PWM signals with 6 different duty cycles.

    In this example TIM1 input clock (TIM1CLK) is set to APB0 clock (PCLK1),
    since APB0 prescaler is equal to 1.
      TIM1CLK = PCLK1
        => TIM1CLK = HCLK = 64 MHz

    To get TIM1 counter clock at 1 MHz, the prescaler is computed as follows:
       Prescaler = (TIM1CLK / TIM1 counter clock) - 1
       Prescaler = (64 MHz /1 MHz) - 1

    To get TIM1 output clock at 24 KHz, the period (ARR)) is computed as follows:
       ARR = (TIM1 counter clock / TIM1 output clock) - 1
           = 40

    TIM1 Channel1 duty cycle = (TIM1_CCR1/ TIM1_ARR + 1)* 100 = 50%
    TIM1 Channel2 duty cycle = (TIM1_CCR2/ TIM1_ARR + 1)* 100 = 37.5%
    TIM1 Channel3 duty cycle = (TIM1_CCR3/ TIM1_ARR + 1)* 100 = 25%
    TIM1 Channel4 duty cycle = (TIM1_CCR4/ TIM1_ARR + 1)* 100 = 12.5%
    TIM1 Channel5 duty cycle = (TIM1_CCR3/ TIM1_ARR + 1)* 100 = 75%
    TIM1 Channel6 duty cycle = (TIM1_CCR4/ TIM1_ARR + 1)* 100 = 62.5%

  ----------------------------------------------------------------------- */

/* Initialize TIMx peripheral as follows:
   + Prescaler = (HAL_TIM_GetPeriphClock(htimx.Instance) / 1000000) - 1
   + Period = (41 - 1)
   + ClockDivision = 0
   + Counter direction = Up
*/
#define  PERIOD_VALUE       (uint32_t)(41 - 1)  /* Period Value  */
#define  PULSE1_VALUE       (uint32_t)(PERIOD_VALUE*0.50)        /* Capture Compare 1 Value  */
#define  PULSE2_VALUE       (uint32_t)(PERIOD_VALUE*0.375)       /* Capture Compare 2 Value  */
#define  PULSE3_VALUE       (uint32_t)(PERIOD_VALUE*0.25)        /* Capture Compare 3 Value  */
#define  PULSE4_VALUE       (uint32_t)(PERIOD_VALUE*0.125)       /* Capture Compare 4 Value  */
#define  PULSE5_VALUE       (uint32_t)(PERIOD_VALUE*0.75)        /* Capture Compare 3 Value  */
#define  PULSE6_VALUE       (uint32_t)(PERIOD_VALUE*0.625)       /* Capture Compare 4 Value  */


//extern StSafeA_Handle_t stsafea_handle;
/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/


/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
void MX_GPIO_LP_Init(void);
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private defines -----------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
