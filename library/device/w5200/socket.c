
//#define FILE_LOG_SILENCE
#include "common/common.h"
//#include "device/socket.h"

#define SUCCESS	1
#define FAIL	0

static uint8 Subnet[4]={0};
static uint8 DNSServerIP[4]={0};
static uint8 DHCPEnable=0;
static uint16 local_port;
extern uint16 sent_ptr;

extern uint16 SMASK[TOTAL_SOCK_NUM]; /**< Variable for Tx buffer MASK in each channel */
extern uint16 RMASK[TOTAL_SOCK_NUM]; /**< Variable for Rx buffer MASK in each channel */
extern uint16 SSIZE[TOTAL_SOCK_NUM]; /**< Max Tx buffer size by each channel */
extern uint16 RSIZE[TOTAL_SOCK_NUM]; /**< Max Rx buffer size by each channel */
extern uint16 SBUFBASEADDRESS[TOTAL_SOCK_NUM]; /**< Tx buffer base address by each channel */
extern uint16 RBUFBASEADDRESS[TOTAL_SOCK_NUM]; /**< Rx buffer base address by each channel */

void SetSubnet(uint8 *Subnet)
{
	setSUBR(Subnet); // set subnet mask address
}

void ClearSubnet()
{
	uint8 Subnet[4] = {0, 0, 0, 0};
	setSUBR(Subnet); // set subnet mask address
}

/**
@brief	This function initialize the W5200.
@return 	None.
*/  
void device_init(uint8 *tx_size, uint8 *rx_size)
{
	device_SW_reset();
	device_mem_init(tx_size, rx_size);
}

/**
@brief  This function is for resetting of the iinchip. Initializes the iinchip to work in whether DIRECT or INDIRECT mode
*/ 
void device_SW_reset()
{ 
	setMR( MR_RST );
	DBGA("MR value is %02x", IINCHIP_READ(WIZC_MR));
}


/**
@brief  This function set the transmit & receive buffer size as per the channels
*/ 
void device_mem_init( uint8 * tx_size, uint8 * rx_size  )
{
	int16 i;
	int16 ssum,rsum;

	ssum = 0;
	rsum = 0;
  
	SBUFBASEADDRESS[0] = (uint16)(__DEF_IINCHIP_MAP_TXBUF__);   /* Set base address of Tx memory for channel #0 */
	RBUFBASEADDRESS[0] = (uint16)(__DEF_IINCHIP_MAP_RXBUF__);   /* Set base address of Rx memory for channel #0 */

	for (i = 0 ; i < TOTAL_SOCK_NUM; i++)	// Set the size, masking and base address of Tx & Rx memory by each channel
	{
		IINCHIP_WRITE((Sn_TXMEM_SIZE(i)),tx_size[i]);
		IINCHIP_WRITE((Sn_RXMEM_SIZE(i)),rx_size[i]);

		DBGA("Sn_TXMEM_SIZE = %d",IINCHIP_READ(Sn_TXMEM_SIZE(i)));
		DBGA("Sn_RXMEM_SIZE = %d",IINCHIP_READ(Sn_RXMEM_SIZE(i)));

		SSIZE[i] = (int16)(0);
		RSIZE[i] = (int16)(0);

		if (ssum <= 16384)
		{
			switch( tx_size[i] )
			{
			case 1:
				SSIZE[i] = (int16)(1024);
				SMASK[i] = (uint16)(0x03FF);
				break;
			case 2:
				SSIZE[i] = (int16)(2048);
				SMASK[i] = (uint16)(0x07FF);
				break;
			case 4:
				SSIZE[i] = (int16)(4096);
				SMASK[i] = (uint16)(0x0FFF);
				break;
			case 8:
				SSIZE[i] = (int16)(8192);
				SMASK[i] = (uint16)(0x1FFF);
				break;
			case 16:
				SSIZE[i] = (int16)(16384);
				SMASK[i] = (uint16)(0x3FFF);
				break;
			}
		}

		if (rsum <= 16384)
		{
			switch( rx_size[i] )
			{
			case 1:
				RSIZE[i] = (int16)(1024);
				RMASK[i] = (uint16)(0x03FF);
				break;
			case 2:
				RSIZE[i] = (int16)(2048);
				RMASK[i] = (uint16)(0x07FF);
				break;
			case 4:
				RSIZE[i] = (int16)(4096);
				RMASK[i] = (uint16)(0x0FFF);
				break;
			case 8:
				RSIZE[i] = (int16)(8192);
				RMASK[i] = (uint16)(0x1FFF);
				break;
			case 16:
				RSIZE[i] = (int16)(16384);
				RMASK[i] = (uint16)(0x3FFF);
				break;
			}
		}
		ssum += SSIZE[i];
		rsum += RSIZE[i];

		if (i != 0)			// Sets base address of Tx and Rx memory for channel #1,#2,#3
		{
			SBUFBASEADDRESS[i] = SBUFBASEADDRESS[i-1] + SSIZE[i-1];
			RBUFBASEADDRESS[i] = RBUFBASEADDRESS[i-1] + RSIZE[i-1];
		}
		DBGA("ch = %d",i);
		DBGA("SBUFBASEADDRESS = %d",(uint16)SBUFBASEADDRESS[i]);
		DBGA("RBUFBASEADDRESS = %d",(uint16)RBUFBASEADDRESS[i]);
		DBGA("SSIZE = %d",SSIZE[i]);
		DBGA("RSIZE = %d",RSIZE[i]);

	}
}

