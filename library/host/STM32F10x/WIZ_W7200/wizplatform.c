/**
 * @file		WIZ_W7200/wizplatform.c
 * @brief		Platform Specific Source File - For W7200 Evaluation Board
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

//#define FILE_LOG_SILENCE
#include "common/common.h"
//#include "host/wizplatform.h"


#define USART1_RX_INTERRUPT VAL_ENABLE
#define USART2_RX_INTERRUPT VAL_ENABLE
#define SYSTICK_HZ			1000
#define GPIO_INIT_SIMP(mode_v, port_p, pin_p) do { \
	GPIO_InitTypeDef GPIO_InitStructure; \
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; \
	GPIO_InitStructure.GPIO_Mode = mode_v; \
	GPIO_InitStructure.GPIO_Pin =  pin_p; \
	GPIO_Init(port_p, &GPIO_InitStructure); \
} while(0)

extern void EXTI_IMR_EMR_enable(void);

static __IO uint32 msTicks = 0;	// Max: about 50 days
static USART_TypeDef *std_usart = USART1;


void SysTick_Handler(void)	// SysTick ISR
{
	msTicks++;
}

#if (USART1_RX_INTERRUPT == VAL_ENABLE)
#define U1RX_BUF_SIZE		300
uint8  u1rx_buf[U1RX_BUF_SIZE];
uint16 u1rx_wr=0, u1rx_rd=0;
void USART1_IRQHandler(void)	// USART1 ISR
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		if( (u1rx_wr > u1rx_rd && u1rx_wr-u1rx_rd >= U1RX_BUF_SIZE-1) ||
			(u1rx_wr < u1rx_rd && u1rx_rd == u1rx_wr+1) )	// Buffer Overflow
		{
			//USART_ClearITPendingBit(USART1, USART_IT_RXNE);
			//wizpf_led_set(WIZ_LED1, VAL_TOG);
			USART_SendData(USART1, (uint8)'@');
			return;
		}
		u1rx_buf[u1rx_wr] = (uint8)USART_ReceiveData(USART1);
		if(u1rx_wr < U1RX_BUF_SIZE-1) u1rx_wr++;
		else u1rx_wr = 0;
	}
}
#endif

#if (USART2_RX_INTERRUPT == VAL_ENABLE)
#define U2RX_BUF_SIZE		300
uint8  u2rx_buf[U2RX_BUF_SIZE];
uint16 u2rx_wr=0, u2rx_rd=0;
void USART2_IRQHandler(void)	// USART2 ISR
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		if( (u2rx_wr > u2rx_rd && u2rx_wr-u2rx_rd >= U2RX_BUF_SIZE-1) ||
			(u2rx_wr < u2rx_rd && u2rx_rd == u2rx_wr+1) )	// Buffer Overflow
		{
			//USART_ClearITPendingBit(USART2, USART_IT_RXNE);
			//wizpf_led_set(WIZ_LED2, VAL_TOG);
			USART_SendData(USART2, (uint8)'@');
			return;
		}
		u2rx_buf[u2rx_wr] = (uint8)USART_ReceiveData(USART2);
		if(u2rx_wr < U2RX_BUF_SIZE-1) u2rx_wr++;
		else u2rx_wr = 0;
	}
}
#endif

int8 platform_init(usart_param *up)
{
	int8 ret8;
	uint32 retu32;
	usart_param up_tmp;
	uint8 tx_size[8]={2, 2, 2, 2, 2, 2, 2, 2}; // Device default memory setting
	uint8 rx_size[8]={2, 2, 2, 2, 2, 2, 2, 2};

	RCC_Configuration();	// Configure the system clocks
	GPIO_Configuration();	// Configure GPIO Pin setting
	NVIC_Configuration();	// Configure the vector table
	retu32 = SysTick_Config(SystemCoreClock/SYSTICK_HZ);	// SysTick Configuration - 1ms
	if(retu32 != 0) return RET_NOK;

	if(up == NULL) {
		WIZPF_USART_SET_PARAM(&up_tmp, UBR_115200, UWL_8, UST_1, UPB_NO, UFC_NO);
		up = &up_tmp;
	}
	ret8 = wizpf_usart_init(WIZ_USART1, up);
	if(ret8 != RET_OK) return RET_NOK;

	ret8 = wizspi_init(WIZ_SPI1);
	if(ret8 != RET_OK) {
		ERR("wizspi_init fail");
		return RET_NOK;
	}

	device_HW_reset();
	DEVICE_INIT_WITH_MEMCHK(tx_size, rx_size);

	wizpf_led_set(WIZ_LED1, VAL_ON);	// LED3 and LED4 On by default
	wizpf_led_set(WIZ_LED2, VAL_ON);

	return RET_OK;
}

int8 wizpf_usart_init(wizpf_usart usart, usart_param *param)
{
	USART_TypeDef *usartx;
	USART_InitTypeDef USART_InitStructure;

	switch(usart) {
	case WIZ_USART1:
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#ifdef USART1_TX_PIN
		GPIO_INIT_SIMP(GPIO_Mode_AF_PP, USART1_TX_PORT, USART1_TX_PIN);
#endif
#ifdef USART1_RX_PIN
		GPIO_INIT_SIMP(GPIO_Mode_IN_FLOATING, USART1_RX_PORT, USART1_RX_PIN);
#endif
#ifdef USART1_RTS_PIN
		GPIO_INIT_SIMP(GPIO_Mode_AF_PP, USART1_RTS_PORT, USART1_RTS_PIN);
#endif
#ifdef USART1_CTS_PIN
		GPIO_INIT_SIMP(GPIO_Mode_IN_FLOATING, USART1_CTS_PORT, USART1_CTS_PIN);
#endif
#if (USART1_RX_INTERRUPT == VAL_ENABLE)
{		NVIC_InitTypeDef NVIC_InitStructure;
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
}
#endif
		usartx = USART1;
		break;
	case WIZ_USART2:
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
#ifdef USART2_TX_PIN
		GPIO_INIT_SIMP(GPIO_Mode_AF_PP, USART2_TX_PORT, USART2_TX_PIN);
#endif
#ifdef USART2_RX_PIN
		GPIO_INIT_SIMP(GPIO_Mode_IN_FLOATING, USART2_RX_PORT, USART2_RX_PIN);
#endif
#ifdef USART2_RTS_PIN
		GPIO_INIT_SIMP(GPIO_Mode_AF_PP, USART2_RTS_PORT, USART2_RTS_PIN);
#endif
#ifdef USART2_CTS_PIN
		GPIO_INIT_SIMP(GPIO_Mode_IN_FLOATING, USART2_CTS_PORT, USART2_CTS_PIN);
#endif
#if (USART2_RX_INTERRUPT == VAL_ENABLE)
{		NVIC_InitTypeDef NVIC_InitStructure;
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
}
#endif
		usartx = USART2;
		break;
	//case WIZ_USART3:
	//	break;
	default:
		return RET_NOK;
	}

	USART_InitStructure.USART_BaudRate = param->baudrate;
	switch(param->wordlen) {
	case UWL_8: USART_InitStructure.USART_WordLength = USART_WordLength_8b; break;
	case UWL_9: USART_InitStructure.USART_WordLength = USART_WordLength_9b; break;
	}
	switch(param->stopbit) {
	case UST_0d5: USART_InitStructure.USART_StopBits = USART_StopBits_0_5; break;
	case UST_1:   USART_InitStructure.USART_StopBits = USART_StopBits_1; break;
	case UST_1d5: USART_InitStructure.USART_StopBits = USART_StopBits_1_5; break;
	case UST_2:   USART_InitStructure.USART_StopBits = USART_StopBits_2; break;
	}
	switch(param->parity) {
	case UPB_NO:  USART_InitStructure.USART_Parity = USART_Parity_No; break;
	case UPB_EVEN:USART_InitStructure.USART_Parity = USART_Parity_Even; break;
	case UPB_ODD: USART_InitStructure.USART_Parity = USART_Parity_Odd; break;
	}
	switch(param->flowcon) {
	case UFC_NO:  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; break;
	case UFC_HW:  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS; break;
	//case UFC_SW:
	}
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(usartx, &USART_InitStructure);

#if (USART1_RX_INTERRUPT == VAL_ENABLE)
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
#endif
#if (USART2_RX_INTERRUPT == VAL_ENABLE)
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
#endif

	USART_Cmd(usartx, ENABLE);

	return RET_OK;
}

int8 wizpf_gpio_init(GPIO_TypeDef* GPIOx, uint16 GPIO_Pin, gpio_mode mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	switch(mode) {
	case GMOD_IN_FLOAT:
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		break;
	case GMOD_IN_PULLUP:
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		break;
	case GMOD_IN_PULLDOWN:
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
		break;
	case GMOD_OUT_PUSHPULL:
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		break;
	case GMOD_OUT_OPENDRAIN:
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
		break;
	default:
		ERRA("GPIO Mode(%d) is not allowed", mode);
		return RET_NOK;
	}

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
	GPIO_Init(GPIOx, &GPIO_InitStructure);

	return RET_OK;
}

int8 wizpf_gpio_set(GPIO_TypeDef* GPIOx, uint16 GPIO_Pin, int8 value)
{
	return RET_NOK;
}

int8 wizpf_gpio_get(GPIO_TypeDef* GPIOx, uint16 GPIO_Pin)
{
	return RET_NOK;
}

int8 wizpf_timer_init(void)
{
	ERR("Timer is not implemented yet");
	return RET_NOK;
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	/* TIM2 clock enable */
	/* Enable the TIM2 global Interrupt */
	//NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	//NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	//NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	//NVIC_Init(&NVIC_InitStructure);
}

