/**
 * @file		atcmd.c
 * @brief		AT Command Module - Interface Part Source File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

//#define FILE_LOG_SILENCE
#include "appmod/atcmd/atcmd.h"
#include "appmod/atcmd/cmdrun.h"

#define CMD_CLEAR() { \
	atci.tcmd.op[0] =   atci.tcmd.sign =    atci.tcmd.arg1[0] = atci.tcmd.arg2[0] = 0; \
	atci.tcmd.arg3[0] = atci.tcmd.arg4[0] = atci.tcmd.arg5[0] = atci.tcmd.arg6[0] = 0; \
}
#define RESP_CR(type_v) do{CMD_CLEAR(); cmd_resp(type_v, VAL_NONE); return;}while(0)
#define RESP_CDR(type_v, dgt_v) do{ \
	CMD_CLEAR(); sprintf((char*)atci.tcmd.arg1, "%d", dgt_v); \
	cmd_resp(type_v, VAL_NONE); return; \
}while(0)

#define CMP_CHAR_2(str_p, c1_v, c2_v) \
	(str_p[1] != 0 || ((str_p[0]=toupper(str_p[0]))!=c1_v && str_p[0]!=c2_v))
#define CMP_CHAR_3(str_p, c1_v, c2_v, c3_v) \
	(str_p[1] != 0 || ((str_p[0]=toupper(str_p[0]))!=c1_v && str_p[0]!=c2_v && str_p[0]!=c3_v))
#define CHK_DGT_RANGE(str_p, snum_v, minval_v, maxval_v) \
	((snum_v=atoi((char*)str_p))>maxval_v || snum_v<minval_v)
#define CHK_ARG_LEN(arg_p, maxlen_v, ret_v) { \
	if(maxlen_v == 0) { \
		if(arg_p[0] != 0) RESP_CDR(RET_WRONG_ARG, ret_v); \
	} else if(strlen((char*)arg_p) > maxlen_v) RESP_CDR(RET_WRONG_ARG, ret_v); \
}

static void cmd_set_prev(uint8 buflen);
static int8 cmd_divide(int8 *buf);
static void cmd_assign(void);
static void hdl_nset(void);
static void hdl_nstat(void);
static void hdl_nmac(void);
static void hdl_nopen(void);
static void hdl_ncls(void);
static void hdl_nsend(void);
static void hdl_nrecv(void);
static void hdl_nsock(void);
static void hdl_nopt(void);
#if 0 // for wlan
static void hdl_wset(void);
static void hdl_wstat(void);
static void hdl_wscan(void);
static void hdl_wjoin(void);
static void hdl_wleave(void);
static void hdl_wsec(void);
static void hdl_wwps(void);
#endif
static void hdl_mset(void);
static void hdl_mstat(void);
static void hdl_mevt(void);
static void hdl_musart(void);
static void hdl_mspi(void);
static void hdl_mprof(void);
static void hdl_mrst(void);
static void hdl_fdhcpd(void);
static void hdl_fdns(void);
static void hdl_fping(void);
static void hdl_fgpio(void);
static void hdl_eset(void);
static void hdl_estat(void);


#define ATCMD_BUF_SIZE		100
#define PREVBUF_MAX_SIZE	250
#define PREVBUF_MAX_NUM		10
#define PREVBUF_LAST		(PREVBUF_MAX_NUM-1)
#define CMD_SIGN_NONE		0
#define CMD_SIGN_QUEST		1
#define CMD_SIGN_INDIV		2
#define CMD_SIGN_EQUAL		3

static int8 termbuf[ATCMD_BUF_SIZE];
static int8 *prevbuf[PREVBUF_MAX_NUM];
static uint8 previdx = 0, prevcnt = 0;
static int16 prevlen = 0;

struct atc_info atci;


/**
 * @ingroup atcmd_module
 * Initialize ATCMD Module.
 * This should be called before @ref atc_run
 */
void atc_init(void)
{
	int8 i;

	memset(termbuf, 0, ATCMD_BUF_SIZE);
	for(i=0; i<PREVBUF_MAX_NUM; i++) prevbuf[i] = NULL;
	atci.sendsock = VAL_NONE;
	atci.echo = VAL_ENABLE;
	atci.poll = POLL_MODE_SEMI;

	sockwatch_open(1, atc_async_cb);	// 1~7 번 소켓을 atc에 할당
	sockwatch_open(2, atc_async_cb);
	sockwatch_open(3, atc_async_cb);
	sockwatch_open(4, atc_async_cb);
	sockwatch_open(5, atc_async_cb);
	sockwatch_open(6, atc_async_cb);
	sockwatch_open(7, atc_async_cb);

	printf("\r\n\r\n\r\n[W,0]\r\n");

	// ToDo

	printf("[S,0]\r\n");
}

/**
 * @ingroup atcmd_module
 * ATCMD Module Handler.
 * If you use ATCMD Module, this should run in the main loop
 */