/**
@brief	This function set the network information.
@return 	None.
*/  
void SetNetInfo(wiz_NetInfo *netinfo)
{
	if(netinfo->Mac[0] != 0x00 || netinfo->Mac[1] != 0x00 || netinfo->Mac[2] != 0x00 || netinfo->Mac[3] != 0x00 || netinfo->Mac[4] != 0x00 || netinfo->Mac[5] != 0x00)
		setSHAR(netinfo->Mac); // set local MAC address
	if(netinfo->IP[0] != 0x00 || netinfo->IP[1] != 0x00 || netinfo->IP[2] != 0x00 || netinfo->IP[3] != 0x00)
		setSIPR(netinfo->IP); // set local IP address
	if(netinfo->Subnet[0] != 0x00 || netinfo->Subnet[1] != 0x00 || netinfo->Subnet[2] != 0x00 || netinfo->Subnet[3] != 0x00){
		ClearSubnet(); // clear subnet mask address. for errata
		Subnet[0] = netinfo->Subnet[0];
		Subnet[1] = netinfo->Subnet[1];
		Subnet[2] = netinfo->Subnet[2];
		Subnet[3] = netinfo->Subnet[3];
	}
	if(netinfo->Gateway[0] != 0x00 || netinfo->Gateway[1] != 0x00 || netinfo->Gateway[2] != 0x00 || netinfo->Gateway[3] != 0x00)
		setGAR(netinfo->Gateway); // set gateway address

	if(netinfo->DNSServerIP[0] != 0x00 || netinfo->DNSServerIP[1] != 0x00 || netinfo->DNSServerIP[2] != 0x00 || netinfo->DNSServerIP[3] != 0x00){
		DNSServerIP[0] = netinfo->DNSServerIP[0];
		DNSServerIP[1] = netinfo->DNSServerIP[1];
		DNSServerIP[2] = netinfo->DNSServerIP[2];
		DNSServerIP[3] = netinfo->DNSServerIP[3];
	}

	DHCPEnable = netinfo->DHCPEnable;
}


/**
@brief	This function get the network information.
@return 	None.
*/  
void GetNetInfo(wiz_NetInfo *netinfo)
{
	getSHAR(netinfo->Mac); // get local MAC address
	getSIPR(netinfo->IP); // get local IP address

	//getSUBR(netinfo->Subnet); // get subnet mask address
	netinfo->Subnet[0] = Subnet[0];
	netinfo->Subnet[1] = Subnet[1];
	netinfo->Subnet[2] = Subnet[2];
	netinfo->Subnet[3] = Subnet[3];

	getGAR(netinfo->Gateway); // get gateway address

	netinfo->DNSServerIP[0] = DNSServerIP[0];
	netinfo->DNSServerIP[1] = DNSServerIP[1];
	netinfo->DNSServerIP[2] = DNSServerIP[2];
	netinfo->DNSServerIP[3] = DNSServerIP[3];

	netinfo->DHCPEnable = DHCPEnable;
}


