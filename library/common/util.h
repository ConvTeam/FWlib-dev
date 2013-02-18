
#ifndef _UTIL_H
#define _UTIL_H

//#include "common/common.h"

#define MEM_FREE(mem_p) do{if(mem_p){free(mem_p);mem_p = NULL;}}while(0)
#define BITSET(var_v, bit_v) (var_v |= bit_v)
#define BITCLR(var_v, bit_v) (var_v &= ~(bit_v))

typedef void (*pFunc)(void);
typedef void (*alarm_cbfunc)(int8 arg);
typedef union _var_l2c {
	uint32	l;
	uint8	c[4];
}var_l2c;
typedef union _var_s2c {
	uint16	s;
	uint8	c[2];
}var_s2c;


int8 alarm_set(uint32 time, alarm_cbfunc cb, int8 arg);
int8 alarm_del(alarm_cbfunc cb, int8 arg);
int8 alarm_chk(alarm_cbfunc cb, int8 arg);
void alarm_run(void);
int8 digit_length(int32 dgt, int8 base);
int32 str_check(int (*method)(int), int8 *str);
int8 *strsep(register int8 **stringp, register const int8 *delim);
int32 base64_decode(int8 *text, uint8 *dst, int32 numBytes );
int32 base64_encode(int8 *text, int32 numBytes, int8 *encodedText);

#endif //_UTIL_H