uint32 wizpf_get_systick(void)
{
	return msTicks;
}

uint32 wizpf_tick_conv(bool istick2sec, uint32 tickorsec)
{
	if(istick2sec) return tickorsec / SYSTICK_HZ;	// tick to seconds
	else return tickorsec * SYSTICK_HZ;	// seconds to tick
}

int32 wizpf_tick_elapse(uint32 tick)	// + Elapsed time, - Remaining time
{
	uint32 cur = wizpf_get_systick();

	return cur - tick;
}

int8 wizpf_led_set(wizpf_led led, uint8 action)
{
	GPIO_TypeDef* GPIOx = NULL;
	uint16 GPIO_Pin = 0;

	switch(led) {
#ifdef LED1_PIN
	case WIZ_LED1:
		GPIOx = LED1_PORT;
		GPIO_Pin = LED1_PIN;
		break;
#endif
#ifdef LED2_PIN
	case WIZ_LED2:
		GPIOx = LED2_PORT;
		GPIO_Pin = LED2_PIN;
		break;
#endif
#ifdef LED3_PIN
	case WIZ_LED3:
		GPIOx = LED3_PORT;
		GPIO_Pin = LED3_PIN;
		break;
#endif
#ifdef LED4_PIN
	case WIZ_LED4:
		GPIOx = LED4_PORT;
		GPIO_Pin = LED4_PIN;
		break;
#endif
	default:
		ERRA("LED(%d) is not allowed", led);
		action = VAL_TOG + 1;
	}

	
	if(action == VAL_OFF) {
		GPIO_SetBits(GPIOx, GPIO_Pin); // LED off
	} else if(action == VAL_ON) {
		GPIO_ResetBits(GPIOx, GPIO_Pin); // LED on
	} else if(action == VAL_TOG) {
		GPIOx->ODR ^= GPIO_Pin;
	} else return RET_NOK;

	return RET_OK;
}

