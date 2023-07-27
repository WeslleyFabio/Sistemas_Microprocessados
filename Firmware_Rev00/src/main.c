
//DEBUG CONFIG FILE:
#ifdef DEBUG
#include "debug_configs.h"
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "uart.h"
#include "spi.h"
#include "rtc.h"
#include "crc.h"
#include "i2c.h"
#include "sx126x.h"
#include "sx126x_board.h"
#include "radio.h"
#include "peripheral_init.h"
#include "lorawan_setup.h"
#include "lora-test.h"
#include "LoRaMac.h"
#include "hal_wrappers.h"
//#include "ht_crypto.h"
#include "stsafea_core.h"

//RNG_HandleTypeDef hrng;
TIM_HandleTypeDef htimx;
uint8_t buffer_Lora[55];

#include "lorawandefines.h"

//void setPWM(TIM_HandleTypeDef timer, uint32_t channel,uint32_t pulse, uint16_t period);
//static void MX_TIM_Init(void);

//static uint8_t payload[] = {"HelloWorld"};

ulong LoRaDelayHelper; 						//Variable to help with the non-blocking delay
uint16_t delayTimeLoRaTxRx = 7000; 			//7s to make the LoRa procedure, Tx and Rx windows, without any problems, so far.

ulong CheckerDelayHelper; 					//Variable to help with the non-blocking delay
uint32_t CheckerTime = 60000 * 0.5; 		//30 seconds to start a new measure of all 3 ADCs and check the variability.

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	uint8_t status_code = 0;

	/* System initialization function */
	if (SystemInit(SYSCLK_64M, BLE_SYSCLK_NONE) != SUCCESS) {
		/* Error during system clock configuration take appropriate action */
		while(1);
	}
	HAL_Init();
	IRQHandler_Config();
	MX_GPIO_Init();
	MX_USART1_UART_Init();
	MX_SPI1_Init();
	MX_I2C2_Init();
	MX_CRC_Init();
	MX_RTC_Init();
	MX_GPIO_LP_Init();
//	MX_RNG_Init(&hrng);
//	MX_TIM_Init();
	adc_config_INIT();

#ifdef HT_CRYPTO

	if(keys_provisioned()){
		status_code = ht_crypto_init();
		if(status_code){
			printf("STSAFE-A1xx NOT initialized. \n");
		while(1){}
		}
	}else{
		printf("LoRaWAN keys are NOT set, please flash&run provisioner firmware to set the keys\n");
		while(1);
	}
#endif

	CheckerDelayHelper	 = CheckerTime	 + HAL_GetTick();

	LORAWAN_init(DEFAULT_REGION);
	int leitura_LDR = 0;
	int leitura_CORRENTE = 0;
	char lampada[40] = "Stand By...";
	while (1){
		LORAWAN_tick();

		leitura_LDR = 3300 - (pin_voltage_measure(PB1_CHANNEL));
		leitura_CORRENTE = pin_voltage_measure(PB2_CHANNEL) * 0.59;
		printf("LDR: %d\n",leitura_LDR);
		printf("Amp: %d\n",leitura_CORRENTE);

		if (leitura_LDR > 3000){
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, SET);
		}
		else if(leitura_LDR > 1200){
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, SET);
			HAL_Delay(((leitura_LDR * 100) / 3300)/10);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, RESET);
			HAL_Delay((100 - ((leitura_LDR * 100) / 3300))/10);
		}
		else{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 0);
		}

		if (HAL_GetTick() >= CheckerDelayHelper)
		{
			CheckerDelayHelper = CheckerTime + HAL_GetTick();
			LORAWAN_tick();

			memset(buffer_Lora,0,sizeof(buffer_Lora));
			if(leitura_LDR > 1200 && leitura_CORRENTE > 20){
				strcpy(lampada, "Lampada Presente.");
			}
			else if(leitura_LDR < 1200){
				strcpy(lampada, "Stand By...");
			}
			else{
				strcpy(lampada, "Lampada Ausente!");
			}
			LORAWAN_tick();
			sprintf(buffer_Lora,"LDR: %d, Current: %dmA, Lamp: %s",leitura_LDR,leitura_CORRENTE, lampada);

			CallTxEvent(buffer_Lora, sizeof(buffer_Lora));
			strcpy(lampada, "Stand By...");

			LoRaDelayHelper = delayTimeLoRaTxRx + HAL_GetTick();

			while (HAL_GetTick() <= LoRaDelayHelper){
				LORAWAN_tick();

				leitura_LDR = 3300 - (pin_voltage_measure(PB1_CHANNEL));
				leitura_CORRENTE = pin_voltage_measure(PB2_CHANNEL) * 0.59;

				printf("LDR: %d\n",leitura_LDR);
				printf("Amp: %d\n",leitura_CORRENTE);

				if (leitura_LDR > 3000){
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, SET);
				}
				else if(leitura_LDR > 1200){
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, SET);
					HAL_Delay(((leitura_LDR * 100) / 3300)/10);
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, RESET);
					HAL_Delay((100 - ((leitura_LDR * 100) / 3300))/10);
				}
				else{
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 0);
				}
			}
		}
	}
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */

