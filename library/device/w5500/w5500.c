/**
 * @file		w5500.c
 * @brief		W5500 HAL Source File.
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

/*
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "W5500\spi2.h"   
#include "W5500\w5500.h"
#include "W5500\socket.h"

#ifdef __DEF_IINCHIP_PPP__
   #include "md5.h"
#endif

//#define CHIP_DEBUG
*/

//static uint8 I_STATUS[TOTAL_SOCK_NUM];
//static uint16 RMASK[MAX_SOCK_NUM]; //< Variable for Rx buffer MASK in each channel */ 
//static uint16 SSIZE[TOTAL_SOCK_NUM]; //< Max Tx buffer size by each channel */
//static uint16 RSIZE[TOTAL_SOCK_NUM]; //< Max Rx buffer size by each channel */
//static uint16 SBUFBASEADDRESS[MAX_SOCK_NUM]; //< Tx buffer base address by each channel */ 
//static uint16 RBUFBASEADDRESS[MAX_SOCK_NUM]; //< Rx buffer base address by each channel */ 


uint8 I_STATUS[TOTAL_SOCK_NUM];
//static uint16 RMASK[MAX_SOCK_NUM]; //< Variable for Rx buffer MASK in each channel */ 
uint16 SSIZE[TOTAL_SOCK_NUM]; //< Max Tx buffer size by each channel */
uint16 RSIZE[TOTAL_SOCK_NUM]; //< Max Rx buffer size by each channel */
//static uint16 SBUFBASEADDRESS[MAX_SOCK_NUM]; //< Tx buffer base address by each channel */ 
//static uint16 RBUFBASEADDRESS[MAX_SOCK_NUM]; //< Rx buffer base address by each channel */ 

uint8 windowfull_retry_cnt[TOTAL_SOCK_NUM];

uint8 incr_windowfull_retry_cnt(uint8 s)
{
	return ++windowfull_retry_cnt[s];
}

void init_windowfull_retry_cnt(uint8 s)
{
	windowfull_retry_cnt[s] = 0;
}

uint8 getSn_TSR(uint8 s)
{
   return IINCHIP_READ_SOCKETREG(s, WIZS_TSR);
}

uint8 getISR(uint8 s)
{
  return I_STATUS[s];
}

void putISR(uint8 s, uint8 val)
{
   I_STATUS[s] = val;
}

uint16 getIINCHIP_RxMAX(uint8 s)
{
   return RSIZE[s];
}

uint16 getIINCHIP_TxMAX(uint8 s)
{
   return SSIZE[s];
}
//uint16 getIINCHIP_RxMASK(uint8 s)
//{
//   return RMASK[s];
//}
//uint16 getIINCHIP_TxMASK(uint8 s)
//{
//   return SMASK[s];
//}
//uint16 getIINCHIP_RxBASE(uint8 s)
//{
//   return RBUFBASEADDRESS[s];
//}
//uint16 getIINCHIP_TxBASE(uint8 s)
//{
//   return SBUFBASEADDRESS[s];
//}
void IINCHIP_CSoff(void)
{
  //WIZ_CS(LOW);
  //WIZ_CS2(LOW);
  wizspi_cs(WIZ_SPI2, VAL_LOW);
}
void IINCHIP_CSon(void)
{
  //WIZ_CS(HIGH);
  //WIZ_CS2(HIGH);
  wizspi_cs(WIZ_SPI2, VAL_HIGH);
}
uint8  IINCHIP_SpiSendData(uint8 dat)
{
  //return(SPI1_SendByte(dat));
  //return(SPI2_SendByte(dat));
  return(wizspi_byte(WIZ_SPI2, dat));
}


uint8 g_rx_wr_cntl_bits = (uint8)((3&0x07)<<5) | CB_TAIL_RXBUF_WR;
uint8 g_rx_rd_cntl_bits = (uint8)((3%0x07)<<5) | CB_TAIL_RXBUF_RD;
    