int8 wizpf_led_get(wizpf_led led)
{
	GPIO_TypeDef* GPIOx = NULL;
	uint16 GPIO_Pin = 0;

	switch(led) {
#ifdef LED1_PIN
	case WIZ_LED1:
		GPIOx = LED1_PORT;
		GPIO_Pin = LED1_PIN;
		break;
#endif
#ifdef LED2_PIN
	case WIZ_LED2:
		GPIOx = LED2_PORT;
		GPIO_Pin = LED2_PIN;
		break;
#endif
#ifdef LED3_PIN
	case WIZ_LED3:
		GPIOx = LED3_PORT;
		GPIO_Pin = LED3_PIN;
		break;
#endif
#ifdef LED4_PIN
	case WIZ_LED4:
		GPIOx = LED4_PORT;
		GPIO_Pin = LED4_PIN;
		break;
#endif
	default:
		ERRA("LED(%d) is not allowed", led);
	}

	if(GPIOx != NULL) return (GPIOx->ODR & GPIO_Pin)? VAL_OFF: VAL_ON;
	return RET_NOK;
}

void wizpf_led_trap(uint8 repeat)
{
#ifdef LED1_PIN
	wizpf_led_set(WIZ_LED1, VAL_OFF);
	while(1) {
		Delay_ms(1500);
		for(uint32 i=0; i<repeat; i++) {
			wizpf_led_set(WIZ_LED1, VAL_TOG);
			Delay_ms(200);
			wizpf_led_set(WIZ_LED1, VAL_TOG);
			Delay_ms(200);
		}
	}
#endif
}

void device_HW_reset(void)
{
#ifdef WIZ_RESET_PIN
	GPIO_ResetBits(WIZ_RESET_PORT, WIZ_RESET_PIN);
	Delay_us(8);
	GPIO_SetBits(WIZ_RESET_PORT, WIZ_RESET_PIN);
	Delay_ms(50);
#endif
}

