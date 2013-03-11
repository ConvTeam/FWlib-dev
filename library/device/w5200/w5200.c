/**
 * @file		w5200/w5200.c
 * @brief		W5200 HAL Source File.
 * This is used by socket.c
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

//#define FILE_LOG_SILENCE
#include "common/common.h"
//#include "device/w5200/w5200.h"

//static uint8 I_STATUS[TOTAL_SOCK_NUM];
uint16 SMASK[TOTAL_SOCK_NUM]; //< Variable for Tx buffer MASK in each channel */
uint16 RMASK[TOTAL_SOCK_NUM]; //< Variable for Rx buffer MASK in each channel */
uint16 SSIZE[TOTAL_SOCK_NUM]; //< Max Tx buffer size by each channel */
uint16 RSIZE[TOTAL_SOCK_NUM]; //< Max Rx buffer size by each channel */
uint16 SBUFBASEADDRESS[TOTAL_SOCK_NUM]; //< Tx buffer base address by each channel */
uint16 RBUFBASEADDRESS[TOTAL_SOCK_NUM]; //< Rx buffer base address by each channel */

uint8 windowfull_retry_cnt[TOTAL_SOCK_NUM];

uint8 incr_windowfull_retry_cnt(uint8 s)
{
	return ++windowfull_retry_cnt[s];
}

void init_windowfull_retry_cnt(uint8 s)
{
	windowfull_retry_cnt[s] = 0;
}

//uint8 getISR(uint8 s)
//{
//	return I_STATUS[s];
//}
//void putISR(uint8 s, uint8 val)
//{
//	I_STATUS[s] = val;
//}
uint16 getIINCHIP_RxMAX(uint8 s)
{
	return RSIZE[s];
}
uint16 getIINCHIP_TxMAX(uint8 s)
{
	return SSIZE[s];
}
uint16 getIINCHIP_RxMASK(uint8 s)
{
	return RMASK[s];
}
uint16 getIINCHIP_TxMASK(uint8 s)
{
	return SMASK[s];
}
uint16 getIINCHIP_RxBASE(uint8 s)
{
	return RBUFBASEADDRESS[s];
}
uint16 getIINCHIP_TxBASE(uint8 s)
{
	return SBUFBASEADDRESS[s];
}
void IINCHIP_CSoff(void)
{
	wizspi_cs(WIZ_SPI1, VAL_LOW);
}
void IINCHIP_CSon(void)
{
	wizspi_cs(WIZ_SPI1, VAL_HIGH);
}
uint8  IINCHIP_SpiSendData(uint8 dat)
{
	return(wizspi_byte(WIZ_SPI1, dat));
}


 /*
@brief  This function writes the data into W5200 registers.
*/
uint8 IINCHIP_WRITE(uint16 addr,uint8 data)
{
  IINCHIP_ISR_DISABLE();                      // Interrupt Service Routine Disable

  //SPI MODE I/F
  IINCHIP_CSoff();                            // CS=0, SPI start

  IINCHIP_SpiSendData((addr & 0xFF00) >> 8);  // Address byte 1
  IINCHIP_SpiSendData(addr & 0x00FF);         // Address byte 2
  IINCHIP_SpiSendData(0x80);                  // Data write command and Write data length 1
  IINCHIP_SpiSendData(0x01);                  // Write data length 2
  IINCHIP_SpiSendData(data);                  // Data write (write 1byte data)

  IINCHIP_CSon();                             // CS=1,  SPI end

  IINCHIP_ISR_ENABLE();                       // Interrupt Service Routine Enable
  return 1;
}
 
/*
@brief  This function reads the value from W5200 registers.
*/
uint8 IINCHIP_READ(uint16 addr)
{
  uint8 data;
        
  IINCHIP_ISR_DISABLE();                       // Interrupt Service Routine Disable
  
  IINCHIP_CSoff();                             // CS=0, SPI start
  
  IINCHIP_SpiSendData((addr & 0xFF00) >> 8);   // Address byte 1
  IINCHIP_SpiSendData(addr & 0x00FF);          // Address byte 2
  IINCHIP_SpiSendData(0x00);                   // Data read command and Read data length 1
  IINCHIP_SpiSendData(0x01);                   // Read data length 2    
  data = IINCHIP_SpiSendData(0x00);            // Data read (read 1byte data)
  
  IINCHIP_CSon();                              // CS=1,  SPI end
  
  IINCHIP_ISR_ENABLE();                        // Interrupt Service Routine Enable
  return data;
}

