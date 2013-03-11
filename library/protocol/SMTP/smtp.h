/**
 * @file		smtp.h
 * @brief		SMTP (Simple Mail Transfer Protocol) Module Header File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef	_SMTP_H
#define	_SMTP_H

#include "common/common.h"


int8 send_mail(uint8 s, uint8 * sender, uint8 * passwd, uint8 * recipient, uint8 * subject, uint8 * content, uint8 * pSip);

#endif //_SMTP_H
