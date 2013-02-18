
//#define FILE_LOG_SILENCE
#include "appmod/atcmd/cmdrun.h"


#define SOCK_STAT_TCP_MASK	0x2
#define SOCK_STAT_PROTMASK	0x3
#define SOCK_STAT_IDLE		0x0
#define SOCK_STAT_UDP		0x1
#define SOCK_STAT_TCP_SRV	0x2
#define SOCK_STAT_TCP_CLT	0x3
#define SOCK_STAT_CONNECTED	0x4

#define EVENT_RESP(id_v, evt_v)			printf("[V,%d,%d]\r\n", id_v, evt_v)
#define EVENT_RESP_SIZE(id_v, evt_v, size_v) printf("[V,%d,%d,%d]\r\n", id_v, evt_v, size_v)
#define CMD_RESP_RET(type_v, id_v)		do{DBG("#");cmd_resp(type_v, id_v); return;}while(0)
#define MAKE_TCMD_DIGIT(arg_p, dgt_p)	sprintf((char*)arg_p, "%d", dgt_p)
#define MAKE_TCMD_CHAR(arg_p, char_p)	sprintf((char*)arg_p, "%c", char_p)
#define MAKE_TCMD_ADDR(arg_p, a1_p, a2_p, a3_p, a4_p) \
	sprintf((char*)arg_p, "%d.%d.%d.%d", a1_p, a2_p, a3_p, a4_p)
#define MAKE_TCMD_STRING(arg_p, argsize_v, str_p) do { \
	uint8 len = strlen((char*)str_p); \
	if(len > argsize_v-1) {memcpy((char*)arg_p, (char*)str_p, argsize_v-1); arg_p[argsize_v-1]=0;} \
	else {memcpy((char*)arg_p, (char*)str_p, len); arg_p[len]=0;} \
} while(0)

enum {
	SOCKEVENT_CONN = 0,
	SOCKEVENT_RECV = 1,
	SOCKEVENT_CLS  = 2
};

struct atc_eventq {
	struct atc_eventq *next;
	int8 id;
	int8 event;
};

extern void cmd_resp(int8 retval, int8 idval);

static int8  sockstat[ATC_SOCK_NUM_TOTAL+ATC_SOCK_AO] = {0,};	// sock 상태 확인 용	// 0번은 편의를 위해 희생함 (총 8바이트 버림)
static int16 sockport[ATC_SOCK_NUM_TOTAL+ATC_SOCK_AO] = {0,};	// src포트 확인 용
static int8  sockbusy[ATC_SOCK_NUM_TOTAL+ATC_SOCK_AO] = {0,};	// 명령 가능상태 확인 용
static int8  udpip[ATC_SOCK_NUM_TOTAL+ATC_SOCK_AO][4] = {0,};	// UDP Destination 주소 저장용
static int16 udpport[ATC_SOCK_NUM_TOTAL+ATC_SOCK_AO]  = {0,};	// UDP Destination 포트 저장용
static int16 tcpleft[ATC_SOCK_NUM_TOTAL+ATC_SOCK_AO]  = {0,};	// TCP Resend를 대비하여 전송될 데이터 길이 저장용
static int8  recvord[ATC_SOCK_NUM_TOTAL+ATC_SOCK_AO]  = {0,};	// 데이터 수신 순서 저장
static int8  recvnum = 0;							// 데이터를 수신한 소켓의 수
struct atc_eventq *eventqueue;
uint8 eqp_cnt = 0;


static int8 sock_get(int8 initval, uint16 srcport)
{
	int8 i;

	DBGCRTCA(initval<1||initval>3, "wrong init value(%d)", initval);

	for(i=ATC_SOCK_NUM_START; i<=ATC_SOCK_NUM_END; i++) {
		if(sockstat[i] == SOCK_STAT_IDLE) {
			sockstat[i] = initval;
			sockport[i] = srcport;			//DBGA("Sock(%d) Assign stt(%d), port(%d)", i, sockstat[i], sockport[i]);
			return i;
		}
	}

	return RET_NOK;
}

static int8 sock_put(uint8 sock)
{
	DBGCRTCA(sock<ATC_SOCK_NUM_START||sock>ATC_SOCK_NUM_END, "wrong sock(%d)", sock);

	if(sockstat[sock] & SOCK_STAT_UDP) {
		udpip[sock][0] = udpip[sock][1] = udpip[sock][2] = 0;
		udpip[sock][3] = udpport[sock] = 0;
	}
	sockstat[sock] = SOCK_STAT_IDLE;
	sockport[sock] = 0;	// 없어도 되지만 걍 혹시나 해서
	sockwatch_clr(sock, WATCH_SOCK_ALL_MASK);
	return RET_OK;
}

