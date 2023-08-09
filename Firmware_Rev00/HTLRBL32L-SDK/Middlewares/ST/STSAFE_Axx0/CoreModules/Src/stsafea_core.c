/**
  ******************************************************************************
  * @file    stsafea_core.c
  * @author  SMD/AME application teams
  * @version V3.3.2
  * @brief   STSAFE-A Middleware Core module.
  *          Provide all the core services related to the features offered by
  *          the STSAFE-A device such as:
  *           + Initialization and Configuration functions
  *           + General Purpose commands
  *           + Data Partition commands
  *           + Companion commands
  *           + Private and Public Key commands
  *           + Administrative commands
  *           + Additional commands
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
  *
  * STSAFE DRIVER SOFTWARE LICENSE AGREEMENT (SLA0088)
  *
  * BY INSTALLING, COPYING, DOWNLOADING, ACCESSING OR OTHERWISE USING THIS SOFTWARE
  * OR ANY PART THEREOF (AND THE RELATED DOCUMENTATION) FROM STMICROELECTRONICS
  * INTERNATIONAL N.V, SWISS BRANCH AND/OR ITS AFFILIATED COMPANIES (STMICROELECTRONICS),
  * THE RECIPIENT, ON BEHALF OF HIMSELF OR HERSELF, OR ON BEHALF OF ANY ENTITY BY WHICH
  * SUCH RECIPIENT IS EMPLOYED AND/OR ENGAGED AGREES TO BE BOUND BY THIS SOFTWARE LICENSE
  * AGREEMENT.
  *
  * Under STMicroelectronics’ intellectual property rights, the redistribution,
  * reproduction and use in source and binary forms of the software or any part thereof,
  * with or without modification, are permitted provided that the following conditions
  * are met:
  * 1.  Redistribution of source code (modified or not) must retain any copyright notice,
  *     this list of conditions and the disclaimer set forth below as items 10 and 11.
  * 2.  Redistributions in binary form, except as embedded into a microcontroller or
  *     microprocessor device or a software update for such device, must reproduce any
  *     copyright notice provided with the binary code, this list of conditions, and the
  *     disclaimer set forth below as items 10 and 11, in documentation and/or other
  *     materials provided with the distribution.
  * 3.  Neither the name of STMicroelectronics nor the names of other contributors to this
  *     software may be used to endorse or promote products derived from this software or
  *     part thereof without specific written permission.
  * 4.  This software or any part thereof, including modifications and/or derivative works
  *     of this software, must be used and execute solely and exclusively in combination
  *     with a secure microcontroller device from STSAFE family manufactured by or for
  *     STMicroelectronics.
  * 5.  No use, reproduction or redistribution of this software partially or totally may be
  *     done in any manner that would subject this software to any Open Source Terms.
  *     “Open Source Terms” shall mean any open source license which requires as part of
  *     distribution of software that the source code of such software is distributed
  *     therewith or otherwise made available, or open source license that substantially
  *     complies with the Open Source definition specified at www.opensource.org and any
  *     other comparable open source license such as for example GNU General Public
  *     License(GPL), Eclipse Public License (EPL), Apache Software License, BSD license
  *     or MIT license.
  * 6.  STMicroelectronics has no obligation to provide any maintenance, support or
  *     updates for the software.
  * 7.  The software is and will remain the exclusive property of STMicroelectronics and
  *     its licensors. The recipient will not take any action that jeopardizes
  *     STMicroelectronics and its licensors' proprietary rights or acquire any rights
  *     in the software, except the limited rights specified hereunder.
  * 8.  The recipient shall comply with all applicable laws and regulations affecting the
  *     use of the software or any part thereof including any applicable export control
  *     law or regulation.
  * 9.  Redistribution and use of this software or any part thereof other than as  permitted
  *     under this license is void and will automatically terminate your rights under this
  *     license.
  * 10. THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" AND ANY
  *     EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
  *     OF THIRD PARTY INTELLECTUAL PROPERTY RIGHTS, WHICH ARE DISCLAIMED TO THE FULLEST
  *     EXTENT PERMITTED BY LAW. IN NO EVENT SHALL STMICROELECTRONICS OR CONTRIBUTORS BE
  *     LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  *     DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  *     THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  *     NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  *     ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  * 11. EXCEPT AS EXPRESSLY PERMITTED HEREUNDER, NO LICENSE OR OTHER RIGHTS, WHETHER EXPRESS
  *     OR IMPLIED, ARE GRANTED UNDER ANY PATENT OR OTHER INTELLECTUAL PROPERTY RIGHTS OF
  *     STMICROELECTRONICS OR ANY THIRD PARTY.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stsafea_core.h"
#include "stsafea_crypto.h"
#include "stsafea_service.h"
#include <string.h>


/* Private macros ------------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static StSafeA_ResponseCode_t StSafeA_TransmitCommand(StSafeA_Handle_t *pStSafeA);
static StSafeA_ResponseCode_t StSafeA_ReceiveResponse(StSafeA_Handle_t *pStSafeA);
static StSafeA_ResponseCode_t StSafeA_AssignLVResponse(StSafeA_LVBuffer_t *pDestLVBuffer,
                                                       StSafeA_LVBuffer_t *pSrcLVBuffer, uint16_t ExpectedLen);
static StSafeA_ResponseCode_t StSafeA_AssignLVBuffer(StSafeA_LVBuffer_t *pDestLVBuffer, uint8_t *pDataBuffer,
                                                     uint16_t ExpectedLen);
static void StSafeA_SetLVData(StSafeA_LVBuffer_t *pDestLVBuffer, uint8_t *pSrcLVData, uint16_t Length);
static void StSafeA_BuildCommandHeaderCMAC(StSafeA_Handle_t *pStSafeA, uint8_t CommandCode, uint8_t *pMAC);
#if (!STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT)
static void StSafeA_GetHostMacSequenceCounter(StSafeA_Handle_t *pStSafeA);
#endif /* STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT */

/* Functions Definition ------------------------------------------------------*/

StSafeA_ResponseCode_t StSafeA_Init(StSafeA_Handle_t *pStSafeA, uint8_t *pAllocatedRxTxBufferData) {
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* STSAFE MW requires a data buffer to send/receive bytes ove the bus.
     For memory optimization reasons it's up to the application to allocate it,
     so that the application can deallocate it when STSAFE services are not required anymore */
  if ((pStSafeA != NULL) && (pAllocatedRxTxBufferData != NULL))
  {
    pStSafeA->CrcSupport    = STSAFEA_CRC_SUPPORT;
    pStSafeA->MacCounter    = 0;
    pStSafeA->InOutBuffer.LV.Length = 0;
    pStSafeA->InOutBuffer.LV.Data   = pAllocatedRxTxBufferData;
    pStSafeA->HostMacSequenceCounter = STSAFEA_HOST_CMAC_INVALID_COUNTER;

    pStSafeA->HashObj.HashType = STSAFEA_SHA_256;
    pStSafeA->HashObj.HashCtx = NULL;
    (void)memset(pStSafeA->HashObj.HashRes, 0, sizeof(pStSafeA->HashObj.HashRes));

    status_code = STSAFEA_UNEXPECTED_ERROR;

    /* Initialize/Retrieve the Host MAC and Cipher Keys  */
    if (StSafeA_HostKeys_Init() == 0)
    	status_code = STSAFEA_OK;

  }

  return status_code;
}

int32_t StSafeA_GetVersion(void)
{
  return (int32_t)STSAFEA_VERSION;
}

StSafeA_ResponseCode_t StSafeA_Echo(
  StSafeA_Handle_t *pStSafeA,
  uint8_t *pInEchoData,
  uint16_t InRespDataLen,
  StSafeA_LVBuffer_t *pOutLVResponse,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA) &&
      (pInEchoData != NULL))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));
    stsafea_assert_param(IS_STSAFEA_LEN(InRespDataLen));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_ECHO, &InMAC);

    /* Build command Data */
    STSAFEA_CHECK_SIZE(InMAC, 0U, InRespDataLen);
    (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[0], pInEchoData, InRespDataLen);
    pStSafeA->InOutBuffer.LV.Length = InRespDataLen;

    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = InRespDataLen + STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_ECHO);

      status_code = StSafeA_ReceiveResponse(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        /* Assign the received response to the output parameter */
        status_code = StSafeA_AssignLVResponse(pOutLVResponse, &pStSafeA->InOutBuffer.LV, InRespDataLen);
      }
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_Reset(
  StSafeA_Handle_t *pStSafeA,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_RESET, &InMAC);

    pStSafeA->InOutBuffer.LV.Length = 0U;

    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_RESET);

      status_code = StSafeA_ReceiveResponse(pStSafeA);
      if (status_code == STSAFEA_OK)
      {
        pStSafeA->MacCounter = 0;
      }
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_GenerateRandom(
  StSafeA_Handle_t *pStSafeA,
  StSafeA_RndSubject_t InRndSubject,
  uint8_t InRespDataLen,
  StSafeA_LVBuffer_t *pOutLVResponse,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;
  uint16_t tmp_len;

#if defined(STSAFE_A110)
  (void)InRndSubject;
#endif /* STSAFE_A110 */

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));
    stsafea_assert_param(IS_STSAFEA_LEN(InRespDataLen));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_GENERATE_RANDOM, &InMAC);

    pStSafeA->InOutBuffer.LV.Data[0] = 0U;
    pStSafeA->InOutBuffer.LV.Data[1] = InRespDataLen;
    pStSafeA->InOutBuffer.LV.Length = 2U;

    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      tmp_len = InRespDataLen;

      tmp_len += STSAFEA_R_MAC_LENGTH(InMAC);
      pStSafeA->InOutBuffer.LV.Length = tmp_len;

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_GENERATE_RANDOM);

      status_code = StSafeA_ReceiveResponse(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        /* Assign the received response to the output parameter */
        status_code = StSafeA_AssignLVResponse(pOutLVResponse, &pStSafeA->InOutBuffer.LV, InRespDataLen);
      }
    }
  }

  return status_code;
}


