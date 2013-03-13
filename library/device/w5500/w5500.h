/**
 * @file		w5500.h
 * @brief		W5500 HAL Header File.
 * This is used by socket.c
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef  _W5500_H_
#define  _W5500_H_

//#include "Types.h"

#define IINCHIP_ISR_DISABLE()  __disable_fault_irq()
#define IINCHIP_ISR_ENABLE() __enable_fault_irq()

//----------------------------- W5500 Common Registers : WIZC_Reg -----------------------------
#define WIZC_MR                          (0x0000)    /** @brief Mode Register address */
#define WIZC_GAR0                        (0x0001)    /** @brief Gateway IP Register address */
#define WIZC_SUBR0                       (0x0005)    /** @brief Subnet mask Register address */
#define WIZC_SHAR0                       (0x0009)    /** @brief Source MAC Register address */
#define WIZC_SIPR0                       (0x000F)    /** @brief Source IP Register address */
#define WIZC_INTLEVEL0                   (0x0013)    /** @brief set Interrupt low level timer register address */
#define WIZC_INTLEVEL1                   (0x0014)
#define WIZC_IR                          (0x0015)    /** @brief Interrupt Register */
#define WIZC_IMR                         (0x0016)    /** @brief Interrupt mask register */
#define WIZC_SIR                         (0x0017)    /** @brief Socket Interrupt Register */
#define WIZC_SIMR                        (0x0018)    /** @brief Socket Interrupt Mask Register */
#define WIZC_RTR0                        (0x0019)    /** @brief Timeout register address( 1 is 100us ) */
#define WIZC_RCR                         (0x001B)    /** @brief Retry count reigster */
#define WIZC_PTIMER                      (0x001C)    /** @briefPPP LCP Request Timer register  in PPPoE mode */
#define WIZC_PMAGIC                      (0x001D)    /** @brief PPP LCP Magic number register  in PPPoE mode */
#define WIZC_PHA0                        (0x001E)    /** @brief PPP Destination MAC Register address */
#define WIZC_PSID0                       (0x0024)    /** @brief PPP Session Identification Register */
#define WIZC_PMR0                        (0x0026)    /** @brief PPP Maximum Segment Size(MSS) register */
#define WIZC_UIPR0                       (0x0028)    /** @brief Unreachable IP register address in UDP mode */
#define WIZC_UPORT0                      (0x002C)    /** @brief Unreachable Port register address in UDP mode */
#define WIZC_PSTATUS                     (0x002E)    /** @brief PHY Status Register */
// Reserved			         (0x002F)
// Reserved			         (0x0030)
// Reserved			         (0x0031)
// Reserved			         (0x0032)
// Reserved			         (0x0033)
// Reserved			         (0x0034)
// Reserved			         (0x0035)
// Reserved			         (0x0036)
// Reserved			         (0x0037)
// Reserved			         (0x0038)
#define WIZC_VERSIONR                    (0x0039)    /** @brief chip version register address */


//----------------------------- W5500 Socket Registers : WIZS_Reg -----------------------------
#define CH_SIZE                       (0x0100)    /** @brief  size of each channel register map */

#define WIZS_MR                       (0x0000)    /** @brief socket Mode register */
#define WIZS_CR                       (0x0001)    /** @brief channel Sn_CR register */
#define WIZS_IR                       (0x0002)    /** @brief channel interrupt register */
#define WIZS_SR                       (0x0003)    /** @brief channel status register */
#define WIZS_PORT0                    (0x0004)    /** @brief source port register */
#define WIZS_DHAR0                    (0x0006)    /** @brief Peer MAC register address */
#define WIZS_DIPR0                    (0x000C)    /** @brief Peer IP register address */
#define WIZS_DPORT0                   (0x0010)    /** @brief Peer port register address */
#define WIZS_MSSR0                    (0x0012)    /** @brief Maximum Segment Size(Sn_MSSR0) register address */
// Reserved			      (0x0014)
#define WIZS_TOS                      (0x0015)    /** @brief IP Type of Service(TOS) Register */
#define WIZS_TTL                      (0x0016)    /** @brief IP Time to live(TTL) Register */
// Reserved			      (0x0017)
// Reserved			      (0x0018)
// Reserved			      (0x0019)
// Reserved			      (0x001A)
// Reserved			      (0x001B)
// Reserved			      (0x001C)
// Reserved			      (0x001D)
#define WIZS_RXMEM_SIZE               (0x001E)    /** @brief Receive memory size reigster */
#define WIZS_TXMEM_SIZE               (0x001F)    /** @brief Transmit memory size reigster */
#define WIZS_TX_FSR0                  (0x0020)    /** @brief Transmit free memory size register */
#define WIZS_TX_RD0                   (0x0022)    /** @brief Transmit memory read pointer register address */
#define WIZS_TX_WR0                   (0x0024)    /** @brief Transmit memory write pointer register address */
#define WIZS_RX_RSR0                  (0x0026)    /** @brief Received data size register */
#define WIZS_RX_RD0                   (0x0028)    /** @brief Read point of Receive memory */
#define WIZS_RX_WR0                   (0x002A)    /** @brief Write point of Receive memory */
#define WIZS_IMR                      (0x002C)    /** @brief socket interrupt mask register */
#define WIZS_FRAG                     (0x002D)    /** @brief Fragment field value in IP header register */
#define WIZS_KPALVTR                  (0x002F)    /** @brief Keep Alive Timer register */
#define WIZS_TSR                      (0x0030)    /** @brief Timer Status register */