/**
@brief	This function set the network option.
@return 	None.
*/  
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


/**
@brief	This function get the interrupt vector.
@return 	interrupt vector.
*/  
uint8 GetInterrupt(SOCKET s)
{
#ifdef __DEF_IINCHIP_INT__
	return getISR(s);
#else
	return IINCHIP_READ(Sn_IR(s));
#endif
}


/**
@brief	This function put the interrupt vector.
@return 	None.
*/  
void PutInterrupt(SOCKET s, uint8 vector)
{
#ifdef __DEF_IINCHIP_INT__
	putISR(s, vector);
#else
	IINCHIP_WRITE(Sn_IR(s), vector);
#endif
}


/**
@brief	This function get the TCP socket status.
@return 	TCP socket status.
*/  
int8 GetTCPSocketStatus(SOCKET s)
{
	int8 ret=0;

	switch(getSn_SR(s)){
	case SOCK_CLOSED:		/**< closed */
		ret = (int8)STATUS_CLOSED;
		break;
	case SOCK_INIT:			/**< init state */
		ret = STATUS_INIT;
		break;
	case SOCK_LISTEN:		/**< listen state */
		ret = STATUS_LISTEN;
		break;
	case SOCK_SYNSENT:		/**< connection state */
		ret = STATUS_SYNSENT;
		break;
	case SOCK_SYNRECV:		/**< connection state */
		ret = STATUS_SYNRECV;
		break;
	case SOCK_ESTABLISHED:		/**< success to connect */
		ret = STATUS_ESTABLISHED;
		break;
	case SOCK_FIN_WAIT:		/**< closing state */
		ret = STATUS_FIN_WAIT;
		break;
	case SOCK_CLOSING:		/**< closing state */
		ret = STATUS_CLOSING;
		break;
	case SOCK_TIME_WAIT:		/**< closing state */
		ret = STATUS_TIME_WAIT;
		break;
	case SOCK_CLOSE_WAIT:		/**< closing state */
		ret = STATUS_CLOSE_WAIT;
		break;
	case SOCK_LAST_ACK:		/**< closing state */
		ret = STATUS_LAST_ACK;
		break;
	default:
		if((IINCHIP_READ(Sn_MR(s))&0x0F) != Sn_MR_TCP)	ret = (int8)ERROR_NOT_TCP_SOCKET;
		break;
	}

	return ret;
}


/**
@brief	This function get the UDP socket status.
@return 	UDP socket status.
*/  
int8 GetUDPSocketStatus(SOCKET s)
{
	int8 ret=0;

	switch(getSn_SR(s)){
	case SOCK_CLOSED:		/**< closed */
		ret = (int8)STATUS_CLOSED;
		break;
	case SOCK_UDP:			/**< udp socket */
		ret = STATUS_UDP;
		break;
#if 0
	case SOCK_IPRAW:		/**< ip raw mode socket */
		ret = 11;
		break;
	case SOCK_MACRAW:		/**< mac raw mode socket */
		ret = 12;
		break;
	case SOCK_PPPOE:		/**< pppoe socket */
		ret = 13;
		break;
#endif
	default:
		if((IINCHIP_READ(Sn_MR(s))&0x0F) != Sn_MR_UDP)	ret = (int8)ERROR_NOT_UDP_SOCKET;
		break;
	}

	return ret;
}


/**
@brief	This Socket function get the TX free buffer size.
@return 	size of TX free buffer size.
*/  
uint16 GetSocketTxFreeBufferSize(SOCKET s)
{
	return getSn_TX_FSR(s); // get socket TX free buf size
}


/**
@brief	This Socket function get the RX recv buffer size.
@return 	size of RX recv buffer size.
*/  
uint16 GetSocketRxRecvBufferSize(SOCKET s)
{
	return getSn_RX_RSR(s); // get socket RX recv buf size
}