/*
@brief  This function writes into W5200 memory(Buffer)
*/ 
uint16 IINCHIP_WRITE_BLOCK(uint16 addr,uint8* buf,uint16 len)
{
  uint16 idx = 0;

  if(len == 0)
    return 0;

  IINCHIP_ISR_DISABLE();

  //SPI MODE I/F
  IINCHIP_CSoff();                                        // CS=0, SPI start 
  
  IINCHIP_SpiSendData(((addr+idx) & 0xFF00) >> 8);        // Address byte 1
  IINCHIP_SpiSendData((addr+idx) & 0x00FF);               // Address byte 2
  IINCHIP_SpiSendData((0x80 | ((len & 0x7F00) >> 8)));    // Data write command and Write data length 1
  IINCHIP_SpiSendData((len & 0x00FF));                    // Write data length 2
  for(idx = 0; idx < len; idx++)                          // Write data in loop
  {   
    IINCHIP_SpiSendData(buf[idx]);
  }
  
  IINCHIP_CSon();                                         // CS=1, SPI end 
        
  IINCHIP_ISR_ENABLE();                                   // Interrupt Service Routine Enable        
  return len;
}

/*
@brief  This function reads into W5200 memory(Buffer)
*/ 
uint16 IINCHIP_READ_BLOCK(uint16 addr, uint8* buf,uint16 len)
{
  uint16 idx = 0;
        
  IINCHIP_ISR_DISABLE();                                  // Interrupt Service Routine Disable
        
  IINCHIP_CSoff();                                        // CS=0, SPI start 
        
  IINCHIP_SpiSendData(((addr+idx) & 0xFF00) >> 8);        // Address byte 1
  IINCHIP_SpiSendData((addr+idx) & 0x00FF);               // Address byte 2
  IINCHIP_SpiSendData((0x00 | ((len & 0x7F00) >> 8)));    // Data read command
  IINCHIP_SpiSendData((len & 0x00FF));            

  for(idx = 0; idx < len; idx++)                          // Read data in loop
  {
    buf[idx] = IINCHIP_SpiSendData(0x00);
    
  }
        
  IINCHIP_CSon();                                         // CS=0, SPI end      
        
  IINCHIP_ISR_ENABLE();                                   // Interrupt Service Routine Enable
  return len;
}

/*
@brief  This function sets up gateway IP address.
@param addr a pointer to a 4 -byte array responsible to set the GW address.
*/ 
void setGAR(uint8 *addr)
{
  IINCHIP_WRITE((WIZC_GAR0),addr[0]);
  IINCHIP_WRITE((WIZC_GAR1),addr[1]);
  IINCHIP_WRITE((WIZC_GAR2),addr[2]);
  IINCHIP_WRITE((WIZC_GAR3),addr[3]);
}

/*
void getGWIP(uint8 * addr)
{
  addr[0] = IINCHIP_READ((WIZC_GAR0));
  addr[1] = IINCHIP_READ((WIZC_GAR1));
  addr[2] = IINCHIP_READ((WIZC_GAR2));
  addr[3] = IINCHIP_READ((WIZC_GAR3));
}
*/

/*
@brief  It sets up SubnetMask address
@param addr a pointer to a 4 -byte array responsible to set the SubnetMask address
*/ 
void setSUBR(uint8 *addr)
{
  IINCHIP_WRITE((WIZC_SUBR0),addr[0]);
  IINCHIP_WRITE((WIZC_SUBR1),addr[1]);
  IINCHIP_WRITE((WIZC_SUBR2),addr[2]);
  IINCHIP_WRITE((WIZC_SUBR3),addr[3]);
}

/*
@brief  This function sets up MAC address.
@param addr a pointer to a 6 -byte array responsible to set the MAC address. 
*/ 
void setSHAR(uint8 *addr)
{
  IINCHIP_WRITE((WIZC_SHAR0),addr[0]);
  IINCHIP_WRITE((WIZC_SHAR1),addr[1]);
  IINCHIP_WRITE((WIZC_SHAR2),addr[2]);
  IINCHIP_WRITE((WIZC_SHAR3),addr[3]);
  IINCHIP_WRITE((WIZC_SHAR4),addr[4]);
  IINCHIP_WRITE((WIZC_SHAR5),addr[5]);
}