void atc_run(void)
{
	int8 i, ret, recv_char;
	static int8 curidx = -1, curcnt = 0;
	static uint8 buflen = 0;
	//static bool prompt = TRUE;

	recv_char = (int8)getchar_nonblk();
	if(recv_char == RET_NOK) return; // 입력 값 없는 경우		printf("RECV: 0x%x\r\n", recv_char);

	if(atci.sendsock != VAL_NONE)
	{
		//if(atci.sendsock == VAL_INVALID) {	// send fail등으로 더 받을 필요가 없더라도 정해진 size는 입력이 들어올 것으로 가정
		//	if(++atci.worklen >= atci.sendlen) {
		//		atci.sendsock = VAL_NONE;	// 그냥 카운트만 하고 버림
		//		cmd_resp(ret, VAL_NONE);	// 마지막으로 실패 응답하고 나감
		//	}
		//	return;
		//}

		atci.sendbuf[atci.worklen++] = recv_char;
		if(atci.worklen >= atci.sendlen) { // 입력이 완료되면
			act_nsend(atci.sendsock, atci.sendbuf, atci.worklen, atci.sendip, &atci.sendport);
			atci.sendsock = VAL_NONE;
		}
		return;
	}
	//if(prompt == FALSE) {
	//	prompt = TRUE;
	//	PUTCHAR('>');
	//}

	if(isgraph(recv_char) == 0)	// 제어 문자 처리
	{	//printf("ctrl\r\n");
		switch(recv_char) {
		case 0x0d:	// CR(\r)
			break;	//		do nothing
		case 0x0a:	// LF(\n)				printf("<ENT>");
			if(atci.echo) printf("\r\n");
			termbuf[buflen] = 0;
			curidx = -1;
			curcnt = 0;
			//prompt = FALSE;
			break;
		case 0x08:	// BS			printf("<BS>\r\n");
			if(buflen != 0) {
				buflen--;
				termbuf[buflen] = 0;
				if(atci.echo) printf("\b \b");
			}
			break;
		case 0x1b:	// ESC					printf("<ESC>\r\n");
			Delay_ms(5);	// For receiving rest key. (this is for the users using terminal, so little delay doesn't matter)
			{
				int8 sec_char = (int8)getchar_nonblk();
				int8 trd_char = (int8)getchar_nonblk();				//printf("s(%x),t(%x)", sec_char, trd_char);
				if(sec_char == '[') {
					switch(trd_char) {
					case 'A': //printf("<U>\r\n"); 
						if(curcnt >= prevcnt) break;			// 최대 히스토리 수를 넘기면 break
						if(curidx == -1) curidx = previdx;		// 처음 누르는 경우 현제 idx값 지정
						for(i=0; i<buflen; i++) if(atci.echo) PUTCHAR('\b');	// 입력화면 삭제
						for(i=0; i<buflen; i++) if(atci.echo) PUTCHAR(' ');	// 입력화면 삭제
						for(i=0; i<buflen; i++) if(atci.echo) PUTCHAR('\b');	// 입력화면 삭제
						if(curidx == 0) curidx = PREVBUF_LAST;	// 직전 값 지정
						else curidx--;
						curcnt++;				//printf("##%d, %d$$\r\n", curidx, prevlen);Delay_ms(5);printf("##%s$$\r\n", prevbuf[curidx]);Delay_ms(5);
						if(prevbuf[curidx]) {
							buflen = strlen((char*)prevbuf[curidx]);
							strcpy((char*)termbuf, (char*)prevbuf[curidx]);
						} else CRITICAL_ERR("prevbuf NULL");
						if(atci.echo) printf("%s", termbuf);
						break;
					case 'B': //printf("<D>\r\n"); 
						if(curcnt <= 0) break;			// 처음이면 break
						for(i=0; i<buflen; i++) if(atci.echo) PUTCHAR('\b');	// 입력화면 삭제
						for(i=0; i<buflen; i++) if(atci.echo) PUTCHAR(' ');	// 입력화면 삭제
						for(i=0; i<buflen; i++) if(atci.echo) PUTCHAR('\b');	// 입력화면 삭제
						if(curidx == PREVBUF_LAST) curidx = 0;	// 다음 값 지정
						else curidx++;
						curcnt--;				//printf("##%d, %d$$\r\n", curidx, prevlen);Delay_ms(5);printf("##%s$$\r\n", prevbuf[curidx]);Delay_ms(5);
						if(curcnt == 0) {
							buflen = 0;
						} else if(prevbuf[curidx]) {
							buflen = strlen((char*)prevbuf[curidx]);
							strcpy((char*)termbuf, (char*)prevbuf[curidx]);
							if(atci.echo) printf("%s", termbuf);
						} else CRITICAL_ERR("prevbuf NULL");
						break;
					case 'C': break;//printf("<R>\r\n"); break;
					case 'D': break;//printf("<L>\r\n"); break;
					}
				}
			}
			break;
		//case 0x20:
		//	break;
		//case 0x7f:	//printf("<DEL>\r\n"); 
		//	memset(termbuf, 0, CMD_BUF_SIZE);
		//	buflen = 0;					//printf("DEL: cur(%d), mfunc(%c), depth(%d)\r\n", mi.cur, mtree[mi.cur-1].mfunc==NULL?'N':'Y', depth);
		//	printf("\r\n");
		//	break;
		//case 0x1b:			//printf("<ESC>\r\n");
		//	break;
		}

	}
	else if(buflen < ATCMD_BUF_SIZE-1)		// -1 이유 : 0 이 하나 필요하므로 
	{
		termbuf[buflen++] = (uint8)recv_char;	//termbuf[buflen] = 0;
		if(atci.echo) PUTCHAR(recv_char);						//printf(" termbuf(%c, %s)\r\n", recv_char, termbuf);
	}//else { printf("input buffer stuffed\r\n"); }

	if(recv_char != 0x0a || buflen == 0) return; 	//LOGA("Command: %d, %s\r\n", buflen, termbuf);

	cmd_set_prev(buflen);
	buflen = 0;

	CMD_CLEAR();
	ret = cmd_divide(termbuf);
	if(ret == RET_OK) {
		cmd_assign();
	} else if(ret != RET_DONE) {
		cmd_resp(ret, VAL_NONE);
	}
}