static int8 event_enqueue(int8 id, int8 event)
{
#define QFULL	-1
	struct atc_eventq **eqdp;

	eqdp = &eventqueue;
	if(eqp_cnt >= EVENT_QUEUE_SIZE-1) {
		while((*eqdp)->next) eqdp = &(*eqdp)->next;
		if((*eqdp)->event != QFULL) {
			eqdp = &(*eqdp)->next;
			*eqdp = malloc(sizeof(struct atc_eventq));
			if(*eqdp == NULL) {
				ERRA("malloc fail - size(%d)", sizeof(struct atc_eventq));
				return RET_NOK;
			}
			(*eqdp)->id = QFULL;
			(*eqdp)->event = QFULL;
			eqp_cnt++;
		}
		return RET_NOK;
	}

	while(*eqdp) eqdp = &(*eqdp)->next;

	*eqdp = malloc(sizeof(struct atc_eventq));
	if(*eqdp == NULL) {
		ERRA("malloc fail - size(%d)", sizeof(struct atc_eventq));
		return RET_NOK;
	}

	(*eqdp)->id = id;
	(*eqdp)->event = event;
	eqp_cnt++;
	return RET_OK;
}

static int8 event_dequeue(int8 *id, int8 *event)
{
	struct atc_eventq *eqp, **eqdp;

	if(id == NULL || event == NULL) return RET_NOK;

	if(*id < 0) {
		if(eqp_cnt == 0) return RET_NOK;
		eqp = eventqueue;
		eventqueue = eventqueue->next;
		eqp_cnt--;
		*id = eqp->id;
		*event = eqp->event;
		free(eqp);
		return RET_OK;
	} else {
		eqdp = &eventqueue;
		while(*eqdp) {
			if((*eqdp)->id == *id) {
				*event = (*eqdp)->event;
				eqp = *eqdp;
				*eqdp = (*eqdp)->next;
				free(eqp);
				eqp_cnt--;
				return RET_OK;
			}
		}
		return RET_NOK;
	}
}

static void atc_resend_alarm(int8 arg)
{
	int8 ret;

	ret = TCPReSendNB(arg);
	if(ret != RET_OK) {
		if(ret == SOCKERR_BUSY) {
			alarm_set(WINDOWFULL_WAIT_TIME, atc_resend_alarm, arg);
		} else if(ret == SOCKERR_WINDOW_FULL) {
			sockbusy[arg] = VAL_FALSE;	// async 명령 완료
			DBGA("WATCH_SOCK_TCP_SEND fail - ret(%d)", ret);
			cmd_resp(RET_TIMEOUT, arg);
		} else {	// 위랑 다른 것은 소켓이 종료하는 것임
			sockbusy[arg] = VAL_FALSE;	// async 명령 완료
			DBGA("WATCH_SOCK_TCP_SEND fail - ret(%d)", ret);
			sock_put(arg);	// 이미 종료된 상태임?? 일단 해보기
			cmd_resp(RET_TIMEOUT, arg);
		}
	} else sockwatch_set(arg, WATCH_SOCK_TCP_SEND);
}

