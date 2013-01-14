
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

#define DHCP_ACT_NONE			0	// DHCP action
#define DHCP_ACT_START			1
#define DHCP_ACT_RENEW			2
#define DHCP_ACT_REBIND			3

#define DHCP_INTERVAL_INIT_RETRY	60000	// tick
#define DHCP_INTERVAL_SEND_RETRY	10000	// tick
#define DHCP_INTERVAL_OPEN_RETRY	1000	// tick
#define DHCP_RECV_WAIT_TIME			5000	// tick
#define DHCP_SEND_RETRY_COUNT		3

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
	uint8 assigned_ip[4];	// Old Souce IP Address - previous source IP which will be used for internal ip update check

	SOCKET sock;
	uint8 state;
	uint8 action;
	uint32 lease_time;
	uint32 renew_time;
	uint32 rebind_time;
	uint32 xid;
	pFunc ip_update;
	pFunc ip_conflict;
};

static int8 send_discover(void);
static int8 send_request(void);
static int8 send_rel_dec(char msgtype);
static int8 send_checker(void);
static int8 recv_handler(void);
static void default_ip_update(void);
static void default_ip_conflict(void);


static wiz_NetInfo ni;
static struct dhcp_info di;
static struct dhcp_msg dm;

int8 dhcp_init(SOCKET sock, pFunc ip_update, pFunc ip_conflict, uint8 *my_mac)
{
	if(sock >= TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", sock);
		return RET_NOK;
	}

	memset(&di, 0, sizeof(di));
	memset(&ni, 0, sizeof(ni));
	memcpy(ni.Mac, my_mac, 6);
	di.xid = 0x12345678;
	di.sock = sock;

	if(!ip_update)
		di.ip_update = default_ip_update;
	else di.ip_update = ip_update;

	if(!ip_conflict)
		di.ip_conflict = default_ip_conflict;
	else di.ip_conflict = ip_conflict;

	IINCHIP_WRITE(WIZC_SIPR0, 0);	//SetNetInfo(&ni);	왜 0000 은 설정하면 안됨 ??
	IINCHIP_WRITE(WIZC_SIPR1, 0);
	IINCHIP_WRITE(WIZC_SIPR2, 0);
	IINCHIP_WRITE(WIZC_SIPR3, 0);

	IINCHIP_WRITE(WIZC_SHAR0, ni.Mac[0]);
	IINCHIP_WRITE(WIZC_SHAR1, ni.Mac[1]);
	IINCHIP_WRITE(WIZC_SHAR2, ni.Mac[2]);
	IINCHIP_WRITE(WIZC_SHAR3, ni.Mac[3]);
	IINCHIP_WRITE(WIZC_SHAR4, ni.Mac[4]);
	IINCHIP_WRITE(WIZC_SHAR5, ni.Mac[5]);

#ifdef __DEF_IINCHIP_INT__
	SetSocketOption(2, 0xFF);	// ??
#endif 

	SET_STATE(DHCP_STATE_INIT);
	di.action = DHCP_ACT_START;

	return RET_OK;
}