static void cmd_set_prev(uint8 buflen)
{
	int8 idx;

	while(prevcnt >= PREVBUF_MAX_NUM || (prevcnt && prevlen+buflen > PREVBUF_MAX_SIZE-1)) {
		idx = (previdx + PREVBUF_MAX_NUM - prevcnt) % PREVBUF_MAX_NUM;	// oldest index
		if(prevbuf[idx]) {
			prevlen -= strlen((char*)prevbuf[idx]) + 1;
			free(prevbuf[idx]);
			prevcnt--;
		} else CRITICAL_ERR("ring buf 1");
	}

	prevbuf[previdx] = malloc(buflen+1);

	while(prevcnt && prevbuf[previdx] == NULL) {
		idx = (previdx + PREVBUF_MAX_NUM - prevcnt) % PREVBUF_MAX_NUM;	// oldest index
		if(prevbuf[idx]) {
			prevlen -= strlen((char*)prevbuf[idx]) + 1;
			free(prevbuf[idx]);
			prevcnt--;
			prevbuf[previdx] = malloc(buflen+1);
		} else CRITICAL_ERR("ring buf 2");
	}

	if(prevbuf[previdx] == NULL) CRITICAL_ERR("malloc fail");	//  만약 실패해도 걍 하고 싶으면 수정
	else {
		strcpy((char*)prevbuf[previdx], (char*)termbuf);	//printf("$$%s## was set\r\n", prevbuf[previdx]);
		if(previdx == PREVBUF_LAST) previdx = 0;
		else previdx++;
		prevcnt++;
		prevlen += buflen + 1;
	}
}

static int8 cmd_divide(int8 *buf)
{
	int8 ret, *split, *noteptr, *tmpptr = buf;					//printf("cmd_divide 1 \r\n");

	if(strchr((char*)tmpptr, '=')) atci.tcmd.sign = CMD_SIGN_EQUAL;
	else if(strchr((char*)tmpptr, '?')) atci.tcmd.sign = CMD_SIGN_QUEST;
	else if(strchr((char*)tmpptr, '-')) atci.tcmd.sign = CMD_SIGN_INDIV;

	split = strsep(&tmpptr, "=-?");
	if(split != NULL)
	{
		for (noteptr = split; *noteptr; noteptr++) *noteptr = toupper(*noteptr);
		if(strlen((char*)split) > OP_SIZE+3-1 || 
			split[0] != 'A' || split[1] != 'T' || split[2] != '+')
		{
			if(split[0] == 'A' && split[1] == 'T' && split[2] == 0) {	// Just 'AT' input
				if(atci.tcmd.sign == CMD_SIGN_QUEST) {
					if(prevcnt < 2) printf("[D,,0]\r\n");
					else {
						uint8 idx = previdx;
						if(previdx < 2) idx += PREVBUF_MAX_NUM - 2; //printf("==%d,%d==", previdx, idx);}
						else idx -= 2; 						//printf("++%d,%d++", previdx, idx);}printf("--%d--", idx);Delay_ms(5);
						printf("[D,,%d]\r\n%s\r\n", strlen((char*)prevbuf[idx]), prevbuf[idx]);
					}
				}
				else if(atci.tcmd.sign == CMD_SIGN_NONE) printf("[S]\r\n");
				else return RET_WRONG_SIGN;
				return RET_DONE;
			} else {
				strcpy((char*)atci.tcmd.op, (char*)split);
			}
		} else {
			strcpy((char*)atci.tcmd.op, (char*)&split[3]);
		}
	}
	else return RET_WRONG_OP;			//printf("first splite is NULL\r\n");

#define ARG_PARSE(arg_v, size_v, dgt_v) \
{ \
	split = strsep(&tmpptr, ","); \
	if(split != NULL) { \
		if(strlen((char*)split) > size_v-1) { \
			ret = RET_WRONG_ARG; \
			CMD_CLEAR(); \
			sprintf((char*)atci.tcmd.arg1, "%d", dgt_v); \
			goto FAIL_END; \
		} else strcpy((char*)arg_v, (char*)split); \
	} else goto OK_END; \
} \

	ARG_PARSE(atci.tcmd.arg1, ARG_1_SIZE, 1);
	ARG_PARSE(atci.tcmd.arg2, ARG_2_SIZE, 2);
	ARG_PARSE(atci.tcmd.arg3, ARG_3_SIZE, 3);
	ARG_PARSE(atci.tcmd.arg4, ARG_4_SIZE, 4);
	ARG_PARSE(atci.tcmd.arg5, ARG_5_SIZE, 5);
	ARG_PARSE(atci.tcmd.arg6, ARG_6_SIZE, 6);
	if(*tmpptr != 0) {
		ret = RET_WRONG_ARG;
		CMD_CLEAR();
		goto FAIL_END;
	} //DBGA("Debug: (%s)", tmpptr);	최대 arg넘게 들어온 것 확인용 - Strict Param 정책

OK_END:
	ret = RET_OK;
FAIL_END:
	DBGA("[%s] S(%d),OP(%s),A1(%s),A2(%s),A3(%s),A4(%s),A5(%s),A6(%s)", 
		ret==RET_OK?"OK":"ERR", atci.tcmd.sign, atci.tcmd.op?atci.tcmd.op:"<N>", atci.tcmd.arg1?atci.tcmd.arg1:"<N>", 
		atci.tcmd.arg2?atci.tcmd.arg2:"<N>", atci.tcmd.arg3?atci.tcmd.arg3:"<N>", atci.tcmd.arg4?atci.tcmd.arg4:"<N>", 
		atci.tcmd.arg5?atci.tcmd.arg5:"<N>", atci.tcmd.arg6?atci.tcmd.arg6:"<N>");
	return ret;
}

