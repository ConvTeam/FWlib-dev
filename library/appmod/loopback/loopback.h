/**
 * @file		loopback.h
 * @brief		Loopback Test Module Header File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef _LOOPBACK_H
#define _LOOPBACK_H

#include "common/common.h"


void loopback_tcps(uint8 sock, uint16 port);
void loopback_tcpc(uint8 sock, uint16 port);
void loopback_udp(uint8 sock, uint16 port);

#endif //_LOOPBACK_H