StSafeA_ResponseCode_t StSafeA_Hibernate(
  StSafeA_Handle_t *pStSafeA,
  uint8_t InWakeUpMode,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));
    stsafea_assert_param(IS_STSAFEA_WAKEUP_MODE(InWakeUpMode));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_HIBERNATE, &InMAC);

    pStSafeA->InOutBuffer.LV.Data[0] = (uint8_t)InWakeUpMode;
    pStSafeA->InOutBuffer.LV.Length = 1U;

    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_HIBERNATE);

      /* Receive */
      status_code = StSafeA_ReceiveResponse(pStSafeA);
    }
  }
  return status_code;
}

StSafeA_ResponseCode_t StSafeA_DataPartitionQuery(
  StSafeA_Handle_t *pStSafeA,
  uint8_t InZoneMaxNum,
  StSafeA_DataPartitionBuffer_t *pOutDataPartition,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA)         &&
      (pOutDataPartition != NULL)                    &&
      (IS_STSAFEA_CONDITIONAL_VALID_PTR(pOutDataPartition->pZoneInfoRecord)))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_QUERY, &InMAC);

    /* Build command Data */
    pStSafeA->InOutBuffer.LV.Data[0] = (uint8_t)STSAFEA_TAG_DATA_PARTITION_CONFIGURATION;
    pStSafeA->InOutBuffer.LV.Length = 1U;

    /* Transmit */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_DATA_PARTITION_QUERY_MIN_RESPONSE_LENGTH + STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_QUERY);

      /* Receive */
      status_code = StSafeA_ReceiveResponse(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        pOutDataPartition->Length = pStSafeA->InOutBuffer.LV.Length;
        pOutDataPartition->NumberOfZones = pStSafeA->InOutBuffer.LV.Data[0];
#if (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
        (void)InZoneMaxNum;
        pOutDataPartition->pZoneInfoRecord = (StSafeA_ZoneInformationRecordBuffer_t *)&pStSafeA->InOutBuffer.LV.Data[1];
#else
        if (pOutDataPartition->NumberOfZones > InZoneMaxNum)
        {
          /* Return an error so the application can read the correct number of zones,
             allocate the right space (if not under STSAFEA_USE_OPTIMIZATION_SHARED_RAM optimization),
             and call back this API with the right InZoneMaxNum parameter */
          return STSAFEA_INVALID_RESP_LENGTH;
        }

        (void)memcpy(pOutDataPartition->pZoneInfoRecord,
                     &pStSafeA->InOutBuffer.LV.Data[1],
                     (uint32_t)pOutDataPartition->Length - 1U);
#endif
        uint16_t uIdx;
        for (uint8_t i = 0; i < pOutDataPartition->NumberOfZones; i++)
        {
          uIdx = (uint16_t)(i * sizeof(StSafeA_ZoneInformationRecordBuffer_t)) + 1U;
          if (pOutDataPartition->pZoneInfoRecord[i].ZoneType == STSAFEA_ONE_WAY_COUNTER_PRESENCE)
          {
            (void)memmove(&pOutDataPartition->pZoneInfoRecord[i].ReadAccessCondition + 3U,
                          &pOutDataPartition->pZoneInfoRecord[i].ReadAccessCondition,
                          (uint32_t)pOutDataPartition->Length - uIdx - 3U);
            /* Add bytes for AC */
             pOutDataPartition->Length += 3U;
            /* Adjust little/big endian bytes */
            pOutDataPartition->pZoneInfoRecord[i].OneWayCounter =
                                  SWAP4BYTES(pOutDataPartition->pZoneInfoRecord[i].OneWayCounter);
          }
          else
          {
            if ((i + 1U) < pOutDataPartition->NumberOfZones)
            {
              (void)memmove(&pOutDataPartition->pZoneInfoRecord[i + 1U].Index,
                            &pOutDataPartition->pZoneInfoRecord[i].UpdateAccessCondition,
                            (uint32_t)pOutDataPartition->Length - uIdx - 5U);

              /* Add bytes for AC & counter */
              pOutDataPartition->Length += 7U;
            }
            /* One way counter set to 0 */
            pOutDataPartition->pZoneInfoRecord[i].OneWayCounter = 0;
            /* Copy data segment length */
            pOutDataPartition->pZoneInfoRecord[i].DataSegmentLength =
                                  pOutDataPartition->pZoneInfoRecord[i].ReadAccessCondition +
                                  ((uint16_t)pOutDataPartition->pZoneInfoRecord[i].UpdateAcChangeRight << 8U);
          }
          /* Adjust little/big endian bytes */
          pOutDataPartition->pZoneInfoRecord[i].DataSegmentLength =
                                  (uint16_t)SWAP2BYTES(pOutDataPartition->pZoneInfoRecord[i].DataSegmentLength);

          /* Adjust AC bytes */
          pOutDataPartition->pZoneInfoRecord[i].UpdateAccessCondition =
                                  pOutDataPartition->pZoneInfoRecord[i].ReadAcChangeRight & 0X07U;
          pOutDataPartition->pZoneInfoRecord[i].UpdateAcChangeRight   =
                                  (pOutDataPartition->pZoneInfoRecord[i].ReadAcChangeRight & 0X08U) >> 3;
          pOutDataPartition->pZoneInfoRecord[i].ReadAccessCondition   =
                                  (pOutDataPartition->pZoneInfoRecord[i].ReadAcChangeRight & 0X70U) >> 4;
          pOutDataPartition->pZoneInfoRecord[i].ReadAcChangeRight     =
                                  (pOutDataPartition->pZoneInfoRecord[i].ReadAcChangeRight & 0X80U) >> 7;
        }
      }
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_Decrement(
  StSafeA_Handle_t *pStSafeA,
  uint8_t InChangeACIndicator,
  uint8_t InNewDecrementACRight,
  uint8_t InNewDecrementAC,
  uint8_t InZoneIndex,
  uint16_t InOffset,
  uint32_t InAmount,
  StSafeA_LVBuffer_t *pInData,
  StSafeA_DecrementBuffer_t *pOutDecrement,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA) &&
      (pOutDecrement != NULL))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));
    stsafea_assert_param(IS_STSAFEA_FLAG(InChangeACIndicator));
    stsafea_assert_param(IS_STSAFEA_FLAG(InNewDecrementACRight));
    stsafea_assert_param(IS_STSAFEA_AC(InNewDecrementAC));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_DECREMENT, &InMAC);

    /* Change AC indicator + New AC change right + New update AC */
    uint8_t tmp = (InChangeACIndicator & 0x01U) << STSAFEA_CMD_DECREMENT_HEADER_CHAC_POS;
    tmp |= (InNewDecrementACRight & 0x01U) << STSAFEA_CMD_DECREMENT_HEADER_NEWUPDRIGHT_POS;
    tmp |= (InNewDecrementAC & STSAFEA_AC_MSK) << STSAFEA_CMD_DECREMENT_HEADER_NEWUPDAC_POS;

    pStSafeA->InOutBuffer.LV.Data[0] = tmp;

    /* Zone index */
    pStSafeA->InOutBuffer.LV.Data[1] = (uint8_t)InZoneIndex;

    /* Zone offset */
    uint16_t offset = (uint16_t)SWAP2BYTES(InOffset);
    (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[2], &offset, sizeof(offset));

    /* Amount */
    uint32_t amount = (uint32_t)SWAP4BYTES(InAmount);
    (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[4], &amount, sizeof(amount));

    pStSafeA->InOutBuffer.LV.Length = 8U;

    /* Data */
    if ((pInData != NULL) && (pInData->Length > 0U))
    {
      STSAFEA_CHECK_SIZE(InMAC, pStSafeA->InOutBuffer.LV.Length, pInData->Length);
      (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length],
                   pInData->Data,
                   pInData->Length);

      pStSafeA->InOutBuffer.LV.Length += pInData->Length;
    }

    /* Transmit command */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_DECREMENT_RESPONSE_LENGTH + STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_DECREMENT);

      /* Read response */
      status_code = StSafeA_ReceiveResponse(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        pOutDecrement->Length = pStSafeA->InOutBuffer.LV.Length;
        (void)memcpy((uint8_t *)&pOutDecrement->OneWayCounter, pStSafeA->InOutBuffer.LV.Data,
                     sizeof(pOutDecrement->OneWayCounter));
        pOutDecrement->OneWayCounter = (uint32_t)SWAP4BYTES(pOutDecrement->OneWayCounter);
      }
    }
  }
  return status_code;
}

StSafeA_ResponseCode_t StSafeA_Read(
  StSafeA_Handle_t *pStSafeA,
  uint8_t InChangeACIndicator,
  uint8_t InNewReadACRight,
  uint8_t InNewReadAC,
  uint8_t InZoneIndex,
  uint16_t InOffset,
  uint16_t InAmount,
  uint16_t InRespDataLen,
  StSafeA_LVBuffer_t *pOutLVResponse,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;
  uint32_t length;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));
    stsafea_assert_param(IS_STSAFEA_LEN(InRespDataLen));
    stsafea_assert_param(IS_STSAFEA_FLAG(InChangeACIndicator));
    stsafea_assert_param(IS_STSAFEA_AC(InNewReadAC));

    /* Build command Header and process CMAC */
    STSAFEA_CHECK_SIZE(InMAC, 0U, InRespDataLen);
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_READ, &InMAC);

    /* Change AC indicator + New AC change right + New update AC */
    pStSafeA->InOutBuffer.LV.Data[0] = (InChangeACIndicator == 0U) ? 0x00U :
                                       ((uint8_t)((InNewReadACRight == 0U) ? 0x10U : 0x18U) | (uint8_t)InNewReadAC);

    /* Zone index */
    pStSafeA->InOutBuffer.LV.Data[1] = InZoneIndex;

    /* Zone offset */
    uint16_t offset = (uint16_t)SWAP2BYTES(InOffset);
    (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[2], &offset, sizeof(offset));

    /* Length */
    length = (uint16_t)SWAP2BYTES(InAmount);
    (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[4], &length, sizeof(InAmount));
    pStSafeA->InOutBuffer.LV.Length = 6U;

    /* Transmit command */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = InRespDataLen + STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_READ);

      status_code = StSafeA_ReceiveResponse(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        /* Assign the received response to the output parameter */
        status_code = StSafeA_AssignLVResponse(pOutLVResponse, &pStSafeA->InOutBuffer.LV, InRespDataLen);
      }
    }
  }
  return status_code;
}