static void cmd_assign(void)
{
#define CMD_NOT_FOUND() do{CMD_CLEAR(); cmd_resp(RET_WRONG_OP, VAL_NONE);}while(0)

	if(atci.tcmd.op[0] == 'N')	// Network Group
	{
		if(strcmp((char*)&atci.tcmd.op[1], "SET")==0) hdl_nset();
		else if(strcmp((char*)&atci.tcmd.op[1], "STAT")==0) hdl_nstat();
		else if(strcmp((char*)&atci.tcmd.op[1], "MAC")==0) hdl_nmac();
		else if(strcmp((char*)&atci.tcmd.op[1], "OPEN")==0) hdl_nopen();
		else if(strcmp((char*)&atci.tcmd.op[1], "CLS")==0) hdl_ncls();
		else if(strcmp((char*)&atci.tcmd.op[1], "SEND")==0) hdl_nsend();
		else if(strcmp((char*)&atci.tcmd.op[1], "RECV")==0) hdl_nrecv();
		else if(strcmp((char*)&atci.tcmd.op[1], "SOCK")==0) hdl_nsock();
		else if(strcmp((char*)&atci.tcmd.op[1], "OPT")==0) hdl_nopt();
		else CMD_NOT_FOUND();
	}
#if 0
	else if(atci.tcmd.op[0] == 'W')	// WiFi Group
	{
		if(strcmp((char*)&atci.tcmd.op[1], "SET")==0) hdl_wset();
		else if(strcmp((char*)&atci.tcmd.op[1], "STAT")==0) hdl_wstat();
		else if(strcmp((char*)&atci.tcmd.op[1], "SCAN")==0) hdl_wscan();
		else if(strcmp((char*)&atci.tcmd.op[1], "JOIN")==0) hdl_wjoin();
		else if(strcmp((char*)&atci.tcmd.op[1], "LEAVE")==0) hdl_wleave();
		else if(strcmp((char*)&atci.tcmd.op[1], "SEC")==0) hdl_wsec();
		else if(strcmp((char*)&atci.tcmd.op[1], "WPS")==0) hdl_wwps();
		else CMD_NOT_FOUND();
	}
#endif
	else if(atci.tcmd.op[0] == 'M')	// Manage Group
	{
		if(strcmp((char*)&atci.tcmd.op[1], "SET")==0) hdl_mset();
		else if(strcmp((char*)&atci.tcmd.op[1], "STAT")==0) hdl_mstat();
		else if(strcmp((char*)&atci.tcmd.op[1], "EVT")==0) hdl_mevt();
		else if(strcmp((char*)&atci.tcmd.op[1], "USART")==0) hdl_musart();
		else if(strcmp((char*)&atci.tcmd.op[1], "SPI")==0) hdl_mspi();
		else if(strcmp((char*)&atci.tcmd.op[1], "PROF")==0) hdl_mprof();
		else if(strcmp((char*)&atci.tcmd.op[1], "RST")==0) hdl_mrst();
		else CMD_NOT_FOUND();
	}
	else if(atci.tcmd.op[0] == 'F')	// Extra Func Group
	{
		if(strcmp((char*)&atci.tcmd.op[1], "DHCPD")==0) hdl_fdhcpd();
		else if(strcmp((char*)&atci.tcmd.op[1], "DNS")==0) hdl_fdns();
		else if(strcmp((char*)&atci.tcmd.op[1], "PING")==0) hdl_fping();
		else if(strcmp((char*)&atci.tcmd.op[1], "GPIO")==0) hdl_fgpio();
		else CMD_NOT_FOUND();
	}
	else if(atci.tcmd.op[0] == 'E')	// Ethernet Group
	{
		if(strcmp((char*)&atci.tcmd.op[1], "SET")==0) hdl_eset();
		else if(strcmp((char*)&atci.tcmd.op[1], "STAT")==0) hdl_estat();
		else CMD_NOT_FOUND();
	}
	else CMD_NOT_FOUND();
}

