/**
 * @file		wizspi.c
 * @brief		SPI Adaptation Layer Source File - For STM32F10x
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *		2013/02/22 - 1.0 Release
 * @author	Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

//#define FILE_LOG_SILENCE
#include "host/wizspi.h"


static SPI_TypeDef *wizspix = NULL;

int8 wizspi_init(wizpf_spi spi)
{
	SPI_InitTypeDef SPI_InitStructure;

	switch(spi) {
	case WIZ_SPI1:
		wizspix = SPI1;
		break;
	case WIZ_SPI2:
		wizspix = SPI2;
		break;
	//case WIZ_SPI3:
	//	wizspix = ;
	//	break;
	//case WIZ_SPI4:
	//	wizspix = ;
	//	break;
	default:
		ERR("### Currently, Only \"WIZ_SPI1\" is allowed");
		return RET_NOK;
	}

	// SPI Config -------------------------------------------------------------
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(wizspix, &SPI_InitStructure);
	SPI_Cmd(wizspix, ENABLE);		// Enable SPI

	return RET_OK;
}

void wizspi_cs(uint8 val)
{
	if(wizspix != SPI1) {
		ERRA("### Currently, Only SPI1 is allowed - wizspix(%p), SPI1(%p)", wizspix, SPI1);
		return ;
	}

	if (val == VAL_LOW) {
   		GPIO_ResetBits(GPIOA, WIZ_SCS); 
	}else if (val == VAL_HIGH){
   		GPIO_SetBits(GPIOA, WIZ_SCS); 
	}
}

void wizspi_cs2(uint8 val)
{        
	if(wizspix != SPI2) {
		ERRA("### Currently, Only SPI2 is allowed - wizspix(%p), SPI2(%p)", wizspix, SPI2);
		return ;
	}

	if (val == VAL_LOW) {
   		GPIO_ResetBits(GPIOB, WIZ_SCS2); 
	}else if (val == VAL_HIGH){
   		GPIO_SetBits(GPIOB, WIZ_SCS2); 
	}
}

uint8 wizspi_byte(uint8 byte)
{               
	while (SPI_I2S_GetFlagStatus(wizspix, SPI_I2S_FLAG_TXE) == RESET);         
	SPI_I2S_SendData(wizspix, byte);          
	while (SPI_I2S_GetFlagStatus(wizspix, SPI_I2S_FLAG_RXNE) == RESET);          
	return SPI_I2S_ReceiveData(wizspix);
}