/**
@brief	This Socket function open TCP server socket.
@return 	1 - success, 0 - fail.
*/  
int8 TCPServerOpen(SOCKET s, uint16 port)
{
	uint8 ret;
	
	DBG("start");
	
	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return (int8)ERROR_NOT_TCP_SOCKET;
	}

	TCPClose(s);
	IINCHIP_WRITE(Sn_MR(s),Sn_MR_TCP);
	if (port != 0) {
		IINCHIP_WRITE(Sn_PORT0(s),(uint8)((port & 0xff00) >> 8));
		IINCHIP_WRITE((Sn_PORT0(s) + 1),(uint8)(port & 0x00ff));
	} else {
		local_port++; // if don't set the source port, set local_port number.
		IINCHIP_WRITE(Sn_PORT0(s),(uint8)((local_port & 0xff00) >> 8));
		IINCHIP_WRITE((Sn_PORT0(s) + 1),(uint8)(local_port & 0x00ff));
	}
	IINCHIP_WRITE(Sn_CR(s),Sn_CR_OPEN); // run sockinit Sn_CR

	/* wait to process the command... */
	while( IINCHIP_READ(Sn_CR(s)) ) ;

	DBGA("Sn_SR = %.2x , Protocol = %.2x", IINCHIP_READ(Sn_SR(s)), IINCHIP_READ(Sn_MR(s)));

	if (IINCHIP_READ(Sn_SR(s)) == SOCK_INIT)
	{
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_LISTEN);

		/* wait to process the command... */
		while( IINCHIP_READ(Sn_CR(s)) ) ;

		ret = SUCCESS;
	}
	else
	{
		ret = FAIL;
		DBG("Fail[invalid ip,port]");
	}

	return ret;
}


/**
@brief	This Socket function open TCP client socket.
@return 	1 - success, 0 - fail.
*/  
int8 TCPClientOpen(SOCKET s, uint16 port, uint8 * destip, uint16 destport)
{
	int8 ret;

	DBG("start");

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return (int8)ERROR_NOT_TCP_SOCKET;
	}

	TCPClose(s);
	IINCHIP_WRITE(Sn_MR(s),Sn_MR_TCP);
	if (port != 0) {
		IINCHIP_WRITE(Sn_PORT0(s),(uint8)((port & 0xff00) >> 8));
		IINCHIP_WRITE((Sn_PORT0(s) + 1),(uint8)(port & 0x00ff));
	} else {
		local_port++; // if don't set the source port, set local_port number.
		IINCHIP_WRITE(Sn_PORT0(s),(uint8)((local_port & 0xff00) >> 8));
		IINCHIP_WRITE((Sn_PORT0(s) + 1),(uint8)(local_port & 0x00ff));
	}
	IINCHIP_WRITE(Sn_CR(s),Sn_CR_OPEN); // run sockinit Sn_CR

	/* wait to process the command... */
	while( IINCHIP_READ(Sn_CR(s)) ) ;

	DBGA("Sn_SR = %.2x , Protocol = %.2x", IINCHIP_READ(Sn_SR(s)), IINCHIP_READ(Sn_MR(s)));

	if 
	(
		((destip[0] == 0xFF) && (destip[1] == 0xFF) && (destip[2] == 0xFF) && (destip[3] == 0xFF)) ||
	 	((destip[0] == 0x00) && (destip[1] == 0x00) && (destip[2] == 0x00) && (destip[3] == 0x00)) ||
	 	(port == 0x00) 
	) 
	{
		ret = FAIL;
		DBG("Fail[invalid ip,port]");
	}
	else
	{
		ret = SUCCESS;

		// set destination IP
		IINCHIP_WRITE(Sn_DIPR0(s),destip[0]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 1),destip[1]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 2),destip[2]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 3),destip[3]);
		IINCHIP_WRITE(Sn_DPORT0(s),(uint8)((destport & 0xff00) >> 8));
		IINCHIP_WRITE((Sn_DPORT0(s) + 1),(uint8)(destport & 0x00ff));

		SetSubnet(Subnet);	// for errata
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_CONNECT);
		/* wait for completion */
		while ( IINCHIP_READ(Sn_CR(s)) ) ;
		while ( IINCHIP_READ(Sn_SR(s)) != SOCK_SYNSENT )
		{
			if(IINCHIP_READ(Sn_SR(s)) == SOCK_ESTABLISHED)
			{
				break;
			}
#ifdef __DEF_IINCHIP_INT__
			if (getISR(s) & Sn_IR_TIMEOUT)
#else
			if (IINCHIP_READ(Sn_IR(s)) & Sn_IR_TIMEOUT)
#endif
			{
#ifdef __DEF_IINCHIP_INT__
				putISR(s, getISR(s) & ~(Sn_IR_TIMEOUT));  // clear TIMEOUT Interrupt
#else
				IINCHIP_WRITE(Sn_IR(s), (Sn_IR_TIMEOUT));  // clear TIMEOUT Interrupt
#endif
				ret = (int8)ERROR_TIME_OUT;
				break;
			}
		}
		ClearSubnet();	// for errata
	}

	return ret;
}


