
//#define FILE_LOG_SILENCE
#include "protocol/DHCP/dhcp.h"


#define	DHCP_SERVER_PORT		67	// from server to client
#define DHCP_CLIENT_PORT		68	// from client to server

#define DHCP_BOOTREQUEST		1
#define DHCP_BOOTREPLY			2
#define DHCP_HTYPE10MB			1
#define DHCP_HTYPE100MB			2
#define DHCP_HLENETHERNET		6
#define DHCP_HOPS				0
#define DHCP_SECS				0
#define DHCP_UNICAST			0x0
#define DHCP_BROADCAST			0x8000
#define MAGIC_COOKIE			0x63825363
#define HOST_NAME				"WIZnet"

#define	DHCP_MSG_DISCOVER		1	// DHCP message type
#define DHCP_MSG_OFFER			2
#define	DHCP_MSG_REQUEST		3
#define	DHCP_MSG_DECLINE		4
#define	DHCP_MSG_ACK			5
#define DHCP_MSG_NAK			6
#define	DHCP_MSG_RELEASE		7

#define DHCP_START_RETRY_DELAY	60000	// tick
#define DHCP_RETRY_DELAY		5000	// tick
//#define DHCP_INTERVAL_OPEN_RETRY	1000	// tick
//#define DHCP_RECV_WAIT_TIME		3000	// tick
#define DHCP_OPEN_DELAY			0	// tick
#define DHCP_SEND_RETRY_COUNT	3

#define IS_IP_SET(ip_p) (ip_p[0]+ip_p[1]+ip_p[2]+ip_p[3] != 0)
#define IS_TIME_PASSED(tick_v, time_v) (wizpf_tick_elapse(tick_v) > time_v)
#define SET_STATE(_new_state) do { \
	DBGA("DHCP STATE: [%d] => [%d]", di.state, _new_state); \
	di.state = _new_state; \
} while(0)

enum {	//brief	DHCP option and value (cf. RFC1533) 
	padOption				= 0,
	subnetMask				= 1,
	timerOffset				= 2,
	routersOnSubnet			= 3,
	timeServer				= 4,
	nameServer				= 5,
	dns						= 6,
	logServer				= 7,
	cookieServer			= 8,
	lprServer				= 9,
	impressServer			= 10,
	resourceLocationServer	= 11,
	hostName				= 12,
	bootFileSize			= 13,
	meritDumpFile			= 14,
	domainName				= 15,
	swapServer				= 16,
	rootPath				= 17,
	extentionsPath			= 18,
	IPforwarding			= 19,
	nonLocalSourceRouting	= 20,
	policyFilter			= 21,
	maxDgramReasmSize		= 22,
	defaultIPTTL			= 23,
	pathMTUagingTimeout		= 24,
	pathMTUplateauTable		= 25,
	ifMTU					= 26,
	allSubnetsLocal			= 27,
	broadcastAddr			= 28,
	performMaskDiscovery	= 29,
	maskSupplier			= 30,
	performRouterDiscovery	= 31,
	routerSolicitationAddr	= 32,
	staticRoute				= 33,
	trailerEncapsulation	= 34,
	arpCacheTimeout			= 35,
	ethernetEncapsulation	= 36,
	tcpDefaultTTL			= 37,
	tcpKeepaliveInterval	= 38,
	tcpKeepaliveGarbage		= 39,
	nisDomainName			= 40,
	nisServers				= 41,
	ntpServers				= 42,
	vendorSpecificInfo		= 43,
	netBIOSnameServer		= 44,
	netBIOSdgramDistServer	= 45,
	netBIOSnodeType			= 46,
	netBIOSscope			= 47,
	xFontServer				= 48,
	xDisplayManager			= 49,
	dhcpRequestedIPaddr		= 50,
	dhcpIPaddrLeaseTime		= 51,
	dhcpOptionOverload		= 52,
	dhcpMessageType			= 53,
	dhcpServerIdentifier	= 54,
	dhcpParamRequest		= 55,
	dhcpMsg					= 56,
	dhcpMaxMsgSize			= 57,
	dhcpT1value				= 58,
	dhcpT2value				= 59,
	dhcpClassIdentifier		= 60,
	dhcpClientIdentifier	= 61,
	endOption				= 255
};

struct dhcp_msg {
	uint8	op;
	uint8	htype;
	uint8	hlen;
	uint8	hops;
	uint32	xid;
	uint16	secs;
	uint16	flags;
	uint8	ciaddr[4];
	uint8	yiaddr[4];
	uint8	siaddr[4];
	uint8	giaddr[4];
	uint8	chaddr[16];
	uint8	sname[64];
	uint8	file[128];
	uint8	opt[312];
};

struct dhcp_info {
	uint8 srv_ip[4];		// Server IP Address -	get from DHCP packet
	uint8 srv_ip_real[4];	// Real Server IP Address - get from UDP info
	//uint8 assigned_ip[4];	// Old Souce IP Address - previous source IP which will be used for internal ip update check

	uint8 sock;
	uint8 state;
	uint8 action;
	uint32 lease_time;
	uint32 renew_time;
	uint32 rebind_time;
	uint32 xid;
	pFunc ip_update;
	pFunc ip_conflict;
};