void IINCHIP_RXBUF_WRRD(uint16 addr, uint8 data)
{
   uint8 rdata, redata;
   IINCHIP_ISR_DISABLE();                      // Interrupt Service Routine Disable
   
   //SPI MODE I/F
   IINCHIP_CSoff();                            // CS=0, SPI start
   IINCHIP_SpiSendData((addr & 0xFF00) >> 8);  // Address byte 1
   IINCHIP_SpiSendData(addr & 0x00FF);         // Address byte 2
   IINCHIP_SpiSendData(g_rx_wr_cntl_bits);             // Data write command and Write data length 1
   IINCHIP_SpiSendData(data);                  // Data write (write 1byte data)
   IINCHIP_CSon();                             // CS=1,  SPI end
   
   IINCHIP_CSoff();                             // CS=0, SPI start 
   IINCHIP_SpiSendData((addr & 0xFF00) >> 8);   // Address byte 1
   IINCHIP_SpiSendData(addr & 0x00FF);          // Address byte 2
   IINCHIP_SpiSendData(g_rx_rd_cntl_bits);              // Data read command and Read data length 1
   rdata = IINCHIP_SpiSendData(0x00);            // Data read (read 1byte data) 
   IINCHIP_CSon();                             // CS=1,  SPI end
   
   IINCHIP_ISR_ENABLE();                       // Interrupt Service Routine Enable
   
   if( rdata != data )
   {
      printf("addr:%X, data:%X, rdata:%X, ", addr, data, rdata);
      IINCHIP_ISR_DISABLE();                      // Interrupt Service Routine Disable
      IINCHIP_CSoff();                             // CS=0, SPI start 
      IINCHIP_SpiSendData((addr & 0xFF00) >> 8);   // Address byte 1
      IINCHIP_SpiSendData(addr & 0x00FF);          // Address byte 2
      IINCHIP_SpiSendData(g_rx_rd_cntl_bits);              // Data read command and Read data length 1
      redata = IINCHIP_SpiSendData(0x00);            // Data read (read 1byte data) 
      IINCHIP_CSon();                             // CS=1,  SPI end      
      IINCHIP_ISR_ENABLE();                       // Interrupt Service Routine Enable      
      printf("redata:%X \r\n", redata);
   }
}

 /*
@brief  This function writes the data into W5200 registers.
*/
void IINCHIP_WRITE(uint16 addr,  uint8 cntl_bits, uint8 data)
{
   IINCHIP_ISR_DISABLE();                      // Interrupt Service Routine Disable
   //SPI MODE I/F
   IINCHIP_CSoff();                            // CS=0, SPI start
   IINCHIP_SpiSendData((addr & 0xFF00) >> 8);  // Address byte 1
   IINCHIP_SpiSendData(addr & 0x00FF);         // Address byte 2
   IINCHIP_SpiSendData(cntl_bits);             // Data write command and Write data length 1
   IINCHIP_SpiSendData(data);                  // Data write (write 1byte data)
   IINCHIP_CSon();                             // CS=1,  SPI end
   IINCHIP_ISR_ENABLE();                       // Interrupt Service Routine Enable
}
/*
@brief  This function reads the value from W5200 registers.
*/
uint8 IINCHIP_READ(uint16 addr, uint8 cntl_bits)
{
   uint8 data = 0;     
   IINCHIP_ISR_DISABLE();                       // Interrupt Service Routine Disable
   IINCHIP_CSoff();                             // CS=0, SPI start 
   IINCHIP_SpiSendData((addr & 0xFF00) >> 8);   // Address byte 1
   IINCHIP_SpiSendData(addr & 0x00FF);          // Address byte 2
   IINCHIP_SpiSendData(cntl_bits);              // Data read command and Read data length 1
   data = IINCHIP_SpiSendData(0x00);            // Data read (read 1byte data) 
   IINCHIP_CSon();                              // CS=1,  SPI end
   IINCHIP_ISR_ENABLE();                        // Interrupt Service Routine Enable
   return data;
}