void atc_async_cb(uint8 sock, uint8 item, int32 ret)
{
	DBGCRTCA(sock<ATC_SOCK_NUM_START||sock>ATC_SOCK_NUM_END, "wrong sock(%d)", sock);

	switch(item) {
	case WATCH_SOCK_UDP_SEND:	DBG("WATCH_SOCK_UDP_SEND");
		sockbusy[sock] = VAL_FALSE;	// async 명령 완료		//DBGA("WATCH UDP Sent - sock(%d), item(%d)", sock, item);
		if(ret == RET_OK) {
			cmd_resp(RET_OK, sock);
		} else {
			DBGA("WATCH_SOCK_UDP_SEND fail - ret(%d)", ret);
			//sock_put(sock); ?? 종료할려면 act_ncls실행시키던지 해야함. 사용해보고 필요시 할 것
			cmd_resp(RET_TIMEOUT, sock);
		}
		break;
	case WATCH_SOCK_TCP_SEND:	DBG("WATCH_SOCK_TCP_SEND");
		if(ret < RET_OK) {	// 성공하면 길이를 반환하므로 실패를 먼저 처리
			sockbusy[sock] = VAL_FALSE;	// async 명령 완료
			DBGA("WATCH_SOCK_TCP_SEND fail - ret(%d)", ret);
			sock_put(sock);	// 이미 종료된 상태임?? 일단 해보기
			cmd_resp(RET_TIMEOUT, sock);
		} else {
			tcpleft[sock] -= ret;
			if(tcpleft[sock] > 0) {	// 전송할 데이터가 남은 경우
				ret = TCPReSendNB(sock);
				if(ret != RET_OK) {
					if(ret == SOCKERR_BUSY) {
						alarm_set(WINDOWFULL_WAIT_TIME, atc_resend_alarm, sock);
					} else if(ret == SOCKERR_WINDOW_FULL) {
						sockbusy[sock] = VAL_FALSE;	// async 명령 완료
						DBGA("WATCH_SOCK_TCP_SEND fail - ret(%d)", ret);
						cmd_resp(RET_TIMEOUT, sock);
					} else {	// 위랑 다른 것은 소켓이 종료하는 것임
						sockbusy[sock] = VAL_FALSE;	// async 명령 완료
						DBGA("WATCH_SOCK_TCP_SEND fail - ret(%d)", ret);
						sock_put(sock);	// 이미 종료된 상태임?? 일단 해보기
						cmd_resp(RET_TIMEOUT, sock);
					}
				} else sockwatch_set(sock, WATCH_SOCK_TCP_SEND);
			} else {	// 전송 성공
				sockbusy[sock] = VAL_FALSE;	// async 명령 완료
				cmd_resp(RET_OK, sock);
			}
		}
		break;
	case WATCH_SOCK_CONN_TRY:	DBG("WATCH_SOCK_CONN_TRY");
		sockbusy[sock] = VAL_FALSE;	// async 명령 완료
		if(ret == RET_OK) {
			BITSET(sockstat[sock], SOCK_STAT_CONNECTED);
			sockwatch_set(sock, WATCH_SOCK_CLS_EVT);
			sockwatch_set(sock, WATCH_SOCK_RECV);
			cmd_resp(RET_ASYNC, sock);
		} else {
			DBGA("WATCH_SOCK_CONN_EVT fail - ret(%d)", ret);
			sock_put(sock);
			cmd_resp(RET_TIMEOUT, sock);
		}
		break;
	case WATCH_SOCK_CLS_TRY:	DBG("WATCH_SOCK_CLS_TRY");
		sockbusy[sock] = VAL_FALSE;	// async 명령 완료
		if(ret == RET_OK) {
			sock_put(sock);
			cmd_resp(RET_ASYNC, sock);
		} else {
			CRITICAL_ERRA("WATCH_SOCK_CONN_EVT fail - ret(%d)", ret);	// Error 없는 것으로 보임
		}
		break;
	case WATCH_SOCK_CONN_EVT:	DBG("WATCH_SOCK_CONN_EVT");
		if(ret == RET_OK) {
			BITSET(sockstat[sock], SOCK_STAT_CONNECTED);
			sockwatch_set(sock, WATCH_SOCK_CLS_EVT);
			sockwatch_set(sock, WATCH_SOCK_RECV);
			if(atci.poll != POLL_MODE_FULL) EVENT_RESP(sock, SOCKEVENT_CONN);
			else event_enqueue(sock, SOCKEVENT_CONN);
		} else {
			CRITICAL_ERRA("WATCH_SOCK_CONN_EVT fail - ret(%d)", ret);	// Error 없는 것으로 보임
		}
		break;
	case WATCH_SOCK_CLS_EVT:	DBG("WATCH_SOCK_CLS_EVT");
		sockbusy[sock] = VAL_FALSE;	// async 명령 완료
		if(ret == RET_OK) {
			if(sockwatch_chk(sock, WATCH_SOCK_CLS_TRY) == RET_OK) 
				cmd_resp(RET_OK, sock);
			sock_put(sock);
			if(atci.poll != POLL_MODE_FULL) EVENT_RESP(sock, SOCKEVENT_CLS);
			else event_enqueue(sock, SOCKEVENT_CLS);
		} else {
			CRITICAL_ERRA("WATCH_SOCK_CONN_EVT fail - ret(%d)", ret);	// Error 없는 것으로 보임
		}
		break;
	case WATCH_SOCK_RECV:	DBG("WATCH_SOCK_RECV");
		{
			int8 i;

			if(atci.poll != POLL_MODE_NONE) {
				recvnum++;
				if(recvord[sock] == 0) {
					for(i=ATC_SOCK_NUM_START; i<=ATC_SOCK_NUM_END; i++)	// 최근에 온것부터 1~총rx한sock개수, 받은거없으면 0
						if(recvord[i] != 0) recvord[i]++; // 자기말고 전부 하나 뒤로
				} else {
					for(i=ATC_SOCK_NUM_START; i<=ATC_SOCK_NUM_END; i++)	// 최근에 온것부터 1~총rx한sock개수, 받은거없으면 0
						if(recvord[i] != 0 && recvord[i] < recvord[sock]) recvord[i]++; // 자기말고 전부 하나 뒤로
				}
				recvord[sock] = 1;
				if(atci.poll != POLL_MODE_FULL) 
					EVENT_RESP_SIZE(sock, SOCKEVENT_RECV, GetSocketRxRecvBufferSize(sock));	// INT의 경우, POL은 나중에 구현 할 것
				else event_enqueue(sock, SOCKEVENT_RECV);
				// 수신된 값에서 헤더길이 8을 제하는 것은 패킷이 하나인 경우에만 의미가 잇으므로 -8은 안하기로 함
			} else {
				act_nrecv(sock, WORK_BUF_SIZE);
			}
		}
		break;
	default: CRITICAL_ERRA("wrong item(0x%x)", item);
	}
}