void cmd_resp_dump(int8 idval, int8 *dump)
{
	uint16 len = dump!=NULL?strlen((char*)dump):0;

	if(len == 0) {
		if(idval == VAL_NONE) printf("[D,,0]\r\n");
		else printf("[D,%d,0]\r\n", idval);
	} else {
		if(idval == VAL_NONE) printf("[D,,%d]\r\n%s\r\n", len, dump);
		else printf("[D,%d,%d]\r\n%s\r\n", idval, len, dump);
		DBG("going to free");
		MEM_FREE(dump);
		DBG("free done");
	}
}

void cmd_resp(int8 retval, int8 idval)
{
	uint8 cnt, len, idx = 0;

	DBGA("ret(%d), id(%d)", retval, idval);
	cnt = (atci.tcmd.arg1[0] != 0) + (atci.tcmd.arg2[0] != 0) + (atci.tcmd.arg3[0] != 0) + 
		  (atci.tcmd.arg4[0] != 0) + (atci.tcmd.arg5[0] != 0) + (atci.tcmd.arg6[0] != 0);
#define MAKE_RESP(item_v, size_v) \
{ \
	if(item_v[0] != 0) { \
		termbuf[idx++] = ','; \
		len = strlen((char*)item_v); \
		if(len > size_v-1) CRITICAL_ERR("resp buf overflow"); \
		memcpy((char*)&termbuf[idx], (char*)item_v, len); \
		idx += len; \
		cnt--; \
	} else if(cnt) { \
		termbuf[idx++] = ','; \
	} \
}//printf("MakeResp-(%s)(%d)", item_v, len); 
	termbuf[idx++] = '[';
	if(retval >= RET_OK) {
		if(retval == RET_OK) termbuf[idx++] = 'S';
		else if(retval == RET_OK_DUMP) CRITICAL_ERR("use cmd_resp_dump for dump");
		else if(retval == RET_ASYNC) termbuf[idx++] = 'W';
		else if(retval == RET_RECV) termbuf[idx++] = 'R';
		else CRITICAL_ERRA("undefined return value (%d)", retval);

		if(idval != VAL_NONE) {
			termbuf[idx++] = ',';
			//snprintf((char*)tbuf, TBUF_SIZE, "%d", idval);
			//len = strlen((char*)tbuf);
			//strcpy((char*)&termbuf[idx], (char*)tbuf);
			sprintf((char*)&termbuf[idx], "%d", idval);
			len = digit_length(idval, 10);
			idx += len;
		} else if(cnt) termbuf[idx++] = ',';
	} else {
		termbuf[idx++] = 'F';
		termbuf[idx++] = ',';
		if(idval != VAL_NONE) {
			//snprintf((char*)tbuf, TBUF_SIZE, "%d", idval);
			//len = strlen((char*)tbuf);
			//strcpy((char*)&termbuf[idx], (char*)tbuf);
			sprintf((char*)&termbuf[idx], "%d", idval);
			len = digit_length(idval, 10);
			idx += len;
		}
		termbuf[idx++] = ',';
#define CMD_SWT_DEF(errval_v) termbuf[idx++] = errval_v; break;
#define CMD_SWT_EXT(base_v, errval_v) termbuf[idx++]=base_v;termbuf[idx++] = errval_v; break;
		switch(retval) {
		case RET_UNSPECIFIED: CMD_SWT_DEF(ERRVAL_UNSPECIFIED);
		case RET_WRONG_OP: CMD_SWT_DEF(ERRVAL_WRONG_OP);
		case RET_WRONG_SIGN: CMD_SWT_DEF(ERRVAL_WRONG_SIGN);
		case RET_WRONG_ARG: CMD_SWT_DEF(ERRVAL_WRONG_ARG);
		case RET_RANGE_OUT: CMD_SWT_DEF(ERRVAL_RANGE_OUT);
		case RET_DISABLED: CMD_SWT_DEF(ERRVAL_DISABLED);
		case RET_NOT_ALLOWED: CMD_SWT_DEF(ERRVAL_NOT_ALLOWED);
		case RET_BUSY: CMD_SWT_DEF(ERRVAL_BUSY);
		case RET_TIMEOUT: CMD_SWT_DEF(ERRVAL_TIMEOUT);
		case RET_NO_SOCK: CMD_SWT_EXT('1', ERRVAL_NO_SOCK);
		case RET_SOCK_CLS: CMD_SWT_EXT('1', ERRVAL_SOCK_CLS);
		case RET_USING_PORT: CMD_SWT_EXT('1', ERRVAL_USING_PORT);
		case RET_NOT_CONN: CMD_SWT_EXT('1', ERRVAL_NOT_CONN);
		case RET_WRONG_ADDR: CMD_SWT_EXT('1', ERRVAL_WRONG_ADDR);
		case RET_NO_DATA: CMD_SWT_EXT('1', ERRVAL_NO_DATA);
		case RET_NO_FREEMEM: CMD_SWT_EXT('2', ERRVAL_NO_FREEMEM);
		default:termbuf[idx++] = '0';break;
		}
	}
	MAKE_RESP(atci.tcmd.arg1, ARG_1_SIZE);
	MAKE_RESP(atci.tcmd.arg2, ARG_2_SIZE);
	MAKE_RESP(atci.tcmd.arg3, ARG_3_SIZE);
	MAKE_RESP(atci.tcmd.arg4, ARG_4_SIZE);
	MAKE_RESP(atci.tcmd.arg5, ARG_5_SIZE);
	MAKE_RESP(atci.tcmd.arg6, ARG_6_SIZE);
	termbuf[idx++] = ']';
	termbuf[idx++] = 0;
	printf("%s\r\n", termbuf);	// print basic response
}