void dhcp_run(void)
{
	static uint8 cnt = 0;
	static uint32 tick = 0, bound_tick = 0;

	switch(di.state) {
	case DHCP_STATE_INIT:
		if(di.action == DHCP_ACT_NONE) {
			if(wizpf_tick_elapse(tick) > DHCP_INTERVAL_INIT_RETRY)
				di.action = DHCP_ACT_START;
		} else if(di.action == DHCP_ACT_START) {
			if(GetUDPSocketStatus(di.sock) == (int8)STATUS_CLOSED) {
				if(UDPOpen(di.sock, DHCP_CLIENT_PORT) != SUCCESS) {
					ERR("UDPOpen fail");
					return;
				}
			}

			if(IS_IP_SET(di.assigned_ip)) {			// INIT-REBOOT - Request previous IP
				DBG("INIT-REBOOT");
				SET_STATE(DHCP_STATE_REQUESTING);
				tick = wizpf_get_systick();
			} else if((cnt == 0 || IS_TIME_PASSED(tick, DHCP_INTERVAL_SEND_RETRY)) 
									&& send_discover() == RET_OK) {	// Discover ok
				DBG("DHCP Discovery Sent");
				SET_STATE(DHCP_STATE_SEARCHING);
				tick = wizpf_get_systick();
				cnt = 0;
			} else if(++cnt >= DHCP_SEND_RETRY_COUNT) {
				ERRA("DHCP Discovery SEND fail - (%d)times", cnt);
				di.action = DHCP_ACT_NONE;
				tick = wizpf_get_systick();
				cnt = 0;
				UDPClose(di.sock);
			}
		}
		break;
	case DHCP_STATE_SEARCHING:
		if(!IS_TIME_PASSED(tick, DHCP_RECV_WAIT_TIME)) {
			if(recv_handler() == DHCP_MSG_OFFER) {
				SET_STATE(DHCP_STATE_SELECTING);
				tick = wizpf_get_systick();
			}
		} else {
			ERRA("DHCP Offer RECV fail - for (%d)msec", DHCP_RECV_WAIT_TIME);
			SET_STATE(DHCP_STATE_INIT);
			tick = wizpf_get_systick();
		}
		break;
	case DHCP_STATE_SELECTING:
		if(GetUDPSocketStatus(di.sock) == STATUS_CLOSED) {
			if(UDPOpen(di.sock, DHCP_CLIENT_PORT) != SUCCESS) {
				ERR("UDPOpen fail");
				return;
			}
		}

		if((cnt == 0 || IS_TIME_PASSED(tick, DHCP_INTERVAL_SEND_RETRY))
								&& send_request() == RET_OK) {
			DBG("DHCP Request Sent");
			SET_STATE(DHCP_STATE_REQUESTING);
			tick = wizpf_get_systick();
			cnt = 0;
		} else if(++cnt >= DHCP_SEND_RETRY_COUNT) {
			ERRA("DHCP Request SEND fail - (%d)times", cnt);
			SET_STATE(DHCP_STATE_INIT);
			di.action = DHCP_ACT_NONE;
			tick = wizpf_get_systick();
			cnt = 0;
		}
		break;
	case DHCP_STATE_REQUESTING:
		if(!IS_TIME_PASSED(tick, DHCP_RECV_WAIT_TIME)) {
			int8 ret = recv_handler();
			if(ret == DHCP_MSG_ACK) {	// Recv ACK
				LOG("DHCP Success");
				SET_STATE(DHCP_STATE_IP_CHECK);
				tick = wizpf_get_systick();		// ??
			} else if(ret == DHCP_MSG_NAK) {	// Recv NAK
				if(di.action == DHCP_ACT_START) {
					SET_STATE(DHCP_STATE_INIT);
					tick = wizpf_get_systick();
				} else {
					SET_STATE(DHCP_STATE_BOUND);
				}
			}
		} else {
			ERRA("DHCP ACK RECV fail - for (%d)msec", DHCP_RECV_WAIT_TIME);
			if(di.action == DHCP_ACT_START) {
				SET_STATE(DHCP_STATE_INIT);
				tick = wizpf_get_systick();
			} else {
				SET_STATE(DHCP_STATE_BOUND);
			}
		}
		break;
	case DHCP_STATE_IP_CHECK:
		if(send_checker() == RET_OK) {	// IP 체크 이상없음
			SET_STATE(DHCP_STATE_BOUND);
			if(di.ip_update) di.ip_update();
			bound_tick = wizpf_get_systick();
			UDPClose(di.sock);
		} else {
			SET_STATE(DHCP_STATE_INIT);
			send_rel_dec(DHCP_MSG_DECLINE);
			if(di.ip_conflict) (*di.ip_conflict)();
		}
		break;
	case DHCP_STATE_BOUND:
		if(IS_TIME_PASSED(bound_tick, di.renew_time)) {
			SET_STATE(DHCP_STATE_SELECTING);
			di.action = DHCP_ACT_RENEW;
			di.xid++;
		} else if(IS_TIME_PASSED(bound_tick, di.rebind_time)) {
			SET_STATE(DHCP_STATE_SELECTING);
			di.action = DHCP_ACT_REBIND;
			di.xid++;
		}
		break;
	default:
		ERRA("wrong state(%d)", di.state);
	}

}