void GPIO_Configuration(void)
{
	{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);	// Set all as default
	GPIO_Init(PORTA, &GPIO_InitStructure);
	GPIO_Init(PORTB, &GPIO_InitStructure);
	GPIO_Init(PORTC, &GPIO_InitStructure);
	}

#ifdef LED1_PIN
	GPIO_INIT_SIMP(GPIO_Mode_Out_PP, LED1_PORT, LED1_PIN);
	wizpf_led_set(WIZ_LED1, VAL_OFF);
#endif
#ifdef LED2_PIN
	GPIO_INIT_SIMP(GPIO_Mode_Out_PP, LED2_PORT, LED2_PIN);
	wizpf_led_set(WIZ_LED2, VAL_OFF);
#endif
#ifdef WIZ_RESET_PIN
	GPIO_INIT_SIMP(GPIO_Mode_Out_PP, WIZ_RESET_PORT, WIZ_RESET_PIN);
	GPIO_SetBits(WIZ_RESET_PORT, WIZ_RESET_PIN);
#endif
#ifdef WIZ_PWDN_PIN
	GPIO_INIT_SIMP(GPIO_Mode_Out_PP, WIZ_PWDN_PORT, WIZ_PWDN_PIN);
	GPIO_ResetBits(WIZ_PWDN_PORT, WIZ_PWDN_PIN);
#endif
#ifdef WIZ_INT_PIN
	GPIO_INIT_SIMP(GPIO_Mode_IPU, WIZ_INT_PORT, WIZ_INT_PIN);	// For IIN Chip EXTI
#endif

	// ToDo

}

void RCC_Configuration(void)
{
	ErrorStatus HSEStartUpStatus;

	RCC_DeInit();				/* RCC system reset(for debug purpose) */
	RCC_HSEConfig(RCC_HSE_ON);	/* Enable HSE */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();	/* Wait till HSE is ready */

	if(HSEStartUpStatus == SUCCESS)
	{
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);	/* Enable Prefetch Buffer */
		FLASH_SetLatency(FLASH_Latency_2);	/* Flash 2 wait state */
		RCC_HCLKConfig(RCC_SYSCLK_Div1);	/* HCLK = SYSCLK */
		RCC_PCLK2Config(RCC_HCLK_Div1);		/* PCLK2 = HCLK */
		RCC_PCLK1Config(RCC_HCLK_Div2);		/* PCLK1 = HCLK/2 */
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);	/* PLLCLK = 8MHz * 9 = 72 MHz */
		RCC_PLLCmd(ENABLE);	/* Enable PLL */

		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}	/* Wait till PLL is ready */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);	/* Select PLL as system clock source */
		while(RCC_GetSYSCLKSource() != 0x08) {}		/* Wait till PLL is used as system clock source */
	}

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);	// For IIN Chip EXTI
}

void NVIC_Configuration(void)
{
#ifdef VECT_TAB_RAM
	/* Set the Vector Table base location at 0x20000000 */
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else /* VECT_TAB_FLASH */
	/* Set the Vector Table base location at 0x08000000 */
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif

	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);


	// ToDo

}

void Delay_us(uint8 time_us)
{
	register uint8 i, j;

	for(i=0; i<time_us; i++) {
		for(j=0; j<5; j++) {	// 25CLK
			asm("nop");	//1CLK         
			asm("nop");	//1CLK         
			asm("nop");	//1CLK         
			asm("nop");	//1CLK         
			asm("nop");	//1CLK                  
		}      
	}					// 25CLK*0.04us=1us
}

void Delay_ms(uint16 time_ms)
{
	register uint16 i;

	for(i=0; i<time_ms; i++) {
		Delay_us(250);
		Delay_us(250);
		Delay_us(250);
		Delay_us(250);
	}
}

void Delay_tick(uint32 tick)
{
	uint32_t curTicks = msTicks;

	while(msTicks - curTicks < tick);
}


int32 putc(int32 ch, wizpf_usart usart)
{
	USART_TypeDef *USARTx;

	if(usart == WIZ_USART1) USARTx = USART1;
	else if(usart == WIZ_USART2) USARTx = USART2;
	else return RET_NOK;

	USART_SendData(USARTx, (uint8)ch);
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
	return ch;
}