void act_nset_q(int8 num)	// 1~6 해당 인자만 출력, -1 전부 출력
{
	wiz_NetInfo ni;

	if(num == 1) {
		GetNetInfo(&ni);
		if(ni.DHCP > NETINFO_STATIC) 
			MAKE_TCMD_CHAR(atci.tcmd.arg1, 'D');
		else MAKE_TCMD_CHAR(atci.tcmd.arg1, 'S');
		CMD_RESP_RET(RET_OK, VAL_NONE);
	} else if(num > 1) {
		dhcp_get_storage(&ni);
		switch(num) {
		case 2: MAKE_TCMD_ADDR(atci.tcmd.arg1, ni.IP[0], ni.IP[1], ni.IP[2], ni.IP[3]); break;
		case 3: MAKE_TCMD_ADDR(atci.tcmd.arg1, ni.SN[0], ni.SN[1], ni.SN[2], ni.SN[3]); break;
		case 4: MAKE_TCMD_ADDR(atci.tcmd.arg1, ni.GW[0], ni.GW[1], ni.GW[2], ni.GW[3]); break;
		case 5: MAKE_TCMD_ADDR(atci.tcmd.arg1, ni.DNS[0], ni.DNS[1], ni.DNS[2], ni.DNS[3]); break;
		case 6: CMD_RESP_RET(RET_NOT_ALLOWED, VAL_NONE);
		}
		CMD_RESP_RET(RET_OK, VAL_NONE);
	} else {
		GetNetInfo(&ni);
		if(ni.DHCP > NETINFO_STATIC) 
			MAKE_TCMD_CHAR(atci.tcmd.arg1, 'D');
		else MAKE_TCMD_CHAR(atci.tcmd.arg1, 'S');
		dhcp_get_storage(&ni);
		MAKE_TCMD_ADDR(atci.tcmd.arg2, ni.IP[0], ni.IP[1], ni.IP[2], ni.IP[3]);
		MAKE_TCMD_ADDR(atci.tcmd.arg3, ni.SN[0], ni.SN[1], ni.SN[2], ni.SN[3]);
		MAKE_TCMD_ADDR(atci.tcmd.arg4, ni.GW[0], ni.GW[1], ni.GW[2], ni.GW[3]);
		MAKE_TCMD_ADDR(atci.tcmd.arg5, ni.DNS[0], ni.DNS[1], ni.DNS[2], ni.DNS[3]);
		CMD_RESP_RET(RET_OK, VAL_NONE);
	}
}

void act_nset_a(int8 mode, uint8 *ip, uint8 *sn, 
	uint8 *gw, uint8 *dns1, uint8 *dns2)
{
	wiz_NetInfo ni = {0,};

	if(ip) memcpy(ni.IP, ip, 4);
	if(sn) memcpy(ni.SN, sn, 4);
	if(gw) memcpy(ni.GW, gw, 4);
	if(dns1) memcpy(ni.DNS, dns1, 4);
	if(dns2) {	// 2번은 지원안함
		MAKE_TCMD_DIGIT(atci.tcmd.arg1, 6);	// 6번 Arg가 문제임을 표시
		CMD_RESP_RET(RET_NOT_ALLOWED, VAL_NONE);
	}

	if(mode == 'S') {
		if(dhcp_static_mode(&ni) != RET_OK) { // storage에 적용 후 Static으로 실적용
			dhcp_set_storage(&ni);	// 이미 static인 경우, 직접 storage와 실적용을 수행
			SetNetInfo(&ni);
		}
	} else if(mode == 'D') {
		dhcp_set_storage(&ni); // DHCP동작과는 상관없이 storage는 업데이트함
		dhcp_alarm_start(NULL); // 이미 dhcp인 경우라도 성공으로 리턴
	} else {	//  모드전환없이 주소만 설정한 경우
		dhcp_set_storage(&ni); // 일단 storage에 업데이트
		GetNetInfo(&ni);	// static이면 바로 실적용해줌
		if(ni.DHCP == NETINFO_STATIC) SetNetInfo(&ni);
	}
	CMD_RESP_RET(RET_OK, VAL_NONE);
}

