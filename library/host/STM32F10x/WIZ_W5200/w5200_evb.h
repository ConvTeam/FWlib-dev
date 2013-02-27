/**
 * @file		w5200_evb.h
 * @brief		W5200 Evaluation Board Header File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *		2013/02/22 - 1.0 Release
 * @author	Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef _W5200_EVB
#define _W5200_EVB

// GPIO PIN Definition - Common

// Port
#define PORTA				GPIOA
#define PORTB				GPIOB
#define PORTC				GPIOC
// Pin
#define PIN0				GPIO_Pin_0
#define PIN1				GPIO_Pin_1
#define PIN2				GPIO_Pin_2
#define PIN3				GPIO_Pin_3
#define PIN4				GPIO_Pin_4
#define PIN5				GPIO_Pin_5
#define PIN6				GPIO_Pin_6
#define PIN7				GPIO_Pin_7
#define PIN8				GPIO_Pin_8
#define PIN9				GPIO_Pin_9
#define PIN10				GPIO_Pin_10
#define PIN11				GPIO_Pin_11
#define PIN12				GPIO_Pin_12
#define PIN13				GPIO_Pin_13
#define PIN14				GPIO_Pin_14
#define PIN15				GPIO_Pin_15

// GPIO PIN Definition - Platform Specific

#define LED1_PORT			GPIOA
#define LED1_PIN			GPIO_Pin_0
#define LED2_PORT			GPIOA
#define LED2_PIN			GPIO_Pin_1
//#define LED3_PORT			
//#define LED3_PIN			
//#define LED4_PORT			
//#define LED5_PIN			

#define USART1_TX_PORT		GPIOA
#define USART1_TX_PIN		GPIO_Pin_9
#define USART1_RX_PORT		GPIOA
#define USART1_RX_PIN		GPIO_Pin_10
//#define USART1_CTS_PORT		GPIOA
//#define USART1_CTS_PIN		GPIO_Pin_11
//#define USART1_RTS_PORT		GPIOA
//#define USART1_RTS_PIN		GPIO_Pin_12

#define USART2_TX_PORT		GPIOA
#define USART2_TX_PIN		GPIO_Pin_2
#define USART2_RX_PORT		GPIOA
#define USART2_RX_PIN		GPIO_Pin_3
//#define USART2_CTS_PORT		GPIOA
//#define USART2_CTS_PIN		GPIO_Pin_0
//#define USART2_RTS_PORT		GPIOA
//#define USART2_RTS_PIN		GPIO_Pin_1

#define SPI1_SCS_PORT		GPIOA
#define SPI1_SCS_PIN		GPIO_Pin_4
#define SPI1_SCLK_PORT		GPIOA
#define SPI1_SCLK_PIN		GPIO_Pin_5
#define SPI1_MISO_PORT		GPIOA
#define SPI1_MISO_PIN		GPIO_Pin_6
#define SPI1_MOSI_PORT		GPIOA
#define SPI1_MOSI_PIN		GPIO_Pin_7

#define SPI2_SCS_PORT		GPIOB
#define SPI2_SCS_PIN		GPIO_Pin_12
#define SPI2_SCLK_PORT		GPIOB
#define SPI2_SCLK_PIN		GPIO_Pin_13
#define SPI2_MISO_PORT		GPIOB
#define SPI2_MISO_PIN		GPIO_Pin_14
#define SPI2_MOSI_PORT		GPIOB
#define SPI2_MOSI_PIN		GPIO_Pin_15

#define WIZ_INT_PORT		GPIOB
#define WIZ_INT_PIN			GPIO_Pin_0
#define WIZ_RESET_PORT		GPIOB
#define WIZ_RESET_PIN		GPIO_Pin_8
#define WIZ_PWDN_PORT		GPIOB
#define WIZ_PWDN_PIN		GPIO_Pin_9

#endif //_W5200_EVB