/**
@brief	This Socket function open UDP socket.
@return 	1 - success, 0 - fail.
*/  
int8 UDPOpen(SOCKET s, uint16 port)
{
	uint8 ret;

	DBG("start");

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return (int8)ERROR_NOT_UDP_SOCKET;
	}

	UDPClose(s);
	IINCHIP_WRITE(Sn_MR(s),Sn_MR_UDP);
	if (port != 0) {
		IINCHIP_WRITE(Sn_PORT0(s),(uint8)((port & 0xff00) >> 8));
		IINCHIP_WRITE((Sn_PORT0(s) + 1),(uint8)(port & 0x00ff));
	} else {
		local_port++; // if don't set the source port, set local_port number.
		IINCHIP_WRITE(Sn_PORT0(s),(uint8)((local_port & 0xff00) >> 8));
		IINCHIP_WRITE((Sn_PORT0(s) + 1),(uint8)(local_port & 0x00ff));
	}
	IINCHIP_WRITE(Sn_CR(s),Sn_CR_OPEN); // run sockinit Sn_CR

	/* wait to process the command... */
	while( IINCHIP_READ(Sn_CR(s)) ) ;

	ret = SUCCESS;

	DBGA("Sn_SR = %.2x , Protocol = %.2x", IINCHIP_READ(Sn_SR(s)), IINCHIP_READ(Sn_MR(s)));

	return ret;
}


/**
@brief	This function close the TCP socket and parameter is "s" which represent the socket number
@return 	1 - success, 0 - fail.
*/ 
int8 TCPClose(SOCKET s)
{
	uint8 status=0;
	uint8 cnt=0;

	DBG("start");

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return (int8)ERROR_NOT_TCP_SOCKET;
	}

	IINCHIP_WRITE(Sn_CR(s),Sn_CR_DISCON);
	/* wait to process the command... */
	while( IINCHIP_READ(Sn_CR(s)) ) ;

	status = getSn_SR(s);
	if(status == SOCK_ESTABLISHED)
		return FAIL;

	// FIN wait
	while(status != SOCK_CLOSED)
	{
		Delay_ms(100);
		cnt++;
		if(cnt > 2) break;
	}

	IINCHIP_WRITE(Sn_CR(s),Sn_CR_CLOSE);
	/* wait to process the command... */
	while( IINCHIP_READ(Sn_CR(s)) ) ;

	/* clear interrupt */	
	#ifdef __DEF_IINCHIP_INT__
                /* all clear */
	       putISR(s, 0x00);
	#else
                /* all clear */
		IINCHIP_WRITE(Sn_IR(s), 0xFF);
	#endif

	return SUCCESS;
}


/**
@brief	This function close the UDP socket and parameter is "s" which represent the socket number
@return 	1 - success, 0 - fail.
*/ 
int8 UDPClose(SOCKET s)
{
	DBG("start");

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return (int8)ERROR_NOT_UDP_SOCKET;
	}

	IINCHIP_WRITE(Sn_CR(s),Sn_CR_CLOSE);

	/* wait to process the command... */
	while( IINCHIP_READ(Sn_CR(s)) ) ;

	/* clear interrupt */	
	#ifdef __DEF_IINCHIP_INT__
                /* all clear */
	       putISR(s, 0x00);
	#else
                /* all clear */
		IINCHIP_WRITE(Sn_IR(s), 0xFF);
	#endif

	return SUCCESS;
}


