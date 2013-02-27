/**
 * @file		sockutil.h
 * @brief		The Utility of TCP/IP Chip Device Driver Header File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *		2013/02/22 - 1.0 Release
 * @author	modified by Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef _SOCKUTIL_H
#define _SOCKUTIL_H

//#include "common/common.h"

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
int8* inet_ntoa(uint32 addr);					/* Convert 32bit Address into Dotted Decimal Format */
int8* inet_ntoa_pad(uint32 addr);
uint32 inet_addr(uint8* addr);		/* Converts a string containing an (Ipv4) Internet Protocol decimal dotted address into a 32bit address */
uint16 htons( uint16 hostshort);	/* htons function converts a unsigned short from host to TCP/IP network byte order (which is big-endian).*/
uint32 htonl(uint32 hostlong);		/* htonl function converts a unsigned long from host to TCP/IP network byte order (which is big-endian). */
uint32 ntohs(uint16 netshort);		/* ntohs function converts a unsigned short from TCP/IP network byte order to host byte order (which is little-endian on Intel processors). */
uint32 ntohl(uint32 netlong);		/* ntohl function converts a uint32 from TCP/IP network order to host byte order (which is little-endian on Intel processors). */
uint16 checksum(uint8 * src, uint32 len);		/* Calculate checksum of a stream */

#endif	//_SOCKUTIL_H