void IINCHIP_WRITE_SEQ(uint16 addr, uint8 cntl_bits, uint16 len,  uint8 * data)
{
   uint16 idx = 0;
#ifdef CHIP_DEBUG  
  uint16 count = 0;
#endif     

#if 0
  if(len == 0)
  {
    printf("Unexpected1 length 0\r\n"); 
  }   
#endif
  
   IINCHIP_ISR_DISABLE();     
   //SPI MODE I/F
   IINCHIP_CSoff();                                        // CS=0, SPI start  
   IINCHIP_SpiSendData((addr & 0xFF00) >> 8);        // Address byte 1
   IINCHIP_SpiSendData(addr & 0x00FF);               // Address byte 2 
   IINCHIP_SpiSendData(cntl_bits);                        // Control bit  
   for(idx = 0; idx < len; idx++)                          // Write data in loop
   {   
     IINCHIP_SpiSendData(data[idx]);
   }
   IINCHIP_CSon();                                         // CS=1, SPI end         
   IINCHIP_ISR_ENABLE();                                   // Interrupt Service Routine Enable        

#ifdef CHIP_DEBUG     
   for(idx=0; idx< len; idx++)
   {
    if(data[idx] == 0)
      count++;
    if(count > 3)
    {
      printf("SEQ read pattern Error");
      while(1);
    }
   }
#endif
   
}

void IINCHIP_READ_SEQ(uint16 addr, uint8 cntl_bits, uint16 len, uint8 * data)
{
  uint16 idx = 0;
  
#ifdef CHIP_DEBUG  
  uint16 count = 0;
#endif  
    
  if(len == 0)
  {
    printf("Unexpected2 length 0\r\n"); 
  } 
  
  //printf("IINCHIP_READ_SEQ->addr: %X, len : %d \r\n", addr, len);
  IINCHIP_ISR_DISABLE();
  //SPI MODE I/F
  IINCHIP_CSoff();                                        // CS=0, SPI start  
  IINCHIP_SpiSendData((addr & 0xFF00) >> 8);        // Address byte 1
  IINCHIP_SpiSendData(addr & 0x00FF);               // Address byte 2
  IINCHIP_SpiSendData(cntl_bits);                         // Control bit
  for(idx = 0; idx < len; idx++)                          // Write data in loop
  {   
    data[idx] = IINCHIP_SpiSendData(0x00);   
  }
  IINCHIP_CSon();                                         // CS=1, SPI end         
  IINCHIP_ISR_ENABLE();                                   // Interrupt Service Routine Enable         
}


void IINCHIP_WRITE_COMMON( uint16 addr,  uint8 data)
{
  uint8 cntl_bits = 0;
  cntl_bits = CB_TAIL_COMREG_WR;
  IINCHIP_WRITE(addr,  cntl_bits, data);  
}

uint8 IINCHIP_READ_COMMON(uint16 addr)
{
  uint8 data = 0;
  uint8 cntl_bits = 0;
    cntl_bits = CB_TAIL_COMREG_RD;
  data = IINCHIP_READ(addr,  cntl_bits);  
  return data;
}


void IINCHIP_WRITE_DM( uint16 addr,  uint8 data)
{
  uint8 cntl_bits = 0;
  cntl_bits = CM_MEM_DM;
  IINCHIP_WRITE(addr,  cntl_bits, data);  
}

uint8 IINCHIP_READ_DM(uint16 addr)
{
  uint8 data = 0;
  uint8 cntl_bits = 0;
  cntl_bits = CM_MEM_DM;
  data = IINCHIP_READ(addr,  cntl_bits);  
  return data;
}