static void dhcp_alarm_cb(int8 arg);	// for alarm mode;
static void dhcp_async_cb(uint8 sock, uint8 item, int32 ret);
static void dhcp_run(void);
static void dhcp_fail(void);
static int8 recv_handler(void);
static int8 send_discover(void);
static int8 send_request(void);
//static int8 send_rel_dec(int8 msgtype);
//static int8 send_checker(void);
//static void default_ip_update(void);
//static void default_ip_conflict(void);


static wiz_NetInfo workinfo, storage;
static struct dhcp_info di;
static struct dhcp_msg dm;
static bool dhcp_alarm = FALSE;
static bool dhcp_async = FALSE;
//static bool   dhcp_run_wait = FALSE;
static uint8  dhcp_run_cnt = 0;
static uint32 dhcp_run_tick = 0;

int8 dhcp_init(uint8 sock, pFunc ip_update_hook, pFunc ip_conflict_hook, wiz_NetInfo *def)
{
	if(sock >= TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", sock);
		return RET_NOK;
	}

#ifdef DHCP_ALARM
		dhcp_alarm = TRUE;
#endif
#ifdef DHCP_ASYNC
		dhcp_async = TRUE;
#endif

	memset(&di, 0, sizeof(di));	//memset(&workinfo, 0, sizeof(workinfo));
	memcpy(&storage, def, sizeof(storage));	//memcpy(workinfo.Mac, storage.Mac, 6);
	memset(&workinfo, 0, sizeof(workinfo));
	memcpy(workinfo.Mac, storage.Mac, 6);
	SetNetInfo(&workinfo);
	di.xid = 0x12345678;
	di.sock = sock;
	if(ip_update_hook) di.ip_update = ip_update_hook;
	if(ip_conflict_hook) di.ip_conflict = ip_conflict_hook;
	
	clearSIPR();
	//IINCHIP_WRITE(WIZC_SIPR0, 0);
	//IINCHIP_WRITE(WIZC_SIPR1, 0);
	//IINCHIP_WRITE(WIZC_SIPR2, 0);
	//IINCHIP_WRITE(WIZC_SIPR3, 0);

	// ToDo: Remove setting zero IP & SN (set at start func)

	return RET_OK;
}

int8 dhcp_manual(int8 action, uint8 *saved_ip, uint32 *renew, uint32 *rebind)	// blocking function
{
	int8 curstt = dhcp_get_state();

	if(dhcp_alarm == TRUE) return RET_NOK;

	while(curstt != DHCP_STATE_INIT && curstt != DHCP_STATE_BOUND) {	// 어중간한 상황이면 좀 더 돌려준다
		dhcp_run();
		curstt = dhcp_get_state();
	}

	if(curstt == DHCP_STATE_INIT) {		// INIT상태이면
		di.action = DHCP_ACT_START;
		memset(&workinfo, 0, sizeof(workinfo));
		if(saved_ip) memcpy(workinfo.IP, saved_ip, 4);
		workinfo.DHCP = NETINFO_DHCP_BUSY;
		SetNetInfo(&workinfo);
		do {	// 돌려준다
			dhcp_run();
			curstt = dhcp_get_state();
		} while((curstt != DHCP_STATE_INIT || workinfo.DHCP == NETINFO_DHCP_BUSY) 
			&& curstt != DHCP_STATE_BOUND);
		if(curstt != DHCP_STATE_BOUND) return RET_NOK;
		if(renew) *renew = di.renew_time;
		if(rebind) *rebind = di.rebind_time;
	} else if(curstt == DHCP_STATE_BOUND) {
		if(action == DHCP_ACT_START) {
			if(renew) *renew = 0;
			if(rebind) *rebind = 0;
			return RET_OK;
		} else if(action == DHCP_ACT_RENEW) {	// renew
			SET_STATE(DHCP_STATE_SELECTING);
			di.action = DHCP_ACT_RENEW;
			di.xid++;
		} else if(action == DHCP_ACT_REBIND) {	// rebind
			SET_STATE(DHCP_STATE_SELECTING);
			di.action = DHCP_ACT_REBIND;
			di.xid++;
		} else {
			ERRA("wrong action(%d)", action);
			return RET_NOK;
		}
		curstt = dhcp_get_state();
		while(curstt != DHCP_STATE_INIT && curstt != DHCP_STATE_BOUND) {	// 돌려준다
			dhcp_run();
			curstt = dhcp_get_state();
		}
		if(curstt != DHCP_STATE_BOUND) return RET_NOK;
		if(renew) *renew = di.renew_time;
		if(rebind) *rebind = di.rebind_time;
	}

	return RET_OK;
}

int8 dhcp_get_state(void)
{
	return di.state;
}

int8 dhcp_set_storage(wiz_NetInfo *net)	// MAC수정 시 반드시 업데이트 할 것 (아니면 dhcp시 문제)
{
	if(net == NULL) {
		DBG("NULL arg");
		return RET_NOK;
	}

	if(net->Mac[0]!=0 || net->Mac[1]!=0 || net->Mac[2]!=0 || net->Mac[3]!=0 || 
		net->Mac[2]!=0 || net->Mac[3]!=0) memcpy(storage.Mac, net->Mac, 6);
	if(net->IP[0]!=0 || net->IP[1]!=0 || net->IP[2]!=0 || net->IP[3]!=0)
		memcpy(storage.IP, net->IP, 4);
	if(net->SN[0]!=0 || net->SN[1]!=0 || net->SN[2]!=0 || net->SN[3]!=0)
		memcpy(storage.SN, net->SN, 4);
	if(net->GW[0]!=0 || net->GW[1]!=0 || net->GW[2]!=0 || net->GW[3]!=0)
		memcpy(storage.GW, net->GW, 4);
	if(net->DNS[0]!=0 || net->DNS[1]!=0 || net->DNS[2]!=0 || net->DNS[3]!=0)
		memcpy(storage.DNS, net->DNS, 4);

	return RET_OK;
}

