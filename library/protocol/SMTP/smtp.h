
#ifndef	_SMTP_H
#define	_SMTP_H

#include "common/common.h"


uint8 send_mail(uint8 s, uint8 * sender, uint8 * passwd, uint8 * recipient, uint8 * subject, uint8 * content, uint8 * pSip);

#endif //_SMTP_H