StSafeA_ResponseCode_t StSafeA_Process_Update(
  StSafeA_Handle_t *pStSafeA,
  uint8_t InAtomicity,
  uint8_t InChangeACIndicator,
  uint8_t InNewUpdateACRight,
  uint8_t InNewUpdateAC,
  uint8_t InZoneIndex,
  uint16_t InOffset,
  StSafeA_LVBuffer_t *pInLVData,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));
    stsafea_assert_param(IS_STSAFEA_FLAG(InAtomicity));
    stsafea_assert_param(IS_STSAFEA_FLAG(InChangeACIndicator));
    stsafea_assert_param(IS_STSAFEA_FLAG(InNewUpdateACRight));
    stsafea_assert_param(IS_STSAFEA_AC(InNewUpdateAC));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_UPDATE, &InMAC);

    /* If InChangeACIndicator is false then InNewUpdateACRight and InNewUpdateAC must be Zero. Let the STSAFE Hw to manage it */
    uint8_t tmp = (InAtomicity & 0x01U) << STSAFEA_CMD_UPDATE_HEADER_ATOM_POS;
    tmp |= (InChangeACIndicator & 0x01U) << STSAFEA_CMD_UPDATE_HEADER_CHAC_POS;
    tmp |= (InNewUpdateACRight & 0x01U) << STSAFEA_CMD_UPDATE_HEADER_NEWUPDRIGHT_POS;
    tmp |= (InNewUpdateAC & STSAFEA_AC_MSK) << STSAFEA_CMD_UPDATE_HEADER_NEWUPDAC_POS;
    pStSafeA->InOutBuffer.LV.Data[0] = tmp;

    /* Zone index */
    pStSafeA->InOutBuffer.LV.Data[1] = InZoneIndex;

    /* Zone offset */
    uint16_t offset = (uint16_t)SWAP2BYTES(InOffset);
    (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[2], &offset, sizeof(offset));

    pStSafeA->InOutBuffer.LV.Length = 4U;

    /* Data */
    if ((pInLVData != NULL) && (pInLVData->Length > 0U))
    {
      STSAFEA_CHECK_SIZE(InMAC, pStSafeA->InOutBuffer.LV.Length, pInLVData->Length);
      (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length],
                   pInLVData->Data,
                   pInLVData->Length);

      pStSafeA->InOutBuffer.LV.Length += pInLVData->Length;
    }

    /* Transmit command */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_UPDATE);

      status_code = StSafeA_ReceiveResponse(pStSafeA);
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_Update(
  StSafeA_Handle_t *pStSafeA,
  uint8_t InAtomicity,
  uint8_t InChangeACIndicator,
  uint8_t InNewUpdateACRight,
  uint8_t InNewUpdateAC,
  uint8_t InZoneIndex,
  uint16_t InOffset,
  StSafeA_LVBuffer_t *pInLVData,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  if ((InAtomicity == STSAFEA_FLAG_TRUE) && (pInLVData->Length > STSAFEA_ATOMICITY_BUFFER_SIZE))
  {
		uint8_t i;
    StSafeA_LVBuffer_t pLVData;

    pLVData.Length = STSAFEA_ATOMICITY_BUFFER_SIZE;
		pLVData.Data = pInLVData->Data;

    for (i = 0; i < (pInLVData->Length / STSAFEA_ATOMICITY_BUFFER_SIZE); i++)
    {
      status_code = StSafeA_Process_Update(pStSafeA, InAtomicity, InChangeACIndicator, InNewUpdateACRight,
                                           InNewUpdateAC, InZoneIndex, InOffset, &pLVData, InMAC);
      InOffset += STSAFEA_ATOMICITY_BUFFER_SIZE;
      pLVData.Data += STSAFEA_ATOMICITY_BUFFER_SIZE;
      if (status_code != STSAFEA_OK)
      {
        break;
      }
    }

    if (status_code == STSAFEA_OK)
    {
      pLVData.Length = pInLVData->Length % STSAFEA_ATOMICITY_BUFFER_SIZE;
      if (pLVData.Length != 0U)
      {
        status_code = StSafeA_Process_Update(pStSafeA, InAtomicity, InChangeACIndicator, InNewUpdateACRight,
                                             InNewUpdateAC, InZoneIndex, InOffset, &pLVData, InMAC);
      }
    }
  }
  else
  {
    status_code = StSafeA_Process_Update(pStSafeA, InAtomicity, InChangeACIndicator, InNewUpdateACRight,
                                         InNewUpdateAC, InZoneIndex, InOffset, pInLVData, InMAC);
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_GenerateKeyPair(
  StSafeA_Handle_t *pStSafeA,
  uint8_t InKeySlotNum,
  uint16_t InUseLimit,
  uint8_t InChangeAuthFlagsRight,
  uint8_t InAuthorizationFlags,
  StSafeA_CurveId_t InCurveId,
  uint16_t InPubXYLen,
  uint8_t *pOutPointReprensentationId,
  StSafeA_LVBuffer_t *pOutPubX,
  StSafeA_LVBuffer_t *pOutPubY,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA)              &&
      IS_STSAFEA_CONDITIONAL_LVBUFFER_VALID_PTR(pOutPubX) &&
      IS_STSAFEA_CONDITIONAL_LVBUFFER_VALID_PTR(pOutPubY) &&
      (pOutPointReprensentationId != NULL))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));
    stsafea_assert_param(IS_STSAFEA_KEY_SLOT(InKeySlotNum));
    stsafea_assert_param(IS_STSAFEA_FLAG(InChangeAuthFlagsRight));

    uint16_t tmp_len;
    uint16_t tmp_use_limit;

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_GENERATE_KEY, &InMAC);

    /* Attribute Tag */
    pStSafeA->InOutBuffer.LV.Data[0] = (uint8_t)STSAFEA_TAG_PRIVATE_KEY_SLOT;

    /* Slot number */
    pStSafeA->InOutBuffer.LV.Data[1] = (uint8_t)InKeySlotNum;

    /* Use limit */
    tmp_use_limit = InUseLimit;

#ifdef STSAFEA_KEY_SLOT_EPHEMERAL
    if (InKeySlotNum == STSAFEA_KEY_SLOT_EPHEMERAL)
    {
      tmp_use_limit = 1;
    }
#endif /* STSAFEA_KEY_SLOT_EPHEMERAL */
    tmp_use_limit = (uint16_t)SWAP2BYTES(tmp_use_limit);
    (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[2], &tmp_use_limit, sizeof(uint16_t));

    /* Mode of operation */
    tmp_use_limit = (InChangeAuthFlagsRight == 0x0U) ?
                                  0x0000U : STSAFEA_PRIVATE_KEY_MODE_OF_OPERATION_CHANGE_RIGHT_MASK;
    tmp_use_limit |= InAuthorizationFlags & STSAFEA_PRIVATE_KEY_MODE_OF_OPERATION_AUTHORIZATION_FLAGS_MASK;
    tmp_use_limit = (uint16_t)SWAP2BYTES(tmp_use_limit);

    (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[4], &tmp_use_limit, sizeof(tmp_use_limit));

    /* Curve ID */
    uint16_t tmp_curve_len = STSAFEA_GET_ECC_CURVE_OID_LEN(InCurveId);
    tmp_use_limit = (uint16_t)SWAP2BYTES(tmp_curve_len);
    tmp_len = 8U + tmp_curve_len;

    /* Curve identifier */
    (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[8], STSAFEA_GET_ECC_CURVE_OID(InCurveId), tmp_curve_len);
    /* Curve identifier size */
    (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[6], &tmp_use_limit, sizeof(uint16_t));
    /* Update Frame Length */
    pStSafeA->InOutBuffer.LV.Length = tmp_len;


    /* Transmit */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_POINT_REPRESENTATION_ID_LEN +
                             2U * (STSAFEA_LENGTH_SIZE + (uint16_t)STSAFEA_GET_XYRS_LEN_FROM_CURVE(InCurveId)) +
                             STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_GENERATE_KEY);

      /* Receive */
      status_code = StSafeA_ReceiveResponse(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        status_code = STSAFEA_INVALID_RESP_LENGTH;
        if (STSAFEA_GET_XYRS_LEN_FROM_CURVE(InCurveId) <= InPubXYLen)
        {
          uint16_t uIdx = 1U;
          /* Assign Representation Id */
          if (pStSafeA->InOutBuffer.LV.Length >= uIdx)
          {
            *pOutPointReprensentationId = pStSafeA->InOutBuffer.LV.Data[0];
          }

          /* Assign Key X */
          if (pStSafeA->InOutBuffer.LV.Length > (uIdx + sizeof(pOutPubX->Length)))
          {
            /* Assign the received response to the output parameter */
            status_code = StSafeA_AssignLVBuffer(pOutPubX, &pStSafeA->InOutBuffer.LV.Data[uIdx], InPubXYLen);

            uIdx += (uint16_t)sizeof(pOutPubX->Length) + pOutPubX->Length ;
          }

          /* Assign Key Y*/
          if ((pStSafeA->InOutBuffer.LV.Length > (uIdx + sizeof(pOutPubY->Length))) && (status_code == STSAFEA_OK))
          {
            /* Assign the received response to the output parameter */
            status_code = StSafeA_AssignLVBuffer(pOutPubY, &pStSafeA->InOutBuffer.LV.Data[uIdx], InPubXYLen);
          }
        }
      }
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_GenerateSignature(
  StSafeA_Handle_t *pStSafeA,
  uint8_t InKeySlotNum,
  const uint8_t *pInDigest,
  StSafeA_HashTypes_t InDigestType,
  uint16_t InSignRSLen,
  StSafeA_LVBuffer_t *pOutSignR,
  StSafeA_LVBuffer_t *pOutSignS,
  uint8_t InMAC,
  uint8_t InHostEncryption)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

#if defined(STSAFE_A100)
  STSAFEA_UNUSED_VAR(InHostEncryption);