int8 dhcp_get_storage(wiz_NetInfo *net)
{
	if(net == NULL) {
		DBG("NULL arg");
		return RET_NOK;
	}

	memcpy(net->Mac, storage.Mac, 6);
	memcpy(net->IP, storage.IP, 4);
	memcpy(net->SN, storage.SN, 4);
	memcpy(net->GW, storage.GW, 4);
	memcpy(net->DNS, storage.DNS, 4);

	return RET_OK;
}

int8 dhcp_static_mode(wiz_NetInfo *net)
{
	wiz_NetInfo cur;

	if(net == NULL) {
		DBG("NULL arg");
		return RET_NOK;
	}

	GetNetInfo(&cur);
	if(cur.DHCP == NETINFO_STATIC) {
		DBG("Already Static Mode");
		return RET_NOK;
	}
	di.action = DHCP_ACT_NONE;
	SET_STATE(DHCP_STATE_INIT);

	// 입력된 값을 기존 값에 업데이트 & 빈 값은 기존 값 가져와서 설정하기
	if(net->IP[0]!=0 || net->IP[1]!=0 || net->IP[2]!=0 || net->IP[3]!=0)
		memcpy(storage.IP, net->IP, 4);		// dhcp_fail, static에서 사용하는 si를 설정
	else memcpy(net->IP, storage.IP, 4);
	if(net->SN[0]!=0 || net->SN[1]!=0 || net->SN[2]!=0 || net->SN[3]!=0)
		memcpy(storage.SN, net->SN, 4);
	else memcpy(net->SN, storage.SN, 4);
	if(net->GW[0]!=0 || net->GW[1]!=0 || net->GW[2]!=0 || net->GW[3]!=0)
		memcpy(storage.GW, net->GW, 4);
	else memcpy(net->GW, storage.GW, 4);
	if(net->DNS[0]!=0 || net->DNS[1]!=0 || net->DNS[2]!=0 || net->DNS[3]!=0)
		memcpy(storage.DNS, net->DNS, 4);
	else memcpy(net->DNS, storage.DNS, 4);

	net->DHCP = NETINFO_STATIC;
	SetNetInfo(net);
	if(dhcp_alarm) alarm_del(dhcp_alarm_cb, -1);	// 모든 알람 제거
	//send_checker_NB();	// 나중에 구현

	return RET_OK;
}

int8 dhcp_alarm_start(uint8 *saved_ip)
{
	GetNetInfo(&workinfo);
	if(workinfo.DHCP > NETINFO_STATIC) {
		DBGA("Already DHCP Mode(%d)", workinfo.DHCP);
		return RET_NOK;
	} else DBG("DHCP Start");
	SET_STATE(DHCP_STATE_INIT);
	di.action = DHCP_ACT_START;

	memset(&workinfo, 0, sizeof(workinfo));
	if(saved_ip) memcpy(workinfo.IP, saved_ip, 4);
	workinfo.DHCP = NETINFO_DHCP_BUSY;
	SetNetInfo(&workinfo);
	if(dhcp_alarm) alarm_set(10, dhcp_alarm_cb, 0);	// 알람 등록
	return RET_OK;
}

static void dhcp_alarm_cb(int8 arg)	// for alarm mode
{
	if(dhcp_alarm == FALSE) return;
	if(arg == 0) {
		if(workinfo.DHCP == NETINFO_DHCP_FAIL) {
			workinfo.DHCP = NETINFO_DHCP_BUSY;
			di.action = DHCP_ACT_START;
		}
		if(dhcp_get_state() == DHCP_STATE_IP_CHECK) {
			alarm_set(wizpf_tick_conv(FALSE, di.renew_time), dhcp_alarm_cb, 1);
			alarm_set(wizpf_tick_conv(FALSE, di.rebind_time), dhcp_alarm_cb, 2);
		}
		dhcp_run();	//alarm_set은 dhcp_run 안에서 필요 시 실행되게 수정됨
	} else if(arg == 1) {	// renew
		SET_STATE(DHCP_STATE_SELECTING);
		di.action = DHCP_ACT_RENEW;
		di.xid++;
		alarm_set(10, dhcp_alarm_cb, 0);
	} else if(arg == 2) {	// rebind
		SET_STATE(DHCP_STATE_SELECTING);
		di.action = DHCP_ACT_REBIND;
		di.xid++;
		alarm_set(10, dhcp_alarm_cb, 0);
	}
}