static void hdl_nset(void)
{
	int8 mode, num = -1;
	uint8 ip[4];

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;	// x는 ?로 치환
	if(atci.tcmd.sign == CMD_SIGN_QUEST)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 6)) RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_nset_q(num);
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 6)) RESP_CDR(RET_RANGE_OUT, 1);
			if(num == 1) {
				if(CMP_CHAR_2(atci.tcmd.arg2, 'D', 'S')) RESP_CDR(RET_WRONG_ARG, 2);
				mode = atci.tcmd.arg2[0];
				CMD_CLEAR();
				act_nset_a(mode, NULL, NULL, NULL, NULL, NULL);
			} else {
				if(ip_check(atci.tcmd.arg2, ip) != RET_OK) RESP_CDR(RET_WRONG_ARG, 2);
				CMD_CLEAR();
				switch(num) {
				case 2: act_nset_a(0, ip, NULL, NULL, NULL, NULL); return;
				case 3: act_nset_a(0, NULL, ip, NULL, NULL, NULL); return;
				case 4: act_nset_a(0, NULL, NULL, ip, NULL, NULL); return;
				case 5: act_nset_a(0, NULL, NULL, NULL, ip, NULL); return;
				case 6: act_nset_a(0, NULL, NULL, NULL, NULL, ip); return;
				default: CRITICAL_ERR("nset wrong num");
				}
			}
		} else RESP_CDR(RET_WRONG_ARG, 1);
	}
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		uint8 sn[4], gw[4], dns1[4], dns2[4], *ptr[5];
		num = 0;
		if(atci.tcmd.arg1[0] != 0) {
			if(CMP_CHAR_2(atci.tcmd.arg1, 'D', 'S')) RESP_CDR(RET_WRONG_ARG, 1);
			else num++;
		}