/**
@brief	This function used to send the data in TCP mode
@return	1 for success else 0.
*/ 
int16 TCPSend(SOCKET s, const uint8 * src, uint16 len)
{
	uint8 status=0;
	uint16 ret=0;
	uint16 freesize=0;
	uint16 txrd, txrd_before_send;

	DBG("start");

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return (int16)ERROR_NOT_TCP_SOCKET;
	}

	status = getSn_SR(s);
	if(status == SOCK_CLOSED)			return (int16)ERROR_CLOSED;
	if((IINCHIP_READ(Sn_MR(s))&0x0F) != Sn_MR_TCP)	return (int16)ERROR_NOT_TCP_SOCKET;
	if(status == SOCK_FIN_WAIT)			return (int16)ERROR_FIN_WAIT;
	if(status != SOCK_ESTABLISHED && status != SOCK_CLOSE_WAIT)	return (int16)ERROR_NOT_ESTABLISHED;

	init_windowfull_retry_cnt(s);

	if (len > getIINCHIP_TxMAX(s)) ret = getIINCHIP_TxMAX(s); // check size not to exceed MAX size.
	else ret = len;

	// if freebuf is available, start.
	do 
	{
		freesize = GetSocketTxFreeBufferSize(s);
		DBGA("socket %d freesize(%d) empty or error", s, freesize);
	} while (freesize < ret);

	// copy data
	send_data_processing(s, (uint8 *)src, ret);
        
	txrd_before_send = IINCHIP_READ(Sn_TX_RD0(s));
	txrd_before_send = (txrd_before_send << 8) + IINCHIP_READ(Sn_TX_RD0(s) + 1);

	IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND);
	
	/* wait to process the command... */
	while( IINCHIP_READ(Sn_CR(s)) ) ;
	

