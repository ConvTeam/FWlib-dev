
//#define FILE_LOG_SILENCE
#include "common/common.h"
//#include "host/wizplatform.h"

extern void EXTI_IMR_EMR_enable(void);

__IO uint32 msTicks = 0;	// 최대값은 대략 50일 정도

#define USART1_RX_INTERRUPT VAL_ENABLE
#define SYSTICK_HZ			1000

void SysTick_Handler(void)	// SysTick ISR
{
	msTicks++;
}

#if (USART1_RX_INTERRUPT == VAL_ENABLE)
#define U1RX_BUF_SIZE	300
int8 u1rx_buf[U1RX_BUF_SIZE];
int16 u1rx_wr=0, u1rx_rd=0;
void USART1_IRQHandler(void)	// USART1 ISR
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {		//
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		if( (u1rx_wr > u1rx_rd && u1rx_wr-u1rx_rd >= U1RX_BUF_SIZE-1) ||
			(u1rx_wr < u1rx_rd && u1rx_rd == u1rx_wr+1) )	// Buffer Overflow
		{
			//USART_ClearITPendingBit(USART1, USART_IT_RXNE);
			wizpf_led_set(WIZ_LED3, VAL_TOG);
			wizpf_led_set(WIZ_LED4, VAL_TOG);
			USART_SendData(USART1, (uint8)'@');
			return;
		}
		//uint16_t kkk = USART_ReceiveData(USART1);
		//for(int ii=0; ii<15; ii++) 
		//printf("%x ", kkk);
		u1rx_buf[u1rx_wr] = (int8)USART_ReceiveData(USART1);
		if(u1rx_wr < U1RX_BUF_SIZE-1) u1rx_wr++;
		else u1rx_wr = 0;
	}
}
#endif