#define NSET_ARG_SET(arg_p, addr_p, idx_v, ret_v) \
if(arg_p[0] != 0) { \
	num++; \
	if(ip_check(arg_p, addr_p) != RET_OK) RESP_CDR(RET_WRONG_ARG, ret_v); \
	ptr[idx_v] = addr_p; \
} else ptr[idx_v] = NULL

		NSET_ARG_SET(atci.tcmd.arg2, ip, 0, 2);
		NSET_ARG_SET(atci.tcmd.arg3, sn, 1, 3);
		NSET_ARG_SET(atci.tcmd.arg4, gw, 2, 4);
		NSET_ARG_SET(atci.tcmd.arg5, dns1, 3, 5);
		NSET_ARG_SET(atci.tcmd.arg6, dns2, 4, 6);
		if(num == 0) RESP_CR(RET_NOT_ALLOWED);
		mode = atci.tcmd.arg1[0];
		CMD_CLEAR();
		act_nset_a(mode, ptr[0], ptr[1], ptr[2], ptr[3], ptr[4]);
	} 
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_nstat(void)
{
	int8 num = -1;

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;
	if(atci.tcmd.sign == CMD_SIGN_QUEST)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 6)) RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_nstat(num);
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL) RESP_CR(RET_WRONG_SIGN);
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_nmac(void)
{
	int8 num = -1;
	uint8 mac[6];

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;
	if(atci.tcmd.sign == CMD_SIGN_QUEST)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 1)) RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_nmac_q();
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		if(mac_check(atci.tcmd.arg1, mac) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
		CMD_CLEAR();
		act_nmac_a(mac);
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_nopen(void)
{
	int8 type=0;
	uint8 DstIP[4], *dip = NULL;
	uint16 SrcPort, DstPort = 0;

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;
	if(atci.tcmd.sign == CMD_SIGN_QUEST)
	{
		CMD_CLEAR();
		act_nopen_q();
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		if(CMP_CHAR_3(atci.tcmd.arg1, 'S', 'C', 'U')) RESP_CDR(RET_WRONG_ARG, 1);
		if(port_check(atci.tcmd.arg2, &SrcPort) != RET_OK) RESP_CDR(RET_WRONG_ARG, 2);

		if(atci.tcmd.arg1[0] == 'C') {
			if(ip_check(atci.tcmd.arg3, DstIP) != RET_OK) RESP_CDR(RET_WRONG_ARG, 3);
			if(port_check(atci.tcmd.arg4, &DstPort) != RET_OK) RESP_CDR(RET_WRONG_ARG, 4);
			dip = DstIP;
		} else if(atci.tcmd.arg1[0] == 'U') {
			if(atci.tcmd.arg3[0] != 0 && atci.tcmd.arg4[0] != 0) {
				if(ip_check(atci.tcmd.arg3, DstIP) != RET_OK) RESP_CDR(RET_WRONG_ARG, 3);
				if(port_check(atci.tcmd.arg4, &DstPort) != RET_OK) RESP_CDR(RET_WRONG_ARG, 4);
				dip = DstIP;
			} else {
				CHK_ARG_LEN(atci.tcmd.arg3, 0, 3);
				CHK_ARG_LEN(atci.tcmd.arg4, 0, 4);
			}
		} else {	// 'S'	무시정책이냐 아니면 전부 확인 정책이냐
			CHK_ARG_LEN(atci.tcmd.arg3, 0, 3);
			CHK_ARG_LEN(atci.tcmd.arg4, 0, 4);
		}

		CHK_ARG_LEN(atci.tcmd.arg5, 0, 5);
		type = atci.tcmd.arg1[0];
		CMD_CLEAR();
		act_nopen_a(type, SrcPort, dip, DstPort);
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_ncls(void)
{
	int8 num = -1;

	if(atci.tcmd.sign == CMD_SIGN_NONE) RESP_CR(RET_WRONG_SIGN);
	if(atci.tcmd.sign == CMD_SIGN_QUEST) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, ATC_SOCK_NUM_START, ATC_SOCK_NUM_END)) 
				RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_ncls(num);
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_nsend(void)
{
	int8 num = -1;
	int32 ret;
	uint8 *dip = NULL;
	uint16 *dport = NULL;

	if(atci.tcmd.sign == CMD_SIGN_NONE) RESP_CR(RET_WRONG_SIGN);
	if(atci.tcmd.sign == CMD_SIGN_QUEST) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, ATC_SOCK_NUM_START, ATC_SOCK_NUM_END)) 
				RESP_CDR(RET_RANGE_OUT, 1);
		}
		if(str_check(isdigit, atci.tcmd.arg2) != RET_OK || 
			(atci.sendlen = atoi((char*)atci.tcmd.arg2)) < 1 || 
			atci.sendlen > WORK_BUF_SIZE) RESP_CDR(RET_RANGE_OUT, 2);

		if(atci.tcmd.arg3[0]) {
			if(ip_check(atci.tcmd.arg3, atci.sendip) == RET_OK) dip = atci.sendip;
			else RESP_CDR(RET_WRONG_ARG, 3);
		}
		if(atci.tcmd.arg4[0]) {
			if(port_check(atci.tcmd.arg4, &atci.sendport)==RET_OK) dport = &atci.sendport;
			else RESP_CDR(RET_WRONG_ARG, 4);
		}

		CHK_ARG_LEN(atci.tcmd.arg5, 0, 5);
		CHK_ARG_LEN(atci.tcmd.arg6, 0, 6);
		CMD_CLEAR();
		ret = act_nsend_chk(num, &atci.sendlen, dip, dport);
		if(ret != RET_OK) return;

		atci.sendsock = num;	// 유효성 검사가 완료되면 SEND모드로 전환
		atci.worklen = 0;
		cmd_resp(RET_ASYNC, num);
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_nrecv(void)
{
	int8 num = VAL_NONE;
	int32 maxlen;

	if(atci.poll == POLL_MODE_NONE) RESP_CR(RET_DISABLED);
	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_EQUAL;
	if(atci.tcmd.sign == CMD_SIGN_QUEST) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, ATC_SOCK_NUM_START, ATC_SOCK_NUM_END)) 
				RESP_CDR(RET_RANGE_OUT, 1);
		}
		if(atci.tcmd.arg2[0] == 0) maxlen = WORK_BUF_SIZE;	// 사이즈 지정안되면 최대값으로 지정
		else if(str_check(isdigit, atci.tcmd.arg2) != RET_OK) RESP_CDR(RET_WRONG_ARG, 2);
		else if((maxlen = atoi((char*)atci.tcmd.arg2)) < 1 || maxlen > WORK_BUF_SIZE) 
			RESP_CDR(RET_RANGE_OUT, 2);
		CMD_CLEAR();
		act_nrecv(num, maxlen);
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_nsock(void)
{
	
	int8 num = -1;

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;
	if(atci.tcmd.sign == CMD_SIGN_QUEST)
	{
		CMD_CLEAR();
		act_nsock(VAL_NONE);
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, ATC_SOCK_NUM_START, ATC_SOCK_NUM_END)) 
				RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_nsock(num);
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_nopt(void)
{
	CMD_CLEAR();
	RESP_CR(RET_NOT_ALLOWED);
}
#if 0
static void hdl_wset(void)
{

}

