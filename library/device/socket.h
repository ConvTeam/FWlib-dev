/**
 * @file		socket.h
 * @brief		Socket Driver Header File - Common
 * @version	1.1
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 *			2013/07/09 - 1.1 Network control part was devided
 * @author	modified by Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef	_SOCKET_H
#define	_SOCKET_H

//#include "common/common.h"

//******  Doxygen Description is moved to doxygen document folder *****
#define SOCKSTAT_CLOSED			-1
#define SOCKSTAT_INIT			0
#define SOCKSTAT_LISTEN			1
#define SOCKSTAT_SYNSENT		2
#define SOCKSTAT_SYNRECV		3
#define SOCKSTAT_ESTABLISHED	4
#define SOCKSTAT_FIN_WAIT		5
#define SOCKSTAT_CLOSING		6
#define SOCKSTAT_TIME_WAIT		7
#define SOCKSTAT_CLOSE_WAIT		8
#define SOCKSTAT_LAST_ACK		9
#define SOCKSTAT_UDP			10

#define SOCKERR_BUSY			-1
#define SOCKERR_NOT_TCP			-2
#define SOCKERR_NOT_UDP			-3
#define SOCKERR_WRONG_ARG		-4
#define SOCKERR_WRONG_STATUS	-5
#define SOCKERR_CLOSED			-6
#define SOCKERR_CLOSE_WAIT		-7
#define SOCKERR_FIN_WAIT		-8
#define SOCKERR_NOT_ESTABLISHED	-9
#define SOCKERR_WINDOW_FULL		-10
#define SOCKERR_TIME_OUT		-11
#define SOCKERR_NULL_SRC_IP		-12
#define SOCKERR_BUF_NOT_ENOUGH	-13
#define SOCKERR_NOT_SPECIFIED	-14


int8  TCPServerOpen(uint8 s, uint16 port);
int8  TCPClientOpen(uint8 s, uint16 sport, uint8 *dip, uint16 dport);
int8  TCPCltOpenNB(uint8 s, uint16 sport, uint8 *dip, uint16 dport);
int8  TCPConnChk(uint8 s);
int8  TCPDisconnect(uint8 s);
int8  TCPClose(uint8 s);
int8  TCPCloseNB(uint8 s);
int8  TCPCloseCHK(uint8 s);
int8  TCPClsRcvCHK(uint8 s);
int32 TCPSend(uint8 s, const int8 *buf, uint16 len);
int8  TCPSendNB(uint8 s, const int8 *buf, uint16 len);
int32 TCPReSend(uint8 s);
int8  TCPReSendNB(uint8 s);
int32 TCPSendCHK(uint8 s);
int32 TCPRecv(uint8 s, int8 *buf, uint16 len);
int8  UDPOpen(uint8 s, uint16 port);
int8  UDPClose(uint8 s);
int32 UDPSend(uint8 s, const int8 *buf, uint16 len, uint8 *addr, uint16 port);
int32 UDPSendNB(uint8 s, const int8 *buf, uint16 len, uint8 *addr, uint16 port);
int8  UDPSendCHK(uint8 s);
int32 UDPRecv(uint8 s, int8 *buf, uint16 len, uint8 *addr, uint16 *port);


#endif //_SOCKET_H





