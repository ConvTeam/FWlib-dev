/**
 * @file		wizplatform.h
 * @brief		Platform Utility Common Header File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *		2013/02/22 - 1.0 Release
 * @author	Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef	_WIZPLATFORM_H
#define	_WIZPLATFORM_H

//#include "common/common.h"

#ifdef COMPILER_IAR_EWARM
#define PUTCHAR(ch_v)		putchar(ch_v)
#define GETCHAR()			getchar()
//#define GETCHAR_NONBLK()	getchar_nonblk()
#elif COMPILER_GCC_ARM

#endif


typedef enum {
	WIZ_USART1 = 0, 
	WIZ_USART2 = 1, 
	WIZ_USART3 = 2
} wizpf_usart;

typedef enum {
	//LED1 = 0, 
	//LED2 = 1,
	WIZ_LED3 = 2,
	WIZ_LED4 = 3
} wizpf_led;

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


int8 platform_init(void);
int8 wizpf_uart_init(wizpf_usart usart);
uint32 wizpf_get_systick(void);
uint32 wizpf_tick_conv(bool istick2sec, uint32 tickorsec);
int32 wizpf_tick_elapse(uint32 tick);
void wizpf_led_set(wizpf_led led, uint8 action);
int8 wizpf_led_get(wizpf_led led);
void wizpf_led_trap(uint8 repeat);
void device_HW_reset(void);

void GPIO_Configuration(void);
void RCC_Configuration(void);
void NVIC_Configuration(void);

void Delay_us(uint8 time_us);
void Delay_ms(uint16 time_ms);
void Delay_tick(uint32 tick);

int32 getchar_nonblk(void);

#endif //_WIZPLATFORM_H