static void hdl_wstat(void)
{

}

static void hdl_wscan(void)
{

}

static void hdl_wjoin(void)
{

}

static void hdl_wleave(void)
{

}

static void hdl_wsec(void)
{

}

static void hdl_wwps(void)
{

}
#endif
static void hdl_mset(void)
{
	int8 echo, poll, num = -1;	//, mode, country

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;	// [?] 구현
	if(atci.tcmd.sign == CMD_SIGN_QUEST)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 4)) RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_mset_q(num);
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 4)) RESP_CDR(RET_RANGE_OUT, 1);
			if(num == 1) {
				if(CMP_CHAR_2(atci.tcmd.arg2, 'E', 'D')) RESP_CDR(RET_WRONG_ARG, 2);
				echo = atci.tcmd.arg2[0];
				CMD_CLEAR();
				act_mset_a(echo, 0, 0, 0);
			} else if(num == 2) {
				RESP_CDR(RET_NOT_ALLOWED, 2);	// 아직 정해진 것 없음
			} else if(num == 3) {
				if(CMP_CHAR_3(atci.tcmd.arg2, 'F', 'S', 'D')) RESP_CDR(RET_WRONG_ARG, 2);
				poll = atci.tcmd.arg2[0];
				CMD_CLEAR();
				act_mset_a(0, 0, poll, 0);
			} else RESP_CDR(RET_NOT_ALLOWED, 2);	// 국가 설정 아직 구현안함
		} else RESP_CDR(RET_WRONG_ARG, 1);
	}
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		num = 0;
		if(atci.tcmd.arg1[0] != 0) {
			num++;
			if(CMP_CHAR_2(atci.tcmd.arg1, 'E', 'D')) RESP_CDR(RET_WRONG_ARG, 1);
		}
		if(atci.tcmd.arg3[0] != 0) {
			num++;
			if(CMP_CHAR_3(atci.tcmd.arg3, 'F', 'S', 'D')) RESP_CDR(RET_WRONG_ARG, 3);
		}
		// arg 2, 4 는 일단 무시
		if(num == 0) RESP_CR(RET_NOT_ALLOWED);
		echo = atci.tcmd.arg1[0];
		poll = atci.tcmd.arg3[0];
		CMD_CLEAR();
		act_mset_a(echo, 0, poll, 0);
	} 
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);	
}

static void hdl_mstat(void)
{
	int8 num = -1;

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;
	if(atci.tcmd.sign == CMD_SIGN_QUEST)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 1)) RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_mstat();
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL) RESP_CR(RET_WRONG_SIGN);
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_mevt(void)
{
	int8 num = -1;

	if(atci.poll != POLL_MODE_FULL) RESP_CR(RET_DISABLED);
	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_EQUAL;
	if(atci.tcmd.sign == CMD_SIGN_QUEST) 
	{
		CMD_CLEAR();
		act_mevt_q();
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, ATC_SOCK_NUM_START, ATC_SOCK_NUM_END)) 
				RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_mevt_a(num);
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_musart(void)
{
	RESP_CR(RET_NOT_ALLOWED);
}

static void hdl_mspi(void)
{
	RESP_CR(RET_NOT_ALLOWED);
}

static void hdl_mprof(void)
{
	RESP_CR(RET_NOT_ALLOWED);
}

static void hdl_mrst(void)
{
	CMD_CLEAR();
	cmd_resp(RET_OK, VAL_NONE);
	NVIC_SystemReset();
}

static void hdl_fdhcpd(void)
{
	RESP_CR(RET_NOT_ALLOWED);
}

static void hdl_fdns(void)
{
	RESP_CR(RET_NOT_ALLOWED);
}

static void hdl_fping(void)
{
	RESP_CR(RET_NOT_ALLOWED);
}

static void hdl_fgpio(void)
{
	RESP_CR(RET_NOT_ALLOWED);
}

static void hdl_eset(void)
{
	RESP_CR(RET_NOT_ALLOWED);
}

static void hdl_estat(void)
{
	RESP_CR(RET_NOT_ALLOWED);
}














