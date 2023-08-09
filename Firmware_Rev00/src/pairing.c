/**
 ******************************************************************************
 * @file    pairing.c
 * @author  SMD application team
 * @version V3.1.2
 * @brief   Pairing use case using STSAFE-A.
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

#include "main.h"

#include "stsafea_interface_conf.h"


#include "stsafea_core.h"


/* Used to force default host keys storage through STM32 flash */
/* Default host keys are not stored through STSAFE-Axx */
/* #define _FORCE_DEFAULT_FLASH_ */


#define STS_CHK(ret, f)                     if ((ret) == 0) { (ret) = (f); }

#ifdef _FORCE_DEFAULT_FLASH_
#undef USE_HOST_KEYS_SET_BY_PAIRING_APP
#define USE_HOST_KEYS_SET_BY_PAIRING_APP    0
#endif

#define GET_TICK()                          HAL_GetTick()

#define PAGE_NUMBER                         255

#ifdef HAL_UART_MODULE_ENABLED
static uint8_t idx = 0;
#endif /* HAL_UART_MODULE_ENABLED */

int32_t pairing(StSafeA_Handle_t* handle);
int32_t check_local_envelope_key(StSafeA_Handle_t* handle);
int32_t check_host_keys(StSafeA_Handle_t* handle);

#if USE_HOST_KEYS_SET_BY_PAIRING_APP
/************************ Generate Unsigned Bytes Array ************************/
static uint8_t GenerateUnsignedChallenge(StSafeA_Handle_t* handle, uint8_t size, uint8_t* random)
{
	if (random == NULL)
	{
		return (1);
	}

#ifdef HAL_UART_MODULE_ENABLED
	printf("\n\r %d. Generate a %d bytes random number", ++idx, size);
	printf("\n\r    => Use StSafeA_GenerateRandom API");
#endif /* HAL_UART_MODULE_ENABLED */

	StSafeA_LVBuffer_t TrueRandom;
	TrueRandom.Data = random;
	return ((uint8_t)StSafeA_GenerateRandom(handle, STSAFEA_EPHEMERAL_RND, size, &TrueRandom, STSAFEA_MAC_NONE));
}
#endif

/************************ CheckLocalEnvelopeKey ************************/
int32_t check_local_envelope_key(StSafeA_Handle_t* handle)
{
	int32_t StatusCode = 0;
	StSafeA_LocalEnvelopeKeyTableBuffer_t LocalEnvelopeKeyTable;
	StSafeA_LocalEnvelopeKeyInformationRecordBuffer_t  LocalEnvelopeInfoSlot0, LocalEnvelopeInfoSlot1;

	printf("\nTrying to delete...\n");
	STS_CHK(StatusCode, (int32_t)StSafeA_DeletePassword(handle, STSAFEA_MAC_HOST_CMAC));

#ifdef HAL_UART_MODULE_ENABLED
	printf("\n\r %d. Check local envelope key presence through STSAFE-A1x0", ++idx);
	printf("\n\r        => StSafeA_LocalEnvelopeKeySlotQuery");
#endif /* HAL_UART_MODULE_ENABLED */



	STS_CHK(StatusCode, (int32_t)StSafeA_LocalEnvelopeKeySlotQuery(handle, &LocalEnvelopeKeyTable, &LocalEnvelopeInfoSlot0,
			&LocalEnvelopeInfoSlot1, STSAFEA_MAC_NONE));

	if ((StatusCode == 0 ) && (LocalEnvelopeKeyTable.NumberOfSlots != 0U) && (LocalEnvelopeInfoSlot0.SlotNumber == 0U) && (LocalEnvelopeInfoSlot0.PresenceFlag == 0U))
	{
#ifdef HAL_UART_MODULE_ENABLED
		printf("\n\r %d. Generate local envelope key", ++idx);
		printf("\n\r        => StSafeA_GenerateLocalEnvelopeKey");
#endif /* HAL_UART_MODULE_ENABLED */

		StatusCode = (int32_t)StSafeA_GenerateLocalEnvelopeKey(handle, STSAFEA_KEY_SLOT_0, STSAFEA_KEY_TYPE_AES_128, NULL, 0U, STSAFEA_MAC_NONE);
	}

	return StatusCode;
}