static void dhcp_async_cb(uint8 sock, uint8 item, int32 ret)	// for async mode
{
	switch(item) {
	case WATCH_SOCK_UDP_SEND:
		if(dhcp_alarm) alarm_set(10, dhcp_alarm_cb, 0);
		dhcp_run_tick = wizpf_get_systick();
		if(ret == RET_OK) {
			DBG("DHCP Discovery Sent Async");
			if(di.state == DHCP_STATE_INIT) SET_STATE(DHCP_STATE_SEARCHING);
			else if(di.state == DHCP_STATE_SELECTING) SET_STATE(DHCP_STATE_REQUESTING);
			else DBGCRTCA(TRUE, "wrong state(%d)", di.state);
		} else {
			DBGA("WATCH_SOCK_UDP_SEND fail - ret(%d)", ret);
		}
		break;
	case WATCH_SOCK_TCP_SEND:
	case WATCH_SOCK_CONN_TRY:
	case WATCH_SOCK_CLS_TRY:
	case WATCH_SOCK_CONN_EVT:
	case WATCH_SOCK_CLS_EVT:
	case WATCH_SOCK_RECV:
		DBGCRTC(TRUE, "DHCP does not use TCP");
	default: DBGCRTCA(TRUE, "wrong item(0x%x)", item);
	}

}

static void dhcp_run(void)
{
	static bool udp_open_fail = FALSE;

	if(di.state == DHCP_STATE_INIT && di.action != DHCP_ACT_START) {
		DBG("wrong attempt");
		return;
	} else if(GetUDPSocketStatus(di.sock) == SOCKSTAT_CLOSED) {	// 소켓이 closed이면 open함
		if(udp_open_fail == TRUE && !IS_TIME_PASSED(dhcp_run_tick, DHCP_RETRY_DELAY)) 
			goto RET_ALARM;
		if(UDPOpen(di.sock, DHCP_CLIENT_PORT) == RET_OK) {
			if(dhcp_async) sockwatch_open(di.sock, dhcp_async_cb);
			udp_open_fail = FALSE;
			dhcp_run_tick = wizpf_get_systick();
			dhcp_run_cnt = 0;
		} else {
			ERR("UDPOpen fail");
			udp_open_fail = TRUE;
			dhcp_run_tick = wizpf_get_systick();
			goto RET_ALARM;
		}
	}

	switch(di.state) {
	case DHCP_STATE_INIT:	// init reboot은 담에 필요하면 flash 사용해서 해볼 것 - 일단 무시
		//if(IS_IP_SET(di.assigned_ip)) {DBG("INIT-REBOOT");		// INIT-REBOOT - Request previous IP
		//	SET_STATE(DHCP_STATE_REQUESTING);
		//	dhcp_run_tick = wizpf_get_systick();
		//} else 
		if(dhcp_run_cnt==0 && !IS_TIME_PASSED(dhcp_run_tick, DHCP_OPEN_DELAY)) 
			goto RET_ALARM;

		if(dhcp_run_cnt < DHCP_SEND_RETRY_COUNT) {
			dhcp_run_cnt++;
			if(send_discover() == RET_OK) {	// Discover ok
				if(dhcp_async) {
					DBG("DHCP Discovery Send Async");
					sockwatch_set(di.sock, WATCH_SOCK_UDP_SEND);
					return;	// alarm등록은 Async Send완료된 후에 다시 함
				} else {
					DBG("DHCP Discovery Sent");
					SET_STATE(DHCP_STATE_SEARCHING);
					dhcp_run_tick = wizpf_get_systick();
				}
			} else {
				ERRA("DHCP Discovery SEND fail - (%d)times", dhcp_run_cnt);
				dhcp_run_tick = wizpf_get_systick();
			}
		} else {
			ERRA("DHCP Discovery SEND fail - (%d)times", dhcp_run_cnt);
			dhcp_run_cnt = 0;
			UDPClose(di.sock);
			if(dhcp_async) sockwatch_close(di.sock);
			dhcp_fail();	//dhcp_run_tick = wizpf_get_systick();
			return; // alarm set 안함
		}
		break;
	case DHCP_STATE_SEARCHING:
		if(!IS_TIME_PASSED(dhcp_run_tick, DHCP_RETRY_DELAY)) {
			int8 ret = recv_handler();
			if(ret == DHCP_MSG_OFFER) {
				SET_STATE(DHCP_STATE_SELECTING);
				dhcp_run_tick = wizpf_get_systick();
				dhcp_run_cnt = 0;
			} else if(ret != RET_NOK) DBGCRTCA(TRUE, "recv wrong packet(%d)", ret);
		} else {
			ERRA("DHCP Offer RECV fail - for (%d)msec", DHCP_RETRY_DELAY);
			SET_STATE(DHCP_STATE_INIT);
			dhcp_run_tick = wizpf_get_systick();
		}
		break;
	case DHCP_STATE_SELECTING:
		if(dhcp_run_cnt < DHCP_SEND_RETRY_COUNT) {
			dhcp_run_cnt++;
			if(send_request() == RET_OK) {	// Request ok
				if(dhcp_async) {
					DBG("DHCP Request Send Async");
					sockwatch_set(di.sock, WATCH_SOCK_UDP_SEND);
					return;	// alarm등록은 Async Send완료된 후에 다시 함
				} else {
					DBG("DHCP Request Sent");
					SET_STATE(DHCP_STATE_REQUESTING);
					dhcp_run_tick = wizpf_get_systick();
				}
			} else {
				ERRA("DHCP Request SEND fail - (%d)times", dhcp_run_cnt);
				dhcp_run_tick = wizpf_get_systick();
			}
		} else {
			ERRA("DHCP Request SEND fail - (%d)times", dhcp_run_cnt);
			dhcp_run_cnt = 0;
			UDPClose(di.sock);
			if(dhcp_async) sockwatch_close(di.sock);
			dhcp_fail();	//dhcp_run_tick = wizpf_get_systick();
			return; // alarm set 안함
		}
		break;
	case DHCP_STATE_REQUESTING:
		if(!IS_TIME_PASSED(dhcp_run_tick, DHCP_RETRY_DELAY)) {
			int8 ret = recv_handler();
			if(ret == DHCP_MSG_ACK) {	// Recv ACK
				LOG("DHCP Success");
				SET_STATE(DHCP_STATE_IP_CHECK);
				dhcp_run_tick = wizpf_get_systick();		// ??
				dhcp_run_cnt = 0;
			} else if(ret == DHCP_MSG_NAK) {	// Recv NAK
				if(di.action == DHCP_ACT_START) {
					SET_STATE(DHCP_STATE_INIT);
					dhcp_run_tick = wizpf_get_systick();
				} else {
					SET_STATE(DHCP_STATE_BOUND);
				}
				dhcp_run_cnt = 0;
			} else if(ret != RET_NOK) DBGCRTCA(TRUE, "recv wrong packet(%d)", ret);
		} else {
			ERRA("DHCP ACK RECV fail - for (%d)msec", DHCP_RETRY_DELAY);
			if(di.action == DHCP_ACT_START) {
				SET_STATE(DHCP_STATE_INIT);
				dhcp_run_tick = wizpf_get_systick();
			} else {
				SET_STATE(DHCP_STATE_BOUND);
			}
		}
		break;
	case DHCP_STATE_IP_CHECK:	// 나중에 send_checker_NB로 갈아탈 것??, 일단 임시제거
		//if(send_checker() == RET_OK) {	// IP 체크 이상없음
			SET_STATE(DHCP_STATE_BOUND);
			workinfo.DHCP = NETINFO_DHCP_STABLE;
			SetNetInfo(&workinfo);
			if(di.ip_update) di.ip_update();
			LOGA("DHCP ok - New IP (%d.%d.%d.%d)", workinfo.IP[0], workinfo.IP[1], workinfo.IP[2], workinfo.IP[3]);
			//bound_tick = wizpf_get_systick();
			UDPClose(di.sock);
			if(dhcp_async) sockwatch_close(di.sock);
		//} else {
		//	SET_STATE(DHCP_STATE_INIT);
		//	ERR("IP Addr conflicted - IP(%d.%d.%d.%d)", workinfo.IP[0], workinfo.IP[1], workinfo.IP[2], workinfo.IP[3]);
		//	send_rel_dec(DHCP_MSG_DECLINE);
		//	if(di.ip_conflict) (*di.ip_conflict)();
		//}
		break;
	case DHCP_STATE_BOUND:
		return; // alarm set 안함
	default:
		ERRA("wrong state(%d)", di.state);
		return; // alarm set 안함
	}

RET_ALARM:
	if(dhcp_alarm) alarm_set(10, dhcp_alarm_cb, 0);
}