/*
@brief  This function sets up Source IP address.
@param addr a pointer to a 4 -byte array responsible to set the Source IP address.
*/
void setSIPR(uint8 *addr)
{
  IINCHIP_WRITE((WIZC_SIPR0),addr[0]);
  IINCHIP_WRITE((WIZC_SIPR1),addr[1]);
  IINCHIP_WRITE((WIZC_SIPR2),addr[2]);
  IINCHIP_WRITE((WIZC_SIPR3),addr[3]);
}

/*
@brief  This function sets up Source IP address.
*/
void getGAR(uint8 *addr)
{
  addr[0] = IINCHIP_READ(WIZC_GAR0);
  addr[1] = IINCHIP_READ(WIZC_GAR1);
  addr[2] = IINCHIP_READ(WIZC_GAR2);
  addr[3] = IINCHIP_READ(WIZC_GAR3);
}
void getSUBR(uint8 *addr)
{
  addr[0] = IINCHIP_READ(WIZC_SUBR0);
  addr[1] = IINCHIP_READ(WIZC_SUBR1);
  addr[2] = IINCHIP_READ(WIZC_SUBR2);
  addr[3] = IINCHIP_READ(WIZC_SUBR3);
}
void getSHAR(uint8 *addr)
{
  addr[0] = IINCHIP_READ(WIZC_SHAR0);
  addr[1] = IINCHIP_READ(WIZC_SHAR1);
  addr[2] = IINCHIP_READ(WIZC_SHAR2);
  addr[3] = IINCHIP_READ(WIZC_SHAR3);
  addr[4] = IINCHIP_READ(WIZC_SHAR4);
  addr[5] = IINCHIP_READ(WIZC_SHAR5);
}
void getSIPR(uint8 *addr)
{
  addr[0] = IINCHIP_READ(WIZC_SIPR0);
  addr[1] = IINCHIP_READ(WIZC_SIPR1);
  addr[2] = IINCHIP_READ(WIZC_SIPR2);
  addr[3] = IINCHIP_READ(WIZC_SIPR3);
}
void getDIPR(uint8 s, uint8 *addr)
{
  addr[0] = IINCHIP_READ(WIZ_SOCK_REG(s, WIZS_DIPR0));
  addr[1] = IINCHIP_READ(WIZ_SOCK_REG(s, WIZS_DIPR1));
  addr[2] = IINCHIP_READ(WIZ_SOCK_REG(s, WIZS_DIPR2));
  addr[3] = IINCHIP_READ(WIZ_SOCK_REG(s, WIZS_DIPR3));
}
void getDPORT(uint8 s, uint16 *port)
{
  *port = IINCHIP_READ(WIZ_SOCK_REG(s, WIZS_DPORT0)) << 8;
  *port += IINCHIP_READ(WIZ_SOCK_REG(s, WIZS_DPORT1));
}

void setMR(uint8 val)
{
  IINCHIP_WRITE(WIZC_MR,val);
}

/*
@brief  This function gets Interrupt register in common register.
 */
uint8 getIR(void)
{
   return IINCHIP_READ(WIZC_IR);
}


/*
 Retransmittion 
 */
 
/*
@brief  This function sets up Retransmission time.

If there is no response from the peer or delay in response then retransmission 
will be there as per RTR (Retry Time-value Register)setting
*/
void setRTR(uint16 timeout)
{
  IINCHIP_WRITE(WIZC_RTR0, (uint8)((timeout & 0xff00) >> 8));
  IINCHIP_WRITE(WIZC_RTR1, (uint8)(timeout & 0x00ff));
}

/*
@brief  This function set the number of Retransmission.

If there is no response from the peer or delay in response then recorded time 
as per RTR & RCR register seeting then time out will occur.
*/
void setRCR(uint8 retry)
{
  IINCHIP_WRITE(WIZC_RCR,retry);
}




/*
@brief  This function set the interrupt mask Enable/Disable appropriate Interrupt. ('1' : interrupt enable)

If any bit in IMR is set as '0' then there is not interrupt signal though the bit is
set in IR register.
*/
void setIMR(uint8 mask)
{
  IINCHIP_WRITE(WIZC_IMR,mask); // must be setted 0x10.
}

