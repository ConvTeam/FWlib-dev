/**
 * @file		sockutil.h
 * @brief		Socket Utility Header File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	modified by Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef _SOCKUTIL_H
#define _SOCKUTIL_H

//#include "common/common.h"

/**
 * @addtogroup sockutil_module
 * @{
 * @def WATCH_SOCK_UDP_SEND
 * Indicate that 'UDP SEND' completion of this socket has to be watched.
 * @def WATCH_SOCK_TCP_SEND
 * Indicate that 'TCP SEND' completion of this socket has to be watched.
 * @def WATCH_SOCK_CONN_TRY
 * Indicate that 'CONNECT' completion of this socket has to be watched.
 * @def WATCH_SOCK_CONN_EVT
 * Indicate that 'CONNECT' event of this socket has to be watched.
 * @def WATCH_SOCK_CLS_TRY
 * Indicate that 'CLOSE' completion of this socket has to be watched.
 * @def WATCH_SOCK_CLS_EVT
 * Indicate that 'CLOSE' event of this socket has to be watched.
 * @def WATCH_SOCK_RECV
 * Indicate that 'RECEIVE' event of this socket has to be watched.
 * @def WATCH_SOCK_MASK_LOW
 * Mask all Completions of the socket.
 * @def WATCH_SOCK_MASK_HIGH
 * Mask all Events of the socket.
 * @def WATCH_SOCK_ALL_MASK
 * Mask all Completions and Events.
 * @typedef watch_cbfunc
 * Watch call back function form.
 * @}
 */
#define WATCH_SOCK_UDP_SEND		0x01
#define WATCH_SOCK_TCP_SEND 	0x02
#define WATCH_SOCK_CONN_TRY		0x04
#define WATCH_SOCK_CLS_TRY		0x08
#define WATCH_SOCK_CONN_EVT		0x10
#define WATCH_SOCK_CLS_EVT		0x20
#define WATCH_SOCK_RECV			0x40
#define WATCH_SOCK_MASK_LOW		0x0F
#define WATCH_SOCK_MASK_HIGH	0x70
#define WATCH_SOCK_ALL_MASK		0x7F

typedef void (*watch_cbfunc)(uint8 id, uint8 item, int32 ret);

/**
 * @ingroup netdev_con_module
 * Call @ref device_init after array param check
 */
#define DEVICE_INIT_WITH_MEMCHK(tx_size_v, rx_size_v) \
{ \
	uint8 _i, *_tx, *_rx, _tx_cnt = 0, _rx_cnt = 0; \
	if(sizeof(tx_size_v)/sizeof(uint8) != TOTAL_SOCK_NUM || \
		sizeof(rx_size_v)/sizeof(uint8) != TOTAL_SOCK_NUM) { \
		printf("Device Memory Configure fail 1"); \
		while(1); \
	} \
	_tx = (uint8*)tx_size_v; \
	_rx = (uint8*)rx_size_v; \
	for(_i=0; _i<TOTAL_SOCK_NUM; _i++) { \
		_tx_cnt += _tx[_i]; \
		_rx_cnt += _rx[_i]; \
	} \
	if(_tx_cnt+_rx_cnt != TOTAL_SOCK_MEM) { \
		printf("Device Memory Configure fail 2"); \
		while(1); \
	} \
	device_init(tx_size_v, rx_size_v); \
}

int8 sockwatch_open(uint8 sock, watch_cbfunc cb);
int8 sockwatch_close(uint8 sock);
int8 sockwatch_set(uint8 sock, uint8 item);
int8 sockwatch_clr(uint8 sock, uint8 item);
int8 sockwatch_chk(uint8 sock, uint8 item);
void sockwatch_run(void);
int8 network_init(uint8 dhcp_sock, void_func ip_update, void_func ip_conflict);
void network_disp(wiz_NetInfo *netinfo);
int8 ip_check(int8 *str, uint8 *ip);
int8 port_check(int8 *str, uint16 *port);
int8 mac_check(int8 *str, uint8 *mac);
int8* inet_ntoa(uint32 addr);
int8* inet_ntoa_pad(uint32 addr);
uint32 inet_addr(uint8* addr);
uint16 htons( uint16 hostshort);
uint32 htonl(uint32 hostlong);	
uint32 ntohs(uint16 netshort);
uint32 ntohl(uint32 netlong);

#endif	//_SOCKUTIL_H



