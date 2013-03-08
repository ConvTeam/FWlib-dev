/**
 * @file		util.c
 * @brief		Common Utility Function Set Header File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

//#define FILE_LOG_SILENCE
#include "common/common.h"
//#include "common/util.h"

#define MAX_TICK_ELAPSE	0x7FFFFFFF	// Maximum elapse time you can set

typedef struct alarm_node_t {
	struct alarm_node_t *next;
	uint32 time;
	alarm_cbfunc cb;
	int8 arg;
} alarm_node;


static alarm_node *alst = NULL;

/*------ Base64 Encoding Table ------*/
static const int8 MimeBase64[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

/*------ Base64 Decoding Table ------*/
static int32 DecodeMimeBase64[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
};

/**
 * @defgroup alarm_module Alarm
 * Event Alarm Module.
 * You can register delayed action through Alarm Module.
 * @ingroup common_util
 * @{
 */

/**
 * Add Alarm event to the waiting queue. 
 * @param time Delay time in tick. \n Max time is defined in common.h 
 * 		\n Zero time param is possible
 * @param cb Callback function in @ref alarm_cbfunc form
 * @param arg The value which will be returned through callback function
 		\n This is for separation in same callback function
 * @return RET_OK: Success
 * @return RET_NOK: Error
 */
int8 alarm_set(uint32 time, alarm_cbfunc cb, int8 arg)
{
	alarm_node *aptr, **adbl;

	if(time > MAX_TICK_ELAPSE || cb == NULL) return RET_NOK;

	//if(time) DBGA("Set with delay: Time(%d), CB(%p), ARG(%d)", time, (void*)cb, arg);
	time += wizpf_get_systick();

	adbl = &alst;
	while(*adbl && (*adbl)->time <= time) {
		adbl = &(*adbl)->next;
	}

	aptr = *adbl;
	*adbl = calloc(1, sizeof(alarm_node));
	if(*adbl == NULL) {
		*adbl = aptr;
		ERRA("calloc fail - size(%d)", sizeof(alarm_node));
		return RET_NOK;
	}
	(*adbl)->next = aptr;
	(*adbl)->time = time;
	(*adbl)->cb = cb;
	(*adbl)->arg = arg;

	return RET_OK;
}

/**
 * Delete Alarm event from the waiting queue. 
 * This function delete every alarm event which have same condition with param\n
 * - alarm_del(NULL, -1) : Delete all event registered in the queue.
 *
 * @param cb Callback function in @ref alarm_cbfunc form
 *		\n NULL value will be ignored
 * @param arg The value which was set when alarm event added as arg
 		\n -1 value will be ignored
 * @return RET_OK: Success
 * @return RET_NOK: Error
 */
int8 alarm_del(alarm_cbfunc cb, int8 arg)
{
	int8 cnt = 0;
	alarm_node *aptr, **adbl = &alst;

	while(*adbl) {
		if( (cb == NULL || (cb != NULL && ((*adbl)->cb  == cb))) &&
			 (arg == -1  || (arg >= 0 && ((*adbl)->arg == arg))) )
		{
			aptr = *adbl;
			*adbl = aptr->next;
			DBGA("Del: CB(%p),ARG(%d)", (void*)aptr->cb, aptr->arg);
			free(aptr);
			cnt++;
		} else adbl = &(*adbl)->next;
	}

	return cnt;
}

/**
 * Count Alarm event which have same condition with param from the waiting queue. 
 * If there is not alarm event same with param, 0 value will be returned.
 * - alarm_chk(NULL, -1) : Count all event registered in the queue.
 *
 * @param cb Callback function in @ref alarm_cbfunc form
 *		\n NULL value will be ignored
 * @param arg The value which was set when alarm event added as arg
 *		\n -1 value will be ignored
 * @return RET_OK: Success
 * @return RET_NOK: Error
 */
int8 alarm_chk(alarm_cbfunc cb, int8 arg)
{
	int8 cnt = 0;
	alarm_node **adbl = &alst;

	while(*adbl) {
		if( (cb == NULL || (cb != NULL && ((*adbl)->cb  == cb))) &&
			 (arg == -1  || (arg >= 0 && ((*adbl)->arg == arg))) )
		{
			DBGA("Chk: CB(%p),ARG(%d)", (void*)(*adbl)->cb, (*adbl)->arg);
			cnt++;
		}
		adbl = &(*adbl)->next;
	}

	return cnt;
}

/**
 * Alarm Module Handler.
 * If you use alarm, this function should run in the main loop
 */
void alarm_run(void)
{
	uint32 cur = wizpf_get_systick();	//for DBG: static uint32 prt=999999;if(prt!=alst->time) {DBGA("cur(%d), time(%d)", wizpf_get_systick(), alst->time);prt = alst->time;}
	if(wizpf_tick_elapse(alst->time) >= 0) {
		alarm_node *aptr = alst;	//for DBG: DBGA("cb call - cur(%d), time(%d), next(%d)", wizpf_get_systick(), alst->time, (alst->next)->time);
		alst = alst->next;
		aptr->cb(aptr->arg);
		free(aptr);
	}
}

/* @} */


/**
 * @addtogroup common_util
 * @{
 */

/**
 * Count digit's letter
 * Ex) digit_length(12345, 10) : This will return 5.
 * @param dgt The digit value to count
 * @param base Digit base like 2, 8, 10, 16
 * @return >0: Counted digit letter
 * @return RET_NOK: Error
 */
int8 digit_length(int32 dgt, int8 base)
{
	int16 i, len = 0;

	if(dgt < 0) {
		len++;
		dgt *= -1;
	}

	for(i=0; i<255; i++) {
		len++;
		dgt /= base;
		if(dgt == 0) return len;
	}

	return RET_NOK;
}

/**
 * Check string with standard library method.
 * Below is the method you can use.
 * - isalpha, isupper, islower
 * - isdigit, isxdigit, isalnum
 * - isspace, ispunct, isprint
 * - isgraph, iscntrl, isascii
 *
 * Ex) str_check(isdigit, "12345") : This will return RET_OK. \n
 * Ex) str_check(islower, "AbcDe") : This will return RET_NOK.
 * @param method The method to use for check
 * @param str The string to check
 * @return RET_OK: Success
 * @return RET_NOK: Error
 */
int32 str_check(int (*method)(int), int8 *str)
{
	if(method == NULL || str == NULL || *str == 0)
		return RET_NOK;

	while(*str) {
		if(!method((int)*str)) return RET_NOK;
		str++;
	}

	return RET_OK;
}

/**
 * Separate string into small peace by delimiter like strtok.
 * But if the input string contains more than one character from delimiter \n
 * in a row, strsep returns an empty string for each pair of characters from delimiter.
 *
 * Ex) strsep("a,b,c,,,f,gh", ",") : When meet ,,, strtok returns 'f' but this returns NULL.
 *
 * @param stringp String to separate
 * @param delim Delimiter
 * @return Next pointer separated by delimiter
 */
int8 *strsep(register int8 **stringp, register const int8 *delim)
{
    register int8 *s;
    register const int8 *spanp;
    register int32 c, sc;
    int8 *tok;

    if ((s = *stringp) == NULL)
        return (NULL);
    for (tok = s;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}

/**
 * Print Binary Dump Data.
 * @param buf The data to print
 * @param len The data length
 */ 
void print_dump(void *buf, uint16 len)
{
	uint8 *tp = (uint8*)buf;
	uint16 i;
	uint16 line = len / 0x10;
	uint16 left = len % 0x10;

	LOG("===========================================================");
	LOG("-ADDR----0--1--2--3--4--5--6--7----8--9--A--B--C--D--E--F--");  
	for(i=0; i<line; i++) {
		LOGA("0x%04x   %02x %02x %02x %02x %02x %02x %02x %02x"
			"   %02x %02x %02x %02x %02x %02x %02x %02x", 0x10*i, 
			tp[0x10*i+0x0], tp[0x10*i+0x1], tp[0x10*i+0x2], tp[0x10*i+0x3], 
			tp[0x10*i+0x4], tp[0x10*i+0x5], tp[0x10*i+0x6], tp[0x10*i+0x7],
			tp[0x10*i+0x8], tp[0x10*i+0x9], tp[0x10*i+0xA], tp[0x10*i+0xB], 
			tp[0x10*i+0xC], tp[0x10*i+0xD], tp[0x10*i+0xE], tp[0x10*i+0xF]);
	}
	if(left) {
		LOGFA("0x%04x   ", 0x10*line);
		for(i=0; i<left; i++) LOGFA("%02x ", tp[0x10*line + i]);
		NL1;
	}
	LOG("===========================================================");
}

/**
 * Calculate checksum of a stream.
 * @param src The string to calculate checksum.
 * @param len The string length.
 * @return Checksum
 */ 
uint16 checksum(uint8 * src, uint32 len)
{
	uint16 sum, tsum, i, j;
	uint32 lsum;

	j = len >> 1;
	lsum = 0;

	for(i = 0; i < j; i++) {
		tsum = src[i*2];
		tsum = tsum << 8;
		tsum += src[i*2+1];
		lsum += tsum;
	}

	if(len % 2) {
		tsum = src[i*2];
		lsum += (tsum << 8);
	}

	sum = lsum;
	sum = ~(sum + (lsum >> 16));

	return (uint16)sum;	
}

/* @} */



/**
 * @defgroup base64_util Base64
 * Base64 Codec.
 * Base64 Utilities which is used at SMTP or something.
 * @ingroup common_util
 * @{
 */

/**
 * Decode string with base64 protocol.
 * Normally, this is used for SMTP or something.
 * @param text String to decode
 * @param dst The buffer in which decoded string will enter
 * @param numBytes Dst buffer size
 * @return Decoded string size
 */
int32 base64_decode(int8 *text, uint8 *dst, int32 numBytes)
{
  const int8* cp;
  int32 space_idx = 0, phase;
  int32 d, prev_d = 0;
  uint8 c;

    space_idx = 0;
    phase = 0;

    for ( cp = text; *cp != '\0'; ++cp ) {
        d = DecodeMimeBase64[(int32) *cp];
        if ( d != -1 ) {
            switch ( phase ) {
                case 0:
                    ++phase;
                    break;
                case 1:
                    c = ( ( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ) );
                    if ( space_idx < numBytes )
                        dst[space_idx++] = c;
                    ++phase;
                    break;
                case 2:
                    c = ( ( ( prev_d & 0xf ) << 4 ) | ( ( d & 0x3c ) >> 2 ) );
                    if ( space_idx < numBytes )
                        dst[space_idx++] = c;
                    ++phase;
                    break;
                case 3:
                    c = ( ( ( prev_d & 0x03 ) << 6 ) | d );
                    if ( space_idx < numBytes )
                        dst[space_idx++] = c;
                    phase = 0;
                    break;
            }
            prev_d = d;
        }
    }

    return space_idx;

}

/**
 * Encode string with base64 protocol.
 * Normally, this is used for SMTP or something.
 * @param text String to encode
 * @param numBytes encodedText buffer size
 * @param encodedText The buffer in which encoded string will enter
 * @return always return 0
 */
int32 base64_encode(int8 *text, int32 numBytes, int8 *encodedText)
{
  uint8 input[3]  = {0,0,0};
  uint8 output[4] = {0,0,0,0};
  int32   index, i, j;
  int8 *p, *plen;

  plen           = text + numBytes - 1;
  j              = 0;

    for  (i = 0, p = text;p <= plen; i++, p++) {
        index = i % 3;
        input[index] = *p;

        if (index == 2 || p == plen) {
            output[0] = ((input[0] & 0xFC) >> 2);
            output[1] = ((input[0] & 0x3) << 4) | ((input[1] & 0xF0) >> 4);
            output[2] = ((input[1] & 0xF) << 2) | ((input[2] & 0xC0) >> 6);
            output[3] = (input[2] & 0x3F);

            encodedText[j++] = MimeBase64[output[0]];
            encodedText[j++] = MimeBase64[output[1]];
            encodedText[j++] = index == 0? '=' : MimeBase64[output[2]];
            encodedText[j++] = index <  2? '=' : MimeBase64[output[3]];

            input[0] = input[1] = input[2] = 0;
        }
    }

    encodedText[j] = '\0';

    return 0;
}

/* @} */

#ifdef USE_FULL_ASSERT
/* NOT USE !!!
 * Asset function which declared at stm32f10x_conf.h file
 * If USE_FULL_ASSERT is defined this will be active.
 * @param file File name in which asset occurred.
 * @param line asserted line number
 */
void assert_failed(uint8* file, uint32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif



