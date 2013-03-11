/**
 * @file		w5200/socket.c
 * @brief		Socket Driver Source File - For w5200
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	modified by Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

//#define FILE_LOG_SILENCE
#include "common/common.h"
//#include "device/socket.h"


extern uint8 I_STATUS[TOTAL_SOCK_NUM];
extern uint16 SMASK[TOTAL_SOCK_NUM]; //  Variable for Tx buffer MASK in each channel
extern uint16 RMASK[TOTAL_SOCK_NUM]; //  Variable for Rx buffer MASK in each channel
extern uint16 SSIZE[TOTAL_SOCK_NUM]; //  Max Tx buffer size by each channel
extern uint16 RSIZE[TOTAL_SOCK_NUM]; //  Max Rx buffer size by each channel
extern uint16 SBUFBASEADDRESS[TOTAL_SOCK_NUM]; //  Tx buffer base address by each channel
extern uint16 RBUFBASEADDRESS[TOTAL_SOCK_NUM]; //  Rx buffer base address by each channel

static uint8 DNS[4]={0};
static dhcp_mode DHCP = NETINFO_STATIC;
static uint16 local_port = 0xC000;	// Dynamic Port: C000(49152) ~ FFFF(65535)

static uint32 tcp_close_elapse[TOTAL_SOCK_NUM] = {0,};
static uint32 tcp_resend_elapse[TOTAL_SOCK_NUM] = {0,};
static uint16 txrd_checker[TOTAL_SOCK_NUM];


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

int8 TCPServerOpen(uint8 s, uint16 port)
{
	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_TCP;
	} else DBG("start");

	if (port == 0) {	// if don't set the source port, set local_port number.
		if(local_port == 0xffff) local_port = 0xc000;
		else local_port++;
		port = local_port;
	}

	TCPClose(s);
	IINCHIP_WRITE(Sn_MR(s),Sn_MR_TCP);
	IINCHIP_WRITE(Sn_PORT0(s),(uint8)((port & 0xff00) >> 8));
	IINCHIP_WRITE((Sn_PORT0(s) + 1),(uint8)(port & 0x00ff));
	IINCHIP_WRITE(Sn_CR(s),Sn_CR_OPEN); // run sockinit Sn_CR
	while(IINCHIP_READ(Sn_CR(s)));		// wait to process the command...
	DBGA("Sn_SR = %.2x , Protocol = %.2x", IINCHIP_READ(Sn_SR(s)), IINCHIP_READ(Sn_MR(s)));

	if (IINCHIP_READ(Sn_SR(s)) != SOCK_INIT) {
		DBGA("wrong status(%d)", IINCHIP_READ(Sn_SR(s)));
		return SOCKERR_WRONG_STATUS;
	} else {
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_LISTEN);
		while(IINCHIP_READ(Sn_CR(s)));	// wait to process the command...
	}

	return RET_OK;
}

int8 TCPClientOpen(uint8 s, uint16 sport, uint8 *dip, uint16 dport)
{
	int8 ret;

	DBG("start");
	ret = TCPCltOpenNB(s, sport, dip, dport);
	if(ret != RET_OK) return ret;

	do {
		ret = TCPConnChk(s);
	} while(ret == SOCKERR_BUSY);

	return ret;
}

int8 TCPCltOpenNB(uint8 s, uint16 sport, uint8 *dip, uint16 dport)
{
	uint8 srcip[4], snmask[4];

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_TCP;
	} else if(dip == NULL) {
		ERR("NULL Dst IP");
		return SOCKERR_WRONG_ARG;
	} else DBG("start");

	if (sport == 0) {	// if don't set the source port, set local_port number.
		if(local_port == 0xffff) local_port = 0xc000;
		else local_port++;
		sport = local_port;
	}

	TCPClose(s);
	IINCHIP_WRITE(Sn_MR(s),Sn_MR_TCP);
	IINCHIP_WRITE(Sn_PORT0(s),(uint8)((sport & 0xff00) >> 8));
	IINCHIP_WRITE((Sn_PORT0(s) + 1),(uint8)(sport & 0x00ff));
	IINCHIP_WRITE(Sn_CR(s),Sn_CR_OPEN); // run sockinit Sn_CR
	while(IINCHIP_READ(Sn_CR(s)) );		// wait to process the command...
	DBGA("Sn_SR = %.2x , Protocol = %.2x", IINCHIP_READ(Sn_SR(s)), IINCHIP_READ(Sn_MR(s)));

	getSIPR(srcip);
	getSUBR(snmask);
        
	if(	((dip[0] == 0xFF) && (dip[1] == 0xFF) && 
		 (dip[2] == 0xFF) && (dip[3] == 0xFF)) ||
	 	((dip[0] == 0x00) && (dip[1] == 0x00) && 
	 	 (dip[2] == 0x00) && (dip[3] == 0x00)) || (sport == 0x00) ) 
	{
		DBG("invalid ip or port");
		DBGA("SOCK(%d)-[%02x.%02x.%02x.%02x, %d]",s, 
			dip[0], dip[1], dip[2], dip[3] , sport);
		return SOCKERR_WRONG_ARG;
	}
	else if( (srcip[0]==0 && srcip[1]==0 && srcip[2]==0 && srcip[3]==0) && 
		(snmask[0]!=0 || snmask[1]!=0 || snmask[2]!=0 || snmask[3]!=0) ) //Mikej : ARP Errata
	{
		DBG("Source IP is NULL while SN Mask is Not NULL");
		return SOCKERR_NULL_SRC_IP;
	}
	else
	{
		IINCHIP_WRITE(Sn_DIPR0(s),dip[0]);	// set destination IP
		IINCHIP_WRITE((Sn_DIPR0(s) + 1),dip[1]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 2),dip[2]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 3),dip[3]);
		IINCHIP_WRITE(Sn_DPORT0(s),(uint8)((dport & 0xff00) >> 8));
		IINCHIP_WRITE((Sn_DPORT0(s) + 1),(uint8)(dport & 0x00ff));
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_CONNECT);
		while (IINCHIP_READ(Sn_CR(s)) );	// wait for completion
	}

	return RET_OK;
}

int8 TCPConnChk(uint8 s)
{
	uint8 socksr;

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_TCP;
	}

	socksr = IINCHIP_READ(Sn_SR(s));
	if(socksr == SOCK_ESTABLISHED) {	// || socksr == SOCK_SYNSENT) {	???
		return RET_OK;
	} else if(IINCHIP_READ(Sn_IR(s)) & Sn_IR_TIMEOUT) {
		IINCHIP_WRITE(Sn_IR(s), (Sn_IR_TIMEOUT));  // clear TIMEOUT Interrupt
		return SOCKERR_TIME_OUT;
	}

	return SOCKERR_BUSY;
}

int8 TCPClose(uint8 s)
{
	int8 ret;

	DBG("start");
	ret = TCPCloseNB(s);
	if(ret != RET_OK) return ret;

	do {
		ret = TCPCloseCHK(s);
	} while(ret == SOCKERR_BUSY);

	return ret;
}

int8 TCPCloseNB(uint8 s)
{
	uint8 status;

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_TCP;
	} else DBG("start");

	IINCHIP_WRITE(Sn_CR(s),Sn_CR_DISCON);
	while(IINCHIP_READ(Sn_CR(s)));  // wait to process the command...

	status = getSn_SR(s);
	if(status == SOCK_CLOSED) return SOCKERR_WRONG_STATUS;
	else tcp_close_elapse[s] = wizpf_get_systick();

	return RET_OK;
}

int8 TCPCloseCHK(uint8 s)
{
#define TIMEOUT_CLOSE_WAIT	200
	uint8 status;

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_TCP;
	}

	status = getSn_SR(s);
	if(status == SOCK_CLOSED) goto END_OK;
	else if(wizpf_tick_elapse(tcp_close_elapse[s]) < TIMEOUT_CLOSE_WAIT)
		return SOCKERR_BUSY;

	IINCHIP_WRITE(Sn_CR(s),Sn_CR_CLOSE);
	while(IINCHIP_READ(Sn_CR(s)));	// wait to process the command...

END_OK:
	IINCHIP_WRITE(Sn_IR(s), 0xFF);	// interrupt all clear
	return RET_OK;
}

int8 TCPClsRcvCHK(uint8 s)
{
#define TIMEOUT_CLOSE_WAIT	200
	uint8 status;

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_TCP;
	}

	status = getSn_SR(s);
	if(status == SOCK_CLOSED) goto END_OK;
	if(status == SOCK_CLOSE_WAIT) {
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_CLOSE);
		while(IINCHIP_READ(Sn_CR(s)));	// wait to process the command...
	} else return SOCKERR_BUSY;

END_OK:
	IINCHIP_WRITE(Sn_IR(s), 0xFF);	// interrupt all clear
	return RET_OK;
}

int32 TCPSend(uint8 s, const int8 *buf, uint16 len)
{
	int32 ret;

	while(1) {
		ret = TCPSendNB(s, buf, len);
		if(ret == RET_OK) break;
		if(ret != SOCKERR_BUSY) return ret;
	}

	while(1) {
		ret = TCPSendCHK(s);
		if(ret >= 0 || ret != SOCKERR_BUSY) break;
	}
	
	return ret;
}

int8 TCPSendNB(uint8 s, const int8 *buf, uint16 len)
{
	uint8 status = 0;

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_TCP;
	} else if(len == 0) {
		ERR("Zero length");
		return SOCKERR_WRONG_ARG;
	} else DBG("start");

	status = getSn_SR(s);
	if(status == SOCK_CLOSED) return SOCKERR_CLOSED;
	if((IINCHIP_READ(Sn_MR(s))&0x0F) != Sn_MR_TCP) return SOCKERR_NOT_TCP;
	if(status == SOCK_FIN_WAIT) return SOCKERR_FIN_WAIT;
	if(status != SOCK_ESTABLISHED && status != SOCK_CLOSE_WAIT) return SOCKERR_NOT_ESTABLISHED;

	init_windowfull_retry_cnt(s);
	if(len > getIINCHIP_TxMAX(s)) len = getIINCHIP_TxMAX(s); // check size not to exceed MAX size.
	if(GetSocketTxFreeBufferSize(s) < len) return SOCKERR_BUSY;

	send_data_processing(s, (uint8*)buf, len);	// copy data
        
	txrd_checker[s] = IINCHIP_READ(Sn_TX_RD0(s));
	txrd_checker[s] = (txrd_checker[s] << 8) + IINCHIP_READ(Sn_TX_RD0(s) + 1);

	IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND);
	while(IINCHIP_READ(Sn_CR(s)));  // wait to process the command...

	return RET_OK;
}

int32 TCPReSend(uint8 s)
{
	int32 ret;

	while(1) {
		ret = TCPReSendNB(s);
		if(ret == RET_OK) break;
		if(ret != SOCKERR_BUSY) return ret;
	}

	while(1) {
		ret = TCPSendCHK(s);
		if(ret >= 0 || ret != SOCKERR_BUSY) break;
	}
	
	return ret;
}

int8 TCPReSendNB(uint8 s)
{
	uint8 status=0;

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_TCP;
	} else DBG("start");

	status = getSn_SR(s);
	if(status == SOCK_CLOSED) return SOCKERR_CLOSED;
	if((IINCHIP_READ(Sn_MR(s))&0x0F) != Sn_MR_TCP) return SOCKERR_NOT_TCP;
	if(status == SOCK_FIN_WAIT) return SOCKERR_FIN_WAIT;
	if(status != SOCK_ESTABLISHED && status != SOCK_CLOSE_WAIT) return SOCKERR_NOT_ESTABLISHED;

	status = incr_windowfull_retry_cnt(s);
	if(status == 1) tcp_resend_elapse[s] = wizpf_get_systick();
	else if(status > WINDOWFULL_MAX_RETRY_NUM) return SOCKERR_WINDOW_FULL;
	else if(wizpf_tick_elapse(tcp_resend_elapse[s]) < WINDOWFULL_WAIT_TIME)
		return SOCKERR_BUSY;

	txrd_checker[s] = IINCHIP_READ(Sn_TX_RD0(s));
	txrd_checker[s] = (txrd_checker[s] << 8) + IINCHIP_READ(Sn_TX_RD0(s) + 1);
  
	IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND);
	while(IINCHIP_READ(Sn_CR(s)));  // wait to process the command...
	
	return RET_OK;
}

int32 TCPSendCHK(uint8 s)
{
	uint16 txrd;

	if(!(IINCHIP_READ(Sn_IR(s)) & Sn_IR_SEND_OK)) {
		if(IINCHIP_READ(Sn_SR(s)) == SOCK_CLOSED) {
			DBG("SOCK_CLOSED");                    
			TCPClose(s);
			return SOCKERR_CLOSED;
		}
		return SOCKERR_BUSY;
	} else IINCHIP_WRITE(Sn_IR(s), Sn_IR_SEND_OK);
         
	txrd = IINCHIP_READ(Sn_TX_RD0(s));
	txrd = (txrd << 8) + IINCHIP_READ(Sn_TX_RD0(s) + 1);

	if(txrd > txrd_checker[s]) return txrd - txrd_checker[s];
	else return (0xffff - txrd_checker[s]) + txrd + 1;
}

int32 TCPRecv(uint8 s, int8 *buf, uint16 len)
{
	uint8 status = 0;
	uint16 RSR_len = 0;

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_TCP;
	} else if(len == 0) {
		ERR("Zero length");
		return SOCKERR_WRONG_ARG;
	}

	RSR_len = GetSocketRxRecvBufferSize(s);	// Check Receive Buffer
	if(RSR_len == 0){
		status = getSn_SR(s);
		if(status == SOCK_CLOSED) return SOCKERR_CLOSED;
		if((IINCHIP_READ(Sn_MR(s))&0x0F) != Sn_MR_TCP) return SOCKERR_NOT_TCP;
		if(status == SOCK_CLOSE_WAIT) return SOCKERR_CLOSE_WAIT;
		if(status != SOCK_ESTABLISHED && status != SOCK_CLOSE_WAIT) return SOCKERR_NOT_ESTABLISHED;
	} else {
		if(len < RSR_len) RSR_len = len;
		recv_data_processing(s, (uint8*)buf, RSR_len);
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV);
		while(IINCHIP_READ(Sn_CR(s)));		// wait to process the command...
	}

	return RSR_len;
}

int8 UDPOpen(uint8 s, uint16 port)
{
	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_UDP;
	} else DBG("start");

	if (port == 0) {	// if don't set the source port, set local_port number.
		if(local_port == 0xffff) local_port = 0xc000;
		else local_port++;
		port = local_port;
	}

	UDPClose(s);
	IINCHIP_WRITE(Sn_MR(s),Sn_MR_UDP);
	IINCHIP_WRITE(Sn_PORT0(s),(uint8)((port & 0xff00) >> 8));
	IINCHIP_WRITE((Sn_PORT0(s) + 1),(uint8)(port & 0x00ff));
	IINCHIP_WRITE(Sn_CR(s),Sn_CR_OPEN); // run sockinit Sn_CR
	while(IINCHIP_READ(Sn_CR(s)));		// wait to process the command...
	DBGA("Sn_SR = %.2x , Protocol = %.2x", IINCHIP_READ(Sn_SR(s)), IINCHIP_READ(Sn_MR(s)));

	return RET_OK;
}

int8 UDPClose(uint8 s)
{
	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_UDP;
	} else DBG("start");

	IINCHIP_WRITE(Sn_CR(s),Sn_CR_CLOSE);
	while(IINCHIP_READ(Sn_CR(s)));  // wait to process the command...
	IINCHIP_WRITE(Sn_IR(s), 0xFF);	// interrupt all clear

	return RET_OK;
}

int32 UDPSend(uint8 s, const int8 *buf, uint16 len, uint8 *addr, uint16 port)
{
	int32 ret = 0;

	ret = UDPSendNB(s, buf, len, addr, port);
	if(ret < RET_OK) return ret;
	else len = ret;

	do {
		ret = UDPSendCHK(s);
		if(ret == RET_OK) return len;
		if(ret != SOCKERR_BUSY) return ret;
	} while(1);
}

int32 UDPSendNB(uint8 s, const int8 *buf, uint16 len, uint8 *addr, uint16 port)
{
	uint8 srcip[4], snmask[4], status = 0;

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_UDP;
	} else if(len == 0 || addr == NULL) {
		if(len == 0) ERR("Zero length");
		else ERR("NULL Dst IP");
		return SOCKERR_WRONG_ARG;
	} else DBG("start");

	status = getSn_SR(s);
	if(status == SOCK_CLOSED) return SOCKERR_CLOSED;
	if((IINCHIP_READ(Sn_MR(s))&0x0F) != Sn_MR_UDP) return SOCKERR_NOT_UDP;
	if(status != SOCK_UDP) return SOCKERR_NOT_UDP;

	if (len > getIINCHIP_TxMAX(s)) len = getIINCHIP_TxMAX(s); // check size not to exceed MAX size.
	getSIPR(srcip);
	getSUBR(snmask);

	if((addr[0]==0x00 && addr[1]==0x00 && addr[2]==0x00 && 
		addr[3]==0x00) || (port==0x00))
	{
		DBG("invalid ip or port");
		DBGA("SOCK(%d)-[%02x.%02x.%02x.%02x, %d, %d]",s, 
			addr[0], addr[1], addr[2], addr[3] , port, len);
		return SOCKERR_WRONG_ARG;
	}
	else if( (srcip[0]==0 && srcip[1]==0 && srcip[2]==0 && srcip[3]==0) && 
		(snmask[0]!=0 || snmask[1]!=0 || snmask[2]!=0 || snmask[3]!=0) ) //Mikej : ARP Errata
	{
		DBG("Source IP is NULL while SN Mask is Not NULL");
		return SOCKERR_NULL_SRC_IP;
	}
	else
	{
		IINCHIP_WRITE(Sn_DIPR0(s),addr[0]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 1),addr[1]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 2),addr[2]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 3),addr[3]);
		IINCHIP_WRITE(Sn_DPORT0(s),(uint8)((port & 0xff00) >> 8));
		IINCHIP_WRITE((Sn_DPORT0(s) + 1),(uint8)(port & 0x00ff));

		send_data_processing(s, (uint8*)buf, len);	// copy data

		IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND);
		while(IINCHIP_READ(Sn_CR(s)));  // wait to process the command...
	}

	return len;
}

int8 UDPSendCHK(uint8 s)
{
	uint8 ir = IINCHIP_READ(Sn_IR(s));

	//DBGA("WATCH UDP Send CHK - sock(%d)", s);
	if(!(ir & Sn_IR_SEND_OK)) {
		if(ir & Sn_IR_TIMEOUT) {
			DBG("send fail");
			IINCHIP_WRITE(Sn_IR(s), (Sn_IR_SEND_OK | Sn_IR_TIMEOUT)); // clear SEND_OK & TIMEOUT
			return SOCKERR_TIME_OUT;
		}
		return SOCKERR_BUSY;
	} else IINCHIP_WRITE(Sn_IR(s), Sn_IR_SEND_OK);

	return RET_OK;
}

int32 UDPRecv(uint8 s, int8 *buf, uint16 len, uint8 *addr, uint16 *port)
{
	uint8 prebuf[8], status = 0;
	uint16 tmp_len = 0, RSR_len = 0;

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_UDP;
	} else if(len == 0) {
		ERR("Zero length");
		return SOCKERR_WRONG_ARG;
	}

	status = getSn_SR(s);
	if(status == SOCK_CLOSED) return SOCKERR_CLOSED;
	if((IINCHIP_READ(Sn_MR(s))&0x0F) != Sn_MR_UDP) return SOCKERR_NOT_UDP;
	if(status != SOCK_UDP) return SOCKERR_NOT_UDP;

	RSR_len = GetSocketRxRecvBufferSize(s);	// Check Receive Buffer of W5200
	if(RSR_len < 8) {
		DBGA("wrong data received (%d)", RSR_len);
		recv_data_ignore(s, RSR_len);
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV);
		while(IINCHIP_READ(Sn_CR(s)));

		return SOCKERR_NOT_SPECIFIED;
	} else {
		recv_data_processing(s, prebuf, 8);
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV);

		if(addr) {		// read peer's IP address, port number.
			addr[0] = prebuf[0];
			addr[1] = prebuf[1];
			addr[2] = prebuf[2];
			addr[3] = prebuf[3];
		}
		if(port) {
			*port = prebuf[4];
			*port = (*port << 8) + prebuf[5];
		}
		tmp_len = prebuf[6];
		tmp_len = (tmp_len << 8) + prebuf[7];
		while(IINCHIP_READ(Sn_CR(s)));

		DBGA("UDP Recv - addr(%d.%d.%d.%d:%d), t(%d), R(%d)", 
			addr[0], addr[1], addr[2], addr[3], *port, tmp_len, RSR_len);
		if(tmp_len == 0) {
			ERR("UDP Recv len Zero - remove rest all");
			recv_data_ignore(s, GetSocketRxRecvBufferSize(s));
			IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV);
			while(IINCHIP_READ(Sn_CR(s)));

			return SOCKERR_NOT_SPECIFIED;
		}
		RSR_len = tmp_len;
	}

	if(len < RSR_len) {
		tmp_len = RSR_len - len;
		RSR_len = len;
		DBGA("Recv buffer not enough - len(%d)", len);
	} else tmp_len = 0;

	switch (IINCHIP_READ(Sn_MR(s)) & 0x07)
	{
	case Sn_MR_UDP:
		recv_data_processing(s, (uint8*)buf, RSR_len);
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV);

		if(tmp_len) {
			while(IINCHIP_READ(Sn_CR(s)));
			DBG("Ignore rest data");
			recv_data_ignore(s, tmp_len);
			IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV);
			while(IINCHIP_READ(Sn_CR(s)));
			tmp_len = GetSocketRxRecvBufferSize(s);
			if(tmp_len) DBGA("another rest data(%d)", tmp_len);
			else DBG("No rest data");
		}
		break;
	case Sn_MR_IPRAW:	
	case Sn_MR_MACRAW:
	default :
		break;
	}
	while(IINCHIP_READ(Sn_CR(s)));
	
	return RSR_len;
}




