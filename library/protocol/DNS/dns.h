/**
 * @file		dns.h
 * @brief		DNS Protocol Module Header File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	modified by Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef	_DNS_H
#define	_DNS_H

#include "common/common.h"


int8 dns_query(uint8 sock, uint8 *domain, uint8 *ip);

#endif //_DNS_H

