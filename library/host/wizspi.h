/**
 * @file		wizspi.h
 * @brief		SPI Adaptation Layer Common Header File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *		2013/02/22 - 1.0 Release
 * @author	Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef _SPI_H
#define _SPI_H

#include "common/common.h"

typedef enum {
	WIZ_SPI1 = 0, 
	WIZ_SPI2 = 1, 
	WIZ_SPI3 = 2, 
	WIZ_SPI4 = 3
} wizpf_spi;


int8 wizspi_init(wizpf_spi spi);
void wizspi_cs(uint8 val);
uint8 wizspi_byte(uint8 byte);

#endif //_SPI_H