void act_nstat(int8 num)
{
	wiz_NetInfo ni = {0,};

	GetNetInfo(&ni);
	if(num == 1) {
		if(ni.DHCP > NETINFO_STATIC) 
			MAKE_TCMD_CHAR(atci.tcmd.arg1, 'D');
		else MAKE_TCMD_CHAR(atci.tcmd.arg1, 'S');
		CMD_RESP_RET(RET_OK, VAL_NONE);
	} else if(num > 1) {
		switch(num) {
		case 2: MAKE_TCMD_ADDR(atci.tcmd.arg1, ni.IP[0], ni.IP[1], ni.IP[2], ni.IP[3]); break;
		case 3: MAKE_TCMD_ADDR(atci.tcmd.arg1, ni.SN[0], ni.SN[1], ni.SN[2], ni.SN[3]); break;
		case 4: MAKE_TCMD_ADDR(atci.tcmd.arg1, ni.GW[0], ni.GW[1], ni.GW[2], ni.GW[3]); break;
		case 5: MAKE_TCMD_ADDR(atci.tcmd.arg1, ni.DNS[0], ni.DNS[1], ni.DNS[2], ni.DNS[3]); break;
		case 6: CMD_RESP_RET(RET_NOT_ALLOWED, VAL_NONE);
		}
		CMD_RESP_RET(RET_OK, VAL_NONE);
	} else {
		if(ni.DHCP > NETINFO_STATIC) 
			MAKE_TCMD_CHAR(atci.tcmd.arg1, 'D');
		else MAKE_TCMD_CHAR(atci.tcmd.arg1, 'S');
		MAKE_TCMD_ADDR(atci.tcmd.arg2, ni.IP[0], ni.IP[1], ni.IP[2], ni.IP[3]);
		MAKE_TCMD_ADDR(atci.tcmd.arg3, ni.SN[0], ni.SN[1], ni.SN[2], ni.SN[3]);
		MAKE_TCMD_ADDR(atci.tcmd.arg4, ni.GW[0], ni.GW[1], ni.GW[2], ni.GW[3]);
		MAKE_TCMD_ADDR(atci.tcmd.arg5, ni.DNS[0], ni.DNS[1], ni.DNS[2], ni.DNS[3]);
		CMD_RESP_RET(RET_OK, VAL_NONE);
	}
}

void act_nmac_q(void)
{
	wiz_NetInfo ni = {0,};

	GetNetInfo(&ni);
	sprintf((char*)atci.tcmd.arg1, "%d:%d:%d:%d:%d:%d", 
		ni.Mac[0], ni.Mac[1], ni.Mac[2], ni.Mac[3], ni.Mac[4], ni.Mac[5]);
	CMD_RESP_RET(RET_OK, VAL_NONE);
}

void act_nmac_a(uint8 *mac)
{
#if 1 // Enable MAC change
	wiz_NetInfo ni = {0,};

	memcpy(ni.Mac, mac, 6);
	SetNetInfo(&ni);
	CMD_RESP_RET(RET_OK, VAL_NONE);
#else
	CMD_RESP_RET(RET_NOT_ALLOWED, VAL_NONE);
#endif
}

void act_nopen_q(void)
{
	// 각이 안나옴
}

void act_nopen_a(int8 type,  int8 save, uint16 sport, uint8 *dip, uint16 dport)
{	// save 부분 구현 할 것
	int8 ret, sock, i;

	for(i=ATC_SOCK_NUM_START; i<=ATC_SOCK_NUM_END; i++) {	// src port 유효성 검사
		if(sockstat[i] != SOCK_STAT_IDLE && sockport[i] == sport) {
			DBGA("src port(%d) is using now by sock(%d)", sport, i);
			MAKE_TCMD_DIGIT(atci.tcmd.arg1, 2);
			CMD_RESP_RET(RET_WRONG_ARG, VAL_NONE);
		}
	}

	if(save == 'S' || save == 'A') {	// 저장되어 booting시 실행
		// 이렇게 되면 소켓이 특정되지가 않으므로 하나만 가능한가? 심지어 삭제도 안됨
	}

	if(save == 'O' || save == 'A') {	// 즉시 실행
		if(type == 'S') {			// TCP Server
			sock = sock_get(SOCK_STAT_TCP_SRV, sport);
			if(sock == RET_NOK) CMD_RESP_RET(RET_NO_SOCK, VAL_NONE);
			ret = TCPServerOpen(sock, sport);
			if(ret != RET_OK) CMD_RESP_RET(RET_UNSPECIFIED, VAL_NONE);
			sockwatch_set(sock, WATCH_SOCK_CONN_EVT);
			CMD_RESP_RET(RET_OK, sock);
		} else if(type == 'C') {	// TCP Client
			sock = sock_get(SOCK_STAT_TCP_CLT, sport);
			if(sock == RET_NOK) CMD_RESP_RET(RET_NO_SOCK, VAL_NONE);
			ret = TCPCltOpenNB(sock, sport, dip, dport);
			if(ret != RET_OK) {
				DBGA("TCPCltOpenNB fail - ret(%d)", ret);
				CMD_RESP_RET(RET_WRONG_ADDR, VAL_NONE);
			}
			sockwatch_set(sock, WATCH_SOCK_CONN_TRY);
			sockbusy[sock] = VAL_TRUE;	// async시 명령 실행 중임을 표시
			CMD_RESP_RET(RET_ASYNC, sock);
		} else {					// UDP
			if(dip != NULL) {
				memcpy(udpip[sock], dip, 4);
				udpport[sock] = dport;
			}
			sock = sock_get(SOCK_STAT_UDP, sport);
			if(sock == RET_NOK) CMD_RESP_RET(RET_NO_SOCK, VAL_NONE);
			UDPOpen(sock, sport);	// 에러 날 것 없음
			sockwatch_set(sock, WATCH_SOCK_RECV);
			CMD_RESP_RET(RET_OK, sock);
		}
	}
}

