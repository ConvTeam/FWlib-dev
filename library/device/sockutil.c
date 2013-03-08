/**
 * @file		sockutil.c
 * @brief		Socket Utility Source File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	modified by Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

//#define FILE_LOG_SILENCE
#include "common/common.h"
//#include "device/sockutil.h"


static watch_cbfunc watch_cb[TOTAL_SOCK_NUM] = {0,};
static uint8 watch_sock[TOTAL_SOCK_NUM] = {0,};


/**
 * @addtogroup sockutil_module
 * @{
 */

/**
 * Initialize Network.
 * This function should be called in the main function.
 * If not, you have to handle network initialization manually.
 * 
 * @param dhcp_sock The socket number which will be used at dhcp action
 * @param ip_update The hook function to be called back when IP Addr update.
 * @param ip_conflict The hook function to be called back when IP Addr conflict.
 * @return RET_OK: Success
 * @return RET_NOK: Error
 */
int8 network_init(uint8 dhcp_sock, void_func ip_update, void_func ip_conflict)
{
#define NETINIT_ADDR_SET(name_p) \
do { \
	if(ip_check(DEFAULT_IP_ADDR, netinfo.ip) != RET_OK) { \
		ERR("Default IP Addr set fail"); return RET_NOK; \
	} else DBGA(name_p" IP Addr(%d.%d.%d.%d)", \
		netinfo.ip[0], netinfo.ip[1], netinfo.ip[2], netinfo.ip[3]); \
	if(ip_check(DEFAULT_SN_MASK, netinfo.sn) != RET_OK) { \
		ERR("Default SN Mask set fail"); return RET_NOK; \
	} else DBGA(name_p" SN Mask(%d.%d.%d.%d)", \
		netinfo.sn[0], netinfo.sn[1], netinfo.sn[2], netinfo.sn[3]); \
	if(ip_check(DEFAULT_GW_ADDR, netinfo.gw) != RET_OK) { \
		ERR("Default GW Addr set fail"); return RET_NOK; \
	} else DBGA(name_p" GW Addr(%d.%d.%d.%d)", netinfo.gw[0],  \
		netinfo.gw[1], netinfo.gw[2], netinfo.gw[3]); \
	if(ip_check(DEFAULT_DNS_ADDR, netinfo.dns) != RET_OK) { \
		ERR("Default DNS Addr set fail"); return RET_NOK; \
	} else DBGA(name_p" DNS Addr(%d.%d.%d.%d)", netinfo.dns[0],  \
		netinfo.dns[1], netinfo.dns[2], netinfo.dns[3]); \
} while(0)

	wiz_NetInfo netinfo;

	memset(&netinfo, 0, sizeof(netinfo));

	if(mac_check(DEFAULT_MAC_ADDR, netinfo.mac) != RET_OK) {
		ERR("Default MAC Addr set fail");
		return RET_NOK;
	} else DBGA("Default MAC Addr(%02x:%02x:%02x:%02x:%02x:%02x)", netinfo.mac[0], 
		netinfo.mac[1], netinfo.mac[2], netinfo.mac[3], netinfo.mac[4], netinfo.mac[5]);

#if (USE_DHCP == VAL_ENABLE)
	NETINIT_ADDR_SET("Default");	// Set the addresses which will be used when DHCP failed
	if(dhcp_init(dhcp_sock, ip_update, ip_conflict, &netinfo) != RET_OK)
		return RET_NOK;
#else
	NETINIT_ADDR_SET("Static");
	netinfo.dhcp = NETINFO_STATIC;
	SetNetInfo(&netinfo);
	network_disp(NULL);
#endif

	return RET_OK;
}

/**
 * Display Current Network Information.
 * Current IP Addr, Subnet Mask, Gateway Addr, \n
 * DNS Server Addr, DHCP mode is displayed.
 *
 * @param netinfo The @ref wiz_NetInfo struct pointer to display, \n
 * if input NULL value, it will display current configuration value.
 */
void network_disp(wiz_NetInfo *netinfo)
{
	wiz_NetInfo cur;

	if(netinfo == NULL) {
		GetNetInfo(&cur);
		netinfo = &cur;
		LOG("---------------------------------------");
		LOG("Current Network Configuration          ");
	}

	LOG("---------------------------------------");
	LOGA("MAC : %02X:%02X:%02X:%02X:%02X:%02X", netinfo->mac[0], netinfo->mac[1], 
		netinfo->mac[2], netinfo->mac[3], netinfo->mac[4], netinfo->mac[5]);
	LOGA("IP  : %d.%d.%d.%d", netinfo->ip[0], netinfo->ip[1], netinfo->ip[2], netinfo->ip[3]);
	LOGA("SN  : %d.%d.%d.%d", netinfo->sn[0], netinfo->sn[1], netinfo->sn[2], netinfo->sn[3]);
	LOGA("GW  : %d.%d.%d.%d", netinfo->gw[0], netinfo->gw[1], netinfo->gw[2], netinfo->gw[3]);
	LOGA("DNS : %d.%d.%d.%d", netinfo->dns[0], netinfo->dns[1], netinfo->dns[2], netinfo->dns[3]);
	LOGA("DHCP: %s", netinfo->dhcp==NETINFO_STATIC? "Static": "DHCP");
	LOG("---------------------------------------");
}

