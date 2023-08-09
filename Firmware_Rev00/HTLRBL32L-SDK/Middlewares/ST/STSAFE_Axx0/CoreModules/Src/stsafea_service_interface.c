
#include "stsafea_service.h"
#include "crc.h"
#include "i2c.h"

int32_t BSP_I2C1_WriteReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length) {
	int32_t ret;
	uint32_t hal_error;

	if(HAL_I2C_Mem_Write(&hi2c, (uint8_t)DevAddr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)pData, Length, BUS_I2C2_POLL_TIMEOUT) == HAL_OK) {
		ret = BSP_ERROR_NONE;
	} else {
		hal_error = HAL_I2C_GetError(&hi2c);
		if( hal_error == HAL_I2C_ERROR_AF)
			return BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE;
		else
			ret =  BSP_ERROR_PERIPH_FAILURE;
	}
	return ret;
}

static int32_t BSP_I2C1_Send(uint16_t DevAddr, uint8_t *pData, uint16_t Length) {
	int32_t ret;
	uint32_t hal_error;

	if (HAL_I2C_Master_Transmit(&hi2c,
			DevAddr,
			pData,
			Length,
			BUS_I2C2_POLL_TIMEOUT) == HAL_OK)
	{
		ret = BSP_ERROR_NONE;
	}
	else
	{
		hal_error = HAL_I2C_GetError(&hi2c);
		if( hal_error == HAL_I2C_ERROR_AF)
		{
			return BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE;
		}
		else
		{
			ret =  BSP_ERROR_PERIPH_FAILURE;
		}
	}
	return ret;
}

int32_t I2C_Send(uint16_t DevAddr, uint8_t *pData, uint16_t Length) {
	int32_t ret;

	ret = BSP_I2C1_Send(DevAddr, pData, Length);

	if (ret == BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE)
		return STSAFEA_BUS_NACK;

	if (ret == BSP_ERROR_PERIPH_FAILURE)
		return STSAFEA_BUS_ERR;

	return ret;
}

static int32_t BSP_I2C1_Recv(uint16_t DevAddr, uint8_t *pData, uint16_t Length) {
	int32_t ret;
	uint32_t hal_error;

	if (HAL_I2C_Master_Receive(&hi2c, DevAddr, pData, Length, BUS_I2C2_POLL_TIMEOUT) == HAL_OK) {
		ret = BSP_ERROR_NONE;
	} else {

		hal_error = HAL_I2C_GetError(&hi2c);
		if( hal_error == HAL_I2C_ERROR_AF)
			return BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE;
		else
			ret =  BSP_ERROR_PERIPH_FAILURE;
	}
	return ret;
}

int32_t I2C_Recv(uint16_t DevAddr, uint8_t *pData, uint16_t Length) {
	int32_t ret;

	ret = BSP_I2C1_Recv(DevAddr, pData, Length);

	if (ret == BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE)
		return STSAFEA_BUS_NACK;

	if (ret == BSP_ERROR_PERIPH_FAILURE)
		return STSAFEA_BUS_ERR;

	return STSAFEA_BUS_OK;
}