void act_ncls(uint8 sock)
{
	int8 ret;

	if(sockbusy[sock] == VAL_TRUE) CMD_RESP_RET(RET_BUSY, VAL_NONE);
	if(sockstat[sock] == SOCK_STAT_IDLE) CMD_RESP_RET(RET_SOCK_CLS, VAL_NONE);
	if(sockstat[sock] & SOCK_STAT_TCP_MASK) {	// TCP
		ret = TCPCloseNB(sock);
		if(ret != RET_OK) CMD_RESP_RET(RET_OK, VAL_NONE);	// 이미 close되어 잇음 - 바로 완료
		sockwatch_set(sock, WATCH_SOCK_CLS_TRY);
		sockbusy[sock] = VAL_TRUE;	// async시 명령 실행 중임을 표시
		CMD_RESP_RET(RET_ASYNC, sock);
	} else {	// UDP
		UDPClose(sock);	// 에러 날 것 없음
		sock_put(sock);
		CMD_RESP_RET(RET_OK, VAL_NONE);
	}
}

int8 act_nsend_chk(uint8 sock, uint16 *len, uint8 *dip, uint16 *dport)	// 전송 가능 여부를 판단
{
	uint16 availlen;

	if(sockbusy[sock] == VAL_TRUE) {
		cmd_resp(RET_BUSY, VAL_NONE);
		return RET_NOK;
	}
	if(sockstat[sock] == SOCK_STAT_IDLE) {
		cmd_resp(RET_SOCK_CLS, VAL_NONE);
		return RET_NOK;
	}

	if(sockstat[sock] & SOCK_STAT_TCP_MASK) {	// TCP	//TCP이면 주소값 와도 무시 ??
		if(!(sockstat[sock] & SOCK_STAT_CONNECTED)) {
			cmd_resp(RET_NOT_CONN, VAL_NONE);
			return RET_NOK;
		}
	} else {									// UDP
		if(dip == NULL) {
			if(udpip[sock][0]==0 && udpip[sock][1]==0 && udpip[sock][2]==0 && udpip[sock][3]==0) {
				DBG("no prev udpip");
				cmd_resp(RET_WRONG_ADDR, VAL_NONE);
				return RET_NOK;
			}
			else memcpy(dip, udpip[sock], 4);
		} else memcpy(udpip[sock], dip, 4);
		if(dport == NULL) {
			if(udpport[sock] == 0) {
				DBG("no prev udpport");
				cmd_resp(RET_WRONG_ADDR, VAL_NONE);
				return RET_NOK;
			} else *dport = udpport[sock];
		} else udpport[sock] = *dport;
	}

	availlen = GetSocketTxFreeBufferSize(sock);
	if(*len > availlen) {
		DBGA("tx buf busy - req(%d), avail(%d)", *len, availlen);
		MAKE_TCMD_DIGIT(atci.tcmd.arg1, availlen);
		cmd_resp(RET_BUSY, VAL_NONE);
		return RET_NOK;
	}

	return RET_OK;
}

void act_nsend(uint8 sock, int8 *buf, uint16 len, uint8 *dip, uint16 *dport)
{
	int8 ret;

	if(sockstat[sock] & SOCK_STAT_TCP_MASK) {	// TCP
		ret = TCPSendNB(sock, buf, len);
		if(ret == SOCKERR_BUSY) // I think this would not be called
			CRITICAL_ERRA("Impossible TCP send busy - len(%d), avail(%d)", 
			len, GetSocketTxFreeBufferSize(sock));
		if(ret != RET_OK) CMD_RESP_RET(RET_NOT_CONN, VAL_NONE);	// 위에서 체크하지만 부담없으므로 일단 놔둠
		tcpleft[sock] = len;	// 비교를 위해서 저장해둠
		sockwatch_set(sock, WATCH_SOCK_TCP_SEND);
		sockbusy[sock] = VAL_TRUE;	// async시 명령 실행 중임을 표시
	} else {									// UDP
		ret = UDPSendNB(sock, buf, len, dip, *dport);
		if(ret == SOCKERR_BUSY) 
			CRITICAL_ERRA("Impossible UDP send busy - len(%d), avail(%d)", 
			len, GetSocketTxFreeBufferSize(sock));
		if(ret < RET_OK) {
			DBGA("UDPSendNB fail - ret(%d)", ret);
			CMD_RESP_RET(RET_WRONG_ADDR, VAL_NONE);
		}
		sockwatch_set(sock, WATCH_SOCK_UDP_SEND);
		sockbusy[sock] = VAL_TRUE;	// async시 명령 실행 중임을 표시
	}
}