static void dhcp_fail(void)
{
	LOG("DHCP Fail - set temp addr");
	di.action = DHCP_ACT_NONE;
	SET_STATE(DHCP_STATE_INIT);	//storage.DHCP = 0;	// 오류 방지용
	memcpy(&workinfo, &storage, sizeof(storage));	// 일단 디폴트(혹은 이전) 값을 설정
	memset(workinfo.Mac, 0, 6);	// mac설정은 DHCP권한이 아님 (버그 방지)
	workinfo.DHCP = NETINFO_DHCP_FAIL;
	SetNetInfo(&workinfo);	// DHCP 상태를 설정
	network_disp(&workinfo);
	if(dhcp_alarm) 
		alarm_set(DHCP_START_RETRY_DELAY, dhcp_alarm_cb, 0);	// 알람 등록
	//send_checker_NB();	// 나중에 구현
}

static int8 recv_handler(void)
{
	uint8 *cur, *end;
	uint8 recv_ip[4], opt_len, msg_type;
	int32 recv_len;
	uint16 recv_port;

	recv_len = GetSocketRxRecvBufferSize(di.sock);
	if(recv_len == 0) return RET_NOK;
	else memset(&dm, 0, sizeof(struct dhcp_msg));

	recv_len = UDPRecv(di.sock, (int8*)&dm, sizeof(struct dhcp_msg), recv_ip, &recv_port);
	if(recv_len < 0) {
		ERRA("UDPRecv fail - ret(%d)", recv_len);
		return RET_NOK;
	}

	DBGA("DHCP_SIP:%d.%d.%d.%d",di.srv_ip[0],di.srv_ip[1],di.srv_ip[2],di.srv_ip[3]);
	DBGA("DHCP_RIP:%d.%d.%d.%d",di.srv_ip_real[0],di.srv_ip_real[1],di.srv_ip_real[2],di.srv_ip_real[3]);
	DBGA("recv_ip:%d.%d.%d.%d",recv_ip[0],recv_ip[1],recv_ip[2],recv_ip[3]);
	
	if(dm.op != DHCP_BOOTREPLY || recv_port != DHCP_SERVER_PORT) {
		if(dm.op != DHCP_BOOTREPLY) DBG("DHCP : NO DHCP MSG");
		if(recv_port != DHCP_SERVER_PORT)  DBG("DHCP : WRONG PORT");
		return RET_NOK;
	}

	if(memcmp(dm.chaddr, storage.Mac, 6) != 0 || dm.xid != htonl(di.xid)) {
		DBG("No My DHCP Message. This message is ignored.");
		DBGA("SRC_MAC_ADDR(%02X:%02X:%02X:%02X:%02X:%02X)", 
			storage.Mac[0], storage.Mac[1], storage.Mac[2], 
			storage.Mac[3], storage.Mac[4], storage.Mac[5]);
		DBGA("chaddr(%02X:%02X:%02X:%02X:%02X:%02X)", dm.chaddr[0], 
			dm.chaddr[1], dm.chaddr[2], dm.chaddr[3], dm.chaddr[4], dm.chaddr[5]);
		DBGA("DHCP_XID(%08lX), xid(%08lX), yiaddr(%d.%d.%d.%d)", htonl(di.xid), 
			dm.xid, dm.yiaddr[0], dm.yiaddr[1], dm.yiaddr[2], dm.yiaddr[3]);
		return RET_NOK;
	}

	if( *((uint32*)di.srv_ip) != 0x00000000 ) {
		if( *((uint32*)di.srv_ip_real) != *((uint32*)recv_ip) && 
										*((uint32*)di.srv_ip) != *((uint32*)recv_ip) ) 	{
			DBG("Another DHCP sever send a response message. This is ignored.");
			DBGA("IP:%d.%d.%d.%d",recv_ip[0],recv_ip[1],recv_ip[2],recv_ip[3]);
			return RET_NOK;
		}
	}
	
	memcpy(workinfo.IP, dm.yiaddr, 4);
	DBG("DHCP MSG received..");
	DBGA("yiaddr : %d.%d.%d.%d",workinfo.IP[0],workinfo.IP[1],workinfo.IP[2],workinfo.IP[3]);

	// 길이로 에러 검출
	msg_type = 0;
	cur = (uint8 *)(&dm.op);
	cur = cur + 240;
	end = cur + (recv_len - 240);	//printf("cur : 0x%08X  end : 0x%08X  recv_len : %d\r\n", cur, end, recv_len);
	while ( cur < end ) 
	{
		switch ( *cur++ ) 
		{
		case padOption:
			break;
		case endOption:
			return msg_type;
		case dhcpMessageType:
			opt_len = *cur++;
			msg_type = *cur;
			DBGA("dhcpMessageType : %x", msg_type);
			break;
		case subnetMask:
			opt_len =* cur++;
			memcpy(workinfo.SN,cur,4);
			DBGA("subnetMask : %d.%d.%d.%d",
				workinfo.SN[0],workinfo.SN[1],workinfo.SN[2],workinfo.SN[3]);
			break;
		case routersOnSubnet:
			opt_len = *cur++;
			memcpy(workinfo.GW,cur,4);
			DBGA("routersOnSubnet : %d.%d.%d.%d",
				workinfo.GW[0],workinfo.GW[1],workinfo.GW[2],workinfo.GW[3]);
			break;
		case dns:
			opt_len = *cur++;
			memcpy(workinfo.DNS,cur,4);
			break;
		case dhcpIPaddrLeaseTime:
			opt_len = *cur++;
			di.lease_time = ntohl(*((uint32*)cur));
			di.renew_time = di.lease_time / 2;			// 0.5
			di.rebind_time = di.lease_time / 8 * 7;		// 0.875
			DBGA("lease(%d), renew(%d), rebind(%d)", di.lease_time, di.renew_time, di.rebind_time);
			break;
		case dhcpServerIdentifier:
			opt_len = *cur++;
			DBGA("DHCP_SIP : %d.%d.%d.%d", di.srv_ip[0], di.srv_ip[1], di.srv_ip[2], di.srv_ip[3]);
			if( *((uint32*)di.srv_ip) == 0 || *((uint32*)di.srv_ip_real) == *((uint32*)recv_ip) || 
													*((uint32*)di.srv_ip) == *((uint32*)recv_ip) ) {
				memcpy(di.srv_ip,cur,4);
				memcpy(di.srv_ip_real,recv_ip,4);	// Copy the real ip address of my DHCP server
				DBGA("My dhcpServerIdentifier : %d.%d.%d.%d", 
					di.srv_ip[0], di.srv_ip[1], di.srv_ip[2], di.srv_ip[3]);
				DBGA("My DHCP server real IP address : %d.%d.%d.%d", 
					di.srv_ip_real[0], di.srv_ip_real[1], di.srv_ip_real[2], di.srv_ip_real[3]);
			} else {
				DBGA("Another dhcpServerIdentifier : MY(%d.%d.%d.%d)", 
					di.srv_ip[0], di.srv_ip[1], di.srv_ip[2], di.srv_ip[3]);
				DBGA("Another(%d.%d.%d.%d)", recv_ip[0], recv_ip[1], recv_ip[2], recv_ip[3]);
			}
			break;
		default:
			opt_len = *cur++;
			DBGA("opt_len : %d", opt_len);
			break;
		} // switch
		cur+=opt_len;
	} // while

	return RET_NOK;
}