#ifdef __DEF_IINCHIP_INT__
	while ( (getISR(s) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK )
#else
	while ( (IINCHIP_READ(Sn_IR(s)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK )
#endif
	{
		if(IINCHIP_READ(Sn_SR(s)) == SOCK_CLOSED)
		{
			DBG("SOCK_CLOSED");                    
			TCPClose(s);
			return (int16)ERROR_CLOSED;
		}
	}
#ifdef __DEF_IINCHIP_INT__
	putISR(s, getISR(s) & (~Sn_IR_SEND_OK));
#else
	IINCHIP_WRITE(Sn_IR(s), Sn_IR_SEND_OK);
#endif

                
	txrd = IINCHIP_READ(Sn_TX_RD0(s));
	txrd = (txrd << 8) + IINCHIP_READ(Sn_TX_RD0(s) + 1);

	if(txrd > txrd_before_send) {
		ret = txrd - txrd_before_send;
	} else {
		ret = (0xffff - txrd_before_send) + txrd + 1;
	}

	return ret;
}


/**
@brief	This function used to send the data in TCP mode
@return	1 for success else 0.
*/ 
int16 TCPReSend(SOCKET s)
{
	uint8 status=0;
	uint16 ret=0;
	uint16 txrd, txrd_before_send;

	DBG("start");

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return (int16)ERROR_NOT_TCP_SOCKET;
	}

	status = getSn_SR(s);
	if(status == SOCK_CLOSED)			return (int16)ERROR_CLOSED;
	if((IINCHIP_READ(Sn_MR(s))&0x0F) != Sn_MR_TCP)	return (int16)ERROR_NOT_TCP_SOCKET;
	if(status == SOCK_FIN_WAIT)			return (int16)ERROR_FIN_WAIT;
	if(status != SOCK_ESTABLISHED && status != SOCK_CLOSE_WAIT)	return (int16)ERROR_NOT_ESTABLISHED;

	if(incr_windowfull_retry_cnt(s) > WINDOWFULL_MAX_RETRY_NUM)
		return ERROR_WINDOW_FULL;

	txrd_before_send = IINCHIP_READ(Sn_TX_RD0(s));
	txrd_before_send = (txrd_before_send << 8) + IINCHIP_READ(Sn_TX_RD0(s) + 1);
  
	IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND);
	
	/* wait to process the command... */
	while( IINCHIP_READ(Sn_CR(s)) ) ;
	

#ifdef __DEF_IINCHIP_INT__
	while ( (getISR(s) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK )
#else
	while ( (IINCHIP_READ(Sn_IR(s)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK )
#endif
	{
		if(IINCHIP_READ(Sn_SR(s)) == SOCK_CLOSED)
		{
			DBG("SOCK_CLOSED");                    
			TCPClose(s);
			return (int16)ERROR_CLOSED;
		}
	}
#ifdef __DEF_IINCHIP_INT__
	putISR(s, getISR(s) & (~Sn_IR_SEND_OK));
#else
	IINCHIP_WRITE(Sn_IR(s), Sn_IR_SEND_OK);
#endif

                
	txrd = IINCHIP_READ(Sn_TX_RD0(s));
	txrd = (txrd << 8) + IINCHIP_READ(Sn_TX_RD0(s) + 1);

	if(txrd > txrd_before_send) {
		ret = txrd - txrd_before_send;
	} else {
		ret = (0xffff - txrd_before_send) + txrd + 1;
	}

	Delay_ms(WINDOWFULL_WAIT_TIME);

	return ret;
}


/**
@brief	This function is an application I/F function which is used to receive the data in TCP mode.
		It continues to wait for data as much as the application wants to receive.
		
@return	received data size for success else -1.
*/ 
int16 TCPRecv(SOCKET s, uint8 * buf, uint16 len)
{
	uint8 status=0;
	uint16 ret=0, RSR_len=0;

	DBG("start");

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return (int16)ERROR_NOT_TCP_SOCKET;
	}

	RSR_len = GetSocketRxRecvBufferSize(s);
	if(RSR_len == 0){
		status = getSn_SR(s);
		if(status == SOCK_CLOSED)				return (int16)ERROR_CLOSED;
		if((IINCHIP_READ(Sn_MR(s))&0x0F) != 0x01)		return (int16)ERROR_NOT_TCP_SOCKET;
		if(status == SOCK_CLOSE_WAIT)				return (int16)ERROR_CLOSE_WAIT;
		if(status != SOCK_ESTABLISHED && status != SOCK_FIN_WAIT)	return (int16)ERROR_NOT_ESTABLISHED;
		if(RSR_len == 0)					return  RSR_len;	// Check Receive Buffer of W5200
	}

	if ( len > 0 )
	{
		if(len < RSR_len) RSR_len = len;

		recv_data_processing(s, buf, RSR_len);
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV);

		/* wait to process the command... */
		while( IINCHIP_READ(Sn_CR(s)));
		/* ------- */

		ret = RSR_len;
	}
	return ret;
}


/**
@brief	This function is an application I/F function which is used to send the data for other then TCP mode. 
		Unlike TCP transmission, The peer's destination address and the port is needed.
		
@return	This function return send data size for success else -1.
*/ 
int16 UDPSend(SOCKET s, const uint8 * buf, uint16 len, uint8 * addr, uint16 port)
{
	uint8 status=0;
	uint16 ret=0;
	
	DBG("start");

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return (int16)ERROR_NOT_UDP_SOCKET;
	}

	status = getSn_SR(s);
	if(status == SOCK_CLOSED)			return (int16)ERROR_CLOSED;
	if((IINCHIP_READ(Sn_MR(s))&0x0F) != Sn_MR_UDP)	return (int16)ERROR_NOT_UDP_SOCKET;
	if(status != SOCK_UDP)				return (int16)ERROR_NOT_UDP_SOCKET;

	if (len > getIINCHIP_TxMAX(s)) ret = getIINCHIP_TxMAX(s); // check size not to exceed MAX size.
	else ret = len;

	if(	((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
		((port == 0x00)) ||(ret == 0) ) 
	{
		/* added return value */
		ret = 0;
		DBGA("%d Fail[%.2x.%.2x.%.2x.%.2x, %.d, %d]",s, addr[0], addr[1], addr[2], addr[3] , port, len);
		DBG("Fail[invalid ip,port]");
	}
	else
	{
		IINCHIP_WRITE(Sn_DIPR0(s),addr[0]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 1),addr[1]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 2),addr[2]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 3),addr[3]);
		IINCHIP_WRITE(Sn_DPORT0(s),(uint8)((port & 0xff00) >> 8));
		IINCHIP_WRITE((Sn_DPORT0(s) + 1),(uint8)(port & 0x00ff));
		// copy data
		send_data_processing(s, (uint8 *)buf, ret);

		SetSubnet(Subnet);	// for errata
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND);

		/* wait to process the command... */
		while( IINCHIP_READ(Sn_CR(s)) ) ;

