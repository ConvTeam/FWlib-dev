/**
 * @file		wizplatform.h
 * @brief		Platform Utility Common Header File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef	_WIZPLATFORM_H
#define	_WIZPLATFORM_H

//#include "common/common.h"


/**
 * @ingroup usart_module
 * Indicate the USART index number
 */
typedef enum {
	WIZ_USART1,	///< Indicate the 1st USART
	WIZ_USART2,	///< Indicate the 2nd USART
	WIZ_USART3	///< Indicate the 3rd USART
} wizpf_usart;

/**
 * @ingroup platform_util_module
 * Indicate the LED index number
 */
typedef enum {
	WIZ_LED1,	///< Indicate the 1st LED
	WIZ_LED2,	///< Indicate the 2nd LED
	WIZ_LED3,	///< Indicate the 3rd LED
	WIZ_LED4	///< Indicate the 4th LED
} wizpf_led;

/**
 * @ingroup gpio_module
 * Indicate the GPIO mode
 */
typedef enum {
	WIZ_GPIO_IN_FLOAT,		///< Indicate Floating Input
	WIZ_GPIO_IN_PULLUP,		///< Indicate Pulled up Input
	WIZ_GPIO_IN_PULLDOWN,	///< Indicate Pulled down Input
	WIZ_GPIO_OUT_PUSHPULL,	///< Indicate Push-Pull Output
	WIZ_GPIO_OUT_OPENDRAIN,	///< Indicate Open-Drain Output
} wizpf_gpio_mode;

/**
 * @def PUTCHAR
 * @ingroup usart_module
 * Define the compiler independent function which put character.
 * @def GETCHAR
 * @ingroup usart_module
 * Define the compiler independent function which get character.
 */
#ifdef COMPILER_IAR_EWARM
#define PUTCHAR(ch_v)		putchar(ch_v)
#define GETCHAR()			getchar()
#elif COMPILER_GCC_ARM
#define PUTCHAR(ch_v)		// ToDo
#define GETCHAR()			// ToDo
#else // for doxygen
#define PUTCHAR(ch_v)
#define GETCHAR()
#endif

/**
 * @def wizpf_led_flicker
 * @ingroup platform_util_module
 * Flicker a LED for debug with some interval.
 * @param led_v LED Index number (@ref wizpf_led)
 * @param interval_v Interval time (ms)
 */
#define wizpf_led_flicker(led_v, interval_v) do { \
	static uint32 tick = 0; \
	if(wizpf_tick_elapse(tick) > interval_v) { \
		wizpf_led_set(led_v, VAL_TOG); \
		tick = wizpf_get_systick(); \
	} \
} while(0)


int8 platform_init(void);
int8 wizpf_uart_init(wizpf_usart usart);
int8 wizpf_gpio_init(GPIO_TypeDef* GPIOx, uint16 GPIO_Pin, wizpf_gpio_mode mode);
int8 wizpf_gpio_set(GPIO_TypeDef* GPIOx, uint16 GPIO_Pin, int8 value);
int8 wizpf_gpio_get(GPIO_TypeDef* GPIOx, uint16 GPIO_Pin);
int8 wizpf_timer_init(void);
uint32 wizpf_get_systick(void);
uint32 wizpf_tick_conv(bool istick2sec, uint32 tickorsec);
int32 wizpf_tick_elapse(uint32 tick);
int8 wizpf_led_set(wizpf_led led, uint8 action);
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