int8 dhcp_get_state(void)
{
	return di.state;
}

static int8 send_discover(void)
{
	uint8 srv_ip[4];
	uint16 len = 0;
	//struct dhcp_msg *dm;

	//dm = (struct dhcp_msg*)calloc(1, sizeof(struct dhcp_msg));
	//if(dm == NULL) {
	//	ERR("calloc fail");
	//	return RET_NOK;
	//}
	memset(&dm, 0, sizeof(struct dhcp_msg));

	*((uint32*)di.srv_ip)=0;
	*((uint32*)di.srv_ip_real)=0;

	dm.op = DHCP_BOOTREQUEST;
	dm.htype = DHCP_HTYPE10MB;
	dm.hlen = DHCP_HLENETHERNET;
	dm.hops = DHCP_HOPS;
	dm.xid = htonl(di.xid);
	dm.secs = htons(DHCP_SECS);
	memcpy(dm.chaddr, ni.Mac, 6);
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
	memcpy(&dm.opt[len], ni.Mac, 6);
	len += 6;
	
	// host name
	dm.opt[len++] = hostName;
	dm.opt[len++] = strlen(HOST_NAME)+3; // length of hostname + 3
	strcpy((char *)&(dm.opt[len]),HOST_NAME);
	len += strlen(HOST_NAME);
	memcpy(&dm.opt[len], &ni.Mac[3], 3);
	len += 3;

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

	len = UDPSend(di.sock, (uint8*)&dm, sizeof(struct dhcp_msg), srv_ip, DHCP_SERVER_PORT);
	if(len <= 0) {
		ERR("UDPSend fail");
		return RET_NOK;
	}

	//MEM_FREE(dm);
	return RET_OK;
}

static int8 send_request(void)
{
	uint8 srv_ip[4];
	uint16 len = 0;

	memset(&dm, 0, sizeof(struct dhcp_msg));

	dm.op = DHCP_BOOTREQUEST;
	dm.htype = DHCP_HTYPE10MB;
	dm.hlen = DHCP_HLENETHERNET;
	dm.hops = DHCP_HOPS;
	dm.xid = htonl(di.xid);
	dm.secs = htons(DHCP_SECS);

	if(di.action != DHCP_ACT_RENEW) {
		dm.flags = htons(DHCP_BROADCAST);
	} else {
		dm.flags = 0;		// For Unicast
		memcpy(dm.ciaddr, ni.IP, 4);
	}		

	memcpy(dm.chaddr, ni.Mac, 6);

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
	memcpy(&dm.opt[len], ni.Mac, 6);
	len += 6;

	if(di.action != DHCP_ACT_RENEW) {
		dm.opt[len++] = dhcpRequestedIPaddr;
		dm.opt[len++] = 0x04;
		memcpy(&dm.opt[len], ni.IP, 4);
		len += 4;
		dm.opt[len++] = dhcpServerIdentifier;
		dm.opt[len++] = 0x04;
		memcpy(&dm.opt[len], di.srv_ip, 4);
		len += 4;
	}
	
	// host name
	dm.opt[len++] = hostName;
	dm.opt[len++] = strlen(HOST_NAME)+3; // length of hostname + 3
	strcpy((char *)&(dm.opt[len]),HOST_NAME);
	len += strlen(HOST_NAME);
	memcpy(&dm.opt[len], &ni.Mac[3], 3);
	len += 3;

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
	if(di.action != DHCP_ACT_RENEW) {
		srv_ip[0] = srv_ip[1] = srv_ip[2] = srv_ip[3] = 255;
	} else {
		memcpy(srv_ip, di.srv_ip, 4);
	}

	len == UDPSend(di.sock, (uint8*)&dm, sizeof(struct dhcp_msg), srv_ip, DHCP_SERVER_PORT);
	if(len <= 0) {
		ERR("UDPSend fail");
		return RET_NOK;
	}

	return RET_OK;
}

