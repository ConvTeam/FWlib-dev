
#ifndef	_SOCKET_H
#define	_SOCKET_H

//#include "common/common.h"

#define STATUS_CLOSED			-1
#define STATUS_INIT				0
#define STATUS_LISTEN			1
#define STATUS_SYNSENT			2
#define STATUS_SYNRECV			3
#define STATUS_ESTABLISHED		4
#define STATUS_FIN_WAIT			5
#define STATUS_CLOSING			6
#define STATUS_TIME_WAIT		7
#define STATUS_CLOSE_WAIT		8
#define STATUS_LAST_ACK			9
#define STATUS_UDP				10

#define ERROR_NOT_TCP_SOCKET	-1
#define ERROR_NOT_UDP_SOCKET	-2
#define ERROR_CLOSED			-3
#define ERROR_NOT_ESTABLISHED	-4
#define ERROR_FIN_WAIT			-5
#define ERROR_CLOSE_WAIT		-6
#define ERROR_WINDOW_FULL		-7
#define ERROR_TIME_OUT			-8

#define MAX_BUF_SIZE			1460		// ??
#define KEEP_ALIVE_TIME			30	// 30sec // ??

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


void device_init(uint8 *tx_size, uint8 *rx_size);
void device_SW_reset();
void device_mem_init(uint8 * tx_size, uint8 * rx_size);
void SetNetInfo(wiz_NetInfo *netinfo);
void GetNetInfo(wiz_NetInfo *netinfo);

void SetSocketOption(uint8 option_type, uint16 option_value);
int8 GetTCPSocketStatus(SOCKET s);
int8 GetUDPSocketStatus(SOCKET s);

int8 TCPServerOpen(SOCKET s, uint16 port);
int8 TCPClientOpen(SOCKET s, uint16 port, uint8 * destip, uint16 destport);
int16 TCPSend(SOCKET s, const uint8 * src, uint16 len);
int16 TCPReSend(SOCKET s);
int16 TCPRecv(SOCKET s, uint8 * buf, uint16 len);
int8 TCPClose(SOCKET s);

int8 UDPOpen(SOCKET s, uint16 port);
int16 UDPSend(SOCKET s, const uint8 * buf, uint16 len, uint8 * addr, uint16 port);
int16 UDPRecv(SOCKET s, uint8 * buf, uint16 len, uint8 * addr, uint16 *port);
int8 UDPClose(SOCKET s);

uint8 GetInterrupt(SOCKET s);
void PutInterrupt(SOCKET s, uint8 vector);
uint16 GetSocketTxFreeBufferSize(SOCKET s);
uint16 GetSocketRxRecvBufferSize(SOCKET s);

#endif //_SOCKET_H





