/**
 ******************************************************************************
 * @file    wrap_unwrap.c
 * @author  SMD application team
 * @version V3.1.1
 * @brief   Wrap unwrap use case using STSAFE-A and MbedTLS cryptographic library.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
 *
 * Licensed under ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>

//#include "stm32l4xx_nucleo.h"
#ifdef HAL_UART_MODULE_ENABLED
#include <stdio.h>
#endif /* HAL_UART_MODULE_ENABLED */

#include "main.h"
#include "stsafea_interface_conf.h"
/*#ifdef MCU_PLATFORM_INCLUDE
#include MCU_PLATFORM_INCLUDE
#endif */

#include "stsafea_core.h"


#define STS_CHK(ret, f)                     if ((ret) == 0) { (ret) = (f); }
#define ENVELOPE_SIZE                       (8*60)   /* non-zero multiple of 8 bytes; max 480(=8*60) */
#define WRAP_RESPONSE_SIZE                  (ENVELOPE_SIZE + 8) /* Local Envelope response data is 8-bytes longer than the working key (see User Manual). */

#define GET_TICK()                          HAL_GetTick()

#ifdef HAL_UART_MODULE_ENABLED
static uint8_t idx = 0;
#endif /* HAL_UART_MODULE_ENABLED */

int32_t wrap_unwrap(StSafeA_Handle_t* handle, uint8_t slot);
static uint8_t GenerateUnsignedChallenge(StSafeA_Handle_t* handle, uint16_t size, uint8_t* random);


/************************ Generate Unsigned Bytes Array ************************/
static uint8_t GenerateUnsignedChallenge(StSafeA_Handle_t* handle, uint16_t size, uint8_t* random)
{
	STSAFEA_UNUSED_PTR(handle);

	if (random == NULL)
	{
		return (1);
	}

	uint16_t i;

	srand(GET_TICK());

	for (i = 0U; i < size; i++)
	{
		random[i] = (uint8_t)((uint16_t)rand() % 256U);
	}

	random[0] &= 0x7FU;

	return (0);
}


