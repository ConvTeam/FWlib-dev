
#ifndef _W5200_EVB
#define _W5200_EVB

// Port A
#define LED3				GPIO_Pin_0	// out
#define LED4				GPIO_Pin_1	// out

#define PA2				GPIO_Pin_2	// 
#define PA3				GPIO_Pin_3	// 

#define WIZ_SCS			GPIO_Pin_4	// out
#define WIZ_SCLK			GPIO_Pin_5	// out
#define WIZ_MISO			GPIO_Pin_6	// in
#define WIZ_MOSI			GPIO_Pin_7	// out

#define PA8				GPIO_Pin_8	// 

#define USART1_TX		GPIO_Pin_9	// out
#define USART1_RX		GPIO_Pin_10	// in 
#define USART2_TX		GPIO_Pin_2	// out
#define USART2_RX		GPIO_Pin_3	// in 

#define PA11				GPIO_Pin_11	// 
#define PA12				GPIO_Pin_12	// 
#define PA13				GPIO_Pin_13	// 
#define PA14				GPIO_Pin_14	// 
#define PA15				GPIO_Pin_15	// 

// Port B
#define WIZ_INT			GPIO_Pin_0	// in
#define PB1				GPIO_Pin_1	// 
#define PB2				GPIO_Pin_2	// 
#define PB3				GPIO_Pin_3	// 
#define PB4				GPIO_Pin_4	// 
#define PB5				GPIO_Pin_5	// 
#define PB6				GPIO_Pin_6	// 
#define PB7				GPIO_Pin_7	// 
#define WIZ_RESET		GPIO_Pin_8	// out
#define WIZ_PWDN		GPIO_Pin_9	// out
#define PB10				GPIO_Pin_10	// 
#define PB11				GPIO_Pin_11	// 
#define PB12				GPIO_Pin_12	// 
#define PB13				GPIO_Pin_13	// 
#define PB14				GPIO_Pin_14	// 
#define PB15				GPIO_Pin_15	// 

// Port C
#define PC13				GPIO_Pin_13	//
#define PC14				GPIO_Pin_14	//
#define PC15				GPIO_Pin_15	//

#define STM32F10X_MD

#endif //_W5200_EVB



