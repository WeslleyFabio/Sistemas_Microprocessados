/**
 ******************************************************************************
 * @file    UART/UART_Printf/Src/bluenrg_lp_hal_msp.c
 * @author  RF Application Team
 * @brief   HAL MSP module.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "peripheral_init.h"
#include "radio.h"
#include "rf_driver_ll_pwr.h"
#include "gpio.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/


/* External functions --------------------------------------------------------*/

void MX_GPIO_LP_Init(void) {
	LL_PWR_EnablePDA(LL_PWR_PUPD_IO0);
	LL_PWR_EnablePDA(LL_PWR_PUPD_IO1);
	LL_PWR_EnablePDA(LL_PWR_PUPD_IO2);
	LL_PWR_EnablePDA(LL_PWR_PUPD_IO3);
	LL_PWR_EnablePDA(LL_PWR_PUPD_IO4);
	LL_PWR_EnablePDA(LL_PWR_PUPD_IO5);
	LL_PWR_EnablePDA(LL_PWR_PUPD_IO6);
	LL_PWR_EnablePDA(LL_PWR_PUPD_IO7);
	LL_PWR_EnablePUA(LL_PWR_PUPD_IO8);
	LL_PWR_EnablePUA(LL_PWR_PUPD_IO9);
	LL_PWR_EnablePDA(LL_PWR_PUPD_IO10);
	LL_PWR_EnablePUA(LL_PWR_PUPD_IO11);
	LL_PWR_EnablePDA(LL_PWR_PUPD_IO12);
	LL_PWR_EnablePDA(LL_PWR_PUPD_IO13);
	LL_PWR_EnablePDA(LL_PWR_PUPD_IO14);
	LL_PWR_EnablePDA(LL_PWR_PUPD_IO15);

	LL_PWR_EnablePUB(LL_PWR_PUPD_IO0);
	LL_PWR_EnablePDB(LL_PWR_PUPD_IO1);
	LL_PWR_EnablePDB(LL_PWR_PUPD_IO2);
	LL_PWR_EnablePDB(LL_PWR_PUPD_IO3);
	LL_PWR_EnablePDB(LL_PWR_PUPD_IO4);
	LL_PWR_EnablePDB(LL_PWR_PUPD_IO5);
	LL_PWR_EnablePUB(LL_PWR_PUPD_IO6); //it must be changed after the next version of HTLRBL32L (STSAFE)
	LL_PWR_EnablePUB(LL_PWR_PUPD_IO7);
	LL_PWR_EnablePUB(LL_PWR_PUPD_IO8);
	LL_PWR_EnablePUB(LL_PWR_PUPD_IO9);
	LL_PWR_EnablePDB(LL_PWR_PUPD_IO10);
	LL_PWR_EnablePDB(LL_PWR_PUPD_IO11);
	LL_PWR_EnablePDB(LL_PWR_PUPD_IO12);
	LL_PWR_EnablePDB(LL_PWR_PUPD_IO13);
	LL_PWR_EnablePUB(LL_PWR_PUPD_IO14);
	LL_PWR_EnablePUB(LL_PWR_PUPD_IO15);
}

/**
 * Initializes the Global MSP.
 */
void HAL_MspInit(void)
{
	/* System interrupt init*/
	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn,1);
	__HAL_RCC_SYSCFG_CLK_ENABLE();

}

/**
 * @brief UART MSP Initialization
 * This function configures the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	if(huart->Instance==USART1)
	{

		/* Peripheral clock enable */
		__HAL_RCC_USART_CLK_ENABLE();


		/**USART1 GPIO Configuration
    PA9/AF0     ------> USART1_TX
    PA8/AF0     ------> USART1_RX 
		 */
		GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_8;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF0_USART1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


	}

}

