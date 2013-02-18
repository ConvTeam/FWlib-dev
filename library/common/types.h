
#ifndef _TYPE_H
#define _TYPE_H

#ifndef int8
typedef signed char int8;
#endif
#ifndef uint8
typedef unsigned char uint8;
#endif
#ifndef vint8
typedef volatile signed char vint8;
#endif
#ifndef vuint8
typedef volatile unsigned char vuint8;
#endif

#ifndef int16
typedef signed short int16;
#endif
#ifndef uint16
typedef unsigned short uint16;
#endif
#ifndef vint16
typedef volatile signed short vint16;
#endif
#ifndef vuint16
typedef volatile unsigned short vuint16;
#endif

#ifndef int32
typedef signed long int32;
#endif
#ifndef uint32
typedef unsigned long uint32;
#endif
#ifndef vint32
typedef volatile signed long vint32;
#endif
#ifndef vuint32
typedef volatile unsigned long vuint32;
#endif

#define RET_FAIL	1
#define RET_OK		0
#define RET_NOK		-1

#define VAL_HIGH	1
#define VAL_LOW		0

#define VAL_TOG		2
#define VAL_ON		1
#define VAL_OFF		0

#define VAL_SET		1
#define VAL_CLEAR	0

#define VAL_TRUE	1
#define VAL_FALSE	0

#define VAL_ENABLE	1
#define VAL_DISABLE	0

#define VAL_NONE	-1
#define VAL_INVALID	-2

#define CRLF	"\r\n"

#endif //_TYPE_H