#endif /* STSAFE_A100 */

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA)                &&
      IS_STSAFEA_CONDITIONAL_VALID_PTR(pInDigest)           &&
      IS_STSAFEA_CONDITIONAL_LVBUFFER_VALID_PTR(pOutSignR)  &&
      IS_STSAFEA_CONDITIONAL_LVBUFFER_VALID_PTR(pOutSignS))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));
    stsafea_assert_param(IS_STSAFEA_LEN(InSignRSLen));
    stsafea_assert_param(IS_STSAFEA_KEY_SLOT(InKeySlotNum));
    stsafea_assert_param(IS_STSAFEA_ENCRYPT(InHostEncryption));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_GENERATE_SIGNATURE, &InMAC);

    /* Private key slot number */
    pStSafeA->InOutBuffer.LV.Data[0] = (uint8_t)InKeySlotNum;

    /* Digest size */
    pStSafeA->InOutBuffer.LV.Data[1] = (uint8_t)((((InDigestType == STSAFEA_SHA_384) ?
                                                   STSAFEA_SHA_384_LENGTH : STSAFEA_SHA_256_LENGTH) & 0xFF00U) >> 8);
    pStSafeA->InOutBuffer.LV.Data[2] = ((InDigestType == STSAFEA_SHA_384) ?
                                        STSAFEA_SHA_384_LENGTH : STSAFEA_SHA_256_LENGTH);

    /* Digest */
#ifdef WORKAROUND_GENERATE_SIGNATURE
    uint8_t DigestLength = (InDigestType == STSAFEA_SHA_384) ? STSAFEA_SHA_384_LENGTH : STSAFEA_SHA_256_LENGTH;
    uint8_t i, sum = 0;
    for (i = 0; i < DigestLength; i++)
    {
      sum |= pInDigest[i];
    }
    if (sum == 0U)
    {
      return STSAFEA_INVALID_PARAMETER;
    }

    (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[3],
                 pInDigest,
                 DigestLength);
#else
    (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[3],
                 pInDigest,
                 ((InDigestType == STSAFEA_SHA_384) ? STSAFEA_SHA_384_LENGTH : STSAFEA_SHA_256_LENGTH));
#endif

    /* Length */
    pStSafeA->InOutBuffer.LV.Length = (uint16_t)((InDigestType == STSAFEA_SHA_384) ?
                                                 STSAFEA_SHA_384_LENGTH : STSAFEA_SHA_256_LENGTH) + 3U;

#if (!STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT)
#if defined(STSAFE_A110)
    /* Encrypt data */
    if (((uint8_t)InHostEncryption & (uint8_t)STSAFEA_ENCRYPTION_COMMAND) == (uint8_t)STSAFEA_ENCRYPTION_COMMAND)
    {
      status_code = StSafeA_DataEncryption(pStSafeA);
    }
    else
    {
      status_code = STSAFEA_OK;
    }

    if (status_code == STSAFEA_OK)
#endif /* STSAFE_A110 */
#endif /* STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT */
    {
      /* Transmit command */
      status_code = StSafeA_TransmitCommand(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        uint16_t tmp_len = (InDigestType == STSAFEA_SHA_384) ? STSAFEA_LENGTH_CMD_RESP_GENERATE_SIGNATURE_SHA_384 :
                           STSAFEA_LENGTH_CMD_RESP_GENERATE_SIGNATURE_SHA_256;
        /* Set response length */
        pStSafeA->InOutBuffer.LV.Length = tmp_len + STSAFEA_R_MAC_LENGTH(InMAC); ;

        /* Wait for the command processing. Then check for the response */
        StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_GENERATE_SIGNATURE);

        /* Read response */
        status_code = StSafeA_ReceiveResponse(pStSafeA);

        if (status_code == STSAFEA_OK)
        {
#if (!STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT)
#if defined(STSAFE_A110)
          /* Decrypt data */
          if (((uint8_t)InHostEncryption & (uint8_t)STSAFEA_ENCRYPTION_RESPONSE) == (uint8_t)STSAFEA_ENCRYPTION_RESPONSE)
          {
            status_code = StSafeA_DataDecryption(pStSafeA);
          }

          if (status_code == STSAFEA_OK)
#endif /* STSAFE_A110 */
#endif /* STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT */
          {
            /* Assign the received response to the output parameter */
            status_code = StSafeA_AssignLVBuffer(pOutSignR, &pStSafeA->InOutBuffer.LV.Data[0], InSignRSLen);
            if (status_code == STSAFEA_OK)
            {
              status_code = StSafeA_AssignLVBuffer(
                                  pOutSignS,
                                  &pStSafeA->InOutBuffer.LV.Data[STSAFEA_LENGTH_SIZE + pOutSignR->Length],
                                  InSignRSLen);
            }
          }
        }
      }
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_VerifyMessageSignature(
  StSafeA_Handle_t *pStSafeA,
  const StSafeA_CurveId_t InCurveId,
  const StSafeA_LVBuffer_t *pInPubX,
  const StSafeA_LVBuffer_t *pInPubY,
  const StSafeA_LVBuffer_t *pInSignR,
  const StSafeA_LVBuffer_t *pInSignS,
  const StSafeA_LVBuffer_t *pInDigest,
  StSafeA_VerifySignatureBuffer_t *pOutRespVerifySignature,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA)       &&
      (pInPubX != NULL)                            &&
      (pInPubY != NULL)                            &&
      (pInSignR != NULL)                           &&
      (pInSignS != NULL)                           &&
      (pInDigest != NULL)                          &&
      (pOutRespVerifySignature != NULL))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_VERIFY_SIGNATURE, &InMAC);

    /* Verify signature [message authentication] */
    pStSafeA->InOutBuffer.LV.Data[0] = 0x00U;

    /* Curve ID */
    pStSafeA->InOutBuffer.LV.Data[1] = (uint8_t)((STSAFEA_GET_ECC_CURVE_OID_LEN(InCurveId) & 0xFF00U) >> 8);
    pStSafeA->InOutBuffer.LV.Data[2] = (uint8_t)(STSAFEA_GET_ECC_CURVE_OID_LEN(InCurveId) & 0x00FFU);

    (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[3],
                 STSAFEA_GET_ECC_CURVE_OID(InCurveId),
                 STSAFEA_GET_ECC_CURVE_OID_LEN(InCurveId));

    pStSafeA->InOutBuffer.LV.Length = (0x0001U + STSAFEA_ECC_CURVE_OID_LEN +
                                      (uint16_t)STSAFEA_GET_ECC_CURVE_OID_LEN(InCurveId));

    /* Public key */
    pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length] = STSAFEA_POINT_REPRESENTATION_ID;
    pStSafeA->InOutBuffer.LV.Length++;

    /* X coordinate */
    if ((pInPubX->Length > 0U))
    {
      pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length] = (uint8_t)((pInPubX->Length & 0xFF00U) >> 8);
      pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length + 1U] = (uint8_t)(pInPubX->Length & 0x00FFU);

      STSAFEA_CHECK_SIZE(InMAC, pStSafeA->InOutBuffer.LV.Length + STSAFEA_LENGTH_SIZE, pInPubX->Length);
      (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length + 2U],
                   pInPubX->Data,
                   pInPubX->Length);

      pStSafeA->InOutBuffer.LV.Length += (uint16_t)sizeof(pInPubX->Length) + pInPubX->Length;
    }

    /* Y coordinate */
    if ((pInPubY->Length > 0U))
    {
      pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length] = (uint8_t)((pInPubY->Length & 0xFF00U) >> 8);
      pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length + 1U] = (uint8_t)(pInPubY->Length & 0x00FFU);

      STSAFEA_CHECK_SIZE(InMAC, pStSafeA->InOutBuffer.LV.Length + STSAFEA_LENGTH_SIZE, pInPubY->Length);
      (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length + 2U],
                   pInPubY->Data,
                   pInPubY->Length);

      pStSafeA->InOutBuffer.LV.Length += (uint16_t)sizeof(pInPubY->Length) + pInPubY->Length;
    }

    /* Signature */
    if (pInSignR->Length > 0U)
    {
      pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length] = (uint8_t)((pInSignR->Length & 0xFF00U) >> 8);
      pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length + 1U] = (uint8_t)(pInSignR->Length & 0x00FFU);

      STSAFEA_CHECK_SIZE(InMAC, pStSafeA->InOutBuffer.LV.Length + STSAFEA_LENGTH_SIZE, pInSignR->Length);
      (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length + 2U],
                   pInSignR->Data,
                   pInSignR->Length);

      pStSafeA->InOutBuffer.LV.Length += 2U + pInSignR->Length;
    }

    if (pInSignS->Length > 0U)
    {
      pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length] = (uint8_t)((pInSignS->Length & 0xFF00U) >> 8);
      pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length + 1U] = (uint8_t)(pInSignS->Length & 0x00FFU);

      STSAFEA_CHECK_SIZE(InMAC, pStSafeA->InOutBuffer.LV.Length + STSAFEA_LENGTH_SIZE, pInSignS->Length);
      (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length + 2U],
                   pInSignS->Data,
                   pInSignS->Length);

      pStSafeA->InOutBuffer.LV.Length += 2U + pInSignS->Length;
    }

    /* Hash */
    if ((pInDigest->Length > 0U))
    {
      /* Digest size */
      pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length] = (uint8_t)((pInDigest->Length & 0xFF00U) >> 8);
      pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length + 1U] = (uint8_t)(pInDigest->Length & 0x00FFU);

      /* Digest */
      STSAFEA_CHECK_SIZE(InMAC, pStSafeA->InOutBuffer.LV.Length + STSAFEA_LENGTH_SIZE, pInDigest->Length);
      (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length + 2U],
                   pInDigest->Data,
                   pInDigest->Length);

      pStSafeA->InOutBuffer.LV.Length += 2U + pInDigest->Length;
    }

    /* Transmit command */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_VERIFY_MSG_SIGNATURE_RESPONSE_LENGTH + STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_VERIFY_MSG_SIGNATURE);

      /* Read response */
      status_code = StSafeA_ReceiveResponse(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        pOutRespVerifySignature->Length = pStSafeA->InOutBuffer.LV.Length;
        pOutRespVerifySignature->SignatureValidity = pStSafeA->InOutBuffer.LV.Data[0];
      }
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_EstablishKey(
  StSafeA_Handle_t *pStSafeA,
  uint8_t InKeySlotNum,
  const StSafeA_LVBuffer_t *pInPubX,
  const StSafeA_LVBuffer_t *pInPubY,
  uint16_t InSharedSecretLength,
  StSafeA_SharedSecretBuffer_t *pOutSharedSecret,
  uint8_t InMAC,
  uint8_t InHostEncryption)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA)              &&
      (pInPubX != NULL)                                   &&
      (pInPubY != NULL)                                   &&
      (pOutSharedSecret != NULL)                          &&
      IS_STSAFEA_CONDITIONAL_VALID_PTR(&pOutSharedSecret->SharedKey))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));
    stsafea_assert_param(IS_STSAFEA_KEY_SLOT(InKeySlotNum));
    stsafea_assert_param(IS_STSAFEA_ENCRYPT(InHostEncryption));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_ESTABLISH_KEY, &InMAC);

    /* Private key slot number */
    pStSafeA->InOutBuffer.LV.Data[0] = (uint8_t)InKeySlotNum;
    /* Point representation identifier */
    pStSafeA->InOutBuffer.LV.Data[1] = STSAFEA_POINT_REPRESENTATION_ID;
    pStSafeA->InOutBuffer.LV.Length = 2U;

    /* X public key */
    if ((pInPubX->Length > 0U))
    {
      STSAFEA_CHECK_SIZE(InMAC, pStSafeA->InOutBuffer.LV.Length, pInPubX->Length + STSAFEA_LENGTH_SIZE);

      pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length] = (uint8_t)((pInPubX->Length & 0xFF00U) >> 8);
      pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length + 1U] = (uint8_t)(pInPubX->Length & 0x00FFU);
      (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length + 2U],
                   pInPubX->Data,
                   (uint32_t)pInPubX->Length);

      pStSafeA->InOutBuffer.LV.Length += (uint16_t)sizeof(pInPubX->Length) + pInPubX->Length;
    }

    /* Y public key */
    if ((pInPubY->Length > 0U))
    {
      STSAFEA_CHECK_SIZE(InMAC, pStSafeA->InOutBuffer.LV.Length, pInPubY->Length + STSAFEA_LENGTH_SIZE);

      pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length] = (uint8_t)((pInPubY->Length & 0xFF00U) >> 8);
      pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length + 1U] = (uint8_t)(pInPubY->Length & 0x00FFU);
      (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[pStSafeA->InOutBuffer.LV.Length + 2U],
                   pInPubY->Data,
                   (uint32_t)pInPubY->Length);

      pStSafeA->InOutBuffer.LV.Length += (uint16_t)sizeof(pInPubY->Length) + pInPubY->Length;
    }