/**
 * @brief USART MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param husart: USART handle pointer
 * @retval None
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
	if(huart->Instance==USART1)
	{
		__HAL_RCC_USART_FORCE_RESET();
		__HAL_RCC_USART_RELEASE_RESET();
		/* Peripheral clock disable */
		__HAL_RCC_USART_CLK_DISABLE();

		/**USART1 GPIO Configuration
		PA9/AF0     ------> USART1_TX
    PA8/AF0     ------> USART1_RX 
		 */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_8|GPIO_PIN_9);

	}

}
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	if(hspi->Instance==SPI1)
	{
		/* Peripheral clock enable */
		__HAL_RCC_SPI1_CLK_ENABLE();

		//MISO
		GPIO_InitStruct.Pin = GPIO_PIN_14;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF2_SPI1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		//SCLK
		GPIO_InitStruct.Pin = GPIO_PIN_13;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLDOWN; //PULLDOWN
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF2_SPI1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		//MOSI SIP
		GPIO_InitStruct.Pin = GPIO_PIN_14;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLDOWN; //PULLDOWN
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	}


	if(hspi->Instance==SPI2)
	{
		/* Peripheral clock enable */
		__HAL_RCC_SPI2_CLK_ENABLE();

		//MISO
		GPIO_InitStruct.Pin = GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF1_SPI2; //THIS IS VERY IMPORTANT, SEEK IN DATASHEET FOR ALTERNATE FUNCTIONS.
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		//SCLK
		GPIO_InitStruct.Pin = GPIO_PIN_5;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF1_SPI2;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		//MOSI - SOLUÇÃO HE
		GPIO_InitStruct.Pin = GPIO_PIN_12;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF3_SPI2;  //verificado no datasheet
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//		//MOSI
//		GPIO_InitStruct.Pin = GPIO_PIN_5;
//		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//		GPIO_InitStruct.Pull = GPIO_NOPULL;
//		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//		GPIO_InitStruct.Alternate = GPIO_AF1_SPI2;
//		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		//CS
		GPIO_InitStruct.Pin = GPIO_PIN_4;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF1_SPI2;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	}

}

/**
 * @brief SPI MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hspi: SPI handle pointer
 * @retval None
 */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
	if(hspi->Instance==SPI1)
	{
		/* Reset peripherals */
		__HAL_RCC_SPI1_FORCE_RESET();
		__HAL_RCC_SPI1_RELEASE_RESET();

		/* Peripheral clock disable */
		__HAL_RCC_SPI1_CLK_DISABLE();

		/**SPI1 GPIO Configuration
    PA13     ------> SPI1_SCK
    PA14     ------> SPI1_MISO
    PB14     ------> SPI1_MOSI
		 */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_13|GPIO_PIN_14);
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_14);
	}

	if(hspi->Instance==SPI2)
	{
		/* Reset peripherals */
		__HAL_RCC_SPI1_FORCE_RESET();
		__HAL_RCC_SPI1_RELEASE_RESET();

		/* Peripheral clock disable */
		__HAL_RCC_SPI1_CLK_DISABLE();

		/**SPI1 GPIO Configuration
      PA5     ------> SPI1_SCK
      PA7     ------> SPI1_MISO
      PB5     ------> SPI1_MOSI
		 */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_7);
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5);
	}

}

EXTI_HandleTypeDef HEXTI_InitStructure;

/**
 * @brief IRQ Handler Configuration Function
 * @param None
 * @retval None
 */