/*
@brief  This sets the maximum segment size of TCP in Active Mode), while in Passive Mode this is set by peer
*/
void setSn_MSS(uint8 s, uint16 Sn_MSSR0)
{
  IINCHIP_WRITE(Sn_MSSR0(s),(uint8)((Sn_MSSR0 & 0xff00) >> 8));
  IINCHIP_WRITE((Sn_MSSR0(s) + 1),(uint8)(Sn_MSSR0 & 0x00ff));
}

void setSn_TTL(uint8 s, uint8 ttl)
{
   IINCHIP_WRITE(Sn_TTL(s), ttl);
}


/*
@brief  These below function is used to setup the Protocol Field of IP Header when
    executing the IP Layer RAW mode.
*/
void setSn_PROTO(uint8 s, uint8 proto)
{
  IINCHIP_WRITE(Sn_PROTO(s),proto);
}


/*
@brief  get socket interrupt status

These below functions are used to read the Interrupt & Soket Status register
*/
uint8 getSn_IR(uint8 s)
{
   return IINCHIP_READ(Sn_IR(s));
}


/*
@brief   get socket status
*/
uint8 getSn_SR(uint8 s)
{
   return IINCHIP_READ(Sn_SR(s));
}


/*
@brief  get socket TX free buf size

This gives free buffer size of transmit buffer. This is the data size that user can transmit.
User shuold check this value first and control the size of transmitting data
*/
uint16 getSn_TX_FSR(uint8 s)
{
  uint16 val=0,val1=0;
  do
  {
    val1 = IINCHIP_READ(Sn_TX_FSR0(s));
    val1 = (val1 << 8) + IINCHIP_READ(Sn_TX_FSR0(s) + 1);
      if (val1 != 0)
    {
        val = IINCHIP_READ(Sn_TX_FSR0(s));
        val = (val << 8) + IINCHIP_READ(Sn_TX_FSR0(s) + 1);
    }
  } while (val != val1);
   return val;
}


/*
@brief   get socket RX recv buf size

This gives size of received data in receive buffer. 
*/
uint16 getSn_RX_RSR(uint8 s)
{
  uint16 val=0,val1=0;
  do
  {
    val1 = IINCHIP_READ(Sn_RX_RSR0(s));
    val1 = (val1 << 8) + IINCHIP_READ(Sn_RX_RSR0(s) + 1);
      if(val1 != 0)
    {
        val = IINCHIP_READ(Sn_RX_RSR0(s));
        val = (val << 8) + IINCHIP_READ(Sn_RX_RSR0(s) + 1);
    }
  } while (val != val1);
   return val;
}


/*
@brief  This function is being called by send() and sendto() function also. for copy the data form application buffer to Transmite buffer of the chip.

This function read the Tx write pointer register and after copy the data in buffer update the Tx write pointer
register. User should read upper byte first and lower byte later to get proper value.
And this function is being used for copy the data form application buffer to Transmite
buffer of the chip. It calculate the actual physical address where one has to write
the data in transmite buffer. Here also take care of the condition while it exceed
the Tx memory uper-bound of socket.

*/
void send_data_processing(uint8 s, uint8 *data, uint16 len)
{
  
  uint16 ptr;
  uint16 size;
  uint16 dst_mask;
  uint8 * dst_ptr;

  ptr = IINCHIP_READ(Sn_TX_WR0(s));
  ptr = (ptr << 8) + IINCHIP_READ(Sn_TX_WR0(s) + 1);

  dst_mask = (uint32)ptr & getIINCHIP_TxMASK(s);
  dst_ptr = (uint8 *)(getIINCHIP_TxBASE(s) + dst_mask);
  
  if (dst_mask + len > getIINCHIP_TxMAX(s)) 
  {
    size = getIINCHIP_TxMAX(s) - dst_mask;
    IINCHIP_WRITE_BLOCK((uint32)dst_ptr, (uint8*)data, size);
    data += size;
    size = len - size;
    dst_ptr = (uint8 *)(getIINCHIP_TxBASE(s));
    IINCHIP_WRITE_BLOCK((uint32)dst_ptr, (uint8*)data, size);
  } 
  else
  {
    IINCHIP_WRITE_BLOCK((uint32)dst_ptr, (uint8*)data, len);
  }

  ptr += len;

  IINCHIP_WRITE(Sn_TX_WR0(s),(uint8)((ptr & 0xff00) >> 8));
  IINCHIP_WRITE((Sn_TX_WR0(s) + 1),(uint8)(ptr & 0x00ff));
  
}