#ifdef __DEF_IINCHIP_INT__
		while ( (getISR(s) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK ) 
#else
		while ( (IINCHIP_READ(Sn_IR(s)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK ) 
#endif
		{
#ifdef __DEF_IINCHIP_INT__
		if (getISR(s) & Sn_IR_TIMEOUT)
#else
			if (IINCHIP_READ(Sn_IR(s)) & Sn_IR_TIMEOUT)
#endif
			{
				DBG("send fail");
				/* clear interrupt */
#ifdef __DEF_IINCHIP_INT__
				putISR(s, getISR(s) & ~(Sn_IR_SEND_OK | Sn_IR_TIMEOUT));  /* clear SEND_OK & TIMEOUT */
#else
				IINCHIP_WRITE(Sn_IR(s), (Sn_IR_SEND_OK | Sn_IR_TIMEOUT)); /* clear SEND_OK & TIMEOUT */
#endif
				return (int16)ERROR_TIME_OUT;
			}
		}

#ifdef __DEF_IINCHIP_INT__
		putISR(s, getISR(s) & (~Sn_IR_SEND_OK));
#else
		IINCHIP_WRITE(Sn_IR(s), Sn_IR_SEND_OK);
#endif

		ClearSubnet();	// for errata
	}

	return ret;
}


/**
@brief	This function is an application I/F function which is used to receive the data in other then
	TCP mode. This function is used to receive UDP, IP_RAW and MAC_RAW mode, and handle the header as well. 
	
@return	This function return received data size for success else -1.
*/ 
int16 UDPRecv(SOCKET s, uint8 * buf, uint16 len, uint8 * addr, uint16 *port)
{
	uint8 status=0;
	uint16 data_len=0, RSR_len=0;

	DBG("start");

	if(s > TOTAL_SOCK_NUM) {
		ERRA("wrong socket number(%d)", s);
		return (int16)ERROR_NOT_UDP_SOCKET;
	}

	status = getSn_SR(s);
	if(status == SOCK_CLOSED)				return (int16)ERROR_CLOSED;
	if((IINCHIP_READ(Sn_MR(s))&0x0F) != Sn_MR_UDP)		return (int16)ERROR_NOT_UDP_SOCKET;
	if(status != SOCK_UDP)					return (int16)ERROR_NOT_UDP_SOCKET;
	
	RSR_len = GetSocketRxRecvBufferSize(s);
	if(RSR_len == 0) return RSR_len;	// Check Receive Buffer of W5200

	if ( len > 0 )
	{
		switch (IINCHIP_READ(Sn_MR(s)) & 0x07)
		{
		case Sn_MR_UDP :
			recv_data_processing(s, buf, RSR_len);
			// read peer's IP address, port number.
			addr[0] = buf[0];
			addr[1] = buf[1];
			addr[2] = buf[2];
			addr[3] = buf[3];
			*port = buf[4];
			*port = (*port << 8) + buf[5];
			data_len = buf[6];
			data_len = (data_len << 8) + buf[7];
			
			DBG("UDP msg arrived");
			DBGA("source Port : %d", *port);
			DBGA("source IP : %d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
			DBGA("payload length : %d", data_len);

			memcpy(buf, &buf[8], data_len); // data copy.

			break;

		case Sn_MR_IPRAW :
		case Sn_MR_MACRAW :
		default :
			break;
	}
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV);

		/* wait to process the command... */
		while( IINCHIP_READ(Sn_CR(s)) ) ;
	}
	
	DBG("recvfrom() end ..");
	return data_len;
}