/**
 * Assign a callback function to a socket.
 * When @ref sockwatch_run function detected a event, \n
 * this callback function will be called.
 * 
 * @param sock The socket number which is corresponding to 'cb' param
 * @param cb The callback function to be called when \n 
 * 		the socket has any completion or event.
 * @return RET_OK: Success
 * @return RET_NOK: Error
 */
int8 sockwatch_open(uint8 sock, watch_cbfunc cb)
{
	DBGA("WATCH Open - sock(%d), CB(%p)", sock, (void*)cb);
	if(cb == NULL || sock >= TOTAL_SOCK_NUM) {
		ERRA("wrong arg - sock(%d)", sock);
		return RET_NOK;
	}
	if(watch_cb[sock] == NULL) watch_cb[sock] = cb;
	else return RET_NOK;

	return RET_OK;
}

/**
 * Remove callback function from a socket and Stop to watch all event.
 *
 * @param sock The socket number to close
 * @return RET_OK: Success
 * @return RET_NOK: Error
 */
int8 sockwatch_close(uint8 sock)
{
	DBGA("WATCH Close - sock(%d)", sock);
	if(sock >= TOTAL_SOCK_NUM) {
		ERRA("wrong sock(%d)", sock);
		return RET_NOK;
	}

	sockwatch_clr(sock, WATCH_SOCK_ALL_MASK);
	watch_cb[sock] = NULL;

	return RET_OK;
}

/**
 * Set a item of event to watch at @ref sockwatch_run.
 * Once opened a socket by @ref sockwatch_open, \n
 * you have to register events for watching that using this function.
 * It is possible to set plural item of event at the same time.
 *
 * @param sock The socket number to watch
 * @param item The item of event to watch
 * @return RET_OK: Success
 * @return RET_NOK: Error
 */
int8 sockwatch_set(uint8 sock, uint8 item)
{
	DBGA("WATCH Set - sock(%d), item(0x%x)", sock, item);
	if(sock >= TOTAL_SOCK_NUM) {
		ERRA("wrong sock(%d)", sock);
		return RET_NOK;
	}

	BITSET(watch_sock[sock], 0x7F & item);

	return RET_OK;
}

/**
 * Clear a item of event which you don't care anymore.
 * @ref sockwatch_run stop to detect the item on the socket.
 * It is possible to set plural item of event at the same time.
 *
 * @param sock The socket number to clear
 * @param item The item of event to clear
 * @return RET_OK: Success
 * @return RET_NOK: Error
 */
int8 sockwatch_clr(uint8 sock, uint8 item)
{
	DBGA("WATCH Clear - sock(%d), item(0x%x)", sock, item);
	if(sock >= TOTAL_SOCK_NUM) {
		ERRA("wrong sock(%d)", sock);
		return RET_NOK;
	}

	BITCLR(watch_sock[sock], 0x7F & item);

	return RET_OK;
}

/**
 * Check a item of event has been set on the socket.
 *
 * @param sock The socket number to clear
 * @param item The item of event to clear
 * @return RET_OK: There is the item queried on the socket.
 * @return RET_NOK: There is not the item queried on the socket.
 */
int8 sockwatch_chk(uint8 sock, uint8 item)
{
	if((sock < TOTAL_SOCK_NUM) && (watch_sock[sock] & item)) 
		return RET_OK;

	return RET_NOK;
}

/**
 * Sockwatch Module Handler
 * If you use Sockwatch Module, this should run in the main loop
 */