//----------------------------- W5500 Register values  -----------------------------
/*
 @brief Sn_TSR Values
 */
#define TS_SEND_ACK                 (0x04) 
#define TS_WAIT_ACK                 (0x02)
#define TS_DISABLE                  (0x01)

/* MODE register values */
#define MR_RST                       0x80     //< reset */
#define MR_WOL                       0x20     //< Wake on Lan */
#define MR_PB                        0x10     //< ping block */
#define MR_PPPOE                     0x08     //< enable pppoe */
#define MR_MACRAW_NOSIZECHK          0x04     //< enbale MACRAW NO SIZE CHECHK */
#define MR_UDP_FORCE_ARP             0x02     //< enbale UDP_FORCE_ARP CHECHK */

/* IR register values */
#define IR_CONFLICT                  0x80     //< check ip confict */
#define IR_UNREACH                   0x40     //< get the destination unreachable message in UDP sending */
#define IR_PPPoE                     0x20     //< get the PPPoE close message */
#define IR_MAGIC                     0x10     //< get the magic packet interrupt */
#define IR_SOCK(ch)                  (0x01 << ch) //< check socket interrupt */

/* Sn_MR values */
#define Sn_MR_CLOSE                  0x00     //< unused socket */
#define Sn_MR_TCP                    0x01     //< TCP */
#define Sn_MR_UDP                    0x02     //< UDP */
#define Sn_MR_IPRAW                  0x03     //< IP LAYER RAW SOCK */
#define Sn_MR_MACRAW                 0x04     //< MAC LAYER RAW SOCK */
#define Sn_MR_PPPOE                  0x05     //< PPPoE */
#define Sn_MR_UNIBLOCK               0x10     //< Unicast Block in UDP Multicating*/
#define Sn_MR_ND                     0x20     //< No Delayed Ack(TCP) flag */
#define Sn_MR_BROADBLOCK             0x40     //< Broadcast blcok in UDP Multicating */
#define Sn_MR_MULTI                  0x80     //< support UDP Multicating */

/* Sn_MR values on MACRAW MODE */
#define Sn_MR_MAWRAW_BCASTBLOCK      0xC0     //< support Broadcasting On MACRAW MODE */
#define Sn_MR_MAWRAW_MCASTBLOCK      0xA0     //< support IPv4 Multicasting On MACRAW MODE */
#define Sn_MR_MAWRAW_IPV6BLOCK       0x90     //< support IPv6 Multicasting On MACRAW MODE */
#define Sn_MR_MAWRAW_BCASTMCAST      0xE0     //< support Broadcasting On MACRAW MODE */
#define Sn_MR_MAWRAW_BCASTIPV6       0xD0     //< support Broadcasting On MACRAW MODE */
#define Sn_MR_MAWRAW_MCASTIPV6       0xB0     //< support Broadcasting On MACRAW MODE */
//#define Sn_MR_MAWRAW_MFENALE       0x10     //< support MAC Fileter Enable On MACRAW MODE */

/* Sn_CR values */
#define Sn_CR_OPEN                   0x01     //< initialize or open socket */
#define Sn_CR_LISTEN                 0x02     //< wait connection request in tcp mode(Server mode) */
#define Sn_CR_CONNECT                0x04     //< send connection request in tcp mode(Client mode) */
#define Sn_CR_DISCON                 0x08     //< send closing reqeuset in tcp mode */
#define Sn_CR_CLOSE                  0x10     //< close socket */
#define Sn_CR_SEND                   0x20     //< update txbuf pointer, send data */
#define Sn_CR_SEND_MAC               0x21     //< send data with MAC address, so without ARP process */
#define Sn_CR_SEND_KEEP              0x22     //<  send keep alive message */
#define Sn_CR_RECV                   0x40     //< update rxbuf pointer, recv data */