void IINCHIP_READ_COMMON_SEQ(uint16 addr, uint8 len, uint8 * data)
{
  uint8 cntl_bits = 0;
  cntl_bits = (uint8)CB_TAIL_COMMONREG_RD_SEQ;     
  IINCHIP_READ_SEQ(addr,  cntl_bits, len, (uint8 *)data);
}

void IINCHIP_WRITE_COMMON_SEQ(uint16 addr, uint8 len, uint8 * data)
{  
   uint8 cntl_bits = 0;
   cntl_bits = (uint8)CB_TAIL_COMMONREG_WR_SEQ;     
   IINCHIP_WRITE_SEQ(addr,  cntl_bits, len, (uint8 *)data);
}

void IINCHIP_WRITE_SOCKETREG(uint8 sock_num, uint16 addr,  uint8 data)
{
  uint8 cntl_bits = 0;
  cntl_bits = (uint8)((sock_num&0x07)<<5) | CB_TAIL_SOCKETREG_WR;
  //printf("cntl_bits %X \r\n", cntl_bits);
  IINCHIP_WRITE(addr,  cntl_bits, data);
}

uint8 IINCHIP_READ_SOCKETREG(uint8 sock_num, uint16 addr)
{
  uint8 data = 0;
  uint8 cntl_bits = 0;
  cntl_bits = (uint8)((sock_num&0x07)<<5) | CB_TAIL_SOCKETREG_RD;
  //printf("cntl_bits %X \r\n", cntl_bits); 
  data = IINCHIP_READ(addr,  cntl_bits);  
  return data;
}


void IINCHIP_READ_SOCKETREG_SEQ(uint8 sock_num,  uint16 addr, uint8 len, uint8 * data)
{
  uint8 cntl_bits = 0;
  cntl_bits = (uint8)((sock_num&0x07)<<5) | CB_TAIL_SOCKETREG_RD_SEQ;
  IINCHIP_READ_SEQ(addr,  cntl_bits, len, (uint8 *)data);
}

void IINCHIP_WRITE_SOCKETREG_SEQ(uint8 sock_num, uint16 addr, uint8 len, uint8 * data)
{
   uint8 cntl_bits = 0;
   cntl_bits = (uint8)((sock_num&0x07)<<5) | CB_TAIL_SOCKETREG_WR_SEQ;     

  IINCHIP_WRITE_SEQ(addr,  cntl_bits, len, (uint8 *)data);  
}
                           
void IINCHIP_WRITE_TXBUF(uint8 sock_num, uint16 addr, uint8 data)
{
  uint8 cntl_bits = 0;
  cntl_bits = (uint8)((sock_num&0x07)<<5) | CB_TAIL_TXBUF_WR;
  IINCHIP_WRITE(addr,  cntl_bits, data);  
}

uint8 IINCHIP_READ_TXBUF(uint8 sock_num, uint16 addr)
{
    uint8 cntl_bits = 0;
    uint8 data = 0;
  cntl_bits = (uint8)((sock_num&0x07)<<5) | CB_TAIL_TXBUF_RD;
  data = IINCHIP_READ(addr,  cntl_bits);  
  return data; 
}

void IINCHIP_WRITE_RXBUF(uint8 sock_num, uint16 addr, uint8 data)
{
    uint8 cntl_bits = 0;
    cntl_bits = (uint8)((sock_num&0x07)<<5) | CB_TAIL_RXBUF_WR;
    IINCHIP_WRITE(addr,  cntl_bits, data);  
}

uint8 IINCHIP_READ_RXBUF(uint8 sock_num, uint16 addr)
{
  uint8 cntl_bits = 0;
    uint8 data = 0;
  cntl_bits = (uint8)((sock_num&0x07)<<5) | CB_TAIL_RXBUF_RD;
  data = IINCHIP_READ(addr,  cntl_bits);  
  return data; 
}