static int8 send_discover(void)
{
	uint8 srv_ip[4];
	int32 len = 0;

	memset(&dm, 0, sizeof(struct dhcp_msg));

	*((uint32*)di.srv_ip)=0;
	*((uint32*)di.srv_ip_real)=0;

	dm.op = DHCP_BOOTREQUEST;
	dm.htype = DHCP_HTYPE10MB;
	dm.hlen = DHCP_HLENETHERNET;
	dm.hops = DHCP_HOPS;
	dm.xid = htonl(di.xid);
	dm.secs = htons(DHCP_SECS);
	memcpy(dm.chaddr, storage.Mac, 6);
	dm.flags = htons(DHCP_BROADCAST);

	// MAGIC_COOKIE 
	*(uint32*)&dm.opt[len] = htonl(MAGIC_COOKIE);
	len += 4;

	// Option Request Param. 
	dm.opt[len++] = dhcpMessageType;
	dm.opt[len++] = 0x01;
	dm.opt[len++] = DHCP_MSG_DISCOVER;

	// Client identifier
	dm.opt[len++] = dhcpClientIdentifier;
	dm.opt[len++] = 0x07;
	dm.opt[len++] = 0x01;
	memcpy(&dm.opt[len], storage.Mac, 6);
	len += 6;
	
	// host name
	dm.opt[len++] = hostName;
	dm.opt[len++] = strlen(HOST_NAME) + 6; // length of hostname + 3
	strcpy((char*)&dm.opt[len], HOST_NAME);
	len += strlen(HOST_NAME);
	sprintf((char*)&dm.opt[len], "%02x%02x%02x", 
		storage.Mac[3], storage.Mac[4], storage.Mac[5]);
	len += 6;

	dm.opt[len++] = dhcpParamRequest;
	dm.opt[len++] = 0x06;
	dm.opt[len++] = subnetMask;
	dm.opt[len++] = routersOnSubnet;
	dm.opt[len++] = dns;
	dm.opt[len++] = domainName;
	dm.opt[len++] = dhcpT1value;
	dm.opt[len++] = dhcpT2value;
	dm.opt[len++] = endOption;

	// send broadcasting packet 
	srv_ip[0] = srv_ip[1] = srv_ip[2] = srv_ip[3] = 255;

	if(dhcp_async) {
		len = UDPSendNB(di.sock, (int8*)&dm, sizeof(struct dhcp_msg), srv_ip, DHCP_SERVER_PORT);
		if(len < sizeof(struct dhcp_msg)) {
			if(len < 0) ERRA("UDPSend fail - ret(%d)", len);
			else ERRA("UDPSend sent less than size - size(%d), sent(%d)", 
				sizeof(struct dhcp_msg), len);
			return RET_NOK;
		} else sockwatch_set(di.sock, WATCH_SOCK_UDP_SEND);
	} else {
		len = UDPSend(di.sock, (int8*)&dm, sizeof(struct dhcp_msg), srv_ip, DHCP_SERVER_PORT);
		if(len <= 0) {
			ERRA("UDPSend fail - ret(%d)", len);
			return RET_NOK;
		}
	}

	return RET_OK;
}