static int8 send_rel_dec(char msgtype)
{
	uint16 len =0;
	uint8 srv_ip[4];

	memset(&dm, 0, sizeof(struct dhcp_msg));

	dm.op = DHCP_BOOTREQUEST;
	dm.htype = DHCP_HTYPE10MB;
	dm.hlen = DHCP_HLENETHERNET;
	dm.hops = DHCP_HOPS;
	dm.xid = htonl(di.xid);
	dm.secs = htons(DHCP_SECS);
	dm.flags = DHCP_UNICAST;	//DHCP_BROADCAST;
	memcpy(dm.chaddr, ni.Mac, 6);

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
	memcpy(&dm.opt[len], ni.Mac, 6);
	len += 6;

	dm.opt[len++] = dhcpServerIdentifier;
	dm.opt[len++] = 0x04;
	memcpy(&dm.opt[len], di.srv_ip, 4);
	len += 4;

	if(msgtype == DHCP_MSG_DECLINE) {
		dm.opt[len++] = dhcpRequestedIPaddr;
		dm.opt[len++] = 0x04;
		memcpy(&dm.opt[len], ni.IP, 4);
		len += 4;
		dm.opt[len++] = endOption;
		srv_ip[0] = srv_ip[1] = srv_ip[2] = srv_ip[3] = 255;
	} else {
		dm.opt[len++] = endOption;
		memcpy(srv_ip, di.srv_ip, 4);
	}

	len == UDPSend(di.sock, (uint8*)&dm, sizeof(struct dhcp_msg), srv_ip, DHCP_SERVER_PORT);
	if(len <= 0) {
		ERR("UDPSend fail");
		return RET_NOK;
	}

	return RET_OK;
}

static int8 send_checker(void)
{
	int16 len;

	// sendto is complete. that means there is a node which has a same IP.
	len = UDPSend(di.sock, "CHECK_IP_CONFLICT", 17, ni.IP, 5000);
	if(len > 0) {
		ERR("IP Conflict");
		return RET_NOK;
	}
	
	return RET_OK;
}

