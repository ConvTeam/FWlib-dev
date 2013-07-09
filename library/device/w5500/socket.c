/**
 * @file		w5500/socket.c
 * @brief		Socket Driver Source File - For w5500
 * @version	1.1
 * @date		2013/02/27
 * @par Revision
 *			2013/02/27 - 1.0 Release
 *			2013/07/09 - 1.1 Network control part was devided
 * @author	
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

//#define FILE_LOG_SILENCE
#include "common/common.h"
//#include "device/socket.h"


static uint16 local_port = 0xC000;	// Dynamic Port: C000(49152) ~ FFFF(65535)
static uint16 txrd_checker[TOTAL_SOCK_NUM];
static uint32 tcp_close_elapse[TOTAL_SOCK_NUM] = {0,};
static uint32 tcp_resend_elapse[TOTAL_SOCK_NUM] = {0,};


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
	//IINCHIP_WRITE(Sn_MR(s),Sn_MR_TCP);
	//IINCHIP_WRITE(Sn_PORT0(s),(uint8)((port & 0xff00) >> 8));
	//IINCHIP_WRITE((Sn_PORT0(s) + 1),(uint8)(port & 0x00ff));
	//IINCHIP_WRITE(Sn_CR(s),Sn_CR_OPEN); // run sockinit Sn_CR
	//while(IINCHIP_READ(Sn_CR(s)));	// wait to process the command...
	//DBGA("Sn_SR = %.2x , Protocol = %.2x", IINCHIP_READ(Sn_SR(s)), IINCHIP_READ(Sn_MR(s)));
        
	IINCHIP_WRITE_SOCKETREG(s, WIZS_MR, Sn_MR_TCP);
	IINCHIP_WRITE_SOCKETREG(s, WIZS_PORT0 + 0, (uint8)((port & 0xff00) >> 8));
	IINCHIP_WRITE_SOCKETREG(s, WIZS_PORT0 + 1, (uint8)(port & 0x00ff));
	IINCHIP_WRITE_SOCKETREG(s, WIZS_CR, Sn_CR_OPEN);  // run sockinit Sn_CR
	while(IINCHIP_READ_SOCKETREG(s, WIZS_CR));        // wait to process the command...
	DBGA("Sn_SR = %.2x , Protocol = %.2x", IINCHIP_READ_SOCKETREG(s, WIZS_SR), IINCHIP_READ_SOCKETREG(s, WIZS_MR));

	//if (IINCHIP_READ(Sn_SR(s)) != SOCK_INIT) {
		//DBGA("wrong status(%d)", IINCHIP_READ(Sn_SR(s)));
		if(IINCHIP_READ_SOCKETREG(s, WIZS_SR) != SOCK_INIT) {
		DBGA("wrong status(%d)", IINCHIP_READ_SOCKETREG(s, WIZS_SR)); 
		return SOCKERR_WRONG_STATUS;
	} else {
		//IINCHIP_WRITE(Sn_CR(s),Sn_CR_LISTEN);
		//while(IINCHIP_READ(Sn_CR(s)));	          // wait to process the command...
		IINCHIP_WRITE_SOCKETREG(s, WIZS_CR, Sn_CR_LISTEN);
		while(IINCHIP_READ_SOCKETREG(s, WIZS_CR));        // wait to process the command...
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
	//IINCHIP_WRITE(Sn_MR(s),Sn_MR_TCP);
	//IINCHIP_WRITE(Sn_PORT0(s),(uint8)((sport & 0xff00) >> 8));
	//IINCHIP_WRITE((Sn_PORT0(s) + 1),(uint8)(sport & 0x00ff));
	//IINCHIP_WRITE(Sn_CR(s),Sn_CR_OPEN); // run sockinit Sn_CR
	//while(IINCHIP_READ(Sn_CR(s)) );	// wait to process the command...
	//DBGA("Sn_SR = %.2x , Protocol = %.2x", IINCHIP_READ(Sn_SR(s)), IINCHIP_READ(Sn_MR(s)));

	IINCHIP_WRITE_SOCKETREG(s, WIZS_MR, Sn_MR_TCP);
	IINCHIP_WRITE_SOCKETREG(s, WIZS_PORT0 + 0, (uint8)((sport & 0xff00) >> 8));
	IINCHIP_WRITE_SOCKETREG(s, WIZS_PORT0 + 1, (uint8)(sport & 0x00ff));
	IINCHIP_WRITE_SOCKETREG(s, WIZS_CR, Sn_CR_OPEN);  // run sockinit Sn_CR
	while(IINCHIP_READ_SOCKETREG(s, WIZS_CR));        // wait to process the command...
	DBGA("Sn_SR = %.2x , Protocol = %.2x", IINCHIP_READ_SOCKETREG(s, WIZS_SR), IINCHIP_READ_SOCKETREG(s, WIZS_MR));

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
		//IINCHIP_WRITE(Sn_DIPR0(s),dip[0]);	// set destination IP
		//IINCHIP_WRITE((Sn_DIPR0(s) + 1),dip[1]);
		//IINCHIP_WRITE((Sn_DIPR0(s) + 2),dip[2]);
		//IINCHIP_WRITE((Sn_DIPR0(s) + 3),dip[3]);
		//IINCHIP_WRITE(Sn_DPORT0(s),(uint8)((dport & 0xff00) >> 8));
		//IINCHIP_WRITE((Sn_DPORT0(s) + 1),(uint8)(dport & 0x00ff));
		////SetSubnet(sn);	// for ARP Errata
		//IINCHIP_WRITE(Sn_CR(s),Sn_CR_CONNECT);
		//while (IINCHIP_READ(Sn_CR(s)) );	// wait for completion
		IINCHIP_WRITE_SOCKETREG(s, WIZS_DIPR0 + 0, dip[0]);	// set destination IP
		IINCHIP_WRITE_SOCKETREG(s, WIZS_DIPR0 + 1, dip[1]);
		IINCHIP_WRITE_SOCKETREG(s, WIZS_DIPR0 + 2, dip[2]);
		IINCHIP_WRITE_SOCKETREG(s, WIZS_DIPR0 + 3, dip[3]);
		IINCHIP_WRITE_SOCKETREG(s, WIZS_DPORT0 + 0, (uint8)((dport & 0xff00) >> 8));
		IINCHIP_WRITE_SOCKETREG(s, WIZS_DPORT0 + 1, (uint8)(dport & 0x00ff));
		//SetSubnet(sn);	// for ARP Errata
		IINCHIP_WRITE_SOCKETREG(s, WIZS_CR,Sn_CR_CONNECT);
		while (IINCHIP_READ_SOCKETREG(s, WIZS_CR));	// wait for completion
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

	//socksr = IINCHIP_READ(Sn_SR(s));
	socksr = IINCHIP_READ_SOCKETREG(s, WIZS_SR);

	//if(socksr == SOCK_ESTABLISHED || socksr == SOCK_SYNSENT) {		????????
	if(socksr == SOCK_ESTABLISHED) {
		//ClearSubnet();	// for ARP Errata
		return RET_OK;
	//} else if(IINCHIP_READ(Sn_IR(s)) & Sn_IR_TIMEOUT) {
		//IINCHIP_WRITE(Sn_IR(s), (Sn_IR_TIMEOUT));           // clear TIMEOUT Interrupt
        } else if(IINCHIP_READ_SOCKETREG(s, WIZS_IR) & Sn_IR_TIMEOUT) {
		IINCHIP_WRITE_SOCKETREG(s, WIZS_IR, Sn_IR_TIMEOUT);   // clear TIMEOUT Interrupt
		//ClearSubnet();	// for ARP Errata
		return SOCKERR_TIME_OUT;
	}

	return SOCKERR_BUSY;
}

int8 TCPDisconnect(uint8 s)
{
	ERR("Not implemented yet");
	return RET_NOK;
#if 0
	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_TCP;
	} else DBG("start");

	IINCHIP_WRITE(Sn_CR(s),Sn_CR_DISCON);
	while(IINCHIP_READ(Sn_CR(s)));  // wait to process the command...

	return RET_OK;
#endif
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

	//IINCHIP_WRITE(Sn_CR(s),Sn_CR_DISCON);
	//while(IINCHIP_READ(Sn_CR(s)));            // wait to process the command...
	IINCHIP_WRITE_SOCKETREG(s, WIZS_CR, Sn_CR_DISCON);
	while(IINCHIP_READ_SOCKETREG(s, WIZS_CR));  // wait to process the command...

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
	} else DBG("start");

	status = getSn_SR(s);
	if(status == SOCK_CLOSED) goto END_OK;
	else if(wizpf_tick_elapse(tcp_close_elapse[s]) < TIMEOUT_CLOSE_WAIT)
		return SOCKERR_BUSY;

	//IINCHIP_WRITE(Sn_CR(s),Sn_CR_CLOSE);
	//while(IINCHIP_READ(Sn_CR(s)));	    // wait to process the command...
	IINCHIP_WRITE_SOCKETREG(s, WIZS_CR, Sn_CR_CLOSE);
	while(IINCHIP_READ_SOCKETREG(s, WIZS_CR));  // wait to process the command...

END_OK:
	//IINCHIP_WRITE(Sn_IR(s), 0xFF);	// interrupt all clear
	IINCHIP_WRITE_SOCKETREG(s, WIZS_IR, 0xFF);	// interrupt all clear
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
		//IINCHIP_WRITE(Sn_CR(s),Sn_CR_CLOSE);
		//while(IINCHIP_READ(Sn_CR(s)));	    // wait to process the command...
		IINCHIP_WRITE_SOCKETREG(s, WIZS_CR, Sn_CR_CLOSE);
		while(IINCHIP_READ_SOCKETREG(s, WIZS_CR));  // wait to process the command...
	} else return SOCKERR_BUSY;

END_OK:
	//IINCHIP_WRITE(Sn_IR(s), 0xFF);	// interrupt all clear
	IINCHIP_WRITE_SOCKETREG(s, WIZS_IR, 0xFF);	// interrupt all clear
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
	//if((IINCHIP_READ(Sn_MR(s))&0x0F) != Sn_MR_TCP) return SOCKERR_NOT_TCP;
	if((IINCHIP_READ_SOCKETREG(s, WIZS_MR) & 0x0F) != Sn_MR_TCP) return SOCKERR_NOT_TCP;

	if(status == SOCK_FIN_WAIT) return SOCKERR_FIN_WAIT;
	if(status != SOCK_ESTABLISHED && status != SOCK_CLOSE_WAIT) return SOCKERR_NOT_ESTABLISHED;

	init_windowfull_retry_cnt(s);
	if(len > getIINCHIP_TxMAX(s)) len = getIINCHIP_TxMAX(s); // check size not to exceed MAX size.
	if(GetSocketTxFreeBufferSize(s) < len) return SOCKERR_BUSY;

	send_data_processing(s, (uint8*)buf, len);	// copy data
        
	//txrd_checker[s] = IINCHIP_READ(Sn_TX_RD0(s));
	//txrd_checker[s] = (txrd_checker[s] << 8) + IINCHIP_READ(Sn_TX_RD0(s) + 1);

	//IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND);
	//while(IINCHIP_READ(Sn_CR(s)));                // wait to process the command...

	txrd_checker[s] = IINCHIP_READ_SOCKETREG(s, WIZS_TX_RD0 + 0);
	txrd_checker[s] = (txrd_checker[s] << 8) + IINCHIP_READ_SOCKETREG(s, WIZS_TX_RD0 + 1);

	IINCHIP_WRITE_SOCKETREG(s, WIZS_CR,Sn_CR_SEND);
	while (IINCHIP_READ_SOCKETREG(s, WIZS_CR));	// wait to process the command...

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
	//if((IINCHIP_READ(Sn_MR(s))&0x0F) != Sn_MR_TCP) return SOCKERR_NOT_TCP;
        if((IINCHIP_READ_SOCKETREG(s, WIZS_MR) & 0x0F) != Sn_MR_TCP) return SOCKERR_NOT_TCP;

	if(status == SOCK_FIN_WAIT) return SOCKERR_FIN_WAIT;
	if(status != SOCK_ESTABLISHED && status != SOCK_CLOSE_WAIT) return SOCKERR_NOT_ESTABLISHED;

	status = incr_windowfull_retry_cnt(s);
	if(status == 1) tcp_resend_elapse[s] = wizpf_get_systick();
	else if(status > WINDOWFULL_MAX_RETRY_NUM) return SOCKERR_WINDOW_FULL;
	else if(wizpf_tick_elapse(tcp_resend_elapse[s]) < WINDOWFULL_WAIT_TIME)
		return SOCKERR_BUSY;

	//txrd_checker[s] = IINCHIP_READ(Sn_TX_RD0(s));
	//txrd_checker[s] = (txrd_checker[s] << 8) + IINCHIP_READ(Sn_TX_RD0(s) + 1);
  
	//IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND);
	//while(IINCHIP_READ(Sn_CR(s)));                // wait to process the command...
        
	txrd_checker[s] = IINCHIP_READ_SOCKETREG(s, WIZS_TX_RD0 + 0);
	txrd_checker[s] = (txrd_checker[s] << 8) + IINCHIP_READ_SOCKETREG(s, WIZS_TX_RD0 + 1);
        
	IINCHIP_WRITE_SOCKETREG(s, WIZS_CR,Sn_CR_SEND);
	while (IINCHIP_READ_SOCKETREG(s, WIZS_CR));	// wait to process the command...
	
	return RET_OK;
}

int32 TCPSendCHK(uint8 s)
{
	uint16 txrd;

	//if(!(IINCHIP_READ(Sn_IR(s)) & Sn_IR_SEND_OK)) {
		//if(IINCHIP_READ(Sn_SR(s)) == SOCK_CLOSED) {
	if(!(IINCHIP_READ_SOCKETREG(s, WIZS_IR) & Sn_IR_SEND_OK)) {
		if(IINCHIP_READ_SOCKETREG(s, WIZS_SR) == SOCK_CLOSED) {
			DBG("SOCK_CLOSED");                    
			TCPClose(s);
			return SOCKERR_CLOSED;
		}
		return SOCKERR_BUSY;
	//} else IINCHIP_WRITE(Sn_IR(s), Sn_IR_SEND_OK);                
	} else IINCHIP_WRITE_SOCKETREG(s, WIZS_IR, Sn_IR_SEND_OK);
         
	//txrd = IINCHIP_READ(Sn_TX_RD0(s));
	//txrd = (txrd << 8) + IINCHIP_READ(Sn_TX_RD0(s) + 1);

	txrd = IINCHIP_READ_SOCKETREG(s, WIZS_TX_RD0 + 0);
	txrd = (txrd << 8) + IINCHIP_READ_SOCKETREG(s, WIZS_TX_RD0 + 1);

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
		//if((IINCHIP_READ(Sn_MR(s))&0x0F) != Sn_MR_TCP) return SOCKERR_NOT_TCP;
		if((IINCHIP_READ_SOCKETREG(s, WIZS_MR) & 0x0F) != Sn_MR_TCP) return SOCKERR_NOT_TCP;

		if(status == SOCK_CLOSE_WAIT) return SOCKERR_CLOSE_WAIT;
		if(status != SOCK_ESTABLISHED && status != SOCK_CLOSE_WAIT) return SOCKERR_NOT_ESTABLISHED;
	} else {
		if(len < RSR_len) RSR_len = len;
		recv_data_processing(s, (uint8*)buf, RSR_len);
		//IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV);
		//while(IINCHIP_READ(Sn_CR(s)));		        // wait to process the command...

		IINCHIP_WRITE_SOCKETREG(s, WIZS_CR, Sn_CR_RECV);
		while(IINCHIP_READ_SOCKETREG(s, WIZS_CR));		// wait to process the command...
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
        
	//IINCHIP_WRITE(Sn_MR(s),Sn_MR_UDP);
	//IINCHIP_WRITE(Sn_PORT0(s),(uint8)((port & 0xff00) >> 8));
	//IINCHIP_WRITE((Sn_PORT0(s) + 1),(uint8)(port & 0x00ff));
	//IINCHIP_WRITE(Sn_CR(s),Sn_CR_OPEN); // run sockinit Sn_CR
	//while(IINCHIP_READ(Sn_CR(s)));	// wait to process the command...
	//DBGA("Sn_SR = %.2x , Protocol = %.2x", IINCHIP_READ(Sn_SR(s)), IINCHIP_READ(Sn_MR(s)));

	IINCHIP_WRITE_SOCKETREG(s, WIZS_MR, Sn_MR_UDP);
	IINCHIP_WRITE_SOCKETREG(s, WIZS_PORT0 + 0, (uint8)((port & 0xff00) >> 8));
	IINCHIP_WRITE_SOCKETREG(s, WIZS_PORT0 + 1, (uint8)(port & 0x00ff));
	IINCHIP_WRITE_SOCKETREG(s, WIZS_CR, Sn_CR_OPEN);  // run sockinit Sn_CR
	while(IINCHIP_READ_SOCKETREG(s, WIZS_CR));        // wait to process the command...
	DBGA("Sn_SR = %.2x , Protocol = %.2x", IINCHIP_READ_SOCKETREG(s, WIZS_SR), IINCHIP_READ_SOCKETREG(s, WIZS_MR));

	return RET_OK;
}

int8 UDPClose(uint8 s)
{
	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return SOCKERR_NOT_UDP;
	} else DBG("start");

	//IINCHIP_WRITE(Sn_CR(s),Sn_CR_CLOSE);
	//while(IINCHIP_READ(Sn_CR(s)));                // wait to process the command...
	//IINCHIP_WRITE(Sn_IR(s), 0xFF);	        // interrupt all clear

	IINCHIP_WRITE_SOCKETREG(s, WIZS_CR, Sn_CR_CLOSE);
	while(IINCHIP_READ_SOCKETREG(s, WIZS_CR));      // wait to process the command...
	IINCHIP_WRITE_SOCKETREG(s, WIZS_IR, 0xFF);	// interrupt all clear

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
	//if((IINCHIP_READ(Sn_MR(s))&0x0F) != Sn_MR_UDP) return SOCKERR_NOT_UDP;
	if((IINCHIP_READ_SOCKETREG(s, WIZS_MR) & 0x0F) != Sn_MR_UDP) return SOCKERR_NOT_UDP;        
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
		//IINCHIP_WRITE(Sn_DIPR0(s),addr[0]);
		//IINCHIP_WRITE((Sn_DIPR0(s) + 1),addr[1]);
		//IINCHIP_WRITE((Sn_DIPR0(s) + 2),addr[2]);
		//IINCHIP_WRITE((Sn_DIPR0(s) + 3),addr[3]);
		//IINCHIP_WRITE(Sn_DPORT0(s),(uint8)((port & 0xff00) >> 8));
		//IINCHIP_WRITE((Sn_DPORT0(s) + 1),(uint8)(port & 0x00ff));
		IINCHIP_WRITE_SOCKETREG(s, WIZS_DIPR0 + 0, addr[0]);
		IINCHIP_WRITE_SOCKETREG(s, WIZS_DIPR0 + 1, addr[1]);
		IINCHIP_WRITE_SOCKETREG(s, WIZS_DIPR0 + 2, addr[2]);
		IINCHIP_WRITE_SOCKETREG(s, WIZS_DIPR0 + 3, addr[3]);
		IINCHIP_WRITE_SOCKETREG(s, WIZS_DPORT0 + 0,(uint8)((port & 0xff00) >> 8));
		IINCHIP_WRITE_SOCKETREG(s, WIZS_DPORT0 + 1,(uint8)(port & 0x00ff));

		send_data_processing(s, (uint8*)buf, len);	// copy data
		//SetSubnet(sn);	// for ARP Errata

		//IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND);
		//while(IINCHIP_READ(Sn_CR(s)));  // wait to process the command...
		IINCHIP_WRITE_SOCKETREG(s, WIZS_CR, Sn_CR_SEND);
		while(IINCHIP_READ_SOCKETREG(s, WIZS_CR));  // wait to process the command...
	}

	return len;
}

int8 UDPSendCHK(uint8 s)
{
	//uint8 ir = IINCHIP_READ(Sn_IR(s));
	uint8 ir = IINCHIP_READ_SOCKETREG(s, WIZS_IR);

	//DBGA("WATCH UDP Send CHK - sock(%d)", s);
	if(!(ir & Sn_IR_SEND_OK)) {
		if(ir & Sn_IR_TIMEOUT) {
			DBG("send fail");
			//IINCHIP_WRITE(Sn_IR(s), (Sn_IR_SEND_OK | Sn_IR_TIMEOUT)); // clear SEND_OK & TIMEOUT
			IINCHIP_WRITE_SOCKETREG(s, WIZS_IR, (Sn_IR_SEND_OK | Sn_IR_TIMEOUT)); // clear SEND_OK & TIMEOUT Interrupt
			return SOCKERR_TIME_OUT;
		}
		return SOCKERR_BUSY;
	//} else IINCHIP_WRITE(Sn_IR(s), Sn_IR_SEND_OK);
	} else IINCHIP_WRITE_SOCKETREG(s, WIZS_IR, Sn_IR_SEND_OK);
	//ClearSubnet();	// for ARP Errata

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
	//if((IINCHIP_READ(Sn_MR(s))&0x0F) != Sn_MR_UDP) return SOCKERR_NOT_UDP;
	if((IINCHIP_READ_SOCKETREG(s, WIZS_MR) & 0x0F) != Sn_MR_UDP) return SOCKERR_NOT_UDP;
	if(status != SOCK_UDP) return SOCKERR_NOT_UDP;

	RSR_len = GetSocketRxRecvBufferSize(s);	// Check Receive Buffer of W5500
	if(RSR_len < 8) {
		DBGA("wrong data received (%d)", RSR_len);
		recv_data_ignore(s, RSR_len);
		//IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV);
		//while(IINCHIP_READ(Sn_CR(s)));
                
		IINCHIP_WRITE_SOCKETREG(s, WIZS_CR, Sn_CR_RECV);
		while(IINCHIP_READ_SOCKETREG(s, WIZS_CR));    

		return SOCKERR_NOT_SPECIFIED;
	} else {
		recv_data_processing(s, prebuf, 8);
		//IINCHIP_WRITE(Sn_CR(s), Sn_CR_RECV);	// 데이터를 처리한 후 이것을 해줘야 적용됨
		IINCHIP_WRITE_SOCKETREG(s, WIZS_CR, Sn_CR_RECV);	// 데이터를 처리한 후 이것을 해줘야 적용됨

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
		//while(IINCHIP_READ(Sn_CR(s)));	// IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV); 명령 후 해야함, 시간 벌려고 바로 안함
                
		while(IINCHIP_READ_SOCKETREG(s, WIZS_CR));	// IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV); 명령 후 해야함, 시간 벌려고 바로 안함

		DBGA("UDP Recv - addr(%d.%d.%d.%d:%d), t(%d), R(%d)", 
			addr[0], addr[1], addr[2], addr[3], *port, tmp_len, RSR_len);
		if(tmp_len == 0) {
			ERR("UDP Recv len Zero - remove rest all");
			recv_data_ignore(s, GetSocketRxRecvBufferSize(s));
			//IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV);
			//while(IINCHIP_READ(Sn_CR(s)));

			IINCHIP_WRITE_SOCKETREG(s, WIZS_CR, Sn_CR_RECV);
			while(IINCHIP_READ_SOCKETREG(s, WIZS_CR));

			return SOCKERR_NOT_SPECIFIED;
		}
		RSR_len = tmp_len;
	}

	if(len < RSR_len) {
		tmp_len = RSR_len - len;
		RSR_len = len;
		DBGA("Recv buffer not enough - len(%d)", len);
	} else tmp_len = 0;

	//switch (IINCHIP_READ(Sn_MR(s)) & 0x07)
	switch (IINCHIP_READ_SOCKETREG(s, WIZS_MR) & 0x07)
	{
	case Sn_MR_UDP:
		recv_data_processing(s, (uint8*)buf, RSR_len);
		//IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV);
		IINCHIP_WRITE_SOCKETREG(s, WIZS_CR, Sn_CR_RECV);

		if(tmp_len) {
			//while(IINCHIP_READ(Sn_CR(s)));
                  
			while(IINCHIP_READ_SOCKETREG(s, WIZS_CR));
			DBG("Ignore rest data");			
			recv_data_ignore(s, tmp_len); // 안버리면 이후 처리가 곤란함
			//IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV);
			//while(IINCHIP_READ(Sn_CR(s)));
                        
			IINCHIP_WRITE_SOCKETREG(s, WIZS_CR, Sn_CR_RECV);
			while(IINCHIP_READ_SOCKETREG(s, WIZS_CR));
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
	//while(IINCHIP_READ(Sn_CR(s)));
	while(IINCHIP_READ_SOCKETREG(s, WIZS_CR));
	
	return RSR_len;
}





