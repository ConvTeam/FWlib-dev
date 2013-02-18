
//#define FILE_LOG_SILENCE
#include "common/common.h"
//#include "common/util.h"


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


int8 alarm_set(uint32 time, alarm_cbfunc cb, int8 arg)
{
	alarm_node *aptr, **adbl;

	if(time > MAX_TICK_ELAPSE || cb == NULL) return RET_NOK;

	//if(time) DBGA("Set with delay: Time(%d), CB(%p), ARG(%d)", time, (void*)cb, arg);
	time += wizpf_get_systick();	// ０A易oCA﹞I０i AEAI卦足 - tick伊伊 ０A易oCA﹞I０i 伊C易C﹞I ∮o﹉u取帚A卦

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

int32 base64_decode(int8 *text, uint8 *dst, int32 numBytes )
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


#ifdef USE_FULL_ASSERT
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