//void setPWM(TIM_HandleTypeDef timer, uint32_t channel,uint32_t pulse, uint16_t period)
//{
//	HAL_TIM_PWM_Stop(&timer, channel); // stop generation of pwm
//	TIM_OC_InitTypeDef sConfigOC;
//
//	timer.Init.Period = period; // set the period duration
//	HAL_TIM_PWM_Init(&timer);   // reinititialise with new period value
//
//	sConfigOC.OCMode = TIM_OCMODE_PWM1;
//	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
//	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
//	sConfigOC.OCIdleState = TIM_OCIDLESTATE_SET;
//	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
//	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
//	sConfigOC.Pulse = pulse;//PULSE1_VALUE;
//
//	HAL_TIM_PWM_ConfigChannel(&timer, &sConfigOC, channel);
//	HAL_TIM_PWM_Start(&timer, channel); // start pwm generation
//}

//static void MX_TIM_Init(void)
//{
//  TIM_OC_InitTypeDef sConfigOC = {0};
//
//  htimx.Instance = TIMx;
//  htimx.Init.Prescaler = PRESCALER_VALUE;
//  htimx.Init.CounterMode = TIM_COUNTERMODE_UP;
//  htimx.Init.Period = PERIOD_VALUE;
//  htimx.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
//  htimx.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
//  if (HAL_TIM_PWM_Init(&htimx) != HAL_OK)
//  {
//    Error_Handler();
//  }
//
//  sConfigOC.OCMode = TIM_OCMODE_PWM1;
//  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
//  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
//  sConfigOC.OCIdleState = TIM_OCIDLESTATE_SET;
//  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
//  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
//  sConfigOC.Pulse = 0;//PULSE1_VALUE;
//  if (HAL_TIM_PWM_ConfigChannel(&htimx, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
//  {
//    Error_Handler();
//  }
//
//  TIM_BreakDeadTimeConfigTypeDef TIM_BDTRInitStruct = {0};
//
//  /* Configure the Break and Dead Time feature of the TIMx */
//  TIM_BDTRInitStruct.OffStateRunMode = TIM_OSSR_ENABLE;
//  TIM_BDTRInitStruct.OffStateIDLEMode = TIM_OSSI_ENABLE;
//  TIM_BDTRInitStruct.LockLevel = TIM_LOCKLEVEL_1;
//  TIM_BDTRInitStruct.DeadTime = __HAL_TIM_CALC_DEADTIME(HAL_TIM_GetPeriphClock(TIMx), 0, 100);
//  TIM_BDTRInitStruct.BreakState = TIM_BREAK_ENABLE;
//  TIM_BDTRInitStruct.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
//  TIM_BDTRInitStruct.BreakFilter = TIM_BREAK_FILTER_FDIV1;
//#ifdef TIM_BREAK2_DISABLE
//  TIM_BDTRInitStruct.Break2State = TIM_BREAK2_DISABLE;
//#endif /* TIM_BREAK2_DISABLE */
//#ifdef TIM_BREAK2POLARITY_HIGH
//  TIM_BDTRInitStruct.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
//#endif /* TIM_BREAK2POLARITY_HIGH */
//  TIM_BDTRInitStruct.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE;
//  HAL_TIMEx_ConfigBreakDeadTime(&htimx, &TIM_BDTRInitStruct);
//
//
//  HAL_TIM_MspPostInit(&htimx);
//}

void Error_Handler(void)
{
	/* User can add his own implementation to report the HAL error return state */
	printf("Error_Handler\n");
	while(1);
}



#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{ 
	/* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	F
	/* Infinite loop */
	while (1)
	{
	}
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