void IINCHIP_WRITE_TXBUF_SEQ(uint8 sock_num, uint16 addr, uint16 len, uint8 * data)
{
  uint8 cntl_bits = 0;
  cntl_bits = (uint8)((sock_num&0x07)<<5) | CB_TAIL_TXBUF_WR_SEQ;     
  IINCHIP_WRITE_SEQ(addr,  cntl_bits, len, (uint8 *)data);      
}

void IINCHIP_READ_TXBUF_SEQ(uint8 sock_num, uint16 addr, uint16 len, uint8 * data)
{
  uint8 cntl_bits = 0;
  cntl_bits = (uint8)((sock_num&0x07)<<5) | CB_TAIL_TXBUF_RD_SEQ;     
  IINCHIP_READ_SEQ(addr,  cntl_bits, len, data);      
}

void IINCHIP_WRITE_RXBUF_SEQ(uint8 sock_num, uint16 addr, uint16 len, uint8 * data)
{
  uint8 cntl_bits = 0;
  cntl_bits = (uint8)((sock_num&0x07)<<5) | CB_TAIL_RXBUF_WR_SEQ;     
  IINCHIP_WRITE_SEQ(addr,  cntl_bits, len, data);     
}

void IINCHIP_READ_RXBUF_SEQ(uint8 sock_num, uint16 addr, uint16 len, uint8 * data)
{
    uint8 cntl_bits = 0;
    cntl_bits = (uint8)((sock_num&0x07)<<5) | CB_TAIL_RXBUF_RD_SEQ;     
  IINCHIP_READ_SEQ(addr,  cntl_bits, len, data);      
}



// added

/*
@brief  This function sets up gateway IP address.
*/ 
void setGAR(
  uint8 * addr  //< a pointer to a 4 -byte array responsible to set the Gateway IP address. */
  )
{
  IINCHIP_WRITE_COMMON( (WIZC_GAR0 + 0), addr[0]);
  //printf("%d -- rd: %X \r\n", 0, IINCHIP_READ_COMMON( (WIZC_GAR0 + 0) ) );
  //IINCHIP_WRITE_COMMON( (WIZC_GAR0 + 0), 0x01);
  IINCHIP_WRITE_COMMON( (WIZC_GAR0 + 1), addr[1]);
  //printf("%d -- rd: %X \r\n", 1, IINCHIP_READ_COMMON( (WIZC_GAR0 + 1) ));
  //IINCHIP_WRITE_COMMON( (WIZC_GAR0 + 1), 0x02);
  IINCHIP_WRITE_COMMON( (WIZC_GAR0 + 2), addr[2]);
  //printf("%d -- rd: %X \r\n", 2, IINCHIP_READ_COMMON( (WIZC_GAR0 + 2) ));
  //IINCHIP_WRITE_COMMON( (WIZC_GAR0 + 2), 0x03);
  IINCHIP_WRITE_COMMON( (WIZC_GAR0 + 3), addr[3]);
  //printf("%d -- rd: %X \r\n", 3, IINCHIP_READ_COMMON( (WIZC_GAR0 + 3) ));
  //IINCHIP_WRITE_COMMON( (WIZC_GAR0 + 3), 0x04);
  
#if 0
    uint16 j =0;
    printf("COMMON REG\r\n");
    for(j=1; j<7; j++)
    {
      printf("%d -- rd: %X ", j, IINCHIP_READ_COMMON( j ) );
      IINCHIP_WRITE_COMMON( j, 0x00+j);     
      printf("wr:%X, rd:%X \r\n", j, IINCHIP_READ_COMMON( j ) );
    }  
#endif    
}

/*
void getGWIP(uint8 * addr)
{
  addr[0] = IINCHIP_READ_COMMON( (WIZC_GAR0 + 0));
  addr[1] = IINCHIP_READ_COMMON( (WIZC_GAR0 + 1));
  addr[2] = IINCHIP_READ_COMMON( (WIZC_GAR0 + 2));
  addr[3] = IINCHIP_READ_COMMON( (WIZC_GAR0 + 3));
}
*/

