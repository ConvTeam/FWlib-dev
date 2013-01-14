
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


int8 platform_init(void);
int8 wizpf_uart_init(wizpf_usart usart);
uint32 wizpf_get_systick(void);
int32 wizpf_tick_elapse(uint32 tick);
void wizpf_led_act(wizpf_led led, uint8 action);
void wizpf_led_trap(uint8 repeat);
void device_HW_reset(void);

void GPIO_Configuration(void);
void RCC_Configuration(void);
void NVIC_Configuration(void);

void Delay_us(uint8 time_us);
void Delay_ms(uint16 time_ms);
void Delay_tick(uint32 tick);

int getchar_nonblk(void);

#endif //_WIZPLATFORM_H



