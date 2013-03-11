/**
 * @file		dhcp.c
 * @brief		DHCP Protocol Module Source File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	modified by Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

//#define FILE_LOG_SILENCE
#include "common/common.h"
#if (USE_DHCP == VAL_DISABLE)
#include "protocol/DHCP/dhcp.h"
#endif

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
#define DHCP_OPEN_DELAY			0		// tick
#define DHCP_SEND_RETRY_COUNT	3

#define IS_IP_SET(ip_p) (ip_p[0]+ip_p[1]+ip_p[2]+ip_p[3] != 0)
#define IS_TIME_PASSED(tick_v, time_v) (wizpf_tick_elapse(tick_v) > time_v)
#define SET_STATE(_new_state) do { \
	DBGA("DHCP STATE: [%d] => [%d]", di.state, _new_state); \
	di.state = _new_state; \
} while(0)

enum {	//DHCP option and value (cf. RFC1533) 
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
	uint8 sock;
	dhcp_state state;
	dhcp_action action;
	uint32 lease_time;
	uint32 renew_time;
	uint32 rebind_time;
	uint32 xid;
	void_func ip_update;
	void_func ip_conflict;
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
static uint8  dhcp_run_cnt = 0;
static uint32 dhcp_run_tick = 0;


/**
 * @addtogroup dhcp_module
 * @{
 */

/**
 * Initialize DHCP module.
 * This should be called just one time at first time
 *
 * @param sock Socket number to use
 * @param ip_update_hook Callback function for IP-update hooking
 * @param ip_conflict_hook Callback function for IP-conflict hooking (Not implemented yet)
 * @param def Default Address to set
 * @return RET_OK: Success
 * @return RET_NOK: Error
 */
int8 dhcp_init(uint8 sock, void_func ip_update_hook, void_func ip_conflict_hook, wiz_NetInfo *def)
{
	if(sock >= TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", sock);
		return RET_NOK;
	}

#ifdef DHCP_AUTO
	dhcp_alarm = TRUE;
#endif
#ifdef DHCP_ASYNC
	dhcp_async = TRUE;
#endif

	memset(&di, 0, sizeof(di));
	memcpy(&storage, def, sizeof(storage));
	memset(&workinfo, 0, sizeof(workinfo));
	memcpy(workinfo.mac, storage.mac, 6);
	workinfo.dhcp = NETINFO_STATIC;
	SetNetInfo(&workinfo);
	di.xid = 0x12345678;
	di.sock = sock;
	if(ip_update_hook) di.ip_update = ip_update_hook;
	if(ip_conflict_hook) di.ip_conflict = ip_conflict_hook;

	// ToDo: Remove setting zero IP & SN (set at start func)

	return RET_OK;
}

/**
 * DHCP manual mode handler.
 * - Blocking Function
 * - Used only at DHCP manual mode (DHCP mode could be chosen at wizconfig.h file)
 * - DHCP_MANUAL mode does not need a loop structure, but if there is no loop structure, \n
 *    you should handle renew & rebind with your own way, or just ignore renew & rebind action
 *
 * @param action The action you want to do. (@ref dhcp_action)
 * @param renew For returning renew time when DHCP be bound (NULL will be ignored)
 * @param rebind For returning rebind time when DHCP be bound (NULL will be ignored)
 * @return RET_OK: Success
 * @return RET_NOK: Error
 */
