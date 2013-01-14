
//#define FILE_LOG_SILENCE
#include "common/common.h"
//#include "host/wizplatform.h"


extern void EXTI_IMR_EMR_enable(void);

__IO uint32 msTicks = 0;	// 최대값은 대략 50일 정도


void SysTick_Handler(void)
{
	msTicks++;
}

int8 platform_init(void)
{
	int8 ret8;
	uint32 retu32;
	uint8 tx_size[8]={2, 2, 2, 2, 2, 2, 2, 2}; // Device default memory setting
	uint8 rx_size[8]={2, 2, 2, 2, 2, 2, 2, 2};

	RCC_Configuration();	// Configure the system clocks
	GPIO_Configuration();	// Configure GPIO Pin setting
	NVIC_Configuration();	// Configure the vector table
	retu32 = SysTick_Config(SystemCoreClock/1000);	// SysTick Configuration - 1ms
	if(retu32 != 0) return RET_NOK;

	ret8 = wizpf_uart_init(WIZ_USART1);
	if(ret8 != RET_OK) return RET_NOK;

	ret8 = wizspi_init(WIZ_SPI1);
	if(ret8 != RET_OK) {
		ERR("wizspi_init fail");
		return RET_NOK;
	}

	device_HW_reset();
	DEVICE_INIT_WITH_MEMCHK(tx_size, rx_size);

	wizpf_led_act(WIZ_LED3, VAL_ON);	// LED3 and LED4 On by default
	wizpf_led_act(WIZ_LED4, VAL_ON);

	return RET_OK;
}

int8 wizpf_uart_init(wizpf_usart usart)
{
	USART_TypeDef *usartx;
	USART_InitTypeDef USART_InitStructure;

	switch(usart) {
	case WIZ_USART1:
		usartx = USART1;
		break;
	case WIZ_USART2:
		usartx = USART2;
		break;
	//case WIZ_USART3:
	//	usartx = USART3;
	//	break;
	default:
		return RET_NOK;
	}

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(usartx, &USART_InitStructure);
	USART_Cmd(usartx, ENABLE);

	return RET_OK;
}

uint32 wizpf_get_systick(void)
{
	return msTicks;
}

int32 wizpf_tick_elapse(uint32 tick)
{
	uint32 cur = wizpf_get_systick();

	if(cur >= tick) {	// normal case
		if(cur - tick > 0x7fffffff)	// 리턴 가능한 양수인지 체크
			return 0x7fffffff;		// +max
		else return cur - tick;
	} else {			// overflow case
		if(tick - cur < 0x80000000)	// 리턴 가능한 음수인지 체크
			return 0x80000000;		// -max
		else return 0xffffffff - (tick - cur);
	}
}

void wizpf_led_act(wizpf_led led, uint8 action)
{
	GPIO_TypeDef* GPIOx = GPIOA;
	uint16 GPIO_Pin;

	switch(led) {
	//case LED1:
	//	GPIO_Pin = LED1;
	//	break;
	//case LED2:
	//	GPIO_Pin = LED2;
	//	break;
	case WIZ_LED3:
		GPIO_Pin = LED3;
		break;
	case WIZ_LED4:
		GPIO_Pin = LED4;
		break;
	default:
		LOGA("LED(%d) is not implemented yet", led);
	}

	if(action == VAL_ON) {
		GPIO_ResetBits(GPIOx, GPIO_Pin); // LED on
	} else if(action == VAL_OFF) {
		GPIO_SetBits(GPIOx, GPIO_Pin); // LED off
	} else {
		GPIOx->ODR ^= GPIO_Pin;
	}
}

void wizpf_led_trap(uint8 repeat)
{
	wizpf_led_act(WIZ_LED3, VAL_OFF);
	wizpf_led_act(WIZ_LED4, VAL_OFF);
	while(1) {
		Delay_ms(1500);
		for(uint32 i=0; i<repeat; i++) {
			wizpf_led_act(WIZ_LED3, VAL_TOG);
			wizpf_led_act(WIZ_LED4, VAL_TOG);
			Delay_ms(200);
			wizpf_led_act(WIZ_LED3, VAL_TOG);
			wizpf_led_act(WIZ_LED4, VAL_TOG);
			Delay_ms(200);
		}
	}
}

