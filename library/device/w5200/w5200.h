/**
 * @file		w5200.h
 * @brief		W5200 HAL Header File.
 * This is used by socket.c
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef	_W5200_H
#define	_W5200_H

//#include "common/common.h"

#define WIZ_COMMON_BASE 0x0000
#define __DEF_IINCHIP_MAP_BASE__ 0x0000
#define __DEF_IINCHIP_MAP_TXBUF__ (WIZ_COMMON_BASE + 0x8000) /* Internal Tx buffer address of the iinchip */
#define __DEF_IINCHIP_MAP_RXBUF__ (WIZ_COMMON_BASE + 0xC000) /* Internal Rx buffer address of the iinchip */

#define IINCHIP_ISR_DISABLE()
#define IINCHIP_ISR_ENABLE()	

//----------------------------- W5200 Common Registers -----------------------------
#define WIZC_MR				(WIZ_COMMON_BASE + 0x0000)	// Mode
#define WIZC_GAR0			(WIZ_COMMON_BASE + 0x0001)	// GW Address
#define WIZC_GAR1			(WIZ_COMMON_BASE + 0x0002)
#define WIZC_GAR2			(WIZ_COMMON_BASE + 0x0003)
#define WIZC_GAR3			(WIZ_COMMON_BASE + 0x0004)
#define WIZC_SUBR0			(WIZ_COMMON_BASE + 0x0005) // SN Mask Address
#define WIZC_SUBR1			(WIZ_COMMON_BASE + 0x0006)
#define WIZC_SUBR2			(WIZ_COMMON_BASE + 0x0007)
#define WIZC_SUBR3			(WIZ_COMMON_BASE + 0x0008)
#define WIZC_SHAR0			(WIZ_COMMON_BASE + 0x0009) // Source Hardware Address
#define WIZC_SHAR1			(WIZ_COMMON_BASE + 0x000A)
#define WIZC_SHAR2			(WIZ_COMMON_BASE + 0x000B)
#define WIZC_SHAR3			(WIZ_COMMON_BASE + 0x000C)
#define WIZC_SHAR4			(WIZ_COMMON_BASE + 0x000D)
#define WIZC_SHAR5			(WIZ_COMMON_BASE + 0x000E)
#define WIZC_SIPR0			(WIZ_COMMON_BASE + 0x000F) // Source IP Address
#define WIZC_SIPR1			(WIZ_COMMON_BASE + 0x0010)
#define WIZC_SIPR2			(WIZ_COMMON_BASE + 0x0011)
#define WIZC_SIPR3			(WIZ_COMMON_BASE + 0x0012)
// Reserved			(WIZ_COMMON_BASE + 0x0013)
// Reserved			(WIZ_COMMON_BASE + 0x0014)
#define WIZC_IR				(WIZ_COMMON_BASE + 0x0015) // Interrupt
#define WIZC_IMR2			(WIZ_COMMON_BASE + 0x0016) // Socket Interrupt Mask
#define WIZC_RTR0			(WIZ_COMMON_BASE + 0x0017) // Retry Time
#define WIZC_RTR1			(WIZ_COMMON_BASE + 0x0018)
#define WIZC_RCR			(WIZ_COMMON_BASE + 0x0019) // Retry Count
// Reserved			(WIZ_COMMON_BASE + 0x001A)
// Reserved			(WIZ_COMMON_BASE + 0x001B)
#define WIZC_PATR0			(WIZ_COMMON_BASE + 0x001C) // Authentication Type in PPPoE
#define WIZC_PATR1			(WIZ_COMMON_BASE + 0x001D)
#define WIZC_PPPALGO		(WIZ_COMMON_BASE + 0x001E) // Authentication Algorithm in PPPoE
#define WIZC_VERSIONR		(WIZ_COMMON_BASE + 0x001F) // Chip version
// Reserved			(WIZ_COMMON_BASE + 0x0020)
// Reserved			(WIZ_COMMON_BASE + 0x0021)
// Reserved			(WIZ_COMMON_BASE + 0x0022)
// Reserved			(WIZ_COMMON_BASE + 0x0023)
// Reserved			(WIZ_COMMON_BASE + 0x0024)
// Reserved			(WIZ_COMMON_BASE + 0x0025)
// Reserved			(WIZ_COMMON_BASE + 0x0026)
// Reserved			(WIZ_COMMON_BASE + 0x0027)
#define WIZC_PTIMER			(WIZ_COMMON_BASE + 0x0028) // PPP LCP RequestTimer
#define WIZC_PMAGIC			(WIZ_COMMON_BASE + 0x0029) // PPP LCP Magic number
// Reserved			(WIZ_COMMON_BASE + 0x002A)
// Reserved			(WIZ_COMMON_BASE + 0x002B)
// Reserved			(WIZ_COMMON_BASE + 0x002C)
// Reserved			(WIZ_COMMON_BASE + 0x002D)
// Reserved			(WIZ_COMMON_BASE + 0x002E)
// Reserved			(WIZ_COMMON_BASE + 0x002F)
#define WIZC_INTLEVEL0		(WIZ_COMMON_BASE + 0x0030) // Interrupt Low Level Timer
#define WIZC_INTLEVEL1		(WIZ_COMMON_BASE + 0x0031)
// Reserved			(WIZ_COMMON_BASE + 0x0032)
// Reserved			(WIZ_COMMON_BASE + 0x0033)
#define WIZC_IR2			(WIZ_COMMON_BASE + 0x0034) // Socket Interrupt
#define WIZC_PSTATUS		(WIZ_COMMON_BASE + 0x0035) // PHY Status
#define WIZC_IMR			(WIZ_COMMON_BASE + 0x0036) // Interrupt Mask


