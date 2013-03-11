/**
 * @file		cmdrun.h
 * @brief		AT Command Module - Implementation Part Header File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef	_CMDRUN_H
#define	_CMDRUN_H

#include "common/common.h"


#define RET_DONE		2
#define RET_OK_DUMP		3
#define RET_ASYNC		4
#define RET_RECV		5

#define RET_UNSPECIFIED	-2
#define RET_WRONG_OP	-3
#define RET_WRONG_SIGN	-4
#define RET_WRONG_ARG	-5
#define RET_RANGE_OUT	-6
#define RET_DISABLED	-7
#define RET_NOT_ALLOWED	-8
#define RET_BUSY		-9
#define RET_TIMEOUT		-10
#define RET_NO_SOCK		-20
#define RET_SOCK_CLS	-21
#define RET_USING_PORT	-22
#define RET_NOT_CONN	-23
#define RET_WRONG_ADDR	-24
#define RET_NO_DATA		-25
#define RET_NO_FREEMEM	-30

#define ERRVAL_UNSPECIFIED	'0'	// under 10
#define ERRVAL_WRONG_OP		'1'
#define ERRVAL_WRONG_SIGN	'2'
#define ERRVAL_WRONG_ARG	'3'
#define ERRVAL_RANGE_OUT	'4'
#define ERRVAL_DISABLED		'5'
#define ERRVAL_NOT_ALLOWED	'6'
#define ERRVAL_BUSY			'7'
#define ERRVAL_TIMEOUT		'8'
#define ERRVAL_NO_SOCK		'0'	// under 20
#define ERRVAL_SOCK_CLS		'1'
#define ERRVAL_USING_PORT	'2'
#define ERRVAL_NOT_CONN		'3'
#define ERRVAL_WRONG_ADDR	'4'
#define ERRVAL_NO_DATA		'5'
#define ERRVAL_NO_FREEMEM	'0'	// under 30

#define ATC_SOCK_NUM_START	1
#define ATC_SOCK_NUM_END	TOTAL_SOCK_NUM-1
#define ATC_SOCK_NUM_TOTAL	ATC_SOCK_NUM_END - ATC_SOCK_NUM_START + 1
#define ATC_SOCK_AO			1	// Array Offset
#define WORK_BUF_SIZE		2048
#define EVENT_QUEUE_SIZE	20
#define ATC_VERSION	"1.0"

#define OP_SIZE 8
#define LONG_ARG_SIZE 36
#define SHORT_ARG_SIZE 20
#define ARG_1_SIZE LONG_ARG_SIZE
#define ARG_2_SIZE LONG_ARG_SIZE
#define ARG_3_SIZE SHORT_ARG_SIZE
#define ARG_4_SIZE SHORT_ARG_SIZE
#define ARG_5_SIZE SHORT_ARG_SIZE
#define ARG_6_SIZE SHORT_ARG_SIZE

#define POLL_MODE_NONE 0x0
#define POLL_MODE_SEMI 0x1
#define POLL_MODE_FULL 0x2


typedef struct atct_t {
	int8 sign;
	int8 op[OP_SIZE];
	int8 arg1[ARG_1_SIZE];	// could be SSID
	int8 arg2[ARG_2_SIZE];
	int8 arg3[ARG_3_SIZE];
	int8 arg4[ARG_4_SIZE];
	int8 arg5[ARG_5_SIZE];
	int8 arg6[ARG_6_SIZE];
} atct;

struct atc_info {
	wiz_NetInfo net;
	atct tcmd;
	int8 sendsock;
	uint8 sendip[4];
	uint16 sendport;
	int8 sendbuf[WORK_BUF_SIZE+1];
	int8 recvbuf[WORK_BUF_SIZE+1];
	uint16 worklen;
	uint16 sendlen;
	uint16 recvlen;
	int8 echo;
	int8 mode;		// Reserved
	int8 poll;
	int8 country;	// Reserved
};

extern struct atc_info atci;

#define CRITICAL_ERR(fmt) \
	do{printf("### ERROR ### %s(%d): "fmt"\r\n", __FUNCTION__, __LINE__); while(1);}while(0)
#define CRITICAL_ERRA(fmt, ...) \
	do{printf("### ERROR ### %s(%d): "fmt"\r\n", __FUNCTION__, __LINE__, __VA_ARGS__); while(1);}while(0)

extern void cmd_resp(int8 retval, int8 idval);
extern void cmd_resp_dump(int8 idval, int8 *dump);

void atc_async_cb(uint8 id, uint8 item, int32 ret);
void act_nset_q(int8 num);
void act_nset_a(int8 mode, uint8 *ip, uint8 *sn, uint8 *gw, uint8 *dns1, uint8 *dns2);
void act_nstat(int8 num);
void act_nmac_q(void);
void act_nmac_a(uint8 *mac);
void act_nopen_q(void);
void act_nopen_a(int8 type, uint16 sport, uint8 *dip, uint16 dport);
void act_ncls(uint8 sock);
int8 act_nsend_chk(uint8 sock, uint16 *len, uint8 *dip, uint16 *dport);
void act_nsend(uint8 sock, int8 *buf, uint16 len, uint8 *dip, uint16 *dport);
void act_nrecv(int8 sock, uint16 maxlen);
void act_nsock(int8 sock);
//void act_nopt(void);
#if 0
void act_wset(void);
void act_wstat(void);
void act_wscan(void);
void act_wjoin(void);
void act_wleave(void);
void act_wsec(void);
void act_wwps(void);
#endif
void act_mset_q(int8 num);
void act_mset_a(int8 echo, int8 mode, int8 poll, int8 country);
void act_mstat(void);
void act_mevt_q(void);
void act_mevt_a(int8 id);
//void act_musart(void);
//void act_mspi(void);
//void act_mprof(void);
//void act_fdhcpd(void);
//void act_fdns(void);
//void act_fping(void);
//void act_fgpio(void);
//void act_eset(void);
//void act_estat(void);


#endif	//_CMDRUN_H