void IRQHandler_Config(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;

	EXTI_ConfigTypeDef EXTI_Config_InitStructure;

	/* Enable GPIOC clock */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* Configure PB.4 pin as input floating */
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Pin = GPIO_PIN_4;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	EXTI_Config_InitStructure.Line =    EXTI_LINE_PB4;
	EXTI_Config_InitStructure.Trigger = EXTI_TRIGGER_RISING_EDGE;
	EXTI_Config_InitStructure.Type =    EXTI_TYPE_EDGE;

	HAL_EXTI_SetConfigLine(&HEXTI_InitStructure, &EXTI_Config_InitStructure);
	HAL_EXTI_RegisterCallback(&HEXTI_InitStructure, HAL_EXTI_COMMON_CB_ID, (void(*) (uint32_t))RadioOnDioIrq);
	HAL_EXTI_Cmd(&HEXTI_InitStructure , ENABLE);

	HAL_EXTI_ClearPending(&HEXTI_InitStructure);

	/* Enable and set line 10 Interrupt to the lowest priority */
	HAL_NVIC_SetPriority(GPIOB_IRQn,2);
	HAL_NVIC_EnableIRQ(GPIOB_IRQn);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
void MX_GPIO_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();


	HAL_GPIO_WritePin(GPIOA, RADIO_NSS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, RADIO_NRST_Pin, GPIO_PIN_SET);


	/*Configure GPIO pins : PAPin PAPin PAPin */
	GPIO_InitStruct.Pin = RADIO_NRST_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(RADIO_NRST_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = RADIO_NSS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(RADIO_NSS_Port, &GPIO_InitStruct);


	/*Configure GPIO pin : PtPin */
	GPIO_InitStruct.Pin = RADIO_BUSY_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(RADIO_BUSY_Port, &GPIO_InitStruct);

	HAL_NVIC_EnableIRQ( GPIOB_IRQn);


	GPIO_InitStruct.Pin = RADIO_SWITCH_ENABLE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(RADIO_SWITCH_ENABLE_Port, &GPIO_InitStruct);



	//Configuring the 'dry contacts' as 3v3 digital inputs (3 gpios):
	GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_12|GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);



	GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_4; // 12 and 15 are ADC. 4, 5 and 7 are SPI2. PA10 is boot0.
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*
	GPIO_InitStruct.Pin = GPIO_PIN_0; // 1, 2, 3 are ADC. 6 and 7 are I2C2. 5 is SPI2. 9, 12 and 13 are digital 'dry contacts'. 0 is about to be turned into a RF key.
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	*/

	///////////////////////////////////////////////////

	 //GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	HAL_GPIO_WritePin(GPIOA, RADIO_NSS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, RADIO_NRST_Pin, GPIO_PIN_SET);

	/*Configure GPIO pins : PAPin PAPin PAPin */
	GPIO_InitStruct.Pin = RADIO_NRST_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(RADIO_NRST_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = RADIO_NSS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(RADIO_NSS_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : PtPin */
	GPIO_InitStruct.Pin = RADIO_BUSY_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(RADIO_BUSY_Port, &GPIO_InitStruct);

	HAL_NVIC_EnableIRQ(GPIOB_IRQn);

	GPIO_InitStruct.Pin = RADIO_SWITCH_ENABLE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(RADIO_SWITCH_ENABLE_Port, &GPIO_InitStruct);


}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm)
{
  if(htim_pwm->Instance==TIMx)
  {
    /* Peripheral clock enable */
    EnableClock_TIMx();
  }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if(htim->Instance==TIMx)
  {
	GPIO_InitStruct.Pin = TIMx_CH2_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = TIMx_CH2_AF;
	HAL_GPIO_Init(TIMx_CH2_PORT, &GPIO_InitStruct);

#ifdef TIMx_CH2_PIN
    GPIO_InitStruct.Pin = TIMx_CH2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = TIMx_CH2_AF;
    HAL_GPIO_Init(TIMx_CH2_PORT, &GPIO_InitStruct);
#endif

#ifdef TIMx_CH3_PIN
    GPIO_InitStruct.Pin = TIMx_CH3_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = TIMx_CH3_AF;
    HAL_GPIO_Init(TIMx_CH3_PORT, &GPIO_InitStruct);
#endif

#ifdef TIMx_CH4_PIN
    GPIO_InitStruct.Pin = TIMx_CH4_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = TIMx_CH4_AF;
    HAL_GPIO_Init(TIMx_CH4_PORT, &GPIO_InitStruct);
#endif

#ifdef TIMx_CH5_PIN
    GPIO_InitStruct.Pin = TIMx_CH5_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = TIMx_CH5_AF;
    HAL_GPIO_Init(TIMx_CH5_PORT, &GPIO_InitStruct);
#endif /* TIMx_CH5_PIN */

#ifdef TIMx_CH6_PIN
    GPIO_InitStruct.Pin = TIMx_CH6_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = TIMx_CH6_AF;
    HAL_GPIO_Init(TIMx_CH6_PORT, &GPIO_InitStruct);
#endif /* TIMx_CH6_PIN */

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = TIMx_BKIN_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
    GPIO_InitStruct.Alternate = TIMx_BKIN_AF;
    HAL_GPIO_Init(TIMx_BKIN_PORT, &GPIO_InitStruct);

  }
}


/**
* @brief TIM MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param htim: TIM handle pointer
* @retval None
*/
void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm)
{
  if(htim_pwm->Instance==TIMx)
  {
    /* Peripheral clock disable */
    DisableClock_TIMx();
  }
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