/*
@brief  It sets up SubnetMask address
*/ 
void setSUBR(uint8 * addr)
{   
  IINCHIP_WRITE_COMMON( (WIZC_SUBR0 + 0), addr[0]);
  IINCHIP_WRITE_COMMON( (WIZC_SUBR0 + 1), addr[1]);
  IINCHIP_WRITE_COMMON( (WIZC_SUBR0 + 2), addr[2]);
  IINCHIP_WRITE_COMMON( (WIZC_SUBR0 + 3), addr[3]);
}

/*
@brief  This function sets up MAC address.
*/ 
void setSHAR(
  uint8 * addr  //< a pointer to a 6 -byte array responsible to set the MAC address. */
  )
{
  IINCHIP_WRITE_COMMON((WIZC_SHAR0 + 0),addr[0]);
  IINCHIP_WRITE_COMMON((WIZC_SHAR0 + 1),addr[1]);
  IINCHIP_WRITE_COMMON((WIZC_SHAR0 + 2),addr[2]);
  IINCHIP_WRITE_COMMON((WIZC_SHAR0 + 3),addr[3]);
  IINCHIP_WRITE_COMMON((WIZC_SHAR0 + 4),addr[4]);
  IINCHIP_WRITE_COMMON((WIZC_SHAR0 + 5),addr[5]);
}

/*
@brief  This function sets up Source IP address.
*/
void setSIPR(
  uint8 * addr  //< a pointer to a 4 -byte array responsible to set the Source IP address. */
  )
{
  IINCHIP_WRITE_COMMON((WIZC_SIPR0 + 0),addr[0]);
  IINCHIP_WRITE_COMMON((WIZC_SIPR0 + 1),addr[1]);
  IINCHIP_WRITE_COMMON((WIZC_SIPR0 + 2),addr[2]);
  IINCHIP_WRITE_COMMON((WIZC_SIPR0 + 3),addr[3]);
}

/*
@brief  This function sets up Source IP address.
*/
void getGAR(uint8 * addr)
{
  addr[0] = IINCHIP_READ_COMMON(WIZC_GAR0);
  addr[1] = IINCHIP_READ_COMMON(WIZC_GAR0+1);
  addr[2] = IINCHIP_READ_COMMON(WIZC_GAR0+2);
  addr[3] = IINCHIP_READ_COMMON(WIZC_GAR0+3);
}
void getSUBR(uint8 * addr)
{
  addr[0] = IINCHIP_READ_COMMON(WIZC_SUBR0);
  addr[1] = IINCHIP_READ_COMMON(WIZC_SUBR0+1);
  addr[2] = IINCHIP_READ_COMMON(WIZC_SUBR0+2);
  addr[3] = IINCHIP_READ_COMMON(WIZC_SUBR0+3);
}
void getSHAR(uint8 * addr)
{
  addr[0] = IINCHIP_READ_COMMON(WIZC_SHAR0);
  addr[1] = IINCHIP_READ_COMMON(WIZC_SHAR0+1);
  addr[2] = IINCHIP_READ_COMMON(WIZC_SHAR0+2);
  addr[3] = IINCHIP_READ_COMMON(WIZC_SHAR0+3);
  addr[4] = IINCHIP_READ_COMMON(WIZC_SHAR0+4);
  addr[5] = IINCHIP_READ_COMMON(WIZC_SHAR0+5);
}
void getSIPR(uint8 * addr)
{
  addr[0] = IINCHIP_READ_COMMON(WIZC_SIPR0);
  addr[1] = IINCHIP_READ_COMMON(WIZC_SIPR0+1);
  addr[2] = IINCHIP_READ_COMMON(WIZC_SIPR0+2);
  addr[3] = IINCHIP_READ_COMMON(WIZC_SIPR0+3);
}

