/**
  ******************************************************************************
  * @file    wrap_unwrap.h
  * @author  SMD application team
  * @version V3.1.0
  * @brief   Wrap and unwrap envelope data use case header file.
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


#ifndef WRAP_UNWRAP_H
#define WRAP_UNWRAP_H

#include "stsafea_core.h"

int32_t wrap_unwrap(StSafeA_Handle_t* handle, uint8_t slot);
void HT_STSAFE_Write(StSafeA_Handle_t* handle);
void HT_STSAFE_EchoCmd(StSafeA_Handle_t* handle);

#endif /* WRAP_UNWRAP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