#if (!STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT)
#if defined(STSAFE_A110)
    /* Encrypt data */
    if (((uint8_t)InHostEncryption & (uint8_t)STSAFEA_ENCRYPTION_COMMAND) == (uint8_t)STSAFEA_ENCRYPTION_COMMAND)
    {
      status_code = StSafeA_DataEncryption(pStSafeA);
    }
    else
    {
      status_code = STSAFEA_OK;
    }

    if (status_code == STSAFEA_OK)
#endif /* STSAFE_A110 */
#endif /* STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT */
    {
      /* Transmit command */
      status_code = StSafeA_TransmitCommand(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        /* Set response length */
        pStSafeA->InOutBuffer.LV.Length = InSharedSecretLength +
                                          STSAFEA_LENGTH_SIZE +
                                          STSAFEA_R_MAC_LENGTH(InMAC);

        /* Wait for the command processing. Then check for the response */
        StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_ESTABLISH_KEY);

        /* Read response */
        status_code = StSafeA_ReceiveResponse(pStSafeA);

#if (!STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT)
        if (status_code == STSAFEA_OK)
        {
          /* Decrypt data */
          if (((uint8_t)InHostEncryption & (uint8_t)STSAFEA_ENCRYPTION_RESPONSE) ==
                                  (uint8_t)STSAFEA_ENCRYPTION_RESPONSE)
          {
            status_code = StSafeA_DataDecryption(pStSafeA);
          }
        }
#endif /* STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT */

        if (status_code == STSAFEA_OK)
        {
          pOutSharedSecret->Length = pStSafeA->InOutBuffer.LV.Length;

          /* Assign the internal StSafeA_SharedSecretBuffer_t LVBuffer */
          status_code = StSafeA_AssignLVBuffer(&pOutSharedSecret->SharedKey,
                                               pStSafeA->InOutBuffer.LV.Data,
                                               InSharedSecretLength);
        }
      }
    }
  }

  return status_code;
}