//----------------------------- W5200 Socket Registers -----------------------------
#define WIZ_SOCK_REG(ch_v, reg_offset_v) \
							WIZ_COMMON_BASE + 0x4000 + (ch_v * 0x0100) + reg_offset_v
//----------------- Offset -----------------
#define WIZS_MR				0x00 // Mode
#define WIZS_CR				0x01 // Command
#define WIZS_IR				0x02 // Interrupt
#define WIZS_SR				0x03 // Status
#define WIZS_PORT0			0x04 // Source Port
#define WIZS_PORT1			0x05
#define WIZS_DHAR0			0x06 // Destination Hardware Address
#define WIZS_DHAR1			0x07
#define WIZS_DHAR2			0x08
#define WIZS_DHAR3			0x09
#define WIZS_DHAR4			0x0A
#define WIZS_DHAR5			0x0B
#define WIZS_DIPR0			0x0C // Destination IP Address
#define WIZS_DIPR1			0x0D
#define WIZS_DIPR2			0x0E
#define WIZS_DIPR3			0x0F
#define WIZS_DPORT0			0x10 // Destination Port
#define WIZS_DPORT1			0x11
#define WIZS_MSSR0			0x12 // Maximum Segment Size
#define WIZS_MSSR1			0x13
#define WIZS_PROTO			0x14 // Protocol in IP Raw mode
#define WIZS_TOS			0x15 // IP TOS
#define WIZS_TTL			0x16 // IP TTL
// Reserved			0x17
// Reserved			0x18
// Reserved			0x19
// Reserved			0x1A
// Reserved			0x1B
// Reserved			0x1C
// Reserved			0x1D
#define WIZS_RXMEM_SIZE		0x1E // Receive Memory Size
#define WIZS_TXMEM_SIZE		0x1F // Transmit Memory Size
#define WIZS_TX_FSR0		0x20 // TX Free Size
#define WIZS_TX_FSR1		0x21
#define WIZS_TX_RD0			0x22 // TX Read Pointer
#define WIZS_TX_RD1			0x23
#define WIZS_TX_WR0			0x24 // TX Write Pointer
#define WIZS_TX_WR1			0x25
#define WIZS_RX_RSR0		0x26 // RX Received Size
#define WIZS_RX_RSR1		0x27
#define WIZS_RX_RD0			0x28 // RX Read Pointer
#define WIZS_RX_RD1			0x29
#define WIZS_RX_WR0			0x2A // RX Write Pointer
#define WIZS_RX_WR1			0x2B
#define WIZS_IMR			0x2C // Interrupt Mask
#define WIZS_FRAG0			0x2D // Fragment Offset in IP header
#define WIZS_FRAG1			0x2E

