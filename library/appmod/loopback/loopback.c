
//#define FILE_LOG_SILENCE
#include "appmod/loopback/loopback.h"

#define TX_RX_MAX_BUF_SIZE	2048

//int8 ch_status[TOTAL_SOCK_NUM];
int8 data_buf[TX_RX_MAX_BUF_SIZE];

void loopback_tcps(uint8 ch, uint16 port)
{
	int32 ret;
	int32 SendLen, ReSendLen;

	ret = TCPRecv(ch, data_buf, TX_RX_MAX_BUF_SIZE);

	if(ret > 0){				// Received
		SendLen = TCPSend(ch, data_buf, ret);

		if(SendLen < ret){
			while(SendLen != ret){
				ReSendLen = TCPReSend(ch);

				if(ReSendLen > 0){
					SendLen += ReSendLen;

				} else if(ReSendLen == SOCKERR_WINDOW_FULL){
					LOG("Window Full");
					TCPClose(ch);
					DBG("TCP Socket Close");
					while(1);

				} else{
					break;
				}
			}
		}

	} else if(ret == SOCKERR_NOT_TCP){	// Not TCP Socket, It's UDP Socket
		DBG("UDP Socket Close");
		UDPClose(ch);
	} else if(ret == SOCKERR_CLOSED){		// Socket Closed
		LOGA("TCP Loop-Back Started - ch(%d)",(uint16)ch);
		TCPServerOpen(ch, port);
	}

	if(GetTCPSocketStatus(ch) == SOCKSTAT_CLOSE_WAIT){	// Close waiting
		TCPClose(ch);
	}
}

void loopback_udp(uint8 ch, uint16 port)
{
	int32 ret;
	uint32 destip = 0;
	uint16 destport;

	ret = UDPRecv(ch, data_buf, TX_RX_MAX_BUF_SIZE, (uint8*)&destip, &destport);

	if(ret > 0){				// Received
		ret = UDPSend(ch, data_buf, ret, (uint8*)&destip ,destport);

		if(ret == SOCKERR_TIME_OUT){
			ERR("Timeout");
			UDPClose(ch);
			DBG("UDP Socket Close");
		}

	} else if(ret == SOCKERR_NOT_UDP){	// Not UDP Socket, It's TCP Socket
		DBG("TCP Socket Close");
		TCPClose(ch);
	} else if(ret == SOCKERR_CLOSED){		// Socket Closed
		LOGA("UDP Loop-Back Started - ch(%d)",(uint16)ch);
		UDPOpen(ch, port);
	}
}