StSafeA_ResponseCode_t StSafeA_ProductDataQuery(
  StSafeA_Handle_t *pStSafeA,
  StSafeA_ProductDataBuffer_t *pOutProductData,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA)  &&
      (pOutProductData != NULL))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_QUERY, &InMAC);

    /* Build command Data */
    pStSafeA->InOutBuffer.LV.Data[0] = (uint8_t)STSAFEA_TAG_PRODUCT_DATA;
    pStSafeA->InOutBuffer.LV.Length = 1U;

    /* Transmit */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_PRODUCT_DATA_QUERY_RESPONSE_LENGTH + STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_QUERY);

      /* Receive */
      status_code = StSafeA_ReceiveResponse(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        pOutProductData->Length = pStSafeA->InOutBuffer.LV.Length;

        uint8_t i = 0;

        /* Mask Identification */
        pOutProductData->MaskIdentificationTag    = pStSafeA->InOutBuffer.LV.Data[i];
        pOutProductData->MaskIdentificationLength = pStSafeA->InOutBuffer.LV.Data[++i];
        (void)memcpy(pOutProductData->MaskIdentification,
                     &pStSafeA->InOutBuffer.LV.Data[++i],
                     pOutProductData->MaskIdentificationLength);
        i += pOutProductData->MaskIdentificationLength;

        /* ST Number */
        pOutProductData->STNumberTag = pStSafeA->InOutBuffer.LV.Data[i];
        pOutProductData->STNumberLength = pStSafeA->InOutBuffer.LV.Data[++i];
        (void)memcpy(pOutProductData->STNumber,
                     &pStSafeA->InOutBuffer.LV.Data[++i],
                     pOutProductData->STNumberLength);
        i += pOutProductData->STNumberLength;

        /* Input output buffer length */
        pOutProductData->InputOutputBufferSizeTag = pStSafeA->InOutBuffer.LV.Data[i];
        pOutProductData->InputOutputBufferSizeLength = pStSafeA->InOutBuffer.LV.Data[++i];
        (void)memcpy(&pOutProductData->InputOutputBufferSize,
                     &pStSafeA->InOutBuffer.LV.Data[++i],
                     pOutProductData->InputOutputBufferSizeLength);
        pOutProductData->InputOutputBufferSize = (uint16_t)SWAP2BYTES(pOutProductData->InputOutputBufferSize);
        i += 2U;

        /* Atomicity buffer length */
        pOutProductData->AtomicityBufferSizeTag = pStSafeA->InOutBuffer.LV.Data[i];
        pOutProductData->AtomicityBufferSizeLength = pStSafeA->InOutBuffer.LV.Data[++i];
        (void)memcpy(&pOutProductData->AtomicityBufferSize,
                     &pStSafeA->InOutBuffer.LV.Data[++i],
                     pOutProductData->AtomicityBufferSizeLength);
        pOutProductData->AtomicityBufferSize = (uint16_t)SWAP2BYTES(pOutProductData->AtomicityBufferSize);
        i += 2U;

        /* Size of non-volatile memory */
        pOutProductData->NonVolatileMemorySizeTag = pStSafeA->InOutBuffer.LV.Data[i];
        pOutProductData->NonVolatileMemorySizeLength = pStSafeA->InOutBuffer.LV.Data[++i];
        (void)memcpy(&pOutProductData->NonVolatileMemorySize,
                     &pStSafeA->InOutBuffer.LV.Data[++i],
                     pOutProductData->NonVolatileMemorySizeLength);
        pOutProductData->NonVolatileMemorySize = (uint16_t)SWAP2BYTES(pOutProductData->NonVolatileMemorySize);
        i += 2U;

        /* Test date */
        pOutProductData->TestDateTag = pStSafeA->InOutBuffer.LV.Data[i];
        pOutProductData->TestDateLength = pStSafeA->InOutBuffer.LV.Data[++i];
        (void)memcpy(&pOutProductData->TestDateSize,
                     &pStSafeA->InOutBuffer.LV.Data[++i],
                     pOutProductData->TestDateLength);
        pOutProductData->TestDateSize = (uint16_t)SWAP2BYTES(pOutProductData->TestDateSize);
        i += 2U;

        /* Internal product version */
        pOutProductData->InternalProductVersionTag = pStSafeA->InOutBuffer.LV.Data[i];
        pOutProductData->InternalProductVersionLength = pStSafeA->InOutBuffer.LV.Data[++i];
        pOutProductData->InternalProductVersionSize = pStSafeA->InOutBuffer.LV.Data[++i];

        /* Module date */
        pOutProductData->ModuleDateTag = pStSafeA->InOutBuffer.LV.Data[++i];
        pOutProductData->ModuleDateLength = pStSafeA->InOutBuffer.LV.Data[++i];
        (void)memcpy(&pOutProductData->ModuleDateSize,
                     &pStSafeA->InOutBuffer.LV.Data[++i],
                     pOutProductData->ModuleDateLength);
        pOutProductData->ModuleDateSize = (uint16_t)SWAP2BYTES(pOutProductData->ModuleDateSize);
        i += 2U;
      }
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_I2cParameterQuery(
  StSafeA_Handle_t *pStSafeA,
  StSafeA_I2cParameterBuffer_t *pOutI2CParamData,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA)  &&
      (pOutI2CParamData != NULL))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_QUERY, &InMAC);

    /* Build command Data */

    pStSafeA->InOutBuffer.LV.Data[0] = (uint8_t)STSAFEA_TAG_I2C_PARAMETER;
    pStSafeA->InOutBuffer.LV.Length = 1U;

    /* Transmit */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_I2C_PARAMETERS_QUERY_RESPONSE_LENGTH + STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_QUERY);

      /* Receive */
      status_code = StSafeA_ReceiveResponse(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        pOutI2CParamData->Length               = pStSafeA->InOutBuffer.LV.Length;
        pOutI2CParamData->I2cAddress           = pStSafeA->InOutBuffer.LV.Data[0] >> 1;
        pOutI2CParamData->LockConfig           = pStSafeA->InOutBuffer.LV.Data[1]  & STSAFEA_I2C_LOCK_MSK;   /* 1-bit: the 8th  */
        pOutI2CParamData->LowPowerModeConfig   = pStSafeA->InOutBuffer.LV.Data[1]  & STSAFEA_I2C_LPMODE_MSK;   /* 2-bits : the 6th and 7th */
      }
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_LifeCycleStateQuery(
  StSafeA_Handle_t *pStSafeA,
  StSafeA_LifeCycleStateBuffer_t *pOutLifeCycleState,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA) &&
      (pOutLifeCycleState != NULL))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_QUERY, &InMAC);

    /* Build command Data */
    pStSafeA->InOutBuffer.LV.Data[0] = (uint8_t)STSAFEA_TAG_LIFE_CYCLE_STATE;
    pStSafeA->InOutBuffer.LV.Length = 1U;

    /* Transmit */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_LIFE_CYCLE_STATE_QUERY_RESPONSE_LENGTH + STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_QUERY);

      /* Receive */
      status_code = StSafeA_ReceiveResponse(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        pOutLifeCycleState->Length          = pStSafeA->InOutBuffer.LV.Length;
        pOutLifeCycleState->LifeCycleStatus = pStSafeA->InOutBuffer.LV.Data[0];
      }
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_HostKeySlotQuery(
  StSafeA_Handle_t *pStSafeA,
  StSafeA_HostKeySlotBuffer_t *pOutHostKeySlot,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA) &&
      (pOutHostKeySlot != NULL))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_QUERY, &InMAC);

    /* Build command data */
    pStSafeA->InOutBuffer.LV.Data[0] = (uint8_t)STSAFEA_TAG_HOST_KEY_SLOT;
    pStSafeA->InOutBuffer.LV.Length = 1U;

    /* Transmit */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_HOST_KEY_SLOT_QUERY_RESPONSE_LENGTH + STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_QUERY);

      /* Receive */
      status_code = StSafeA_ReceiveResponse(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        pOutHostKeySlot->Length = pStSafeA->InOutBuffer.LV.Length;
        pOutHostKeySlot->HostKeyPresenceFlag = pStSafeA->InOutBuffer.LV.Data[0];

        /* Only 3-bytes for MAC Counter */
        pOutHostKeySlot->HostCMacSequenceCounter = STSAFEA_HOST_CMAC_INVALID_COUNTER;
        if (pOutHostKeySlot->HostKeyPresenceFlag != 0U)
        {
          pOutHostKeySlot->HostCMacSequenceCounter  = ((uint32_t)pStSafeA->InOutBuffer.LV.Data[1]) << 16;
          pOutHostKeySlot->HostCMacSequenceCounter |= ((uint32_t)pStSafeA->InOutBuffer.LV.Data[2]) << 8;
          pOutHostKeySlot->HostCMacSequenceCounter |= pStSafeA->InOutBuffer.LV.Data[3];
        }
      }
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_PutAttribute(
  StSafeA_Handle_t *pStSafeA,
  uint8_t InAttributeTag,
  const uint8_t *pInData,
  uint16_t InDataSize,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA) &&
      (pInData != NULL))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));
    stsafea_assert_param(IS_STSAFEA_TAG(InAttributeTag));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_PUT_ATTRIBUTE, &InMAC);

    /* Attribute Tag */
    pStSafeA->InOutBuffer.LV.Data[0] = InAttributeTag;
    /* Use limit */
    if (InDataSize > 0U)
    {
      STSAFEA_CHECK_SIZE(InMAC, 1U, InDataSize);
      (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[1], pInData, InDataSize);
    }
    pStSafeA->InOutBuffer.LV.Length = 1U + InDataSize;

    /* Transmit */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_PUT_ATTRIBUTE);

      /* Receive */
      status_code = StSafeA_ReceiveResponse(pStSafeA);
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_DeletePassword(
  StSafeA_Handle_t *pStSafeA,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));


    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_DELETE_KEY, &InMAC);
    pStSafeA->InOutBuffer.LV.Data[0] = (uint8_t)STSAFEA_TAG_HOST_KEY_SLOT;
    pStSafeA->InOutBuffer.LV.Length = 1U;


    /* Transmit */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_DELETE_KEY);

      /* Receive */
      status_code = StSafeA_ReceiveResponse(pStSafeA);
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_VerifyPassword(
  StSafeA_Handle_t *pStSafeA,
  const uint8_t *pInPassword,
  StSafeA_VerifyPasswordBuffer_t *pOutVerifyPassword,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA) &&
      (pInPassword != NULL)                  &&
      (pOutVerifyPassword != NULL))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_VERIFY_PASSWORD, &InMAC);

    /* Build command data */
    /* Password */
    (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[0], pInPassword, STSAFEA_PASSWORD_SIZE);

    pStSafeA->InOutBuffer.LV.Length = STSAFEA_PASSWORD_SIZE;

    /* Transmit */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_VERIFY_PASSWORD_RESPONSE_LENGTH + STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_VERIFY_PASSWORD);

      /* Receive */
      status_code = StSafeA_ReceiveResponse(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        pOutVerifyPassword->Length = pStSafeA->InOutBuffer.LV.Length;
        pOutVerifyPassword->VerificationStatus = pStSafeA->InOutBuffer.LV.Data[0];
        pOutVerifyPassword->RemainingTries     = pStSafeA->InOutBuffer.LV.Data[1];;

      }
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_RawCommand(
  StSafeA_Handle_t *pStSafeA,
  const StSafeA_TLVBuffer_t *pInRawCommand,
  uint16_t InRespDataLen,
  StSafeA_TLVBuffer_t *pOutTLVResponse,
  uint32_t DelayMs,
  uint8_t InMAC)
{

  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA)                  &&
      (pInRawCommand != NULL)                                 &&
      (pOutTLVResponse != NULL)                               &&
      IS_STSAFEA_CONDITIONAL_VALID_PTR(&pOutTLVResponse->LV))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));
    stsafea_assert_param(IS_STSAFEA_LEN(InRespDataLen));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, pInRawCommand->Header, &InMAC);

    /* Build command */
    STSAFEA_CHECK_SIZE(InMAC, 0U, pInRawCommand->LV.Length);
    pStSafeA->InOutBuffer.LV.Length = pInRawCommand->LV.Length;
    (void)memcpy(pStSafeA->InOutBuffer.LV.Data, pInRawCommand->LV.Data, pInRawCommand->LV.Length);

    /* Transmit */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(DelayMs);

      /* Receive */
      status_code = StSafeA_ReceiveResponse(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        /* Assign the received response to the output parameter */
        pOutTLVResponse->Header = pStSafeA->InOutBuffer.Header;
        status_code = StSafeA_AssignLVResponse(&pOutTLVResponse->LV, &pStSafeA->InOutBuffer.LV,  InRespDataLen);
      }
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_LocalEnvelopeKeySlotQuery(
  StSafeA_Handle_t *pStSafeA,
  StSafeA_LocalEnvelopeKeyTableBuffer_t *pOutLocalEnvelopeKeyTable,
  StSafeA_LocalEnvelopeKeyInformationRecordBuffer_t *pOutLlocalEnvelopeKeySlot0InformationRecord,
  StSafeA_LocalEnvelopeKeyInformationRecordBuffer_t *pOutLlocalEnvelopeKeySlot1InformationRecord,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA)                 &&
      (pOutLocalEnvelopeKeyTable != NULL)                    &&
      (pOutLlocalEnvelopeKeySlot0InformationRecord != NULL)  &&
      (pOutLlocalEnvelopeKeySlot1InformationRecord != NULL))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_QUERY, &InMAC);

    /* Build command Data */
    pStSafeA->InOutBuffer.LV.Data[0] = (uint8_t)STSAFEA_TAG_LOCAL_ENVELOPE_KEY_TABLE;
    pStSafeA->InOutBuffer.LV.Length = 1U;

    /* Transmit */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_LOCAL_ENVELOPE_QUERY_MIN_RESPONSE_LENGTH + STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_QUERY);

      /* Receive */
      status_code = StSafeA_ReceiveResponse(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        uint8_t idx = 0;
        pOutLocalEnvelopeKeyTable->Length = pStSafeA->InOutBuffer.LV.Length;
        pOutLocalEnvelopeKeyTable->NumberOfSlots = pStSafeA->InOutBuffer.LV.Data[idx];

        if (pOutLocalEnvelopeKeyTable->NumberOfSlots != 0U)
        {
          /* Fill the Slot-0 Information record */
          idx++;
          pOutLlocalEnvelopeKeySlot0InformationRecord->SlotNumber   = pStSafeA->InOutBuffer.LV.Data[idx];
          idx++;
          pOutLlocalEnvelopeKeySlot0InformationRecord->PresenceFlag = pStSafeA->InOutBuffer.LV.Data[idx];
          if (pOutLlocalEnvelopeKeySlot0InformationRecord->PresenceFlag != 0U)
          {
            idx++;
            pOutLlocalEnvelopeKeySlot0InformationRecord->KeyLength = pStSafeA->InOutBuffer.LV.Data[idx];
          }

          /* Fill the Slot-1 Information record */
          idx++;
          pOutLlocalEnvelopeKeySlot1InformationRecord->SlotNumber   = pStSafeA->InOutBuffer.LV.Data[idx];
          idx++;
          pOutLlocalEnvelopeKeySlot1InformationRecord->PresenceFlag = pStSafeA->InOutBuffer.LV.Data[idx];
          if (pOutLlocalEnvelopeKeySlot1InformationRecord->PresenceFlag != 0U)
          {
            idx++;
            pOutLlocalEnvelopeKeySlot1InformationRecord->KeyLength = pStSafeA->InOutBuffer.LV.Data[idx];
          }
        }
      }
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_GenerateLocalEnvelopeKey(
  StSafeA_Handle_t *pStSafeA,
  uint8_t InKeySlotNum,
  uint8_t InKeyType,
  uint8_t *pInSeed,
  uint16_t InSeedSize,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));
    stsafea_assert_param(IS_STSAFEA_KEY_SLOT(InKeySlotNum));
    stsafea_assert_param(IS_STSAFEA_KEY_TYPE(InKeyType));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_GENERATE_KEY, &InMAC);

    /* Attribute Tag */
    pStSafeA->InOutBuffer.LV.Data[0] = (uint8_t)STSAFEA_TAG_LOCAL_ENVELOPE_KEY_TABLE;

    /* Slot number */
    pStSafeA->InOutBuffer.LV.Data[1] = (uint8_t)InKeySlotNum;

    /* AES key types */
    pStSafeA->InOutBuffer.LV.Data[2] = (uint8_t)InKeyType;

    /*Seed (can be optional, so the related pointer is not checked at the beginning)*/
    uint16_t seed_size = InSeedSize;
    if ((seed_size > 0U)  && (pInSeed != NULL))
    {
      STSAFEA_CHECK_SIZE(InMAC, 3U, seed_size);
      (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[3], pInSeed, seed_size);
    }

    pStSafeA->InOutBuffer.LV.Length = 3U + seed_size;

    /* Transmit */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_GENERATE_KEY);

      /* Receive */
      status_code = StSafeA_ReceiveResponse(pStSafeA);
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_WrapLocalEnvelope(
  StSafeA_Handle_t *pStSafeA,
  uint8_t InKeySlotNum,
  uint8_t *pInData,
  uint16_t InDataSize,
  StSafeA_LVBuffer_t *pOutLVResponse,
  uint8_t InMAC,
  uint8_t InHostEncryption)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