void getDIPR(uint8 s, uint8 *addr)
{
  addr[0] = IINCHIP_READ_SOCKETREG(s, WIZS_DIPR0 + 0);
  addr[1] = IINCHIP_READ_SOCKETREG(s, WIZS_DIPR0 + 1);
  addr[2] = IINCHIP_READ_SOCKETREG(s, WIZS_DIPR0 + 2);
  addr[3] = IINCHIP_READ_SOCKETREG(s, WIZS_DIPR0 + 3);
  //addr[0] = IINCHIP_READ(WIZ_SOCK_REG(s, WIZS_DIPR0));
  //addr[1] = IINCHIP_READ(WIZ_SOCK_REG(s, WIZS_DIPR1));
  //addr[2] = IINCHIP_READ(WIZ_SOCK_REG(s, WIZS_DIPR2));
  //addr[3] = IINCHIP_READ(WIZ_SOCK_REG(s, WIZS_DIPR3));
}

void getDPORT(uint8 s, uint16 *port)
{
  *port = IINCHIP_READ_SOCKETREG(s, WIZS_DPORT0 + 0) << 8;
  *port += IINCHIP_READ_SOCKETREG(s, WIZS_DPORT0 + 1);    
  //*port = IINCHIP_READ(WIZ_SOCK_REG(s, WIZS_DPORT0)) << 8;
  //*port += IINCHIP_READ(WIZ_SOCK_REG(s, WIZS_DPORT1));
}

void setMR(uint8 val)
{  
  IINCHIP_WRITE_COMMON(WIZC_MR, val); 
}

/*
@brief  This function gets Interrupt register in common register.
 */
uint8 getIR( void )
{
   return IINCHIP_READ_COMMON(WIZC_IR);
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
  IINCHIP_WRITE_COMMON(WIZC_RTR0,(uint8)((timeout & 0xff00) >> 8));
  IINCHIP_WRITE_COMMON((WIZC_RTR0 + 1),(uint8)(timeout & 0x00ff));
}

/*
@brief  This function set the number of Retransmission.

If there is no response from the peer or delay in response then recorded time 
as per RTR & RCR register seeting then time out will occur.
*/
void setRCR(uint8 retry)
{
  IINCHIP_WRITE_COMMON(WIZC_RCR,retry);
}

/*
@brief  This function set the interrupt mask Enable/Disable appropriate Interrupt. ('1' : interrupt enable)

If any bit in IMR is set as '0' then there is not interrupt signal though the bit is
set in IR register.
*/
void setIMR(uint8 mask)
{
  IINCHIP_WRITE_COMMON(WIZC_IMR, mask); // must be setted 0x10.
}

/*
@brief  This function set the interrupt mask Enable/Disable appropriate Interrupt. ('1' : interrupt enable)

If any bit in IMR is set as '0' then there is not interrupt signal though the bit is
set in IR register.
*/
void clearIR(uint8 mask)
{
  IINCHIP_WRITE_COMMON(WIZC_IR, ~mask | getIR() ); // must be setted 0x10.
}

/*
@brief  This sets the maximum segment size of TCP in Active Mode), while in Passive Mode this is set by peer
*/
void setSn_MSS(uint8 s, uint16 Sn_MSSR)
{
  IINCHIP_WRITE_SOCKETREG( s, WIZS_MSSR0 ,(uint8)((Sn_MSSR & 0xff00) >> 8));
  IINCHIP_WRITE_SOCKETREG( s,(WIZS_MSSR0 + 1),(uint8)(Sn_MSSR & 0x00ff));
}

void setSn_TTL(uint8 s, uint8 ttl)
{
   IINCHIP_WRITE_SOCKETREG( s, WIZS_TTL , ttl);
}

/*
@brief  get socket interrupt status

These below functions are used to read the Interrupt & Soket Status register
*/
uint8 getSn_IR(uint8 s)
{
   return IINCHIP_READ_SOCKETREG(s, WIZS_IR );
}