void act_nrecv(int8 sock, uint16 maxlen)	// sockbusy로 분리 안시켜도 되나 ??? 일단 그냥 해보자
{
	uint8 dstip[4], i;
	uint16 dstport;
	int32 len;
 DBGA("Asock(%d)", sock);
	if(sock == VAL_NONE) {
		if(recvnum == 0) CMD_RESP_RET(RET_NO_DATA, VAL_NONE);
		for(i=ATC_SOCK_NUM_START; i<=ATC_SOCK_NUM_END; i++) {
			if(recvord[i] == recvnum) {
				sock = i;
				break;
			}
		}
	}DBGA("Bsock(%d)", sock);

	if(sockstat[sock] == SOCK_STAT_IDLE) CMD_RESP_RET(RET_SOCK_CLS, VAL_NONE);
	if(sockstat[sock] & SOCK_STAT_TCP_MASK) {	// TCP
		if(!(sockstat[sock] & SOCK_STAT_CONNECTED)) CMD_RESP_RET(RET_NOT_CONN, VAL_NONE);
		if(GetSocketRxRecvBufferSize(sock) == 0) CMD_RESP_RET(RET_NO_DATA, VAL_NONE);
		len = TCPRecv(sock, atci.recvbuf, maxlen);
	} else {									// UDP
		if(GetSocketRxRecvBufferSize(sock) == 0) CMD_RESP_RET(RET_NO_DATA, VAL_NONE);
		len = UDPRecv(sock, atci.recvbuf, maxlen, dstip, &dstport);
		if(len == 0) CMD_RESP_RET(RET_NO_DATA, VAL_NONE);
		else if(len == SOCKERR_CLOSED) CMD_RESP_RET(RET_SOCK_CLS, VAL_NONE);
		else if(len < 0) CMD_RESP_RET(RET_UNSPECIFIED, VAL_NONE);
	}
	atci.recvbuf[len] = 0;

	MAKE_TCMD_DIGIT(atci.tcmd.arg1, len);
	MAKE_TCMD_ADDR(atci.tcmd.arg2, dstip[0], dstip[1], dstip[2], dstip[3]);
	MAKE_TCMD_DIGIT(atci.tcmd.arg3, dstport);
	cmd_resp(RET_RECV, sock);
	printf("%s\r\n", atci.recvbuf);

	sockwatch_set(sock, WATCH_SOCK_RECV);
}

void act_nsock(int8 sock)
{
	uint8 tip[4];
	uint16 tport;

	if(sock < ATC_SOCK_NUM_START)
	{
		int8 *dump, i, type, cnt_con=0, cnt_notcon=0;
DBG("NSOCK-start");
		for(i=ATC_SOCK_NUM_START; i<=ATC_SOCK_NUM_END; i++) {
			if(sockstat[i] != SOCK_STAT_IDLE) {
				if(sockstat[i] & SOCK_STAT_CONNECTED) cnt_con++;
				else cnt_notcon++;
			}
		}
DBGA("NSOCK-con(%d),not(%d)", cnt_con, cnt_notcon);
 		if(cnt_con+cnt_notcon == 0) {
			cmd_resp_dump(VAL_NONE, NULL);
			return;
 		}
		dump = malloc((34*cnt_con)+(12*cnt_notcon)+1);
		if(dump == NULL) CMD_RESP_RET(RET_NO_FREEMEM, VAL_NONE);
		cnt_con = 0;
		for(i=ATC_SOCK_NUM_START; i<=ATC_SOCK_NUM_END; i++) {
			if(sockstat[i] == SOCK_STAT_IDLE) continue;
			if(cnt_con != 0) {
				dump[cnt_con++] = '\r';
				dump[cnt_con++] = '\n';
			}
			if((sockstat[i]&SOCK_STAT_PROTMASK)==SOCK_STAT_TCP_SRV) type = 'S';
			else if((sockstat[i]&SOCK_STAT_PROTMASK)==SOCK_STAT_TCP_CLT) type='C';
			else type = 'U';

			if(sockstat[i] & SOCK_STAT_CONNECTED) {
				GetDstInfo((uint8)i, tip, &tport);
				sprintf((char*)&dump[cnt_con], "%d,%c,%d,%d.%d.%d.%d,%d", i, type, 
					sockport[i], tip[0], tip[1], tip[2], tip[3], tport);
			} else {
				sprintf((char*)&dump[cnt_con], "%d,%c,%d", i, type, sockport[i]);
			}

			cnt_con += strlen((char*)&dump[cnt_con]);
		}
		cmd_resp_dump(VAL_NONE, dump);
	} 
	else if(sock <= ATC_SOCK_NUM_END)
	{
		if(sockstat[sock] == SOCK_STAT_IDLE) 
			sprintf((char*)atci.tcmd.arg1, "%c", 'I');
		else if((sockstat[sock] & SOCK_STAT_PROTMASK) == SOCK_STAT_TCP_SRV) 
			sprintf((char*)atci.tcmd.arg1, "%c", 'S');
		else if((sockstat[sock] & SOCK_STAT_PROTMASK) == SOCK_STAT_TCP_CLT) 
			sprintf((char*)atci.tcmd.arg1, "%c", 'C');
		else if((sockstat[sock] & SOCK_STAT_PROTMASK) == SOCK_STAT_UDP) 
			sprintf((char*)atci.tcmd.arg1, "%c", 'U');
		else CRITICAL_ERRA("wrong sock state(0x%d)", sockstat[sock]);
		sprintf((char*)atci.tcmd.arg2, "%d", sockport[sock]);
		if(sockstat[sock] & SOCK_STAT_CONNECTED) {
			GetDstInfo((uint8)sock, tip, &tport);
			sprintf((char*)atci.tcmd.arg3, "%d.%d.%d.%d", tip[0], tip[1], tip[2], tip[3]);
			sprintf((char*)atci.tcmd.arg4, "%d", tport);
		}
		CMD_RESP_RET(RET_OK, VAL_NONE);
	}
	else CMD_RESP_RET(RET_WRONG_ARG, VAL_NONE);
}