static int8 send_request(void)
{
	uint8 srv_ip[4];
	int32 len = 0;

	memset(&dm, 0, sizeof(struct dhcp_msg));

	dm.op = DHCP_BOOTREQUEST;
	dm.htype = DHCP_HTYPE10MB;
	dm.hlen = DHCP_HLENETHERNET;
	dm.hops = DHCP_HOPS;
	dm.xid = htonl(di.xid);
	dm.secs = htons(DHCP_SECS);

	if(di.action == DHCP_ACT_RENEW) {
		dm.flags = 0;		// For Unicast
		memcpy(dm.ciaddr, workinfo.IP, 4);
	} else {
		dm.flags = htons(DHCP_BROADCAST);
	}

	memcpy(dm.chaddr, storage.Mac, 6);

	// MAGIC_COOKIE 
	*(uint32*)&dm.opt[len] = htonl(MAGIC_COOKIE);
	len += 4;

	// Option Request Param. 
	dm.opt[len++] = dhcpMessageType;
	dm.opt[len++] = 0x01;
	dm.opt[len++] = DHCP_MSG_REQUEST;

	dm.opt[len++] = dhcpClientIdentifier;
	dm.opt[len++] = 0x07;
	dm.opt[len++] = 0x01;
	memcpy(&dm.opt[len], storage.Mac, 6);
	len += 6;

	if(di.action != DHCP_ACT_RENEW) {
		dm.opt[len++] = dhcpRequestedIPaddr;
		dm.opt[len++] = 0x04;
		memcpy(&dm.opt[len], workinfo.IP, 4);
		len += 4;
		dm.opt[len++] = dhcpServerIdentifier;
		dm.opt[len++] = 0x04;
		memcpy(&dm.opt[len], di.srv_ip, 4);
		len += 4;
	}
	
	// host name
	dm.opt[len++] = hostName;
	dm.opt[len++] = strlen(HOST_NAME) + 6; // length of hostname + 3
	strcpy((char*)&dm.opt[len], HOST_NAME);
	len += strlen(HOST_NAME);
	sprintf((char*)&dm.opt[len], "%02x%02x%02x", 
		storage.Mac[3], storage.Mac[4], storage.Mac[5]);
	len += 6;

	dm.opt[len++] = dhcpParamRequest;
	dm.opt[len++] = 0x08;
	dm.opt[len++] = subnetMask;
	dm.opt[len++] = routersOnSubnet;
	dm.opt[len++] = dns;
	dm.opt[len++] = domainName;
	dm.opt[len++] = dhcpT1value;
	dm.opt[len++] = dhcpT2value;
	dm.opt[len++] = performRouterDiscovery;
	dm.opt[len++] = staticRoute;
	dm.opt[len++] = endOption;

	// send broadcasting packet 
	if(di.action == DHCP_ACT_RENEW) {
		memcpy(srv_ip, di.srv_ip, 4);
	} else {
		srv_ip[0] = srv_ip[1] = srv_ip[2] = srv_ip[3] = 255;
	}

	if(dhcp_async) {
		len = UDPSendNB(di.sock, (int8*)&dm, sizeof(struct dhcp_msg), srv_ip, DHCP_SERVER_PORT);
		if(len < sizeof(struct dhcp_msg)) {
			if(len < 0) ERRA("UDPSend fail - ret(%d)", len);
			else ERRA("UDPSend sent less than size - size(%d), sent(%d)", sizeof(struct dhcp_msg), len);
			return RET_NOK;
		} else sockwatch_set(di.sock, WATCH_SOCK_UDP_SEND);
	} else {
		len = UDPSend(di.sock, (int8*)&dm, sizeof(struct dhcp_msg), srv_ip, DHCP_SERVER_PORT);
		if(len <= 0) {
			ERRA("UDPSend fail - ret(%d)", len);
			return RET_NOK;
		}
	}

	return RET_OK;
}