void sockwatch_run(void)
{
#define WCF_HANDLE(item_v, ret_v) \
do { \
	BITCLR(watch_sock[i], item_v); \
	watch_cb[i](i, item_v, ret_v); \
} while(0)

	uint8 i;
	int32 ret;

	for(i=0; i<TOTAL_SOCK_NUM; i++) {
		if(watch_sock[i] == 0) continue;
		if(watch_sock[i] & WATCH_SOCK_RECV) {	// checked every time when 'connected' state
			if(GetSocketRxRecvBufferSize(i) > 0) WCF_HANDLE(WATCH_SOCK_RECV, RET_OK);
		}
		if(watch_sock[i] & WATCH_SOCK_CLS_EVT) {	// checked every time when 'connected' state
			ret = TCPClsRcvCHK(i);
			if(ret != SOCKERR_BUSY) WCF_HANDLE(WATCH_SOCK_CLS_EVT, ret);
		}
		if(watch_sock[i] & WATCH_SOCK_CONN_EVT) {	// checked every time when 'listen' state
			ret = TCPConnChk(i);
			if(ret != SOCKERR_BUSY) WCF_HANDLE(WATCH_SOCK_CONN_EVT, ret);
		}
		if((watch_sock[i] & WATCH_SOCK_MASK_LOW) == 0) continue;	// things which occurs occasionally will be checked all together
		if(watch_sock[i] & WATCH_SOCK_CLS_TRY) {
			ret = TCPCloseCHK(i);
			if(ret != SOCKERR_BUSY) WCF_HANDLE(WATCH_SOCK_CLS_TRY, ret);
		}
		if(watch_sock[i] & WATCH_SOCK_CONN_TRY) {
			ret = TCPConnChk(i);
			if(ret != SOCKERR_BUSY) WCF_HANDLE(WATCH_SOCK_CONN_TRY, ret);
		}
		if(watch_sock[i] & WATCH_SOCK_TCP_SEND) {
			ret = TCPSendCHK(i);
			if(ret != SOCKERR_BUSY) WCF_HANDLE(WATCH_SOCK_TCP_SEND, ret);
		}
		if(watch_sock[i] & WATCH_SOCK_UDP_SEND) {
			ret = UDPSendCHK(i);
			if(ret != SOCKERR_BUSY) WCF_HANDLE(WATCH_SOCK_UDP_SEND, ret);
		}
	}

	// ToDo: not socket part
	
}

/**
 * Check a string is right IP Address, and if right, copy the address to the 'ip' variable as array.
 *
 * @param str The string to investigate if it is right IP Address.
 * @param ip The array pointer in which the address will enter when it is right IP address.
 * @return RET_OK: This is right IP Address.
 * @return RET_NOK: This is not IP Address.
 */
int8 ip_check(int8 *str, uint8 *ip)
{
	uint8_t cnt=0;
	int8 tmp[16], *split;
	int32 digit, sumchk = 0;

	digit = strlen((char*)str);
	if(digit > 15 || digit < 7) {
		return RET_NOK;
	}

	strcpy((char*)tmp, (char*)str);
	split = (int8*)strtok((char*)tmp, ".");
	while(split != NULL && str_check(isdigit, split) == RET_OK) {
		digit = atoi((char*)split);
		if(digit > 255 || digit < 0) return RET_NOK;
		if(ip) ip[cnt] = digit;
		sumchk += digit;
		cnt++;
		split = (int8*)strtok(NULL, ".");
	}

	if(cnt != 4 || sumchk == 0) {		//printf("not 4 digit (%d)\r\n", cnt);
		return RET_NOK;
	}

	return RET_OK;
}

/**
 * Check a string is right TCP Port number, and if right, copy the number to the 'port' variable.
 *
 * @param str The string to investigate if it is right TCP Port.
 * @param port The variable pointer in which the number will enter when it is right TCP Port.
 * @return RET_OK: This is right TCP Port number.
 * @return RET_NOK: This is not TCP Port number.
 */
int8 port_check(int8 *str, uint16 *port)
{
	int8 *ptr;
	uint32 val;

	val = strtol((char*)str, (char**)&ptr, 10);		//printf("ptr(%p, %x), arg(%p), val(%d)\r\n", ptr, *ptr, str, val);

	if(val == 0 || val > 65535 || *ptr != 0) return RET_NOK;
	if(port) *port = val;

	return RET_OK;
}

/**
 * Check a string is right MAC Address, and if right, copy the address to the 'mac' variable as array.
 *
 * @param str The string to investigate if it is right MAC Address.
 * @param mac The array pointer in which the address will enter when it is right MAC address.
 * @return RET_OK: This is right MAC Address.
 * @return RET_NOK: This is not MAC Address.
 */
int8 mac_check(int8 *str, uint8 *mac)
{
	uint8_t cnt=0;
	int8 tmp[18], *split;
	int32 digit;

	if(strlen((char*)str) != 17) {
		return RET_NOK;
	}

	strcpy((char*)tmp, (char*)str);
	split = (int8*)strtok((char*)tmp, ":");
	while(split != NULL && str_check(isxdigit, split) == RET_OK) {
		digit = strtol((char*)split, NULL, 16);
		if(digit > 255 || digit < 0) return RET_NOK;
		if(mac) mac[cnt] = digit;
		cnt++;
		split = (int8*)strtok(NULL, ":");
	}

	if(cnt != 6) {		//printf("not 6 digit (%d)\r\n", cnt);
		return RET_NOK;
	}

	return RET_OK;
}