//void act_nopt(void)
//{
//	
//}
#if 0
void act_wset(void)
{
	
}

void act_wstat(void)
{
	
}

void act_wscan(void)
{
	
}

void act_wjoin(void)
{
	
}

void act_wleave(void)
{
	
}

//void act_wsec(void)
//{
//	
//}

//void act_wwps(void)
//{
//	
//}
#endif
void act_mset_q(int8 num)
{
	if(atci.echo == VAL_ENABLE) 
		MAKE_TCMD_CHAR(atci.tcmd.arg1, 'E');
	else MAKE_TCMD_CHAR(atci.tcmd.arg1, 'D');

	if(atci.poll == POLL_MODE_FULL) 
		MAKE_TCMD_CHAR(atci.tcmd.arg3, 'F');
	else if(atci.poll == POLL_MODE_SEMI) 
		MAKE_TCMD_CHAR(atci.tcmd.arg3, 'S');
	else MAKE_TCMD_CHAR(atci.tcmd.arg3, 'D');

	cmd_resp(RET_OK, VAL_NONE);
}

void act_mset_a(int8 echo, int8 mode, int8 poll, int8 country)
{
	DBGA("Set: echo(%c), poll(%c)", echo, poll);
	if(echo == 'E') atci.echo = VAL_ENABLE;
	else if(echo == 'D') atci.echo = VAL_DISABLE;

	if(poll == 'F') atci.poll = POLL_MODE_FULL;
	else if(poll == 'S') atci.poll = POLL_MODE_SEMI;
	else if(poll == 'D') atci.poll = POLL_MODE_NONE;
 DBGA("echo(%d), poll(%d)", atci.echo, atci.poll);
	cmd_resp(RET_OK, VAL_NONE);
}

void act_mstat(void)
{
	MAKE_TCMD_STRING(atci.tcmd.arg1, ARG_3_SIZE, ATC_VERSION);

	cmd_resp(RET_OK, VAL_NONE);
}

void act_mevt_q(void)
{
	int8 i, cnt, *tbuf;

	for(i=ATC_SOCK_NUM_START; i<=ATC_SOCK_NUM_END; i++) {
		if(sockstat[i] != SOCK_STAT_IDLE) cnt++;
	}

	cnt *= 7;	// 개당 7바이트 소요

	tbuf = malloc(cnt+1);
	if(tbuf == NULL) {
		MAKE_TCMD_DIGIT(atci.tcmd.arg1, cnt+1);
		CMD_RESP_RET(RET_NO_FREEMEM, VAL_NONE);
	}
	for(i=ATC_SOCK_NUM_START; i<=ATC_SOCK_NUM_END; i++) {
		if(sockstat[i] != SOCK_STAT_IDLE) 
			sprintf((char*)&tbuf[7*(i-ATC_SOCK_NUM_START)], "%1d,sock\r\n", i);
	}
	MAKE_TCMD_DIGIT(atci.tcmd.arg1, cnt);
	cmd_resp_dump(VAL_NONE, tbuf);
}

void act_mevt_a(int8 id)
{
	int8 event;

	if(event_dequeue(&id, &event) != RET_OK) {
		CMD_RESP_RET(RET_NO_DATA, VAL_NONE);
	}

	if(id <= ATC_SOCK_NUM_END && event == SOCKEVENT_RECV) 
		EVENT_RESP_SIZE(id, event, GetSocketRxRecvBufferSize(id));
	else EVENT_RESP(id, event);
}

//void act_musart(void)
//{
//
//}

//void act_mspi(void)
//{
//
//}

//void act_mprof(void)
//{
//
//}

//void act_fdhcpd(void)
//{
//	
//}

//void act_fdns(void)
//{
//	
//}

//void act_fping(void)
//{
//	
//}

//void act_fgpio(void)
//{
//	
//}

//void act_eset(void)
//{
//	
//}

//void act_estat(void)
//{
//	
//}






#if 0	// 다른 플랫폼에 포팅하는 경우 추가해야 하는 부분 업데이트 할 것

#endif