/************************ Check host keys ************************/
int32_t check_host_keys(StSafeA_Handle_t* handle)
{
	int32_t StatusCode = 0;
	uint8_t Host_MAC_Cipher_Key[2U * STSAFEA_HOST_KEY_LENGTH] = {
			0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,    /* Host MAC key */
			0x11,0x11,0x22,0x22,0x33,0x33,0x44,0x44,0x55,0x55,0x66,0x66,0x77,0x77,0x88,0x88     /* Host cipher key */
	};
	uint32_t i;
	uint64_t ptr;
	StSafeA_HostKeySlotBuffer_t HostKeySlot;

	//  if (!(IS_FLASH_PAGE(PAGE_NUMBER)))
	//  {
	//#ifdef HAL_UART_MODULE_ENABLED
	//    printf("\n\r    %d Flash page out of range", idx);
	//#endif /* HAL_UART_MODULE_ENABLED */
	//    return 1;
	//  }

#if USE_HOST_KEYS_SET_BY_PAIRING_APP
	/* Generate both keys */
	STS_CHK(StatusCode, (int32_t)GenerateUnsignedChallenge(handle, 2 * STSAFEA_HOST_KEY_LENGTH, Host_MAC_Cipher_Key));
#endif

#ifdef HAL_UART_MODULE_ENABLED
	printf("\n\r %d. Check host keys presence through STSAFE-A1x0", ++idx);
	printf("\n\r        => StSafeA_HostKeySlotQuery");
#endif /* HAL_UART_MODULE_ENABLED */

	/* Check if host cipher key & host MAC key are populated */
	STS_CHK(StatusCode, (int32_t)StSafeA_HostKeySlotQuery(handle, &HostKeySlot, STSAFEA_MAC_NONE));

#ifdef _FORCE_DEFAULT_FLASH_
	if (StatusCode == 0)
	{
#else
		if ((StatusCode == 0) && (HostKeySlot.HostKeyPresenceFlag == 0U))      // Not populated
		{
#ifdef HAL_UART_MODULE_ENABLED
			printf("\n\r %d. Set host keys through STSAFE-A1x0", ++idx);
			printf("\n\r        => StSafeA_PutAttribute");
#endif /* HAL_UART_MODULE_ENABLED */

			/* Send both keys to STSAFE */
			STS_CHK(StatusCode, (int32_t)StSafeA_PutAttribute(handle, STSAFEA_TAG_HOST_KEY_SLOT,
					Host_MAC_Cipher_Key, 2U * STSAFEA_HOST_KEY_LENGTH,
					STSAFEA_MAC_NONE));
#endif

#ifdef HAL_UART_MODULE_ENABLED
printf("\n\r %d. Store host keys through STM32 flash memory", ++idx);
#endif /* HAL_UART_MODULE_ENABLED */

/* Save both keys to STM32 FLASH */
//STS_CHK(StatusCode, (int32_t)HAL_FLASH_Unlock());

//    if (StatusCode == 0)
//    {
//      __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_PROGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR |
//                             FLASH_FLAG_PGSERR | FLASH_FLAG_MISERR | FLASH_FLAG_FASTERR | FLASH_FLAG_RDERR | FLASH_FLAG_OPTVERR);
//      FLASH_EraseInitTypeDef    FlashErase = {FLASH_TYPEERASE_PAGES, FLASH_BANK_2, PAGE_NUMBER, 1};
//
//      uint32_t PageError;
//
//      StatusCode = (int32_t)HAL_FLASHEx_Erase(&FlashErase, &PageError);
//
//      if ((StatusCode == 0) && PageError == 0xFFFFFFFFU)
//      {
//        for (i = 0; i < 2U * STSAFEA_HOST_KEY_LENGTH; i += 8U)
//        {
//          (void)memcpy(&ptr, &Host_MAC_Cipher_Key[i], sizeof(ptr));
//          if (HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,
//                                          FLASH_BASE + FLASH_SIZE - 2U * STSAFEA_HOST_KEY_LENGTH + i,
//                                          ptr))
//          {
//            StatusCode ++;
//            break;
//          }
//        }
//      }
//
//      STS_CHK(StatusCode, (int32_t)HAL_FLASH_Lock());
//    }
		}

		return StatusCode;
	}


	/***************** Authentication *******************/
	int32_t pairing(StSafeA_Handle_t* handle)
	{
		int32_t StatusCode = 0;

#ifdef HAL_UART_MODULE_ENABLED
		idx = 0;
		printf("\n\r\n\rPairing demonstration:");
#endif /* HAL_UART_MODULE_ENABLED */


#ifndef _FORCE_DEFAULT_FLASH_
		/* Check local envelope key */
		STS_CHK(StatusCode, check_local_envelope_key(handle));
#endif

		/* Check cipher key & host CMAC key and provide flash sector where save both keys */
		STS_CHK(StatusCode, check_host_keys(handle));

		return StatusCode;
	}

	/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