//--------------------------- For Backward Compatibility ---------------------------
#define WIZ_CH_BASE			(WIZ_COMMON_BASE + 0x4000)
#define WIZ_CH_SIZE			0x0100
#define Sn_MR(ch)			(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x0000) // socket Mode register
#define Sn_CR(ch)			(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x0001) // channel Sn_CR register
#define Sn_IR(ch)			(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x0002) // channel interrupt register
#define Sn_SR(ch)			(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x0003) // channel status register
#define Sn_PORT0(ch)		(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x0004) // source port register
#define Sn_DHAR0(ch)		(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x0006) // Peer MAC register address
#define Sn_DIPR0(ch)		(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x000C) // Peer IP register address
#define Sn_DPORT0(ch)		(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x0010) // Peer port register address
#define Sn_MSSR0(ch)		(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x0012) // Maximum Segment Size(Sn_MSSR0) register address
#define Sn_PROTO(ch)		(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x0014) // Protocol of IP Header field register in IP raw mode
#define Sn_TOS(ch)			(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x0015) // IP Type of Service(TOS) Register 
#define Sn_TTL(ch)			(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x0016) // IP Time to live(TTL) Register 
#define Sn_RXMEM_SIZE(ch)	(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x001E) // Receive memory size reigster
#define Sn_TXMEM_SIZE(ch)	(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x001F) // Transmit memory size reigster
#define Sn_TX_FSR0(ch)		(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x0020) // Transmit free memory size register
#define Sn_TX_RD0(ch)		(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x0022) // Transmit memory read pointer register address
#define Sn_TX_WR0(ch)		(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x0024) // Transmit memory write pointer register address
#define Sn_RX_RSR0(ch)		(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x0026) // Received data size register
#define Sn_RX_RD0(ch)		(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x0028) // Read point of Receive memory
#define Sn_RX_WR0(ch)		(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x002A) // Write point of Receive memory
#define Sn_IMR(ch)			(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x002C) // socket interrupt mask register
#define Sn_FRAG(ch)			(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x002D) // frag field value in IP header register
#define Sn_KEEP_TIMER(ch)	(WIZ_CH_BASE + ch * WIZ_CH_SIZE + 0x002F) // Keep Timer register

// MODE register values 
#define MR_RST				0x80 //< reset 
#define MR_WOL				0x20 //< Wake on Lan 
#define MR_PB				0x10 //< ping block 
#define MR_PPPOE			0x08 //< enable pppoe 
#define MR_LB				0x04 //< little or big endian selector in indirect mode 
#define MR_AI				0x02 //< auto-increment in indirect mode 
#define MR_IND				0x01 //< enable indirect mode 

// IR register values 
#define IR_CONFLICT			0x80 //< check ip confict 
#define IR_UNREACH			0x40 //< get the destination unreachable message in UDP sending 
#define IR_PPPoE			0x20 //< get the PPPoE close message 
#define IR_MAGIC			0x10 //< get the magic packet interrupt 
#define IR_SOCK(ch)			(0x01 << ch) //< check socket interrupt 

// Sn_MR values 
#define Sn_MR_CLOSE			0x00 //< unused socket 
#define Sn_MR_TCP			0x01 //< TCP 
#define Sn_MR_UDP			0x02 //< UDP 
#define Sn_MR_IPRAW			0x03 //< IP LAYER RAW SOCK 
#define Sn_MR_MACRAW		0x04 //< MAC LAYER RAW SOCK 
#define Sn_MR_PPPOE			0x05 //< PPPoE 
#define Sn_MR_ND			0x20 //< No Delayed Ack(TCP) flag 
#define Sn_MR_MULTI			0x80 //< support multicating 

// Sn_CR values 
#define Sn_CR_OPEN			0x01 //< initialize or open socket 
#define Sn_CR_LISTEN		0x02 //< wait connection request in tcp mode(Server mode) 
#define Sn_CR_CONNECT		0x04 //< send connection request in tcp mode(Client mode) 
#define Sn_CR_DISCON		0x08 //< send closing reqeuset in tcp mode 
#define Sn_CR_CLOSE			0x10 //< close socket 
#define Sn_CR_SEND			0x20 //< update txbuf pointer, send data 
#define Sn_CR_SEND_MAC		0x21 //< send data with MAC address, so without ARP process 
#define Sn_CR_SEND_KEEP		0x22 //<  send keep alive message 
#define Sn_CR_RECV			0x40 //< update rxbuf pointer, recv data 
#define Sn_CR_PCON			0x23		 
#define Sn_CR_PDISCON		0x24		 
#define Sn_CR_PCR			0x25		 
#define Sn_CR_PCN			0x26		
#define Sn_CR_PCJ			0x27		

// Sn_IR values 
#define Sn_IR_PRECV			0x80		
#define Sn_IR_PFAIL			0x40		
#define Sn_IR_PNEXT			0x20		
#define Sn_IR_SEND_OK		0x10 //< complete sending 
#define Sn_IR_TIMEOUT		0x08 //< assert timeout 
#define Sn_IR_RECV			0x04 //< receiving data 
#define Sn_IR_DISCON		0x02 //< closed socket 
#define Sn_IR_CON			0x01 //< established connection 

