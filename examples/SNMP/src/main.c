#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stdio.h"
#include "config.h"
#include "util.h"
#include "W5200\w5200.h"
#include "W5200\socket.h"
#include "W5200\spi1.h"
#include "APPs\loopback.h"

#include <string.h>
#include <stdlib.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

CONFIG_MSG Config_Msg;
CHCONFIG_TYPE_DEF Chconfig_Type_Def; 

// Configuration Network Information of W5200
uint8 Enable_DHCP = OFF;
uint8 Mac[6] = {0x00, 0x08, 0xDC, 0x11, 0x22, 0x33};//MAC Address
uint8 IP[4] = {192, 168, 0, 100};//IP Address
uint8 Gateway[4] = {192, 168, 0, 1};//Gateway Address
uint8 Subnet[4] = {255, 255, 255, 0};//SubnetMask Address
uint8 DNSServerIP[4] = {168, 26, 63, 1};//DNS Server Address

//FOR TCP Client
//Configuration Network Information of TEST PC
uint8 Dest_IP[4] = {192, 168, 0, 3}; //DST_IP Address 
uint16 Dest_PORT = 4000; //DST_IP port

uint8 ch_status[MAX_SOCK_NUM] = { 0, };	/** 0:close, 1:ready, 2:connected */

uint8 TX_BUF[TX_RX_MAX_BUF_SIZE]; // TX Buffer for applications
uint8 RX_BUF[TX_RX_MAX_BUF_SIZE]; // RX Buffer for applications
   
void WIZ_Config(void)
{
	uint8 i;
	wiz_NetInfo netinfo;

	Reset_W5200();
	wizInit();

	for(i=0; i<4; i++){
		netinfo.Mac[i] = Mac[i];
		printf("MAC[%d]: 0x%2x\r\n", i, Mac[i]);
		netinfo.IP[i] = IP[i];
		netinfo.Subnet[i] = Subnet[i];
		netinfo.Gateway[i] = Gateway[i];
		netinfo.DNSServerIP[i] = DNSServerIP[i];
	}
	netinfo.Mac[i] = Mac[i];
	printf("MAC[%d]: 0x%2x\r\n", i, Mac[i]);
	i++;
	netinfo.Mac[i] = Mac[i];
	printf("MAC[%d]: 0x%2x\r\n", i, Mac[i]);

	SetNetInfo(&netinfo);

	printf("\r\n--------------------------------------- \r\n");
	printf("W5200E01-M3                       \r\n");
	printf("Network Configuration Information \r\n");
	printf("--------------------------------------- ");

	GetNetInfo(&netinfo);
	printf("\r\nMAC : %.2X.%.2X.%.2X.%.2X.%.2X.%.2X", netinfo.Mac[0],netinfo.Mac[1],netinfo.Mac[2],netinfo.Mac[3],netinfo.Mac[4],netinfo.Mac[5]);
	printf("\r\nIP : %d.%d.%d.%d", netinfo.IP[0],netinfo.IP[1],netinfo.IP[2],netinfo.IP[3]);
	printf("\r\nSN : %d.%d.%d.%d", netinfo.Subnet[0],netinfo.Subnet[1],netinfo.Subnet[2],netinfo.Subnet[3]);
	printf("\r\nGW : %d.%d.%d.%d", netinfo.Gateway[0],netinfo.Gateway[1],netinfo.Gateway[2],netinfo.Gateway[3]);
	printf("\r\nDNS server : %d.%d.%d.%d", netinfo.DNSServerIP[0],netinfo.DNSServerIP[1],netinfo.DNSServerIP[2],netinfo.DNSServerIP[3]);
}       

__IO uint32_t Timer2_Counter;

uint32_t my_time;
uint32_t presentTime;

/* Private function prototypes -----------------------------------------------*/
void RCC_Configuration(void);
void NVIC_Configuration(void);
void GPIO_Configuration(void);
void Timer_Configuration(void);
extern void EXTI_IMR_EMR_enable(void);


/* Private functions ---------------------------------------------------------*/

void Timer2_ISR(void)
{
	if (Timer2_Counter++ > 1000) { // 1m x 1000 = 1sec
		Timer2_Counter = 0;
		my_time++;
		
	}
	
}


/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main(int argc, char* argv[])
{
	RCC_Configuration(); // Configure the system clocks
	NVIC_Configuration(); // NVIC Configuration
	GPIO_Configuration();
	USART1_Init();
	Timer_Configuration();
	
	Reset_W5200();
	wizspi_init(WIZ_SPI_1);	
	WIZ_Config(); // network config & Call Set_network ();      
	
	// LED3 and LED4 On!
	LED3_onoff(ON);
	//LED4_onoff(ON);
	
	// sekim LED Blink
	for (int m=0; m<10; m++)
	{
		LED3_onoff(OFF);
		Delay_ms(50);
		LED3_onoff(ON);
		Delay_ms(50);
	}
	
	{
		void UserSnmpDemo();
		UserSnmpDemo();
	}
	
	while(1);
//	presentTime = my_time; // For TCP client's connection request delay
//	
//	// Start Application         
//	printf("\r\n\r\n------------------------------------------- \r\n");
//	printf("TCP Server Loopback using W5200\r\n");  
//	printf("------------------------------------------- ");
//	while(1)
//	{
//		loopback_tcps(0, 5000);
//		loopback_tcps(1, 5001);
//		loopback_tcps(2, 5002);
//		loopback_tcps(3, 5003);		
//		loopback_tcpc(4, 5004);              
//		loopback_udp(5, 3000);
//		loopback_udp(6, 3001);
//		loopback_udp(7, 3002);                
//	}

	
}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	// Port A output
	GPIO_InitStructure.GPIO_Pin = WIZ_SCS;
#ifdef __DEF_W5200__
	GPIO_InitStructure.GPIO_Pin |= LED3 | LED4;
#endif
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, WIZ_SCS);
#ifdef __DEF_W5200__
	GPIO_SetBits(GPIOA, LED3); // led off
	GPIO_SetBits(GPIOA, LED4); // led off
#endif

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
	/* Configure SPI pins: SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin = WIZ_SCLK | WIZ_MISO | WIZ_MOSI;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Port B
	GPIO_InitStructure.GPIO_Pin = WIZ_RESET | WIZ_PWDN;
#ifdef __DEF_W7200__
	GPIO_InitStructure.GPIO_Pin |= LED3 | LED4;
#endif
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, WIZ_RESET);
	GPIO_ResetBits(GPIOB, WIZ_PWDN);
#ifdef __DEF_W7200__
	GPIO_SetBits(GPIOB, LED3); // led off
	GPIO_SetBits(GPIOB, LED4); // led off
#endif

	// WIZ Interrupt
	GPIO_InitStructure.GPIO_Pin = WIZ_INT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
#ifdef __DEF_W5200__
	GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif
#ifdef __DEF_W7200__
	GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif
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


/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
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
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC
				|RCC_APB2Periph_AFIO  | RCC_APB2Periph_USART1, ENABLE);

}

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
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
	/*
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	*/

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



void Timer_Configuration(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 1000;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/* Prescaler configuration */
	TIM_PrescalerConfig(TIM2, 71, TIM_PSCReloadMode_Immediate);

	/* TIM enable counter */
	TIM_Cmd(TIM2, ENABLE);

	/* TIM IT enable */
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

}