int32 getc(wizpf_usart usart)
{
#if (USART1_RX_INTERRUPT == VAL_ENABLE)
	if(usart == WIZ_USART1) {
		while(u1rx_rd == u1rx_wr);
		if(u1rx_rd < U1RX_BUF_SIZE-1) {
			u1rx_rd++;
			return u1rx_buf[u1rx_rd-1];
		} else {
			u1rx_rd = 0;
			return u1rx_buf[U1RX_BUF_SIZE-1];
		}
	}
#endif

#if (USART2_RX_INTERRUPT == VAL_ENABLE)
	if(usart == WIZ_USART2) {
		while(u2rx_rd == u2rx_wr);
		if(u2rx_rd < U2RX_BUF_SIZE-1) {
			u2rx_rd++;
			return u2rx_buf[u2rx_rd-1];
		} else {
			u2rx_rd = 0;
			return u2rx_buf[U2RX_BUF_SIZE-1];
		}
	}
#endif

#if (USART1_RX_INTERRUPT == VAL_DISABLE) || (USART2_RX_INTERRUPT == VAL_DISABLE)
	{
	USART_TypeDef *USARTx;

	if(usart == WIZ_USART1) USARTx = USART1;
	else if(usart == WIZ_USART2) USARTx = USART2;
	else return RET_NOK;

	while(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET);
	return USART_ReceiveData(USARTx);
	}
#else
	return RET_NOK;
#endif
}

int32 getc_nonblk(wizpf_usart usart)
{
#if (USART1_RX_INTERRUPT == VAL_ENABLE)
	if(usart == WIZ_USART1) {
		if(u1rx_rd == u1rx_wr) return RET_NOK;
		if(u1rx_rd < U1RX_BUF_SIZE-1) {
			u1rx_rd++;
			return u1rx_buf[u1rx_rd-1];
		} else {
			u1rx_rd = 0;
			return u1rx_buf[U1RX_BUF_SIZE-1];
		}
	}
#endif

#if (USART2_RX_INTERRUPT == VAL_ENABLE)
	if(usart == WIZ_USART2) {
		if(u2rx_rd == u2rx_wr) return RET_NOK;
		if(u2rx_rd < U2RX_BUF_SIZE-1) {
			u2rx_rd++;
			return u2rx_buf[u2rx_rd-1];
		} else {
			u2rx_rd = 0;
			return u2rx_buf[U2RX_BUF_SIZE-1];
		}
	}
#endif

#if (USART1_RX_INTERRUPT == VAL_DISABLE) || (USART2_RX_INTERRUPT == VAL_DISABLE)
	{
	USART_TypeDef *USARTx;

	if(usart == WIZ_USART1) USARTx = USART1;
	else if(usart == WIZ_USART2) USARTx = USART2;
	else return RET_NOK;

	if(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET) return RET_NOK;
	return USART_ReceiveData(USARTx);
	}
#else
	return RET_NOK;
#endif
}

void change_std_usart(wizpf_usart usart)
{
	if(usart == WIZ_USART1) std_usart = USART1;
	else if(usart == WIZ_USART2) std_usart = USART2;
	else ERRA("wrong USART Index (%d)", usart);
}

#ifdef COMPILER_IAR_EWARM
int putchar(int ch)
{
	USART_SendData(std_usart, (uint8)ch);
	while(USART_GetFlagStatus(std_usart, USART_FLAG_TXE) == RESET);
	return ch;
}

int getchar(void)
{
#if (USART1_RX_INTERRUPT == VAL_ENABLE)
	if(std_usart == USART1) {
		while(u1rx_rd == u1rx_wr);
		if(u1rx_rd < U1RX_BUF_SIZE-1) {
			u1rx_rd++;
			return u1rx_buf[u1rx_rd-1];
		} else {
			u1rx_rd = 0;
			return u1rx_buf[U1RX_BUF_SIZE-1];
		}
	}
#endif

#if (USART2_RX_INTERRUPT == VAL_ENABLE)
	if(std_usart == USART2) {
		while(u2rx_rd == u2rx_wr);
		if(u2rx_rd < U2RX_BUF_SIZE-1) {
			u2rx_rd++;
			return u2rx_buf[u2rx_rd-1];
		} else {
			u2rx_rd = 0;
			return u2rx_buf[U2RX_BUF_SIZE-1];
		}
	}
#endif

#if (USART1_RX_INTERRUPT == VAL_DISABLE) || (USART2_RX_INTERRUPT == VAL_DISABLE)
	while(USART_GetFlagStatus(std_usart, USART_FLAG_RXNE) == RESET);
	return USART_ReceiveData(std_usart);
#else
	return RET_NOK;
#endif
}
#elif COMPILER_GCC_ARM

	// Todo

#else	// for Doxygen
int putchar(int ch);
int getchar(void);
#endif