/*
static int8 send_rel_dec(int8 msgtype)
{
	int32 len =0;
	uint8 srv_ip[4];

	memset(&dm, 0, sizeof(struct dhcp_msg));

	dm.op = DHCP_BOOTREQUEST;
	dm.htype = DHCP_HTYPE10MB;
	dm.hlen = DHCP_HLENETHERNET;
	dm.hops = DHCP_HOPS;
	dm.xid = htonl(di.xid);
	dm.secs = htons(DHCP_SECS);
	dm.flags = DHCP_UNICAST;	//DHCP_BROADCAST;
	memcpy(dm.chaddr, storage.Mac, 6);

	// MAGIC_COOKIE 
	*(uint32*)&dm.opt[len] = htonl(MAGIC_COOKIE);
	len += 4;

	// Option Request Param. 
	dm.opt[len++] = dhcpMessageType;
	dm.opt[len++] = 0x01;
	if(msgtype == DHCP_MSG_DECLINE) dm.opt[len++] = DHCP_MSG_DECLINE;
	else  dm.opt[len++] = DHCP_MSG_RELEASE;

	dm.opt[len++] = dhcpClientIdentifier;
	dm.opt[len++] = 0x07;
	dm.opt[len++] = 0x01;
	memcpy(&dm.opt[len], storage.Mac, 6);
	len += 6;

	dm.opt[len++] = dhcpServerIdentifier;
	dm.opt[len++] = 0x04;
	memcpy(&dm.opt[len], di.srv_ip, 4);
	len += 4;

	if(msgtype == DHCP_MSG_DECLINE) {
		dm.opt[len++] = dhcpRequestedIPaddr;
		dm.opt[len++] = 0x04;
		memcpy(&dm.opt[len], workinfo.IP, 4);
		len += 4;
		dm.opt[len++] = endOption;
		srv_ip[0] = srv_ip[1] = srv_ip[2] = srv_ip[3] = 255;
	} else {
		dm.opt[len++] = endOption;
		memcpy(srv_ip, di.srv_ip, 4);
	}

	if(dhcp_async) {
		len = UDPSendNB(di.sock, (int8*)&dm, sizeof(struct dhcp_msg), srv_ip, DHCP_SERVER_PORT);
		if(len < sizeof(struct dhcp_msg)) {
			if(len < 0) ERRA("UDPSend fail - ret(%d)", len);
			else ERRA("UDPSend sent less than size - size(%d), sent(%d)", sizeof(struct dhcp_msg), len);
			return RET_NOK;
		} else sockwatch_set(di.sock, WATCH_SOCK_UDP_SEND);
	} else {
		len = UDPSend(di.sock, (int8*)&dm, sizeof(struct dhcp_msg), srv_ip, DHCP_SERVER_PORT);
		if(len <= 0) {
			ERRA("UDPSend fail - ret(%d)", len);
			return RET_NOK;
		}
	}

	return RET_OK;
}
*/

/*
static int8 send_checker(void)
{
	int16 len;

	if(dhcp_async) {
		len = UDPSendNB(di.sock, "CHECK_IP_CONFLICT", 17, workinfo.IP, 5000);
		if(len < sizeof(struct dhcp_msg)) {
			if(len < 0) ERRA("UDPSend fail - ret(%d)", len);
			else ERRA("UDPSend sent less than size - size(%d), sent(%d)", 17, len);
			return RET_NOK;
		} else sockwatch_set(di.sock, WATCH_SOCK_UDP_SEND);
	} else {
		len = UDPSend(di.sock, "CHECK_IP_CONFLICT", 17, workinfo.IP, 5000);
		if(len > 0) {	// sendto is complete. that means there is a node which has a same IP.
			ERR("IP Conflict");
			return RET_NOK;
		}
	}
	
	return RET_OK;
}

static int8 send_checker_NB(void)
{
	// Todo
	return RET_OK;
}
*/
//static void default_ip_update(void){}
//static void default_ip_conflict(void){}




