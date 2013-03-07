/**
 * @file		wizspi.c
 * @brief		SPI Adaptation Layer Source File - For STM32F10x
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

//#define FILE_LOG_SILENCE
#include "host/wizspi.h"


/**
 * @addtogroup spi_module
 * @{
 */

/**
 * Initialize SPI Peripheral Device.
 * @param spi SPI index number (@ref wizpf_spi)
 * @return RET_OK: Success
 * @return RET_NOK: Error
 */
int8 wizspi_init(wizpf_spi spi)
{
	SPI_TypeDef *SPIx;
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	switch(spi) {
	case WIZ_SPI1:
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Pin = SPI1_SCS_PIN;
		GPIO_Init(SPI1_SCS_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Pin = SPI1_SCLK_PIN;
		GPIO_Init(SPI1_SCLK_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = SPI1_MISO_PIN;
		GPIO_Init(SPI1_MISO_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = SPI1_MOSI_PIN;
		GPIO_Init(SPI1_MOSI_PORT, &GPIO_InitStructure);
		GPIO_SetBits(SPI1_SCS_PORT, SPI1_SCS_PIN);
		SPIx = SPI1;
		break;
	case WIZ_SPI2:
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Pin = SPI2_SCS_PIN;
		GPIO_Init(SPI2_SCS_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Pin = SPI2_SCLK_PIN;
		GPIO_Init(SPI2_SCLK_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = SPI2_MISO_PIN;
		GPIO_Init(SPI2_MISO_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = SPI2_MOSI_PIN;
		GPIO_Init(SPI2_MOSI_PORT, &GPIO_InitStructure);
		GPIO_SetBits(SPI2_SCS_PORT, SPI2_SCS_PIN);
		SPIx = SPI2;
		break;
	//case WIZ_SPI3:
	//	break;
	default:
		ERRA("SPI(%d) is not allowed", spi);
		return RET_NOK;
	}

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPIx, &SPI_InitStructure);
	SPI_Cmd(SPIx, ENABLE);

	return RET_OK;
}

/**
 * Set/Clear SPI CS Pin
 * @param spi SPI index number (@ref wizpf_spi)
 * @param val VAL_LOW: Active(set low) \n VAL_HIGH: Inactive(set high)
 */
void wizspi_cs(wizpf_spi spi, uint8 val)
{
	GPIO_TypeDef* GPIOx;
	uint16 GPIO_Pin;

	switch(spi) {
	case WIZ_SPI1:
		GPIOx = SPI1_SCS_PORT;
		GPIO_Pin = SPI1_SCS_PIN;
		break;
	case WIZ_SPI2:
		GPIOx = SPI2_SCS_PORT;
		GPIO_Pin = SPI2_SCS_PIN;
		break;
	//case WIZ_SPI3:
	//	break;
	default:
		ERRA("SPI(%d) is not allowed", spi);
		return;
	}

	if (val == VAL_LOW) {
   		GPIO_ResetBits(GPIOx, GPIO_Pin);
	}else if (val == VAL_HIGH){
   		GPIO_SetBits(GPIOx, GPIO_Pin); 
	}
}

/**
 * Send/Receive 1 Byte through SPI
 * @param spi SPI index number (@ref wizpf_spi)
 * @param byte 1 Byte to send
 * @return Received 1 Byte
 */
uint8 wizspi_byte(wizpf_spi spi, uint8 byte)
{
	SPI_TypeDef *SPIx;

	switch(spi) {
	case WIZ_SPI1:
		SPIx = SPI1;
		break;
	case WIZ_SPI2:
		SPIx = SPI2;
		break;
	//case WIZ_SPI3:
	//	break;
	default:
		ERRA("SPI(%d) is not allowed", spi);
		return 0;
	}

	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);         
	SPI_I2S_SendData(SPIx, byte);          
	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);          
	return (uint8)SPI_I2S_ReceiveData(SPIx);
}

/* @} */