#ifdef __DEF_IINCHIP_PPP__
   #define Sn_CR_PCON                0x23      
   #define Sn_CR_PDISCON             0x24      
   #define Sn_CR_PCR                 0x25      
   #define Sn_CR_PCN                 0x26     
   #define Sn_CR_PCJ                 0x27     
#endif

/* Sn_IR values */
#ifdef __DEF_IINCHIP_PPP__
   #define Sn_IR_PRECV               0x80     
   #define Sn_IR_PFAIL               0x40     
   #define Sn_IR_PNEXT               0x20     
#endif
#define Sn_IR_SEND_OK                0x10     //< complete sending */
#define Sn_IR_TIMEOUT                0x08     //< assert timeout */
#define Sn_IR_RECV                   0x04     //< receiving data */
#define Sn_IR_DISCON                 0x02     //< closed socket */
#define Sn_IR_CON                    0x01     //< established connection */

/* Sn_SR values */
#define SOCK_CLOSED                  0x00     //< closed */
#define SOCK_INIT                    0x13     //< init state */
#define SOCK_LISTEN                  0x14     //< listen state */
#define SOCK_SYNSENT                 0x15     //< connection state */
#define SOCK_SYNRECV                 0x16     //< connection state */
#define SOCK_ESTABLISHED             0x17     //< success to connect */
#define SOCK_FIN_WAIT                0x18     //< closing state */
#define SOCK_CLOSING                 0x1A     //< closing state */
#define SOCK_TIME_WAIT               0x1B     //< closing state */
#define SOCK_CLOSE_WAIT              0x1C     //< closing state */
#define SOCK_LAST_ACK                0x1D     //< closing state */
#define SOCK_UDP                     0x22     //< udp socket */
#define SOCK_IPRAW                   0x32     //< ip raw mode socket */
#define SOCK_MACRAW                  0x42     //< mac raw mode socket */
#define SOCK_PPPOE                   0x5F     //< pppoe socket */

/* IP PROTOCOL */
#define IPPROTO_IP                   0        //< Dummy for IP */
#define IPPROTO_ICMP                 1        //< Control message protocol */
#define IPPROTO_IGMP                 2        //< Internet group management protocol */
#define IPPROTO_GGP                  3        //< Gateway^2 (deprecated) */
#define IPPROTO_TCP                  6        //< TCP */
#define IPPROTO_PUP                  12       //< PUP */
#define IPPROTO_UDP                  17       //< UDP */
#define IPPROTO_IDP                  22       //< XNS idp */
#define IPPROTO_ND                   77       //< UNOFFICIAL net disk protocol */
#define IPPROTO_RAW                  255      //< Raw IP packet */

/* Control Bits */
#define CB_SOCK_NUM                  0xE0        //< 3bits : Socket[0-2] */
#define CB_MEM_SEL                   0x10        //< 1bit  : Memory Selection */
#define CM_MEM_DM                    0xE0        /** Memory Direct Access Mode **/
#define CB_RXBUF_SOCKREG_SEL         0x08        //< 1bit  : Socket RX Buffers or uint8 Registers Selection */

#define CB_WRITE_EN                  0x04        //< 1bit  : Write Enable */
#define CB_SEQ_EN                    0x02        //< 1bit  : SEQ Enable */

// < SPI Control Byte Structure for W5500 >
//
// |            Control            | : Byte
// =================================
// |  Block Selector   |Instruction| : Bits
// ---------------------------------
// | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 
// ---------------------------------
// |Socket Num |   Control Tail    | : Meaning of Bits
// ---------------------------------
// * By way of exception, Control[7:3] bits '00000' choose the Common registers. 

#define CB_TAIL_COMMONREG_RD_SEQ      0x00 
#define CB_TAIL_COMREG_RD             0x01
#define CB_TAIL_COMMONREG_WR_SEQ      0x04
#define CB_TAIL_COMREG_WR             0x05

#define CB_TAIL_SOCKETREG_RD_SEQ      0x08
#define CB_TAIL_SOCKETREG_RD          0x09
#define CB_TAIL_SOCKETREG_WR_SEQ      0x0C
#define CB_TAIL_SOCKETREG_WR          0x0D

#define CB_TAIL_TXBUF_RD_SEQ          0x10
#define CB_TAIL_TXBUF_RD              0x11
#define CB_TAIL_TXBUF_WR_SEQ          0x14
#define CB_TAIL_TXBUF_WR              0x15

#define CB_TAIL_RXBUF_RD_SEQ          0x18
#define CB_TAIL_RXBUF_RD              0x19
#define CB_TAIL_RXBUF_WR_SEQ          0x1C
#define CB_TAIL_RXBUF_WR              0x1D


