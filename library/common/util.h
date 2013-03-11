/**
 * @file		util.h
 * @brief		Common Utility Function Set Source File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef _UTIL_H
#define _UTIL_H

//#include "common/common.h"

/**
 * @defgroup macro_util Macro Util
 * Simple Macro Utilities.
 * @ingroup common_util
 * @{
 * @def MEM_FREE
 * If pointer is not NULL, free it and set to NULL.
 * @def BITSET
 * Set bit in variable.
 * @def BITCLR
 * Clear bit in variable.
 * @typedef long2char
 * General purpose type change union (32bit <-> 8bit).
 * @typedef short2char
 * General purpose type change union (16bit <-> 8bit).
 * @}
 * @typedef void_func
 * General purpose void function form.
 * @typedef alarm_cbfunc
 * Alarm call back function form.
 * @}
 */
#define MEM_FREE(mem_p) do{ if(mem_p) { free(mem_p); mem_p = NULL; } }while(0)
#define BITSET(var_v, bit_v) (var_v |= bit_v)
#define BITCLR(var_v, bit_v) (var_v &= ~(bit_v))

typedef void (*void_func)(void);
typedef void (*alarm_cbfunc)(int8 arg);
typedef union long2char_t {
	uint32	byte4;
	uint8	byte1[4];
} long2char;

typedef union short2char_t {
	uint16	s;
	uint8	c[2];
} short2char;


int8 alarm_set(uint32 time, alarm_cbfunc cb, int8 arg);
int8 alarm_del(alarm_cbfunc cb, int8 arg);
int8 alarm_chk(alarm_cbfunc cb, int8 arg);
void alarm_run(void);
int8   digit_length(int32 dgt, int8 base);
int32  str_check(int (*method)(int), int8 *str);
int8*  strsep(register int8 **stringp, register const int8 *delim);
void   print_dump(void *buf, uint16 len);
uint16 checksum(uint8 * src, uint32 len);
int32  base64_decode(int8 *text, uint8 *dst, int32 numBytes );
int32  base64_encode(int8 *text, int32 numBytes, int8 *encodedText);

#endif //_UTIL_H