void device_HW_reset(void)
{
	GPIO_ResetBits(GPIOB, WIZ_RESET);
	Delay_us(8); // 
	GPIO_SetBits(GPIOB, WIZ_RESET);
	Delay_ms(50); // 
}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	// Port A output
	GPIO_InitStructure.GPIO_Pin = WIZ_SCS;
	GPIO_InitStructure.GPIO_Pin |= LED3 | LED4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, WIZ_SCS);
	GPIO_SetBits(GPIOA, LED3); // led off
	GPIO_SetBits(GPIOA, LED4); // led off

	// Configure the GPIO ports( USART1 Transmit and Receive Lines)
	// Configure the USART1_Tx as Alternate function Push-Pull
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin =  USART1_TX | USART2_TX;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Configure the USART1_Rx as input floating
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = USART1_RX | USART2_RX;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// SPI 1
	/* Configure SPI pins: SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin = WIZ_SCLK | WIZ_MISO | WIZ_MOSI;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Port B
	GPIO_InitStructure.GPIO_Pin = WIZ_RESET | WIZ_PWDN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, WIZ_RESET);
	GPIO_ResetBits(GPIOB, WIZ_PWDN);

	// WIZ Interrupt
	GPIO_InitStructure.GPIO_Pin = WIZ_INT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	EXTI_DeInit();

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	//EXTI_GenerateSWInterrupt(EXTI_Line0);

	EXTI_IMR_EMR_enable();
}

void RCC_Configuration(void)
{
	ErrorStatus HSEStartUpStatus;

	/* RCC system reset(for debug purpose) */
	RCC_DeInit();

	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);

	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if(HSEStartUpStatus == SUCCESS)
	{
		/* Enable Prefetch Buffer */
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

		/* Flash 2 wait state */
		FLASH_SetLatency(FLASH_Latency_2);

		/* HCLK = SYSCLK */
		RCC_HCLKConfig(RCC_SYSCLK_Div1);

		/* PCLK2 = HCLK */
		RCC_PCLK2Config(RCC_HCLK_Div1);

		/* PCLK1 = HCLK/2 */
		RCC_PCLK1Config(RCC_HCLK_Div2);

		/* PLLCLK = 8MHz * 9 = 72 MHz */
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

		/* Enable PLL */
		RCC_PLLCmd(ENABLE);

		/* Wait till PLL is ready */
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}

		/* Select PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		/* Wait till PLL is used as system clock source */
		while(RCC_GetSYSCLKSource() != 0x08)
		{
		}
	}

	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC
				|RCC_APB2Periph_AFIO  | RCC_APB2Periph_USART1, ENABLE);

}

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Set the Vector Table base location at 0x08000000 */
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);

#ifdef VECT_TAB_RAM
	/* Set the Vector Table base location at 0x20000000 */
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);

#else /* VECT_TAB_FLASH */
	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the USART2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

#ifdef __DEF_IINCHIP_INT__
	/* Enable the W5200 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

	/* Enable the TIM2 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Set the Vector Table base location at 0x08000000 */
	//NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
}

void Delay_us( uint8 time_us )
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

void Delay_ms( uint16 time_ms )
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

	while ((msTicks - curTicks) > tick);
}

int getchar_nonblk(void)
{
	if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
		return RET_NOK;

	return USART_ReceiveData(USART1);
}

#ifdef COMPILER_IAR_EWARM
int putchar(int ch)
{
	USART_SendData(USART1, (uint8) ch);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);

	return ch;
}

int getchar(void)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
	return USART_ReceiveData(USART1);
}
#elif COMPILER_GCC_ARM

#endif