int8 dhcp_manual(dhcp_action action, uint32 *renew, uint32 *rebind)	// blocking function
{
	dhcp_state curstt = di.state;

	if(dhcp_alarm == TRUE) return RET_NOK;

	while(curstt != DHCP_STATE_INIT && curstt != DHCP_STATE_BOUND) {
		dhcp_run();
		curstt = di.state;
	}

	if(curstt == DHCP_STATE_INIT) {
		di.action = DHCP_ACT_START;
		memset(&workinfo, 0, sizeof(workinfo));
		workinfo.dhcp = NETINFO_DHCP;
		SetNetInfo(&workinfo);
		
		// ToDo: Set zero IP & SN
		
		do {
			dhcp_run();
			curstt = di.state;
		} while((curstt != DHCP_STATE_INIT || di.action != DHCP_ACT_START) 
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
		curstt = di.state;
		while(curstt != DHCP_STATE_INIT && curstt != DHCP_STATE_BOUND) {
			dhcp_run();
			curstt = di.state;
		}
		if(curstt != DHCP_STATE_BOUND) return RET_NOK;
		if(renew) *renew = di.renew_time;
		if(rebind) *rebind = di.rebind_time;
	}

	return RET_OK;
}

/**
 * Return current DHCP state.
 * @return DHCP state enum value (@ref dhcp_state)
 */
dhcp_state dhcp_get_state(void)
{
	return di.state;
}

/**
 * Update DHCP default address storage.
 * You can update DHCP internal address storage through this
 *	- Normally, don't need to use this function except MAC address changed case
 *	- MAC address of the storage is used when DHCP action (on the send packet)
 *	- The others are used when Static mode or DHCP failed
 *
 * @param net The addresses you want to set at default address storage
 *	- All zero address (like 0.0.0.0 or 0:0:0:0:0:0) will be ignored \n 
 *		and be returned with address set formerly in it for reference
 *	- Member variable DHCP is not used (just ignored)
 * @see @ref wiz_NetInfo, @ref dhcp_get_storage
 * @note You should update MAC address when chip MAC address is changed.
 *		\n If not, DHCP send packet will have wrong MAC address.
 */
void dhcp_set_storage(wiz_NetInfo *net)	// Should be updated when MAC is changed
{
	if(net == NULL) {
		DBG("NULL arg");
		return;
	}

	if(net->mac[0]!=0 || net->mac[1]!=0 || net->mac[2]!=0 || net->mac[3]!=0 || 
		net->mac[2]!=0 || net->mac[3]!=0) memcpy(storage.mac, net->mac, 6);
	if(net->ip[0]!=0 || net->ip[1]!=0 || net->ip[2]!=0 || net->ip[3]!=0)
		memcpy(storage.ip, net->ip, 4);
	if(net->sn[0]!=0 || net->sn[1]!=0 || net->sn[2]!=0 || net->sn[3]!=0)
		memcpy(storage.sn, net->sn, 4);
	if(net->gw[0]!=0 || net->gw[1]!=0 || net->gw[2]!=0 || net->gw[3]!=0)
		memcpy(storage.gw, net->gw, 4);
	if(net->dns[0]!=0 || net->dns[1]!=0 || net->dns[2]!=0 || net->dns[3]!=0)
		memcpy(storage.dns, net->dns, 4);
}

/**
 * Return DHCP default address storage.
 * - This may be different from real address device, because this is just default value.
 * - MAC address of the storage is used when DHCP action (on the send packet)
 * - The others are used when Static mode or DHCP failed
 *
 * @param net The struct variable in which storage addresses will be returned
 */
void dhcp_get_storage(wiz_NetInfo *net)
{
	if(net == NULL) {
		DBG("NULL arg");
		return;
	}

	memcpy(net->mac, storage.mac, 6);
	memcpy(net->ip, storage.ip, 4);
	memcpy(net->sn, storage.sn, 4);
	memcpy(net->gw, storage.gw, 4);
	memcpy(net->dns, storage.dns, 4);
}

/**
 * Change DHCP mode to Static.
 * Even though DHCP was enabled, it can be changed to Static mode through this function
 *
 * @param net The addresses you want to set as static addresses
 *	- NULL parameter or NULL member variable will be ignored and internal storage addresses will be used
 *		\n and these address will be returned in this net parameter (if not NULL)
 */
void dhcp_static_mode(wiz_NetInfo *net)
{
	di.action = DHCP_ACT_NONE;
	SET_STATE(DHCP_STATE_INIT);

	if(net != NULL) {
		if(net->ip[0]!=0 || net->ip[1]!=0 || net->ip[2]!=0 || net->ip[3]!=0)
			memcpy(storage.ip, net->ip, 4);
		else memcpy(net->ip, storage.ip, 4);
		if(net->sn[0]!=0 || net->sn[1]!=0 || net->sn[2]!=0 || net->sn[3]!=0)
			memcpy(storage.sn, net->sn, 4);
		else memcpy(net->sn, storage.sn, 4);
		if(net->gw[0]!=0 || net->gw[1]!=0 || net->gw[2]!=0 || net->gw[3]!=0)
			memcpy(storage.gw, net->gw, 4);
		else memcpy(net->gw, storage.gw, 4);
		if(net->dns[0]!=0 || net->dns[1]!=0 || net->dns[2]!=0 || net->dns[3]!=0)
			memcpy(storage.dns, net->dns, 4);
		else memcpy(net->dns, storage.dns, 4);

		net->dhcp = NETINFO_STATIC;
		SetNetInfo(net);
	} else {
		SetNetInfo(&storage);
		memset(&workinfo, 0, sizeof(workinfo));
		workinfo.dhcp = NETINFO_STATIC;
		SetNetInfo(&workinfo);
	}

	if(dhcp_alarm) alarm_del(dhcp_alarm_cb, -1);
	//send_checker_NB();
}

/**
 * DHCP Auto mode (alarm mode) start function.
 * - Used for DHCP start action at Auto mode \n
 *	Auto mode can be selected at @ref wizconfig.h file. \n
 *	(set USE_DHCP to VAL_ENABLE, and uncomment DHCP_AUTO define) \n
 *	and in the main loop, @ref alarm_run should be called continuously.
 * - At Static mode, it can be changed to DHCP mode through this function
 */
void dhcp_auto_start(void)
{
	DBG("DHCP Start");
	SET_STATE(DHCP_STATE_INIT);
	di.action = DHCP_ACT_START;

	memset(&workinfo, 0, sizeof(workinfo));
	workinfo.dhcp = NETINFO_DHCP;
	SetNetInfo(&workinfo);
	
	// ToDo: Set zero IP & SN
	
	if(dhcp_alarm) alarm_set(10, dhcp_alarm_cb, 0);
}

/* @} */

static void dhcp_alarm_cb(int8 arg)	// for DHCP auto mode
{
	if(dhcp_alarm == FALSE) return;
	if(arg == 0) {
		if(di.state == DHCP_STATE_BOUND) {
			alarm_set(wizpf_tick_conv(FALSE, di.renew_time), dhcp_alarm_cb, 1);
			alarm_set(wizpf_tick_conv(FALSE, di.rebind_time), dhcp_alarm_cb, 2);
		}
		if(di.state == DHCP_STATE_FAILED) {
			di.state = DHCP_STATE_INIT;
			di.action = DHCP_ACT_START;
		}
		dhcp_run();
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
	} else if(GetUDPSocketStatus(di.sock) == SOCKSTAT_CLOSED) {
		if(udp_open_fail == TRUE && !IS_TIME_PASSED(dhcp_run_tick, DHCP_RETRY_DELAY)) 
			goto RET_ALARM;
		ClsNetInfo(NI_IP_ADDR);
		ClsNetInfo(NI_SN_MASK);
		ClsNetInfo(NI_GW_ADDR);
		ClsNetInfo(NI_DNS_ADDR);
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
	case DHCP_STATE_INIT:
		if(dhcp_run_cnt==0 && !IS_TIME_PASSED(dhcp_run_tick, DHCP_OPEN_DELAY)) 
			goto RET_ALARM;

		if(dhcp_run_cnt < DHCP_SEND_RETRY_COUNT) {
			dhcp_run_cnt++;
			if(send_discover() == RET_OK) {	// Discover ok
				if(dhcp_async) {
					DBG("DHCP Discovery Send Async");
					sockwatch_set(di.sock, WATCH_SOCK_UDP_SEND);
					return;	// alarm set is not needed
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
			dhcp_fail();
			return; // alarm set is not needed
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
					return;	// alarm set is not needed
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
			dhcp_fail();
			return; // alarm set is not needed
		}
		break;
	case DHCP_STATE_REQUESTING:
		if(!IS_TIME_PASSED(dhcp_run_tick, DHCP_RETRY_DELAY)) {
			int8 ret = recv_handler();
			if(ret == DHCP_MSG_ACK) {	// Recv ACK
				LOG("DHCP Success");
				SET_STATE(DHCP_STATE_IP_CHECK);
				dhcp_run_tick = wizpf_get_systick();
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
	case DHCP_STATE_IP_CHECK:
		//if(send_checker() == RET_OK) {
			SET_STATE(DHCP_STATE_BOUND);
			SetNetInfo(&workinfo);
			if(di.ip_update) di.ip_update();
			LOGA("DHCP ok - New IP (%d.%d.%d.%d)", 
				workinfo.ip[0], workinfo.ip[1], workinfo.ip[2], workinfo.ip[3]);
		//} else {
		//	SET_STATE(DHCP_STATE_INIT);
		//	ERR("IP Addr conflicted - IP(%d.%d.%d.%d)", workinfo.ip[0], workinfo.ip[1], workinfo.ip[2], workinfo.ip[3]);
		//	send_rel_dec(DHCP_MSG_DECLINE);
		//	if(di.ip_conflict) (*di.ip_conflict)();
		//}
		break;
	case DHCP_STATE_BOUND:
		di.action = DHCP_ACT_NONE;
		UDPClose(di.sock);
		if(dhcp_async) sockwatch_close(di.sock);
		return; // alarm set is not needed
	case DHCP_STATE_FAILED:
		return; // alarm set is not needed
	default:
		ERRA("wrong state(%d)", di.state);
		return; // alarm set is not needed
	}

RET_ALARM:
	if(dhcp_alarm) alarm_set(10, dhcp_alarm_cb, 0);
}

static void dhcp_fail(void)
{
	LOG("DHCP Fail - set temp addr");
	di.action = DHCP_ACT_NONE;
	SET_STATE(DHCP_STATE_FAILED);
	memcpy(&workinfo, &storage, sizeof(storage));
	memset(workinfo.mac, 0, 6);
	SetNetInfo(&workinfo);
	network_disp(NULL);
	if(dhcp_alarm) 
		alarm_set(DHCP_START_RETRY_DELAY, dhcp_alarm_cb, 0);
	//send_checker_NB();
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

	//DBGFUNC(print_dump(&dm, sizeof(dm)));	// For debugging received packet
	DBGA("DHCP_SIP:%d.%d.%d.%d",di.srv_ip[0],di.srv_ip[1],di.srv_ip[2],di.srv_ip[3]);
	DBGA("DHCP_RIP:%d.%d.%d.%d",di.srv_ip_real[0],di.srv_ip_real[1],di.srv_ip_real[2],di.srv_ip_real[3]);
	DBGA("recv_ip:%d.%d.%d.%d",recv_ip[0],recv_ip[1],recv_ip[2],recv_ip[3]);
	
	if(dm.op != DHCP_BOOTREPLY || recv_port != DHCP_SERVER_PORT) {
		if(dm.op != DHCP_BOOTREPLY) DBG("DHCP : NO DHCP MSG");
		if(recv_port != DHCP_SERVER_PORT)  DBG("DHCP : WRONG PORT");
		return RET_NOK;
	}

	if(memcmp(dm.chaddr, storage.mac, 6) != 0 || dm.xid != htonl(di.xid)) {
		DBG("No My DHCP Message. This message is ignored.");
		DBGA("SRC_MAC_ADDR(%02X:%02X:%02X:%02X:%02X:%02X)", 
			storage.mac[0], storage.mac[1], storage.mac[2], 
			storage.mac[3], storage.mac[4], storage.mac[5]);
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
	
	memcpy(workinfo.ip, dm.yiaddr, 4);
	DBG("DHCP MSG received..");
	DBGA("yiaddr : %d.%d.%d.%d",workinfo.ip[0],workinfo.ip[1],workinfo.ip[2],workinfo.ip[3]);

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
			memcpy(workinfo.sn,cur,4);
			DBGA("subnetMask : %d.%d.%d.%d",
				workinfo.sn[0],workinfo.sn[1],workinfo.sn[2],workinfo.sn[3]);
			break;
		case routersOnSubnet:
			opt_len = *cur++;
			memcpy(workinfo.gw,cur,4);
			DBGA("routersOnSubnet : %d.%d.%d.%d",
				workinfo.gw[0],workinfo.gw[1],workinfo.gw[2],workinfo.gw[3]);
			break;
		case dns:
			opt_len = *cur++;
			memcpy(workinfo.dns,cur,4);
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
	memcpy(dm.chaddr, storage.mac, 6);
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
	memcpy(&dm.opt[len], storage.mac, 6);
	len += 6;
	
	// host name
	dm.opt[len++] = hostName;
	dm.opt[len++] = strlen(HOST_NAME) + 6; // length of hostname + 3
	strcpy((char*)&dm.opt[len], HOST_NAME);
	len += strlen(HOST_NAME);
	sprintf((char*)&dm.opt[len], "%02x%02x%02x", 
		storage.mac[3], storage.mac[4], storage.mac[5]);
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
		memcpy(dm.ciaddr, workinfo.ip, 4);
	} else {
		dm.flags = htons(DHCP_BROADCAST);
	}

	memcpy(dm.chaddr, storage.mac, 6);

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
	memcpy(&dm.opt[len], storage.mac, 6);
	len += 6;

	if(di.action != DHCP_ACT_RENEW) {
		dm.opt[len++] = dhcpRequestedIPaddr;
		dm.opt[len++] = 0x04;
		memcpy(&dm.opt[len], workinfo.ip, 4);
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
		storage.mac[3], storage.mac[4], storage.mac[5]);
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
	memcpy(dm.chaddr, storage.mac, 6);

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
	memcpy(&dm.opt[len], storage.mac, 6);
	len += 6;

	dm.opt[len++] = dhcpServerIdentifier;
	dm.opt[len++] = 0x04;
	memcpy(&dm.opt[len], di.srv_ip, 4);
	len += 4;

	if(msgtype == DHCP_MSG_DECLINE) {
		dm.opt[len++] = dhcpRequestedIPaddr;
		dm.opt[len++] = 0x04;
		memcpy(&dm.opt[len], workinfo.ip, 4);
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
		len = UDPSendNB(di.sock, "CHECK_IP_CONFLICT", 17, workinfo.ip, 5000);
		if(len < sizeof(struct dhcp_msg)) {
			if(len < 0) ERRA("UDPSend fail - ret(%d)", len);
			else ERRA("UDPSend sent less than size - size(%d), sent(%d)", 17, len);
			return RET_NOK;
		} else sockwatch_set(di.sock, WATCH_SOCK_UDP_SEND);
	} else {
		len = UDPSend(di.sock, "CHECK_IP_CONFLICT", 17, workinfo.ip, 5000);
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