/**
 * Convert a 32bit Address into a Dotted Decimal Format string.
 *
 * @param addr 32bit address.
 * @return Dotted Decimal Format string.
 */
int8* inet_ntoa(uint32 addr)
{
	static int8 addr_str[16];
	memset(addr_str,0,16);
	sprintf((char*)addr_str,"%d.%d.%d.%d",(int32)(addr>>24 & 0xFF),(int32)(addr>>16 & 0xFF),(int32)(addr>>8 & 0xFF),(int32)(addr & 0xFF));
	return addr_str;
}

/**
 * Convert a 32bit Address into a Dotted Decimal Format string.
 * This is differ from inet_ntoa in fixed length.
 *
 * @param addr 32bit address.
 * @return Dotted Decimal Format string.
 */
int8* inet_ntoa_pad(uint32 addr)
{
	static int8 addr_str[16];
	memset(addr_str,0,16);
	sprintf((char*)addr_str,"%03d.%03d.%03d.%03d",(int32)(addr>>24 & 0xFF),(int32)(addr>>16 & 0xFF),(int32)(addr>>8 & 0xFF),(int32)(addr & 0xFF));
	return addr_str;
}

/**
 * Converts a string containing an (Ipv4) Internet Protocol decimal dotted address into a 32bit address.
 *
 * @param addr Dotted Decimal Format string.
 * @return 32bit address.
 */
uint32 inet_addr(uint8* addr)
{
	int8 i;
	uint32 inetaddr = 0;
	int8 taddr[30];
	int8 * nexttok;
	int32 num;
	strcpy((char*)taddr,(char*)addr);
	
	nexttok = taddr;
	for(i = 0; i < 4 ; i++)
	{
		nexttok = (int8*)strtok((char*)nexttok,".");
		if(nexttok[0] == '0' && nexttok[1] == 'x') num = strtol((char*)nexttok+2, NULL, 16);
		else num = strtol((char*)nexttok, NULL, 10);
		inetaddr = inetaddr << 8;		
		inetaddr |= (num & 0xFF);
		nexttok = NULL;
	}
	return inetaddr;	
}	

/**
 * Swap the byte order of 16bit(short) wide variable.
 *
 * @param i 16bit value to swap
 * @return Swapped value
 */
uint16 swaps(uint16 i)
{
	uint16 ret=0;
	ret = (i & 0xFF) << 8;
	ret |= ((i >> 8)& 0xFF);
	return ret;	
}

/**
 * Swap the byte order of 32bit(long) wide variable.
 *
 * @param l 32bit value to convert
 * @return Swapped value
 */
uint32 swapl(uint32 l)
{
	uint32 ret=0;
	ret = (l & 0xFF) << 24;
	ret |= ((l >> 8) & 0xFF) << 16;
	ret |= ((l >> 16) & 0xFF) << 8;
	ret |= ((l >> 24) & 0xFF);
	return ret;
}

/**
 * htons function converts a unsigned short from host to TCP/IP network byte order (which is big-endian).
 *
 * @param hostshort The value to convert.
 * @return The value in TCP/IP network byte order.
 */ 
uint16 htons(uint16 hostshort)
{
#ifdef SYSTEM_LITTLE_ENDIAN
	return swaps(hostshort);
#else
	return hostshort;
#endif		
}


/**
 * htonl function converts a unsigned long from host to TCP/IP network byte order (which is big-endian).
 *
 * @param hostlong The value to convert.
 * @return The value in TCP/IP network byte order.
 */ 
uint32 htonl(uint32 hostlong)
{
#ifdef SYSTEM_LITTLE_ENDIAN
	return swapl(hostlong);
#else
	return hostlong;
#endif	
}


/**
 * ntohs function converts a unsigned short from TCP/IP network byte order
 * to host byte order (which is little-endian on Intel processors).
 *
 * @param netshort The value to convert.
 * @return A 16-bit number in host byte order
 */ 
uint32 ntohs(uint16 netshort)
{
#ifdef SYSTEM_LITTLE_ENDIAN
	return htons(netshort);
#else
	return netshort;
#endif		
}

/**
 * converts a unsigned long from TCP/IP network byte order to host byte order 
 * (which is little-endian on Intel processors).
 *
 * @param netlong The value to convert.
 * @return A 16-bit number in host byte order
 */ 
uint32 ntohl(uint32 netlong)
{
#ifdef SYSTEM_LITTLE_ENDIAN
	return swapl(netlong);
#else
	return netlong;
#endif		
}
/**
 * @}
 */