// Sn_SR values 
#define SOCK_CLOSED			0x00 //< closed 
#define SOCK_INIT 			0x13 //< init state 
#define SOCK_LISTEN			0x14 //< listen state 
#define SOCK_SYNSENT		0x15 //< connection state 
#define SOCK_SYNRECV		0x16 //< connection state 
#define SOCK_ESTABLISHED	0x17 //< success to connect 
#define SOCK_FIN_WAIT		0x18 //< closing state 
#define SOCK_CLOSING		0x1A //< closing state 
#define SOCK_TIME_WAIT		0x1B //< closing state 
#define SOCK_CLOSE_WAIT		0x1C //< closing state 
#define SOCK_LAST_ACK		0x1D //< closing state 
#define SOCK_UDP			0x22 //< udp socket 
#define SOCK_IPRAW			0x32 //< ip raw mode socket 
#define SOCK_MACRAW			0x42 //< mac raw mode socket 
#define SOCK_PPPOE			0x5F //< pppoe socket 

// IP PROTOCOL 
#define IPPROTO_IP			0 //< Dummy for IP 
#define IPPROTO_ICMP		1 //< Control message protocol 
#define IPPROTO_IGMP		2 //< Internet group management protocol 
#define IPPROTO_GGP			3 //< GW^2 (deprecated) 
#define IPPROTO_TCP			6 //< TCP 
#define IPPROTO_PUP			12 //< PUP 
#define IPPROTO_UDP			17 //< UDP 
#define IPPROTO_IDP			22 //< XNS idp 
#define IPPROTO_ND			77 //< UNOFFICIAL net disk protocol 
#define IPPROTO_RAW			255 //< Raw IP packet 

#define WINDOWFULL_FLAG_ON			1
#define WINDOWFULL_FLAG_OFF			0 
#define WINDOWFULL_MAX_RETRY_NUM	3
#define WINDOWFULL_WAIT_TIME		1000

/*********************************************************
* iinchip access functions
*********************************************************/
uint8 IINCHIP_READ(uint16 addr); 
uint8 IINCHIP_WRITE(uint16 addr,uint8 data);

uint8 getISR(uint8 s);
void putISR(uint8 s, uint8 val);
uint16 getIINCHIP_RxMAX(uint8 s);
uint16 getIINCHIP_TxMAX(uint8 s);
uint16 getIINCHIP_RxMASK(uint8 s);
uint16 getIINCHIP_TxMASK(uint8 s);
uint16 getIINCHIP_RxBASE(uint8 s);
uint16 getIINCHIP_TxBASE(uint8 s);

void setMR(uint8 val);
void setRTR(uint16 timeout); // set retry duration for data transmission, connection, closing ...
void setRCR(uint8 retry); // set retry count (above the value, assert timeout interrupt)
void setIMR(uint8 mask); // set interrupt mask. 
uint8 getIR( void );
void setSn_MSS(uint8 s, uint16 Sn_MSSR0); // set maximum segment size
void setSn_PROTO(uint8 s, uint8 proto); // set IP Protocol value using IP-Raw mode
uint8 getSn_IR(uint8 s); // get socket interrupt status
uint8 getSn_SR(uint8 s); // get socket status
uint16 getSn_TX_FSR(uint8 s); // get socket TX free buf size
uint16 getSn_RX_RSR(uint8 s); // get socket RX recv buf size
void setSn_TTL(uint8 s, uint8 ttl);

void send_data_processing(uint8 s, uint8 *data, uint16 len);
void recv_data_processing(uint8 s, uint8 *data, uint16 len);
void recv_data_ignore(uint8 s, uint16 len);

void setGAR(uint8 * addr); // set gateway address
void setSUBR(uint8 * addr); // set subnet mask address
void setSHAR(uint8 * addr); // set local MAC address
void setSIPR(uint8 * addr); // set local IP address
void getGAR(uint8 * addr);
void getSUBR(uint8 * addr);
void getSHAR(uint8 * addr);
void getSIPR(uint8 * addr);
void getDIPR(uint8 s, uint8 *addr);
void getDPORT(uint8 s, uint16 *port);

uint8 incr_windowfull_retry_cnt(uint8 s);
void init_windowfull_retry_cnt(uint8 s);

uint32 GetDestAddr(uint8 s);			/* Output destination IP address of appropriate channel */
uint32 GetDestPort(uint8 s);			/* Output destination port number of appropriate channel */
uint8 CheckDestInLocal(uint32 destip);			/* Check Destination in local or remote */
uint8 getSocket(uint8 status, uint8 start); 	/* Get handle of socket which status is same to 'status' */

#endif //_W5200_H



