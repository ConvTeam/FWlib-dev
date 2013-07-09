/**
 * @file		w5200/netctrl.c
 * @brief		Network Control Driver Source File - For w5200
 * @version	1.0
 * @date		2013/07/09
 * @par Revision
 *			2013/07/09 - 1.0 Release (devided from socket)
 * @author	modified by Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

//#define FILE_LOG_SILENCE
#include "common/common.h"
//#include "device/netctrl.h"


extern uint8 I_STATUS[TOTAL_SOCK_NUM];
extern uint16 SMASK[TOTAL_SOCK_NUM]; //  Variable for Tx buffer MASK in each channel
extern uint16 RMASK[TOTAL_SOCK_NUM]; //  Variable for Rx buffer MASK in each channel
extern uint16 SSIZE[TOTAL_SOCK_NUM]; //  Max Tx buffer size by each channel
extern uint16 RSIZE[TOTAL_SOCK_NUM]; //  Max Rx buffer size by each channel
extern uint16 SBUFBASEADDRESS[TOTAL_SOCK_NUM]; //  Tx buffer base address by each channel
extern uint16 RBUFBASEADDRESS[TOTAL_SOCK_NUM]; //  Rx buffer base address by each channel

static uint8 DNS[4]={0};
static dhcp_mode DHCP = NETINFO_STATIC;


void device_init(uint8 *tx_size, uint8 *rx_size)
{
	device_SW_reset();
	device_mem_init(tx_size, rx_size);
}

void device_SW_reset(void)
{ 
	setMR(MR_RST);
	DBGA("MR value is %02x", IINCHIP_READ(WIZC_MR));
}

void device_mem_init(uint8 *tx_size, uint8 *rx_size)
{
	int16 i, mul;
	int16 ssum, rsum;

	DBG("device_mem_init()");

	ssum = 0;
	rsum = 0;
	SBUFBASEADDRESS[0] = (uint16)(__DEF_IINCHIP_MAP_TXBUF__);   // Set base address of Tx memory for channel #0
	RBUFBASEADDRESS[0] = (uint16)(__DEF_IINCHIP_MAP_RXBUF__);   // Set base address of Rx memory for channel #0

	for (i=0; i<TOTAL_SOCK_NUM; i++)	// Set the size, masking and base address of Tx & Rx memory by each channel
	{
		IINCHIP_WRITE((Sn_TXMEM_SIZE(i)),tx_size[i]);
		IINCHIP_WRITE((Sn_RXMEM_SIZE(i)),rx_size[i]);

		DBGA("Sn_TXMEM_SIZE = %d, Sn_RXMEM_SIZE = %d", 
			IINCHIP_READ(Sn_TXMEM_SIZE(i)), IINCHIP_READ(Sn_RXMEM_SIZE(i)));

		SSIZE[i] = (int16)(0);
		RSIZE[i] = (int16)(0);

		if(ssum <= 16384) {	//if(ssum <= 8192)
			if(tx_size[i]==1 || tx_size[i]==2 || tx_size[i]==4 || 
				tx_size[i]==8 || tx_size[i]==16) mul = tx_size[i];
			else mul = 2;	// by Ssoo Default 2K --20120522
			SSIZE[i] = 0x400 * mul;
			SMASK[i] = 0x400 * mul - 1;
		}
		if(rsum <= 16384) {	//if(rsum <= 8192)
			if(rx_size[i]==1 || rx_size[i]==2 || rx_size[i]==4 || 
				rx_size[i]==8 || rx_size[i]==16) mul = rx_size[i];
			else mul = 2;	// by Ssoo Default 2K --20120522
			RSIZE[i] = 0x400 * mul;
			RMASK[i] = 0x400 * mul - 1;
		}

		ssum += SSIZE[i];
		rsum += RSIZE[i];

		if(i != 0) {		// Sets base address of Tx and Rx memory for channel #1,#2,#3
			SBUFBASEADDRESS[i] = SBUFBASEADDRESS[i-1] + SSIZE[i-1];
			RBUFBASEADDRESS[i] = RBUFBASEADDRESS[i-1] + RSIZE[i-1];
		}
		DBGA("ch = %d, SSIZE = %d, RSIZE = %d",i,SSIZE[i],RSIZE[i]);
		DBGA("SBUFBASEADDRESS = %d, RBUFBASEADDRESS = %d", 
			(uint16)SBUFBASEADDRESS[i],(uint16)RBUFBASEADDRESS[i]);
	}
}
 
void SetNetInfo(wiz_NetInfo *netinfo)
{
	if(netinfo->mac[0] != 0x00 || netinfo->mac[1] != 0x00 || netinfo->mac[2] != 0x00 || 
		netinfo->mac[3] != 0x00 || netinfo->mac[4] != 0x00 || netinfo->mac[5] != 0x00)
		setSHAR(netinfo->mac);							// set local MAC address
	if(netinfo->ip[0] != 0x00 || netinfo->ip[1] != 0x00 || netinfo->ip[2] != 0x00 || 
		netinfo->ip[3] != 0x00) setSIPR(netinfo->ip);	// set local IP address
	if(netinfo->sn[0] != 0x00 || netinfo->sn[1] != 0x00 || netinfo->sn[2] != 0x00 || 
		netinfo->sn[3] != 0x00) setSUBR(netinfo->sn);	// set Subnet mask
	if(netinfo->gw[0] != 0x00 || netinfo->gw[1] != 0x00 || netinfo->gw[2] != 0x00 || 
		netinfo->gw[3] != 0x00) setGAR(netinfo->gw);	// set Gateway address
	if(netinfo->dns[0] != 0x00 || netinfo->dns[1] != 0x00 || netinfo->dns[2] != 0x00 || 
		netinfo->dns[3] != 0x00){
		DNS[0] = netinfo->dns[0];
		DNS[1] = netinfo->dns[1];
		DNS[2] = netinfo->dns[2];
		DNS[3] = netinfo->dns[3];
	}

	if(netinfo->dhcp != 0) DHCP = netinfo->dhcp;
}

void ClsNetInfo(netinfo_member member)
{
	uint8 zero[6] = {0,};

	DBGA("Reset Address(%d)", member);
	switch(member) {
	//case NI_MAC_ADDR:	// If need, uncomment
	//	setSHAR(zero);
	//	break;
	case NI_IP_ADDR:
		setSIPR(zero);
		break;
	case NI_SN_MASK:
		setSUBR(zero);
		break;
	case NI_GW_ADDR:
		setGAR(zero);
		break;
	case NI_DNS_ADDR:
		DNS[0] = DNS[1] = DNS[2] = DNS[3] = 0;
		break;
	default:
		ERRA("wrong member value (%d)", member);
	}
}
 
void GetNetInfo(wiz_NetInfo *netinfo)
{
	getSHAR(netinfo->mac); // get local MAC address
	getSIPR(netinfo->ip); // get local IP address
	getSUBR(netinfo->sn); // get subnet mask address
	getGAR(netinfo->gw); // get gateway address
	netinfo->dns[0] = DNS[0];
	netinfo->dns[1] = DNS[1];
	netinfo->dns[2] = DNS[2];
	netinfo->dns[3] = DNS[3];
	netinfo->dhcp = DHCP;
}

void GetDstInfo(uint8 s, uint8 *dstip, uint16 *dstport)
{
	getDIPR(s, dstip);
	getDPORT(s, dstport);
}
 
void SetSocketOption(uint8 option_type, uint16 option_value)
{
	switch(option_type){
	case 0:
		setRTR(option_value); // set retry duration for data transmission, connection, closing ...
		break;
	case 1:
		setRCR((uint8)(option_value&0x00FF)); // set retry count (above the value, assert timeout interrupt)
		break;
	case 2:
		setIMR((uint8)(option_value&0x00FF)); // set interrupt mask.
		break;
	default:
		break;
	}
}
 
int8 GetTCPSocketStatus(uint8 s)
{
	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_TCP;
	}

	switch(getSn_SR(s)){
	case SOCK_CLOSED: return SOCKSTAT_CLOSED;             // closed
	case SOCK_INIT: return SOCKSTAT_INIT;                 // init state
	case SOCK_LISTEN: return SOCKSTAT_LISTEN;             // listen state
	case SOCK_SYNSENT: return SOCKSTAT_SYNSENT;           // connection state
	case SOCK_SYNRECV: return SOCKSTAT_SYNRECV;           // connection state
	case SOCK_ESTABLISHED: return SOCKSTAT_ESTABLISHED;   // success to connect
	case SOCK_FIN_WAIT: return SOCKSTAT_FIN_WAIT;         // closing state
	case SOCK_CLOSING: return SOCKSTAT_CLOSING;           // closing state
	case SOCK_TIME_WAIT: return SOCKSTAT_TIME_WAIT;       // closing state
	case SOCK_CLOSE_WAIT: return SOCKSTAT_CLOSE_WAIT;     // closing state
	case SOCK_LAST_ACK: return SOCKSTAT_LAST_ACK;         // closing state
	default:
		if((IINCHIP_READ(Sn_MR(Sn_MR_TCP))&0x0F) != Sn_MR_TCP)
			return SOCKERR_NOT_TCP;
		else return SOCKERR_WRONG_STATUS;
	}
}

int8 GetUDPSocketStatus(uint8 s)
{
	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_UDP;
	}

	switch(getSn_SR(s)){
	case SOCK_CLOSED: return SOCKSTAT_CLOSED; //  closed
	case SOCK_UDP: return SOCKSTAT_UDP;       //  udp socket
#if 0	
	case SOCK_IPRAW: return 11;		//  ip raw mode socket
	case SOCK_MACRAW: return 12;	//  mac raw mode socket
	case SOCK_PPPOE: return 13;		//  pppoe socket
#endif
	default:
		if((IINCHIP_READ(Sn_MR(Sn_MR_UDP))&0x0F) != Sn_MR_UDP)
			return SOCKERR_NOT_UDP;
		else return SOCKERR_WRONG_STATUS;
	}
}

uint16 GetSocketTxFreeBufferSize(uint8 s)
{
	return getSn_TX_FSR(s); // get socket TX free buf size
}

uint16 GetSocketRxRecvBufferSize(uint8 s)
{
	return getSn_RX_RSR(s); // get socket RX recv buf size
}



