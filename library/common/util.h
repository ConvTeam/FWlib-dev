
#ifndef _UTIL_H
#define _UTIL_H

//#include "common/common.h"

#define MEM_FREE(mem_p) do{if(mem_p){free(mem_p);mem_p = NULL;}}while(0)


int str_check(int (*method)(int), char *str);
int base64_decode(char *text, unsigned char *dst, int numBytes );
int base64_encode(char *text, int numBytes, char *encodedText);

#endif //_UTIL_H



