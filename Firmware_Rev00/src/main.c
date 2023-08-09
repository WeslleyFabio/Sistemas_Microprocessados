
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
#include "lorawandefines.h"
#include "i2c-lcd.h"


TIM_HandleTypeDef htimx;
uint8_t buffer_Lora[55];

//void setPWM(TIM_HandleTypeDef timer, uint32_t channel,uint32_t pulse, uint16_t period);
//static void MX_TIM_Init(void);

//static uint8_t payload[] = {"HelloWorld"};

ulong LoRaDelayHelper; 						//Variable to help with the non-blocking delay
uint16_t delayTimeLoRaTxRx = 7000; 			//7s to make the LoRa procedure, Tx and Rx windows, without any problems, so far.

ulong CheckerDelayHelper; 					//Variable to help with the non-blocking delay
uint32_t CheckerTime = 60000 * 0.5; 		//30 seconds

ulong LcdRefreshDelayHelper; 					//Variable to help with the non-blocking delay
uint32_t DelayLcdRefresh = 1000; 			//10 seconds to start a new measure of all 3 ADCs and check the variability.

void LcdStart();
void RefreshPwmLamp(int leitura_ldr);
void LcdStandBy();

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
	adc_config_INIT();
	LORAWAN_init(DEFAULT_REGION);
	LcdStart();

	CheckerDelayHelper	  = CheckerTime	 + HAL_GetTick();
	LcdRefreshDelayHelper = DelayLcdRefresh	 + HAL_GetTick();
	int leitura_ldr = 0;
	int leitura_corrente = 0;
	char lampada[40] = "Stand By...";
	char str_leitura_ldr[20];  		// Espaço para armazenar a string resultante (LDR)
	char str_leitura_corrente[20];  // Espaço para armazenar a string resultante (Corrente)
	int flag_lampada = 0;
	int flag_msg_lorawan = 0;

	LcdStandBy();

	lcd_put_cur(0,0);
	HAL_Delay(1);
	lcd_send_string("LDR:");
	lcd_put_cur(1,0);
	HAL_Delay(1);
	lcd_send_string("CUR:");
	HAL_Delay(1);
	lcd_put_cur(0,9);
	HAL_Delay(1);
	lcd_send_string("LP:");
	HAL_Delay(1);
	lcd_put_cur(1,9);
	HAL_Delay(1);
	lcd_send_string("LR:");
	HAL_Delay(1);

	while (1){
		LORAWAN_tick();
		RefreshPwmLamp(leitura_ldr);

		leitura_ldr = 3300 - (pin_voltage_measure(PB1_CHANNEL));
		leitura_corrente = pin_voltage_measure(PB2_CHANNEL) * 0.59;

		RefreshPwmLamp(leitura_ldr);

		itoa(leitura_ldr, str_leitura_ldr, 10);

		if(leitura_corrente < 20) itoa(0, str_leitura_corrente, 10);
		else itoa(leitura_corrente, str_leitura_corrente, 10);

		RefreshPwmLamp(leitura_ldr);

		if(leitura_ldr > 1200 && leitura_corrente > 20){
			RefreshPwmLamp(leitura_ldr);
			strcpy(lampada, "Lampada Presente.");
			flag_lampada = 1;
		}
		else if(leitura_ldr < 1200){
			RefreshPwmLamp(leitura_ldr);
			strcpy(lampada, "Stand By...");
			flag_lampada = 0;
		}
		else{
			RefreshPwmLamp(leitura_ldr);
			strcpy(lampada, "Lampada Ausente!");
			flag_lampada = 2;
		}

		RefreshPwmLamp(leitura_ldr);

		//Atualização da tela LCD
		if (HAL_GetTick() >= LcdRefreshDelayHelper)
		{
			LcdRefreshDelayHelper = DelayLcdRefresh + HAL_GetTick();
			LORAWAN_tick();
			RefreshPwmLamp(leitura_ldr);

			if(flag_msg_lorawan == 3){
				lcd_put_cur(1,12);
				lcd_send_string("    ");
				flag_msg_lorawan = 0;
			}
			if(flag_msg_lorawan == 2) flag_msg_lorawan++;	//delay mensagem "send"
			if(flag_msg_lorawan == 1) flag_msg_lorawan++;	//delay mensagem "send"

			RefreshPwmLamp(leitura_ldr);
			lcd_put_cur(0,4);
			lcd_send_string("    ");
			RefreshPwmLamp(leitura_ldr);
			lcd_put_cur(0,4);
			lcd_send_string(str_leitura_ldr);
			RefreshPwmLamp(leitura_ldr);
			lcd_put_cur(1,4);
			lcd_send_string("    ");
			RefreshPwmLamp(leitura_ldr);
			lcd_put_cur(1,4);
			lcd_send_string(str_leitura_corrente);
			RefreshPwmLamp(leitura_ldr);
			lcd_put_cur(0,12);
			lcd_send_string("    ");
			RefreshPwmLamp(leitura_ldr);
			lcd_put_cur(0,12);
			if(flag_lampada == 0){
				lcd_send_string("....");
			}
			else if (flag_lampada == 1) {
				lcd_send_string("OK");
			}
			else{
				lcd_send_string("NOK");
			}
			RefreshPwmLamp(leitura_ldr);
		}

		RefreshPwmLamp(leitura_ldr);

		if (HAL_GetTick() >= CheckerDelayHelper)
		{
			CheckerDelayHelper = CheckerTime + HAL_GetTick();
			LORAWAN_tick();
			RefreshPwmLamp(leitura_ldr);

			memset(buffer_Lora,0,sizeof(buffer_Lora));

			LORAWAN_tick();
			sprintf(buffer_Lora,"LDR: %d, Current: %dmA, Lamp: %s",leitura_ldr,leitura_corrente, lampada);
			CallTxEvent(buffer_Lora, sizeof(buffer_Lora));

			lcd_put_cur(1,12);
			lcd_send_string("Send");
			flag_msg_lorawan = 1;
			RefreshPwmLamp(leitura_ldr);
		}
	}
}
/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */

void LcdStart(){
	HAL_Delay(100);
	lcd_init(&hi2c,0x4E,2,16);
	lcd_clear();
	HAL_Delay(1);
	lcd_put_cur(0,4); //linha | coluna
	HAL_Delay(1);
	lcd_send_string("SISTEMAS");
	lcd_put_cur(1,0);
	HAL_Delay(1);
	lcd_send_string("MICROPROCESSADOS");
	HAL_Delay(1500);
	lcd_clear();
}

void LcdStandBy(){
	HAL_Delay(1);
	lcd_put_cur(0,0);
	HAL_Delay(1);
	lcd_send_string("Iniciando");

	for(int i = 0;i<=5;i++){
		HAL_Delay(800);
		lcd_put_cur(0,9+i);
		lcd_send_string(".");
	}
	HAL_Delay(1);
	lcd_clear();
}

void RefreshPwmLamp(int leitura_ldr){
	LORAWAN_tick();
	if (leitura_ldr > 3000){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, SET);
	}
	else if(leitura_ldr > 1200){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, SET);
		HAL_Delay(((leitura_ldr * 100) / 3300)/10);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, RESET);
		HAL_Delay((100 - ((leitura_ldr * 100) / 3300))/10);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, SET); //TESTE
	}
	else{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 0);
	}
}

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
