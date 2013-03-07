/**
 * @file		w5200_evb.h
 * @brief		W5200 Evaluation Board Header File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef _W5200_EVB
#define _W5200_EVB

// GPIO PIN Definition - Common

// Port
#define PORTA				GPIOA			///< GPIO Group A
#define PORTB				GPIOB			///< GPIO Group B
#define PORTC				GPIOC			///< GPIO Group C
// Pin
#define PIN0				GPIO_Pin_0		///< GPIO Pin 0
#define PIN1				GPIO_Pin_1		///< GPIO Pin 1
#define PIN2				GPIO_Pin_2		///< GPIO Pin 2
#define PIN3				GPIO_Pin_3		///< GPIO Pin 3
#define PIN4				GPIO_Pin_4		///< GPIO Pin 4
#define PIN5				GPIO_Pin_5		///< GPIO Pin 5
#define PIN6				GPIO_Pin_6		///< GPIO Pin 6
#define PIN7				GPIO_Pin_7		///< GPIO Pin 7
#define PIN8				GPIO_Pin_8		///< GPIO Pin 8
#define PIN9				GPIO_Pin_9		///< GPIO Pin 9
#define PIN10				GPIO_Pin_10		///< GPIO Pin 10
#define PIN11				GPIO_Pin_11		///< GPIO Pin 11
#define PIN12				GPIO_Pin_12		///< GPIO Pin 12
#define PIN13				GPIO_Pin_13		///< GPIO Pin 13
#define PIN14				GPIO_Pin_14		///< GPIO Pin 14
#define PIN15				GPIO_Pin_15		///< GPIO Pin 15

// GPIO PIN Definition - Platform Specific

#define LED1_PORT			GPIOA			///< 1st LED Port
#define LED1_PIN			GPIO_Pin_0		///< 1st LED Pin
#define LED2_PORT			GPIOA			///< 2nd LED Port
#define LED2_PIN			GPIO_Pin_1		///< 2nd LED Pin
//#define LED3_PORT			
//#define LED3_PIN			
//#define LED4_PORT			
//#define LED5_PIN			

#define USART1_TX_PORT		GPIOA			///< 1st USART Tx Port
#define USART1_TX_PIN		GPIO_Pin_9		///< 1st USART Tx Pin
#define USART1_RX_PORT		GPIOA			///< 1st USART Rx Port
#define USART1_RX_PIN		GPIO_Pin_10		///< 1st USART Rx Pin
//#define USART1_CTS_PORT		GPIOA
//#define USART1_CTS_PIN		GPIO_Pin_11
//#define USART1_RTS_PORT		GPIOA
//#define USART1_RTS_PIN		GPIO_Pin_12

#define USART2_TX_PORT		GPIOA			///< 2nd USART Tx Port
#define USART2_TX_PIN		GPIO_Pin_2		///< 2nd USART Tx Pin
#define USART2_RX_PORT		GPIOA			///< 2nd USART RX Port
#define USART2_RX_PIN		GPIO_Pin_3		///< 2nd USART RX Pin
//#define USART2_CTS_PORT		GPIOA
//#define USART2_CTS_PIN		GPIO_Pin_0
//#define USART2_RTS_PORT		GPIOA
//#define USART2_RTS_PIN		GPIO_Pin_1

#define SPI1_SCS_PORT		GPIOA			///< 1st SPI SCS Port
#define SPI1_SCS_PIN		GPIO_Pin_4		///< 1st SPI SCS Pin
#define SPI1_SCLK_PORT		GPIOA			///< 1st SPI SCLK Port
#define SPI1_SCLK_PIN		GPIO_Pin_5		///< 1st SPI SCLK Pin
#define SPI1_MISO_PORT		GPIOA			///< 1st SPI MISO Port
#define SPI1_MISO_PIN		GPIO_Pin_6		///< 1st SPI MISO Pin
#define SPI1_MOSI_PORT		GPIOA			///< 1st SPI MOSI Port
#define SPI1_MOSI_PIN		GPIO_Pin_7		///< 1st SPI MOSI Pin

#define SPI2_SCS_PORT		GPIOB			///< 2nd SPI SCS Port
#define SPI2_SCS_PIN		GPIO_Pin_12		///< 2nd SPI SCS Pin
#define SPI2_SCLK_PORT		GPIOB			///< 2nd SPI SCLK Port
#define SPI2_SCLK_PIN		GPIO_Pin_13		///< 2nd SPI SCLK Pin
#define SPI2_MISO_PORT		GPIOB			///< 2nd SPI MISO Port
#define SPI2_MISO_PIN		GPIO_Pin_14		///< 2nd SPI MISO Pin
#define SPI2_MOSI_PORT		GPIOB			///< 2nd SPI MOSI Port
#define SPI2_MOSI_PIN		GPIO_Pin_15		///< 2nd SPI MOSI Pin

#define WIZ_INT_PORT		GPIOB			///< Network Device Interrupt Port
#define WIZ_INT_PIN			GPIO_Pin_0		///< Network Device Interrupt Pin
#define WIZ_RESET_PORT		GPIOB			///< Network Device Reset Port
#define WIZ_RESET_PIN		GPIO_Pin_8		///< Network Device Reset Pin
#define WIZ_PWDN_PORT		GPIOB			///< Network Device PowerDown Port
#define WIZ_PWDN_PIN		GPIO_Pin_9		///< Network Device PowerDown Pin

#endif //_W5200_EVB