#if defined(STSAFE_A100)
  STSAFEA_UNUSED_VAR(InHostEncryption);
#endif /* STSAFE_A100 */

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));
    stsafea_assert_param(IS_STSAFEA_KEY_SLOT(InKeySlotNum));
    stsafea_assert_param(IS_STSAFEA_ENCRYPT(InHostEncryption));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_WRAP_LOCAL_ENVELOPE, &InMAC);

    /* Build command data */
    pStSafeA->InOutBuffer.LV.Data[0] = (uint8_t)InKeySlotNum;

    if ((pInData != NULL) && (InDataSize > 0U))
    {
      STSAFEA_CHECK_SIZE(InMAC, 1U, InDataSize);
      (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[1], pInData, InDataSize);
    }
    pStSafeA->InOutBuffer.LV.Length = InDataSize + 1U;

#if (!STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT)
#if defined(STSAFE_A100)
    status_code = StSafeA_DataEncryption(pStSafeA);
#else
    /* Encrypt data */
    if (((uint8_t)InHostEncryption & (uint8_t)STSAFEA_ENCRYPTION_COMMAND) == (uint8_t)STSAFEA_ENCRYPTION_COMMAND)
    {
      status_code = StSafeA_DataEncryption(pStSafeA);
    }
    else
    {
      status_code = STSAFEA_OK;
    }
#endif /* STSAFE_A100 */
    if (status_code == STSAFEA_OK)
#endif /* STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT */
    /* Transmit */
    {
      status_code = StSafeA_TransmitCommand(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        /* Set response length */
        pStSafeA->InOutBuffer.LV.Length = InDataSize +
                                          STSAFEA_WRAP_UNWRAP_ENVELOPE_ADDITIONAL_RESPONSE_LENGTH +
                                          STSAFEA_R_MAC_LENGTH(InMAC);

        /* Wait for the command processing. Then check for the response */
        StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_WRAP_LOCAL_ENVELOPE);

        /* Receive */
        status_code = StSafeA_ReceiveResponse(pStSafeA);

        if (status_code == STSAFEA_OK)
        {
          /* Local Envelope response data is 8-bytes longer than the working key (see User Manual).
            Need to compensate the input length before the assignment function call.
            WARNING: Must be clear at App level to allocate additional 8-Bytes to avoid overflow!*/


          /* Assign the internal StSafeA_SharedSecretBuffer_t LVBuffer */
          status_code = StSafeA_AssignLVResponse(pOutLVResponse, &pStSafeA->InOutBuffer.LV, InDataSize + 8U);
        }
      }
    }
  }

  return status_code;
}

StSafeA_ResponseCode_t StSafeA_Write(StSafeA_Handle_t* handle, uint8_t *ptr, uint16_t len) {
	StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;
	uint8_t zone = 1;
	uint8_t offset = 0;
	StSafeA_LVBuffer_t in_Data;

	in_Data.Data = ptr;
	in_Data.Length = len;

	status_code = StSafeA_Update(handle, 0, 0, 0, 0, zone, offset, &in_Data, STSAFEA_MAC_HOST_CMAC);

	return status_code;
}

StSafeA_ResponseCode_t StSafeA_ReadZone(StSafeA_Handle_t* handle, uint8_t *output, uint16_t len) {
	StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;
	uint8_t zone = 1;
	uint8_t offset = 0;
	StSafeA_LVBuffer_t sts_read;

	sts_read.Data = output;
	sts_read.Length = len;

	status_code = StSafeA_Read(handle, 0, 0, STSAFEA_AC_ALWAYS, zone,offset, len, len, &sts_read, STSAFEA_MAC_HOST_CMAC);

	return status_code;
}

StSafeA_ResponseCode_t StSafeA_UnwrapLocalEnvelope(
  StSafeA_Handle_t *pStSafeA,
  uint8_t InKeySlotNum,
  uint8_t *pInLocalEnvelope,
  uint16_t InLocalEnvelopeSize,
  StSafeA_LVBuffer_t *pOutLVResponse,
  uint8_t InMAC,
  uint8_t InHostEncryption)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));
    stsafea_assert_param(IS_STSAFEA_KEY_SLOT(InKeySlotNum));
    stsafea_assert_param(IS_STSAFEA_ENCRYPT(InHostEncryption));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_UNWRAP_LOCAL_ENVELOPE, &InMAC);

    /* Build command data */
    pStSafeA->InOutBuffer.LV.Data[0] = (uint8_t)InKeySlotNum;

    if ((pInLocalEnvelope != NULL) && (InLocalEnvelopeSize > 0U))
    {
      STSAFEA_CHECK_SIZE(InMAC, 1U, InLocalEnvelopeSize);
      (void)memcpy(&pStSafeA->InOutBuffer.LV.Data[1], pInLocalEnvelope, InLocalEnvelopeSize);
    }
    pStSafeA->InOutBuffer.LV.Length = InLocalEnvelopeSize + 1U;

    /* Transmit */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = InLocalEnvelopeSize +
                                        STSAFEA_WRAP_UNWRAP_ENVELOPE_ADDITIONAL_RESPONSE_LENGTH +
                                        STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_UNWRAP_LOCAL_ENVELOPE);

      /* Receive */
      status_code = StSafeA_ReceiveResponse(pStSafeA);

#if (!STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT)
      if (status_code == STSAFEA_OK)
      {
        /* Decrypt data */
        if (((uint8_t)InHostEncryption & (uint8_t)STSAFEA_ENCRYPTION_RESPONSE) == (uint8_t)STSAFEA_ENCRYPTION_RESPONSE)
        {
          status_code = StSafeA_DataDecryption(pStSafeA);
        }
      }
#endif /* STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT */

      if (status_code == STSAFEA_OK)
      {
        /* Assign the received response to the output parameter */
        status_code = StSafeA_AssignLVResponse(pOutLVResponse, &pStSafeA->InOutBuffer.LV,  InLocalEnvelopeSize);
      }
    }
  }

  return status_code;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief   StSafeA_TransmitCommand
  *          Static function to transmit the already prepared command to the STSAFE-A device.
  *
  * @param   pStSafeA      : STSAFE-A1xx object pointer.
  * @retval  STSAFEA_OK if success, an error code otherwise.
  */
static StSafeA_ResponseCode_t StSafeA_TransmitCommand(StSafeA_Handle_t *pStSafeA)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;
  if (pStSafeA != NULL)
  {
    status_code = StSafeA_MAC_SHA_PrePostProcess(pStSafeA, STSAFEA_MAC_SHA_PRE_PROCESS);
    if (status_code == STSAFEA_OK)
    {
      status_code = StSafeA_Transmit(&pStSafeA->InOutBuffer, pStSafeA->CrcSupport);
    }
  }

  return status_code;
}

/**
  * @brief   StSafeA_ReceiveResponse
  *          Static function to receive the esponse from the STSAFE-A device, after a command has been transmitted.
  *
  * @param   pStSafeA      : STSAFE-A1xx object pointer.
  * @retval  STSAFEA_OK if success, an error code otherwise.
  */
static StSafeA_ResponseCode_t StSafeA_ReceiveResponse(StSafeA_Handle_t *pStSafeA)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;
  if (pStSafeA != NULL)
  {

    status_code = StSafeA_Receive(&pStSafeA->InOutBuffer, pStSafeA->CrcSupport);

    if (status_code == STSAFEA_OK)
    {
      pStSafeA->MacCounter ++;

      status_code = StSafeA_MAC_SHA_PrePostProcess(pStSafeA, STSAFEA_MAC_SHA_POST_PROCESS);

    }
  }

  return status_code;
}

/**
  * @brief   StSafeA_AssignLVResponse
  *          Static function used to assign the  LV structure from the received response.
  *
  * @param   pDestLVBuffer : Pointer to destination LV structure to be filled.
  * @param   pSrcLVBuffer  : Pointer to source LV structure to fill from.
  * @param   ExpectedLen   : Expected data length.
  *                          If STSAFEA_USE_OPTIMIZATION_SHARED_RAM = 0, must correspond to the memory allocated for pDestLVBuffer->Data.
  * @retval  STSAFEA_OK if success, an error code otherwise.
  */