/*
@brief   get socket status
*/
uint8 getSn_SR(uint8 s)
{
   return IINCHIP_READ_SOCKETREG(s, WIZS_SR );
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
    val1 = IINCHIP_READ_SOCKETREG(s, WIZS_TX_FSR0 );
    val1 = (val1 << 8) + IINCHIP_READ_SOCKETREG(s, WIZS_TX_FSR0  + 1);
      if (val1 != 0)
    {
        val = IINCHIP_READ_SOCKETREG(s, WIZS_TX_FSR0 );
        val = (val << 8) + IINCHIP_READ_SOCKETREG(s, WIZS_TX_FSR0  + 1);
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
    val1 = IINCHIP_READ_SOCKETREG( s, WIZS_RX_RSR0 );
    val1 = (val1 << 8) + IINCHIP_READ_SOCKETREG( s, WIZS_RX_RSR0  + 1);
    if(val1 != 0)
    {
        val = IINCHIP_READ_SOCKETREG(s, WIZS_RX_RSR0 );
        val = (val << 8) + IINCHIP_READ_SOCKETREG(s, WIZS_RX_RSR0  + 1);
    }
  } while (val != val1);
   return val;
}


/*
@brief   This function is being called by send() and sendto() function also. 

This function read the Tx write pointer register and after copy the data in buffer update the Tx write pointer
register. User should read upper byte first and lower byte later to get proper value.
*/
void send_data_processing(uint8 s, uint8 *data, uint16 len)
{
  if(len == 0)
  {
    printf("CH: %d Unexpected1 length 0\r\n", s);
    return;
  }
  
  uint16 ptr = 0;
  ptr = IINCHIP_READ_SOCKETREG( s, WIZS_TX_WR0 );
  ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ_SOCKETREG( s, WIZS_TX_WR0  + 1);
  //printf("TX ptr : %X  ", ptr);
  IINCHIP_WRITE_TXBUF_SEQ(s, ptr, len, data);
  ptr += len;
  IINCHIP_WRITE_SOCKETREG( s,WIZS_TX_WR0 ,(uint8)((ptr & 0xff00) >> 8));
  IINCHIP_WRITE_SOCKETREG( s,(WIZS_TX_WR0  + 1),(uint8)(ptr & 0x00ff));
}


/*
@brief  This function is being called by recv() also.

This function read the Rx read pointer register
and after copy the data from receive buffer update the Rx write pointer register.
User should read upper byte first and lower byte later to get proper value.
*/
void recv_data_processing(uint8 s, uint8 *data, uint16 len)
{
  uint16 ptr = 0;
  
  if(len == 0)
  {
    printf("CH: %d Unexpected2 length 0\r\n", s);
    return;
  }
  
  ptr = IINCHIP_READ_SOCKETREG( s, WIZS_RX_RD0 );
  ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ_SOCKETREG( s, WIZS_RX_RD0  + 1);
  
  //printf("RX ptr : %X ", ptr);  
  IINCHIP_READ_RXBUF_SEQ(s, ptr,  len, data);
  ptr += len;
  //printf("RX ptr : %X \r\n", ptr);
  
  IINCHIP_WRITE_SOCKETREG( s, WIZS_RX_RD0 ,(uint8)((ptr & 0xff00) >> 8));
  IINCHIP_WRITE_SOCKETREG( s, WIZS_RX_RD0  + 1,(uint8)(ptr & 0x00ff));
}


void recv_data_ignore(uint8 s, uint16 len)
{
  uint16 ptr;

  ptr = IINCHIP_READ_SOCKETREG(s, WIZS_RX_RD0 + 0);
  ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ_SOCKETREG(s, WIZS_RX_RD0 + 1);
  ptr += len;
  IINCHIP_WRITE_SOCKETREG(s, WIZS_RX_RD0 + 0, (uint8)((ptr & 0xff00) >> 8));
  IINCHIP_WRITE_SOCKETREG(s, WIZS_RX_RD0 + 1, (uint8)(ptr & 0x00ff));
}