/*
@brief  This function is being called by recv() also. This function is being used for copy the data form Receive buffer of the chip to application buffer.

This function read the Rx read pointer register
and after copy the data from receive buffer update the Rx write pointer register.
User should read upper byte first and lower byte later to get proper value.
It calculate the actual physical address where one has to read
the data from Receive buffer. Here also take care of the condition while it exceed
the Rx memory uper-bound of socket.
*/
void recv_data_processing(uint8 s, uint8 *data, uint16 len)
{
  uint16 ptr;
  uint16 size;
  uint16 src_mask;
  uint8 * src_ptr;

  ptr = IINCHIP_READ(Sn_RX_RD0(s));
  ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ(Sn_RX_RD0(s) + 1);
  
  DBGA("ISR_RX: rd_ptr : %.4x", ptr);

  src_mask = (uint32)ptr & getIINCHIP_RxMASK(s);
  src_ptr = (uint8 *)(getIINCHIP_RxBASE(s) + src_mask);
  
  if( (src_mask + len) > getIINCHIP_RxMAX(s) ) 
  {
    size = getIINCHIP_RxMAX(s) - src_mask;
    IINCHIP_READ_BLOCK((uint32)src_ptr, (uint8*)data,size);
    data += size;
    size = len - size;
    src_ptr = (uint8 *)(getIINCHIP_RxBASE(s));
    IINCHIP_READ_BLOCK((uint32)src_ptr, (uint8*) data,size);
  } 
  else
  {
    IINCHIP_READ_BLOCK((uint32)src_ptr, (uint8*) data,len);
  }
    
  ptr += len;
  IINCHIP_WRITE(Sn_RX_RD0(s),(uint8)((ptr & 0xff00) >> 8));
  IINCHIP_WRITE((Sn_RX_RD0(s) + 1),(uint8)(ptr & 0x00ff));
}

void recv_data_ignore(uint8 s, uint16 len)
{
  uint16 ptr;

  ptr = IINCHIP_READ(Sn_RX_RD0(s));
  ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ(Sn_RX_RD0(s) + 1);
  ptr += len;
  IINCHIP_WRITE(Sn_RX_RD0(s),(uint8)((ptr & 0xff00) >> 8));
  IINCHIP_WRITE((Sn_RX_RD0(s) + 1),(uint8)(ptr & 0x00ff));
}




// ToDo: Check & Remove ???

/*
@brief	Output destination IP address of appropriate channel
@return 	32bit destination address (Host Ordering)
*/ 
uint32 GetDestAddr(
	uint8 s	//< Channel number which try to get destination IP Address */
	)
{
	uint32 addr=0;
	int32 i = 0;
	for(i=0; i < 4; i++)
	{
		addr <<=8;
		addr += IINCHIP_READ(Sn_DIPR0(s)+i);
	}
	return addr;
}

/*
@brief	Output destination port number of appropriate channel
@return 	16bit destination port number
*/ 
uint32 GetDestPort(
	uint8 s	//< Channel number which try to get destination port */
	)
{
	uint16 port;
	port = ((uint16) IINCHIP_READ(Sn_DPORT0(s))) & 0x00FF;
	port <<= 8;
	port += ((uint16) IINCHIP_READ(Sn_DPORT0(s)+1)) & 0x00FF;
	return port;
}

uint8 CheckDestInLocal(uint32 destip)
{
	int32 i = 0;
	uint8 * pdestip = (uint8*)&destip;
	for(i =0; i < 4; i++)
	{
		if((pdestip[i] & IINCHIP_READ(WIZC_SUBR0+i)) != (IINCHIP_READ(WIZC_SIPR0+i) & IINCHIP_READ(WIZC_SUBR0+i)))
			return 1;	// Remote
	}
	return 0;
}

/*
@brief	Get handle of socket which status is same to 'status'
@return 	socket number
*/ 
uint8 getSocket(
	uint8 status, 	//< socket's status to be found */
	uint8 start			//< base of socket to be found */
	)
{
	uint8 i;
	if(start > 3) start = 0;

	for(i = start; i < TOTAL_SOCK_NUM ; i++) if( getSn_SR(i)==status ) return i;
	return TOTAL_SOCK_NUM;	
}