int8 platform_init(void)
{
	int8 ret8;
	uint32 retu32;
	uint8 tx_size[8]={2, 2, 2, 2, 2, 2, 2, 2}; // Device default memory setting
	uint8 rx_size[8]={2, 2, 2, 2, 2, 2, 2, 2};

	RCC_Configuration();	// Configure the system clocks
	GPIO_Configuration();	// Configure GPIO Pin setting
	NVIC_Configuration();	// Configure the vector table
	retu32 = SysTick_Config(SystemCoreClock/SYSTICK_HZ);	// SysTick Configuration - 1ms
	if(retu32 != 0) return RET_NOK;

	ret8 = wizpf_uart_init(WIZ_USART1);
	if(ret8 != RET_OK) return RET_NOK;

        //ret8 = wizspi_init(WIZ_SPI1);
        ret8 = wizspi_init(WIZ_SPI2); // For W5500 FPGA board
        
	if(ret8 != RET_OK) {
		ERR("wizspi_init fail");
		return RET_NOK;
	}
       
	device_HW_reset();       
	DEVICE_INIT_WITH_MEMCHK(tx_size, rx_size);
       
	wizpf_led_set(WIZ_LED3, VAL_ON);	// LED3 and LED4 On by default
	wizpf_led_set(WIZ_LED4, VAL_ON);
       
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

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //enable receive interrupt

	USART_Cmd(usartx, ENABLE);

	return RET_OK;
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

int32 wizpf_tick_elapse(uint32 tick)	// + 지난 시간, - 다가올 시간
{
	uint32 cur = wizpf_get_systick();

	return cur - tick;
}

void wizpf_led_set(wizpf_led led, uint8 action)
{
	GPIO_TypeDef* GPIOx = GPIOB;
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

int8 wizpf_led_get(wizpf_led led)
{
	GPIO_TypeDef* GPIOx = GPIOB;
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

	return (GPIOx->ODR & GPIO_Pin)? VAL_OFF: VAL_ON;
}

void wizpf_led_trap(uint8 repeat)
{
	wizpf_led_set(WIZ_LED3, VAL_OFF);
	wizpf_led_set(WIZ_LED4, VAL_OFF);
	while(1) {
		Delay_ms(1500);
		for(uint32 i=0; i<repeat; i++) {
			wizpf_led_set(WIZ_LED3, VAL_TOG);
			wizpf_led_set(WIZ_LED4, VAL_TOG);
			Delay_ms(200);
			wizpf_led_set(WIZ_LED3, VAL_TOG);
			wizpf_led_set(WIZ_LED4, VAL_TOG);
			Delay_ms(200);
		}
	}
}

void device_HW_reset(void)
{
	// For W5500 FPGA board
        GPIO_ResetBits(GPIOB, WIZ_RESET2);
	Delay_us(8); // 
	GPIO_SetBits(GPIOB, WIZ_RESET2);
	Delay_ms(50); //       
  
        //GPIO_ResetBits(GPIOB, WIZ_RESET);
	//Delay_us(8); // 
	//GPIO_SetBits(GPIOB, WIZ_RESET);
	//Delay_ms(50); // 
}

void GPIO_Configuration(void)
{
        GPIO_InitTypeDef GPIO_InitStructure;

        // Port A output
        //GPIO_InitStructure.GPIO_Pin = WIZ_SCS | LED3 | LED4; 
        GPIO_InitStructure.GPIO_Pin = LED3 | LED4; 
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        //GPIO_SetBits(GPIOA, WIZ_SCS);
        GPIO_SetBits(GPIOA, LED3); // led off
        GPIO_SetBits(GPIOA, LED4); // led off
           
        // Configure the GPIO ports( USART1 Transmit and Receive Lines)
        // Configure the USART1_Tx as Alternate function Push-Pull 
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Pin =  USART1_TX;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        
        // Configure the USART1_Rx as input floating
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_InitStructure.GPIO_Pin = USART1_RX;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
      
        // SPI 1
        /* Configure SPIy pins: SCK, MISO and MOSI */
        //GPIO_InitStructure.GPIO_Pin = WIZ_SCLK | WIZ_MISO | WIZ_MOSI;
        //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        //GPIO_Init(GPIOA, &GPIO_InitStructure);
    
        // SPI 2
        /* Configure SPIy pins: SCK, MISO and MOSI */
        GPIO_InitStructure.GPIO_Pin = WIZ_SCLK2 | WIZ_MISO2 | WIZ_MOSI2;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        
    
        // Port B
        //GPIO_InitStructure.GPIO_Pin = WIZ_RESET|WIZ_PWDN ; 
        GPIO_InitStructure.GPIO_Pin = WIZ_RESET2 | WIZ_SCS2;     
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        //GPIO_SetBits(GPIOB, WIZ_RESET);
        //GPIO_ResetBits(GPIOB, WIZ_PWDN); 
        GPIO_SetBits(GPIOB, WIZ_RESET2);
        GPIO_SetBits(GPIOB, WIZ_SCS2);    
    
        // Port B input
        GPIO_InitStructure.GPIO_Pin   = WIZ_INT;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
        GPIO_Init(GPIOB, &GPIO_InitStructure);   
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
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_USART2, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_SPI2 | RCC_APB1Periph_USART2, ENABLE);
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
	//NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	//NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	//NVIC_Init(&NVIC_InitStructure);

	/* Enable the TIM2 global Interrupt */
	//NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	//NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	//NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	//NVIC_Init(&NVIC_InitStructure);
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

	while(msTicks - curTicks < tick);
}

int32 getchar_nonblk(void)
{
#if (USART1_RX_INTERRUPT == VAL_ENABLE)
	if(u1rx_rd == u1rx_wr) return RET_NOK;
	if(u1rx_rd < U1RX_BUF_SIZE-1) {
		u1rx_rd++;
		return u1rx_buf[u1rx_rd-1];
	} else {
		u1rx_rd = 0;
		return u1rx_buf[U1RX_BUF_SIZE-1];
	}
#else
	if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET) return RET_NOK;
	return USART_ReceiveData(USART1);
#endif
}

#ifdef COMPILER_IAR_EWARM
int putchar(int ch)
{
	USART_SendData(USART1, (uint8)ch);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	return ch;
}

int getchar(void)
{
#if (USART1_RX_INTERRUPT == VAL_ENABLE)
	while(u1rx_rd == u1rx_wr);
	if(u1rx_rd < U1RX_BUF_SIZE-1) {
		u1rx_rd++;
		return u1rx_buf[u1rx_rd-1];
	} else {
		u1rx_rd = 0;
		return u1rx_buf[U1RX_BUF_SIZE-1];
	}
#else
	while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
	return USART_ReceiveData(USART1);
#endif
}
#elif COMPILER_GCC_ARM

	// Todo

#endif