//----------------------------- Windowfull Control Values  -----------------------------
#define WINDOWFULL_FLAG_ON		1
#define WINDOWFULL_FLAG_OFF		0 
#define WINDOWFULL_MAX_RETRY_NUM	3
#define WINDOWFULL_WAIT_TIME		1000

/*********************************************************
* iinchip access functions
*********************************************************/
//--Common register RD/WR
void IINCHIP_WRITE_COMMON( uint16 addr,  uint8 data);
uint8 IINCHIP_READ_COMMON(uint16 addr);
void IINCHIP_READ_COMMON_SEQ(uint16 addr, uint8 len, uint8 * data); 
void IINCHIP_WRITE_COMMON_SEQ(uint16 addr, uint8 len, uint8 * data); 

//--Socket register RD/WR
void IINCHIP_WRITE_SOCKETREG(uint8 sock_num, uint16 addr,  uint8 data);
uint8 IINCHIP_READ_SOCKETREG(uint8 sock_num, uint16 addr);
void IINCHIP_READ_SOCKETREG_SEQ(uint8 sock_num, uint16 addr, uint8 len, uint8 * data); 
void IINCHIP_WRITE_SOCKETREG_SEQ(uint8 sock_num, uint16 addr, uint8 len, uint8 * data); 

//--Socket TX&RX RD/WR
void IINCHIP_WRITE_TXBUF(uint8 sock_num, uint16 addr, uint8 data);
uint8 IINCHIP_READ_TXBUF(uint8 sock_num, uint16 addr);
void IINCHIP_WRITE_RXBUF(uint8 sock_num, uint16 addr, uint8 data);
uint8 IINCHIP_READ_RXBUF(uint8 sock_num, uint16 addr);

void IINCHIP_WRITE_TXBUF_SEQ(uint8 sock_num, uint16 addr, uint16 len, uint8 * data);
void IINCHIP_READ_TXBUF_SEQ(uint8 sock_num, uint16 addr, uint16 len, uint8 * data);
void IINCHIP_WRITE_RXBUF_SEQ(uint8 sock_num, uint16 addr, uint16 len, uint8 * data);
void IINCHIP_READ_RXBUF_SEQ(uint8 sock_num, uint16 addr, uint16 len, uint8 * data);

//-- Direct Access Mode
void IINCHIP_WRITE_DM(uint16 addr,  uint8 data);
uint8 IINCHIP_READ_DM(uint16 addr);

void IINCHIP_RXBUF_WRRD(uint16 addr, uint8 data);

uint8 getISR(uint8 s);
void putISR(uint8 s, uint8 val);
uint16 getIINCHIP_RxMAX(uint8 s);
uint16 getIINCHIP_TxMAX(uint8 s);

void setMR(uint8 val);
void setRTR(uint16 timeout);  // set retry duration for data transmission, connection, closing ...
void setRCR(uint8 retry);     // set retry count (above the value, assert timeout interrupt)
void setIMR(uint8 mask);      // set interrupt mask
void setSn_MSS(uint8 s, uint16 Sn_MSSR);  // set maximum segment size
void setSn_PROTO(uint8 s, uint8 proto);   // set IP Protocol value using IP-Raw mode
void setSn_TTL(uint8 s, uint8 ttl);
uint8 getIR( void );
uint8 getSn_SR(uint8 s); // get socket status
uint8 getSn_IR(uint8 s); // get socket interrupt status
uint8 getSn_TSR(uint8 s);
uint16 getSn_TX_FSR(uint8 s); // get socket TX free buf size
uint16 getSn_RX_RSR(uint8 s); // get socket RX recv buf size

void setSHAR(uint8 * addr);           // set local MAC address
void setSIPR(uint8 * addr);           // set local IP address
void setGAR(uint8 * addr);            // set gateway address
void setSUBR(uint8 * addr);           // set subnet mask address
void getSHAR(uint8 * addr);           // get local MAC address
void getSIPR(uint8 * addr);           // get local IP address
void getGAR(uint8 * addr);            // get gateway address
void getSUBR(uint8 * addr);           // get subnet mask address

void getDIPR(uint8 s, uint8 *addr);   // get destination IP address
void getDPORT(uint8 s, uint16 *port); // get destination Port number

void clearIR(uint8 mask);   // clear interrupt

void send_data_processing(uint8 s, uint8 *wizdata, uint16 len);
void recv_data_processing(uint8 s, uint8 *wizdata, uint16 len);
void recv_data_ignore(uint8 s, uint16 len);

uint8 incr_windowfull_retry_cnt(uint8 s);
void init_windowfull_retry_cnt(uint8 s);

#endif