/*************************** Wrap Unwrap Envelope ************************/
int32_t wrap_unwrap(StSafeA_Handle_t* handle, uint8_t slot)
{
	int32_t StatusCode = 0;
	uint8_t Random[ENVELOPE_SIZE];

#ifdef HAL_UART_MODULE_ENABLED
	idx = 0;
	printf("\n\r\n\rWrap/unwrap local envelope demonstration:");
#endif /* HAL_UART_MODULE_ENABLED */

	/* Declare, define and allocate memory for Wrap Local Envelope */
#if (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
	StSafeA_LVBuffer_t LocalEnvelope;
#else
	StSafeA_LVBuffer_t LocalEnvelope;
	uint8_t data_LocalEnvelope [WRAP_RESPONSE_SIZE] = {0};
	LocalEnvelope.Length = WRAP_RESPONSE_SIZE;
	LocalEnvelope.Data = data_LocalEnvelope;
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */

	/* Generate random */
#ifdef HAL_UART_MODULE_ENABLED
	printf("\n\r %d. Generate local envelope (random data) of %d bytes", ++idx, ENVELOPE_SIZE);
	printf("\n\r    => Use StSafeA_WrapLocalEnvelope API");
#endif /* HAL_UART_MODULE_ENABLED */

	//STS_CHK(StatusCode, (int32_t)GenerateUnsignedChallenge(handle, ENVELOPE_SIZE, &Random[0]));
	memset(Random, 0xAA, ENVELOPE_SIZE);

#ifdef HAL_UART_MODULE_ENABLED
	printf("\n\r %d. Wrap local envelope", ++idx);
	printf("\n\r    => Use StSafeA_WrapLocalEnvelope API");
#endif /* HAL_UART_MODULE_ENABLED */

	/* Wrap local envelope using key slot as argument */
	STS_CHK(StatusCode, (int32_t)StSafeA_WrapLocalEnvelope(handle, slot, &Random[0], ENVELOPE_SIZE, &LocalEnvelope,
			STSAFEA_MAC_HOST_CMAC, STSAFEA_ENCRYPTION_COMMAND));

	printf("\nWrap buffer: \n");
	for(uint8_t i = 0; i < 30; i++)
		printf("%02X ", data_LocalEnvelope[i]);
	printf("\n");

	if (StatusCode == 0)
	{
#if (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
		/* Store Wrapped Local Envelope */
		uint8_t data_WrappedEnvelope[WRAP_RESPONSE_SIZE];
		(void)memcpy(data_WrappedEnvelope, LocalEnvelope.Data, LocalEnvelope.Length);
		LocalEnvelope.Data = data_WrappedEnvelope;
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */

		/* Declare, define and allocate memory for Unwrap Local Envelope */
#if (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
		StSafeA_LVBuffer_t UnwrappedEnvelope;
#else
		StSafeA_LVBuffer_t UnwrappedEnvelope;
		uint8_t data_UnwrappedEnvelope [ENVELOPE_SIZE] = {0};
		UnwrappedEnvelope.Length = ENVELOPE_SIZE;
		UnwrappedEnvelope.Data = data_UnwrappedEnvelope;
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */

#ifdef HAL_UART_MODULE_ENABLED
		printf("\n\r %d. Unrap local envelope", ++idx);
		printf("\n\r    => Use StSafeA_UnwrapLocalEnvelope API");
#endif /* HAL_UART_MODULE_ENABLED */

		/* Unwrap local envelope using key in slot as argument */
		STS_CHK(StatusCode, (int32_t)StSafeA_UnwrapLocalEnvelope(handle, slot, LocalEnvelope.Data,
				LocalEnvelope.Length, &UnwrappedEnvelope,
				STSAFEA_MAC_HOST_CMAC, STSAFEA_ENCRYPTION_RESPONSE));

		printf("\nUnwrap buffer: \n");
		for(uint8_t i = 0; i < 30; i++)
			printf("%02X ", data_UnwrappedEnvelope[i]);
		printf("\n");

#ifdef HAL_UART_MODULE_ENABLED
		printf("\n\r %d. Verify unwrap local envelope is identical to initial generated envelope", ++idx);
#endif /* HAL_UART_MODULE_ENABLED */
		if ((StatusCode == 0) && (memcmp(&Random[0], &UnwrappedEnvelope.Data[0], ENVELOPE_SIZE) != 0))
		{
			StatusCode = (int32_t)~0U;
		}
	}

#ifdef HAL_UART_MODULE_ENABLED
	printf("\n\r %d. Local envelope Local envelope demonstration result (0 means success): %d", ++idx, (int)StatusCode);

#endif /* HAL_UART_MODULE_ENABLED */

	printf("\n\n\nDEUGG\n");

	return (StatusCode);
}

void HT_STSAFE_EchoCmd(StSafeA_Handle_t* handle) {
	StSafeA_LVBuffer_t buffer;
	uint8_t ptr[] = {"helloworld"};
	uint8_t output[11];

	buffer.Data = output;
	memset(output, 0, sizeof(output));

	StSafeA_Echo(handle, ptr, strlen((char *)ptr), &buffer, STSAFEA_MAC_NONE);

	printf("Echo: %s\n", (char *)output);
}


void HT_STSAFE_Write(StSafeA_Handle_t* handle) {
	uint8_t input[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
	uint8_t output[7];
	uint8_t zone = 1;
	uint8_t offset = 0;
	StSafeA_LVBuffer_t in_Data;
	StSafeA_LVBuffer_t sts_read;

	in_Data.Data = input;
	in_Data.Length = sizeof(input);

	memset(input, 0xAB, sizeof(input));
	StSafeA_Update(handle, 0, 0, 0, 0, zone, offset, &in_Data, STSAFEA_MAC_HOST_CMAC);

	memset(output, 0, sizeof(output));
	sts_read.Data = output;
	sts_read.Length = 7;

	StSafeA_Read(handle, /* handle */
			0 /*in_change_ac_indicator*/,
			0 /*in_new_read_ac_right*/,
			STSAFEA_AC_ALWAYS /*in_new_read_ac*/,
			zone,
			offset /*in_offset*/,
			7,
			7/*in_length*/,
			&sts_read /*out_read*/,
			STSAFEA_MAC_HOST_CMAC);

	printf("Output:\n");
	for(int i=0;i<7;i++)
		printf("%02X ", output[i]);
	printf("\n");
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