static StSafeA_ResponseCode_t StSafeA_AssignLVResponse(StSafeA_LVBuffer_t *pDestLVBuffer,
                                                       StSafeA_LVBuffer_t *pSrcLVBuffer,
                                                       uint16_t ExpectedLen)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;
  if ((pDestLVBuffer != NULL)  && (pSrcLVBuffer != NULL))
  {
    /* Check if the length value received by STSAFE is what it was expected */
    if (pSrcLVBuffer->Length > ExpectedLen)
    {
      return STSAFEA_INVALID_RESP_LENGTH;
    }

    status_code = STSAFEA_OK;

    /* The length value received by STSAFE is assigned anyway, to allow the application to re-try with the correct parameter */
    pDestLVBuffer->Length = pSrcLVBuffer->Length;

    /* Set the Data of the LVBuffer according to the selected Shared Ram optimization */
    StSafeA_SetLVData(pDestLVBuffer, pSrcLVBuffer->Data, pDestLVBuffer->Length);
  }
  return status_code;
}

/**
  * @brief   StSafeA_AssignLVBuffer
  *          Static function used to assign the  LV structure from a bytes array.
  *
  * @param   pDestLVBuffer : Pointer to destination LV structure to be filled.
  * @param   pDataBuffer   : Pointer to source bytes array to fill from.
  * @param   ExpectedLen   : Expected data length.
  *                          If STSAFEA_USE_OPTIMIZATION_SHARED_RAM = 0, must correspond to the memory allocated for pDestLVBuffer->Data.
  * @retval  STSAFEA_OK if success, an error code otherwise.
  */
static StSafeA_ResponseCode_t StSafeA_AssignLVBuffer(StSafeA_LVBuffer_t *pDestLVBuffer,
                                                     uint8_t *pDataBuffer,
                                                     uint16_t ExpectedLen)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;
  if ((pDestLVBuffer != NULL)  && (pDataBuffer != NULL))
  {
    /* The length value is assigned anyway, to allow the application to re-try with the correct parameter, in case */
    pDestLVBuffer->Length = (((uint16_t)pDataBuffer[0U]) << 8) + pDataBuffer[1U];

    /* Check if the length value is what it was expected */
    if (pDestLVBuffer->Length > ExpectedLen)
    {
      return STSAFEA_INVALID_RESP_LENGTH;
    }
    else
    {
      status_code = STSAFEA_OK;
    }

    /* Set the Data of the LVBuffer according to the selected Shared Ram optimization */
    StSafeA_SetLVData(pDestLVBuffer, &pDataBuffer[2U], pDestLVBuffer->Length);
  }
  return status_code;
}

/**
  * @brief   StSafeA_SetLVData
  *          Static function used to set the LV structure data according to the STSAFEA_USE_OPTIMIZATION_SHARED_RAM configuration.\n
  *         Used by StSafeA_AssignLVResponse and StSafeA_AssignLVBuffer functions.
  *
  * @param   pDestLVBuffer : Pointer to destination LV structure to be filled.
  * @param   pSrcLVData    : Pointer to source bytes array to fill from.
  * @param   Length        : If STSAFEA_USE_OPTIMIZATION_SHARED_RAM = 0, must correspond to the memory allocated for pDestLVBuffer->Data.
  * @retval  none
  */
static void StSafeA_SetLVData(StSafeA_LVBuffer_t *pDestLVBuffer, uint8_t *pSrcLVData,  uint16_t Length)
{
#if (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
  STSAFEA_UNUSED_VAR(Length);
  /* In this case the direct pointer assignment allows the best memory optimization.
     But up to the caller to manage this memory content properly, copying it to a local buffer if needed */
  pDestLVBuffer->Data = pSrcLVData;
#else
  /* In this case a copy from buffer to buffer is done. It's expected that the
     application have been already allocated the right memory for the pDestLVData.
     Only the expected length of data is copied, but the actual length provided by STSAFE is
     returned, so up to the application to re-allocate properly and re-try  */
  if ((Length > 0U) && (pDestLVBuffer->Data != NULL))
  {
    (void)memcpy(pDestLVBuffer->Data, pSrcLVData, Length);
  }
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
}

/**
  * @brief   StSafeA_BuildCommandHeaderCMAC
  *          Static function used to Build the Command Header and set the C-MAC command field.
  *
  * @param   pStSafeA    : STSAFE-A1xx object pointer.
  * @param   CommandCode : Command code.
  * @param   pMAC        : Pointer to MAC authenticating command/response.
  * @retval  none
  */
static void StSafeA_BuildCommandHeaderCMAC(StSafeA_Handle_t *pStSafeA, uint8_t CommandCode, uint8_t *pMAC)
{
  if ((pStSafeA != NULL) && (pMAC != NULL))
  {
#if (STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT)
    *pMAC = STSAFEA_MAC_NONE;
#else
    /* Get CMAC counter value */
    if ((*pMAC & STSAFEA_CMD_HEADER_SCHN_HOSTEN) != 0x00U)
    {
      StSafeA_GetHostMacSequenceCounter(pStSafeA);
    }
#endif /* STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT */

    pStSafeA->InOutBuffer.Header = (CommandCode | (*pMAC & STSAFEA_CMD_HEADER_MAC_MSK));
  }
}

#if (!STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT)
/**
  * @brief   StSafeA_GetHostMacSequenceCounter
  *          Static function used to retrieve the Host MAC sequence counter and update it's value into the pStSafeA handler pointer.
  *
  * @param   pStSafeA    : STSAFE-A1xx object pointer.
  * @retval  none
  */
static void StSafeA_GetHostMacSequenceCounter(StSafeA_Handle_t *pStSafeA)
{
  StSafeA_HostKeySlotBuffer_t host_key_slot;

  if (pStSafeA != NULL)
  {
    if ((StSafeA_HostKeySlotQuery(pStSafeA, &host_key_slot, STSAFEA_MAC_NONE) == STSAFEA_OK)
        && (host_key_slot.HostKeyPresenceFlag != 0U))
    {

      pStSafeA->HostMacSequenceCounter = host_key_slot.HostCMacSequenceCounter;
    }
  }
}
#endif /* STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT */

/**
  * @}
  */

#if defined(STSAFE_A110)

/* STSAFE-A110 Private functions ---------------------------------------------*/

StSafeA_ResponseCode_t StSafeA_CommandAuthorizationConfigurationQuery(
  StSafeA_Handle_t *pStSafeA,
  uint8_t InCmdAuthRecordNum,
  StSafeA_CommandAuthorizationConfigurationBuffer_t *pOutCmdAuthConfig,
  uint8_t InMAC)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;

  /* Check null pointers */
  if (IS_STSAFEA_HANDLER_VALID_PTR(pStSafeA) && (pOutCmdAuthConfig != NULL))
  {
    /* Check parameters */
    stsafea_assert_param(IS_STSAFEA_MAC(InMAC));

    /* Build command Header and process CMAC */
    StSafeA_BuildCommandHeaderCMAC(pStSafeA, STSAFEA_CMD_QUERY, &InMAC);

    /* Build command Data */
    pStSafeA->InOutBuffer.LV.Data[0] = (uint8_t)STSAFEA_TAG_COMMAND_AUTHORIZATION_CONFIGURATION;
    pStSafeA->InOutBuffer.LV.Length = 1U;

    /* Transmit */
    status_code = StSafeA_TransmitCommand(pStSafeA);

    if (status_code == STSAFEA_OK)
    {
      /* Set response length */
      pStSafeA->InOutBuffer.LV.Length = STSAFEA_CMD_AUTH_CONFIG_QUERY_MIN_RESPONSE_LENGTH +
                                        STSAFEA_R_MAC_LENGTH(InMAC);

      /* Wait for the command processing. Then check for the response */
      StSafeA_Delay(STSAFEA_MS_WAIT_TIME_CMD_QUERY);

      /* Receive */
      status_code = StSafeA_ReceiveResponse(pStSafeA);

      if (status_code == STSAFEA_OK)
      {
        uint8_t uIdx = 0U;
        pOutCmdAuthConfig->Length = pStSafeA->InOutBuffer.LV.Length;
        pOutCmdAuthConfig->ChangeRight = pStSafeA->InOutBuffer.LV.Data[uIdx];
        uIdx++;
        pOutCmdAuthConfig->CommandAuthorizationRecordNumber = pStSafeA->InOutBuffer.LV.Data[uIdx];
        uIdx++;

        if (pOutCmdAuthConfig->CommandAuthorizationRecordNumber > InCmdAuthRecordNum)
        {
          /* Return an error so the application can read the correct number of Records,
             allocate the right space (if not under STSAFEA_USE_OPTIMIZATION_SHARED_RAM optimization),
             and call back this API with the right InCmdAuthRecordNum parameter */
          status_code = STSAFEA_INVALID_RESP_LENGTH;
        }
        else
        {
#if (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
          /* In this case the direct pointer assignment allows the best memory optimization.
             But up to the caller to manage this memory content properly, copying it to a local buffer if needed */
          pOutCmdAuthConfig->pCommandAuthorizationRecord =
                                  (StSafeA_CommandAuthorizationRecordBuffer_t *)&pStSafeA->InOutBuffer.LV.Data[uIdx];
#else
          /* In this case a copy from buffer to buffer is done. It's expected that the
            application have been already allocated the right memory for the StSafeA_CommandAuthorizationRecordBuffer[] */
          if (pOutCmdAuthConfig->CommandAuthorizationRecordNumber > 0U)
          {
            if (pOutCmdAuthConfig->pCommandAuthorizationRecord != NULL)
            {
              uint8_t record_max_len = (uint8_t)sizeof(StSafeA_CommandAuthorizationRecordBuffer_t);
              for (uint8_t i = 0; i < pOutCmdAuthConfig->CommandAuthorizationRecordNumber; i++)
              {
                pOutCmdAuthConfig->pCommandAuthorizationRecord[i].CommandCode =
                                  pStSafeA->InOutBuffer.LV.Data[uIdx];
                pOutCmdAuthConfig->pCommandAuthorizationRecord[i].CommandAC =
                                  pStSafeA->InOutBuffer.LV.Data[uIdx + 1U];
                pOutCmdAuthConfig->pCommandAuthorizationRecord[i].HostEncryptionFlags =
                                  pStSafeA->InOutBuffer.LV.Data[uIdx + 2U];
                uIdx += record_max_len;
              }
            }
            else
            {
              status_code = STSAFEA_INVALID_PARAMETER;
            }
          }
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
        }
      }
    }
  }

  return status_code;
}
/**
  * @}
  */

/**
  * @}
  */

#endif /* STSAFE_A110 */

/**
  * @}
  */

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
