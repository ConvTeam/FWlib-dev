
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

typedef uint8 			SOCKET;


/*	// « ø‰«‘ ??
#ifndef NULL
#define NULL		((void *) 0)
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

typedef enum { false, true } bool;

// bsd
typedef uint8			u_char;		//< 8-bit value
typedef uint16			u_short;		//< 16-bit value
typedef uint16			u_int;		//< 16-bit value
typedef uint32			u_long;		//< 32-bit value
*/

typedef struct _wiz_NetInfo
{
	uint8 Mac[6];
	uint8 IP[4];
	uint8 Subnet[4];
	uint8 Gateway[4];
	uint8 DNSServerIP[4];
	uint8 DHCPEnable;
} wiz_NetInfo;

typedef void (*pFunc)(void);

typedef union _var_l2c {
	uint32	l;
	uint8	c[4];
}var_l2c;

typedef union _var_i2c {
	uint16	i;
	uint8	c[2];
}var_i2c;

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

#define CRLF	"\r\n"

#endif //_TYPE_H