static int8 recv_handler(void)
{
	uint8 *cur, *end;
	uint8 recv_ip[6], opt_len, msg_type;
	int16 recv_len;
	uint16 recv_port;

	recv_len = GetSocketRxRecvBufferSize(di.sock);
	if(recv_len == 0) return RET_NOK;
	else memset(&dm, 0, sizeof(struct dhcp_msg));

	recv_len = UDPRecv(di.sock, (uint8*)&dm, sizeof(struct dhcp_msg), recv_ip, &recv_port);
	if(recv_len < 0) {
		ERRA("UDPRecv fail - ret(%d)", recv_len);
		return RET_NOK;
	}

	DBGA("DHCP_SIP:%d.%d.%d.%d",di.srv_ip[0],di.srv_ip[1],di.srv_ip[2],di.srv_ip[3]);
	DBGA("DHCP_RIP:%d.%d.%d.%d",di.srv_ip_real[0],di.srv_ip_real[1],di.srv_ip_real[2],di.srv_ip_real[3]);
	DBGA("recv_ip:%d.%d.%d.%d",recv_ip[0],recv_ip[1],recv_ip[2],recv_ip[3]);
	
	if(dm.op != DHCP_BOOTREPLY || recv_port != DHCP_SERVER_PORT) {
		if(dm.op != DHCP_BOOTREPLY) printf("DHCP : NO DHCP MSG");
		if(recv_port != DHCP_SERVER_PORT)  printf("DHCP : WRONG PORT");
		return RET_NOK;
	}

	if(memcmp(dm.chaddr,ni.Mac,6) != 0 || dm.xid != htonl(di.xid)) {
		DBG("No My DHCP Message. This message is ignored.");
		DBGFA("\tSRC_MAC_ADDR(%02X.%02X.%02X.",ni.Mac[0],ni.Mac[1],ni.Mac[2]);
		DBGFA("%02X.%02X.%02X)",ni.Mac[3],ni.Mac[4],ni.Mac[5]);
		DBGFA(", dm.chaddr(%02X.%02X.%02X.",dm.chaddr[0],dm.chaddr[1],dm.chaddr[2]);
		DBGFA("%02X.%02X.%02X)",dm.chaddr[3],dm.chaddr[4],dm.chaddr[5]);
		DBGFA("\tdm.xid(%08lX), DHCP_XID(%08lX)",dm.xid,htonl(di.xid));
		DBGA("\tpRIMPMSG->yiaddr:%d.%d.%d.%d",dm.yiaddr[0],dm.yiaddr[1],dm.yiaddr[2],dm.yiaddr[3]);
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
	
	memcpy(ni.IP, dm.yiaddr, 4);
	DBG("DHCP MSG received..");
	DBGA("yiaddr : %d.%d.%d.%d",ni.IP[0],ni.IP[1],ni.IP[2],ni.IP[3]);

	// 길이로 에러 검출
	msg_type = 0;
	cur = (uint8 *)(&dm.op);
	cur = cur + 240;
	end = cur + (recv_len - 240);	//printf("cur : 0x%08X  end : 0x%08X  recv_len : %d\r\n", cur, end, recv_len);
	while ( cur < end ) 
	{
		switch ( *cur++ ) 
		{
		case padOption :
			break;
		case endOption :
			return msg_type;
		case dhcpMessageType :
			opt_len = *cur++;
			msg_type = *cur;
			DBGA("dhcpMessageType : %x", msg_type);
			break;
		case subnetMask :
			opt_len =* cur++;
			memcpy(ni.Subnet,cur,4);
			DBGA("subnetMask : %d.%d.%d.%d",
				ni.Subnet[0],ni.Subnet[1],ni.Subnet[2],ni.Subnet[3]);
			break;
		case routersOnSubnet :
			opt_len = *cur++;
			memcpy(ni.Gateway,cur,4);
			DBGA("routersOnSubnet : %d.%d.%d.%d",
				ni.Gateway[0],ni.Gateway[1],ni.Gateway[2],ni.Gateway[3]);
			break;
		case dns :
			opt_len = *cur++;
			memcpy(ni.DNSServerIP,cur,4);
			break;
		case dhcpIPaddrLeaseTime :
			opt_len = *cur++;
			di.lease_time = ntohl(*((uint32*)cur));
			di.renew_time = di.lease_time / 2;			// 0.5
			di.rebind_time = di.lease_time / 8 * 7;		// 0.875
			DBGA("lease(%d), renew(%d), rebind(%d)", di.lease_time, di.renew_time, di.rebind_time);
			break;
		case dhcpServerIdentifier :
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
		default :
			opt_len = *cur++;
			DBGA("opt_len : %d", opt_len);
			break;
		} // switch
		cur+=opt_len;
	} // while

	return RET_NOK;
}

static void default_ip_update(void)
{
	SetNetInfo(&ni);

#ifdef __DEF_IINCHIP_INT__
	SetSocketOption(2, 0xEF);	// ??
#endif

	LOGA("DHCP ok - New IP (%d.%d.%d.%d)", ni.IP[0], ni.IP[1], ni.IP[2], ni.IP[3]);
}

static void default_ip_conflict(void)
{
	ERR("The IP Address from DHCP server is CONFLICT!!!");
	ERR("Retry to get a IP address from DHCP server");
}




