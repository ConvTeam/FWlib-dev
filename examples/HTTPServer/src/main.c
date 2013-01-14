/********************************************************************
*	2011 WIZnet technology all rights reserved
*	HTTP Server Application Note code for W5200E01-M3
*	--------------------------------------------------
*	Ver 0.9 Release Feb. 2011	First release						
********************************************************************/

#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_flash.h"
#include "stdio.h"
#include "config.h"
#include "util.h"
#include "sockutil.h"
#include "W5200\w5200.h"
#include "W5200\socket.h"
#include "W5200\spi1.h"
#include "Apps\httpd.h"
#include "Apps\romfile.h"
#include <string.h>
#include <stdlib.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/





wiz_NetInfo netinfo,netinfonew,disnetinfo;
uint8 MAC[6] = {0x00, 0x08, 0xDC, 0x01, 0x02, 0x03};//MAC Address
uint8 ch_status[MAX_SOCK_NUM] = { 0, };	/** 0:close, 1:ready, 2:connected */

uint8 * data_buf = (uint8*) TX_BUF;	// Position of receive buffer

uint16 READ_SN_IR =0;

__IO uint32_t Timer2_Counter;


//Added by 2011-9-27
uint32 ChipID = 0x00005200;

/* HTTPs----------------------------------------------------------------------*/
#define DEFAULT_HTTP_PORT			80
static u_char* http_response;		/**< Pointer to HTTP response */
static st_http_request *http_request;	/**< Pointer to HTTP request */
unsigned char *homepage_default = "index.html";
unsigned char bchannel_start;
unsigned char *user_data;
//Added by Gang 2011-9-27
uint8 after_cgi_flag=0;

#define EVB_NET_SIP	"$SRC_IP_ADDRES$"
#define EVB_NET_GWIP	"$GW_IP_ADDRESS$"
#define EVB_NET_SN	"$SUB_NET__MASK$"
#define EVB_NET_MAC	"$SRC_MAC_ADDRESS$"
#define NET_CONFIG_CGI "NETCONF.CGI"

#define EVB_LED0_IMG	"$LED0_IMG$"
#define EVB_LED1_IMG	"$LED1_IMG$"
#define EVB_LED0_STAT	"$LED_0$"
#define EVB_LED1_STAT	"$LED_1$"


//Added by Gang 2011-9-27
pFunction Jump_To_Application;
unsigned long JumpAddress;
unsigned long FlashDestination = 0x0800D000; //you can use any flash space as long as it is empty


/* Private function prototypes -----------------------------------------------*/
void RCC_Configuration(void);
void NVIC_Configuration(void);
void GPIO_Configuration(void);
void Timer_Configuration(void);
extern void EXTI_IMR_EMR_enable(void);

void SetConfig(void);
void RomFileTest(void);
static void proc_http(SOCKET s,	u_char * buf);
void ProcessWebSever(uint8 ch);
void cgi_ipconfig(void);
static u_int replace_sys_env_value(u_char* base, u_int len);
void DisplayNetInfo(void);
void InitNetInfo(void);


/* Private functions ---------------------------------------------------------*/

void Timer2_ISR(void) // Timer2 interrupt service routine
{
	if (Timer2_Counter++ > 1000) { // 1m x 1000 = 1sec
		Timer2_Counter = 0;		
		
	}
	
}


/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main(void)
{  
	
        RCC_Configuration(); // Configure the system clocks
	NVIC_Configuration(); // NVIC Configuration
	GPIO_Configuration();
	USART1_Init();
	Timer_Configuration();
       
	wizspi_init(WIZ_SPI_1);	
        Reset_W5200();
        
        
	              
        // LED3 and LED4 On!
	LED3_onoff(ON);
	LED4_onoff(ON);
        
        
        wizInit();
       
        SetConfig();
        
	DisplayNetInfo();
       

	RomFileTest(); // Use search_file_rom() to read the 
	
	while (1)
	{
		ProcessWebSever(0);
                ProcessWebSever(1);
                ProcessWebSever(2);
                ProcessWebSever(3);
		ProcessWebSever(4);
		ProcessWebSever(5);
		ProcessWebSever(6);
		ProcessWebSever(7);	
	}	
        
       
       
        
        

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



void SetConfig(void)
{
  uint8 i;
  unsigned char Valid[4];
  //Added by Gang 20119-27
  for (i=0 ; i < 4 ; i++)
  {
    Valid[i]= *((const unsigned char*)(FlashDestination + i));
  }

  
  if (Valid[0]== 0x00 && Valid[1]== 0x52 && Valid[2]== 0x00 && Valid[3]== 0x00)
  {
         
         (netinfonew).Mac[0] = 0x00;
         (netinfonew).Mac[1] = 0x08;
         (netinfonew).Mac[2] = 0xDC;
         (netinfonew).Mac[3] = 0x01;
         (netinfonew).Mac[4] = 0x02;
         (netinfonew).Mac[5] = 0x03;
         
            for (i=4 ; i < 8 ; i++)
            {
                    (netinfonew).IP[i-4] = *((const unsigned char*)(FlashDestination + i));
                    
            }
            for (i=8 ; i < 12 ; i++)
            {
                    (netinfonew).Gateway[i-8] = *((const unsigned char*)(FlashDestination + i));
                  
            }
            for (i=12 ; i < 16 ; i++)
            {
                    (netinfonew).Subnet[i-12] = *((const unsigned char*)(FlashDestination + i));
                   
            }
     
        SetNetInfo(&netinfonew);
    
        after_cgi_flag=1;
  }
  else 
  {
    InitNetInfo();
  }
 
}

//processing http protocol , and excuting the followed fuction.
void ProcessWebSever(u_char ch)
{
	int len;
	u_int wait_send=0;

	http_request = (st_http_request*)RX_BUF;		// struct of http request
	
	/* http service start */
	switch(getSn_SR(ch))
	{
	case SOCK_ESTABLISHED:
		if(bchannel_start==1)
		{
			bchannel_start = 2;
		}
		if ((len = getSn_RX_RSR(ch)) > 0)		
		{
			if ((u_int)len > MAX_URI_SIZE) len = MAX_URI_SIZE;				
			len = TCPRecv(ch, (u_char*)http_request, len);
			*(((u_char*)http_request)+len) = 0;
			
#ifdef WEB_DEBUG				
			printf( "- HTTP REQUEST -");
#endif				
			proc_http(ch, (u_char*)http_request);	// request is processed
		
			while(getSn_TX_FSR(ch)!= getIINCHIP_TxMAX(ch))
			{
			
				if(wait_send++ > 1500)
				{
#ifdef WEB_DEBUG				
					printf( "HTTP Response send fail");
#endif				
					break;
				}
				//Delay(1);
				
			}
			TCPClose(ch);
		}
		break;


	case SOCK_CLOSE_WAIT:   
#ifdef WEB_DEBUG	
		printf("CLOSE_WAIT : %d",ch);	// if a peer requests to close the current connection
#endif		
		TCPClose(ch);
		bchannel_start = 0;
	case SOCK_CLOSED:                   
		if(!bchannel_start)
		{
//#ifdef WEB_DEBUG		
			//printf("%d : Web Server Started.\r\n",ch);
			printf("Web Server Started.\r\n");
//#endif				
			bchannel_start = 1;
		}

		  
                if (TCPServerOpen(ch,DEFAULT_HTTP_PORT)==0)/* reinitialize the socket */
		{
			bchannel_start = 0;
		}
		break;
	}	// end of switch 
}

static void proc_http(SOCKET s,	u_char * buf)
{
	char* name;
	char * param;
	unsigned long file_len;
	unsigned int send_len;
	unsigned long content = 0;

	
	http_response = (u_char*)RX_BUF;
	http_request = (st_http_request*)TX_BUF;
	
	parse_http_request(http_request, buf);			// After analyze request, convert into http_request

	//method Analyze
	switch (http_request->METHOD)				
	{
	case METHOD_ERR :	
		memcpy(http_response, ERROR_REQUEST_PAGE, sizeof(ERROR_REQUEST_PAGE));
                TCPSend(s, (u_char *)http_response, strlen((char const*)http_response));
		break;
	case METHOD_HEAD:
	case METHOD_GET:
	case METHOD_POST:
		name = (char*)get_http_uri_name(http_request->URI);
		
		if (!strcmp(name, "/")) strcpy(name, (char const*)homepage_default);	// If URI is "/", respond by index.htm 
#ifdef WEB_DEBUG
		if(strlen(name)	< 80)	printf("PAGE : %s", name);
		else printf("TOO LONG FILENAME");
#endif 			
		find_http_uri_type(&http_request->TYPE, name);	//Check file type (HTML, TEXT, GIF, JPEG are included)
#ifdef WEB_DEBUG			
		printf("find_type() ok");
#endif		

		if(http_request->TYPE == PTYPE_CGI)
		{
			
		        if(strstr(name,"LEDCTL.CGI"))
			{			
				
                                if((param = (char*)get_http_param_value(http_request->URI,"led0")))
				{
                                  if(!strcmp(param,"on")) LED3_onoff(ON);
				}
				else LED3_onoff(OFF);
				
				if((param = (char*)get_http_param_value(http_request->URI,"led1")))
				{
                                  if(!strcmp(param,"on")) LED4_onoff(ON);
				}
			        else LED4_onoff(OFF);

				strcpy(name,"dout.htm");
				find_http_uri_type(&http_request->TYPE, name);
			}
			else	 if(strstr(name,NET_CONFIG_CGI))
			{
				cgi_ipconfig(); //This function is not available now, because the Flash_write operation is not supported in this f/w

				memcpy(http_response, RETURN_CGI_PAGE, sizeof(RETURN_CGI_PAGE));
				TCPSend(s, (u_char *)http_response, strlen((char const*)http_response));				
				
				TCPClose(0);
				//Added by Gang 2011-9-27
                                JumpAddress = *(volatile unsigned long*) (ApplicationAddress + 4); //reset app after cgi_ipconfig();
	                        Jump_To_Application = (pFunction) JumpAddress;
	                        Jump_To_Application();
				
				return;
			}
		}
		
		/* Search the specified file in stored binaray html image */
		
		if(!search_file_rom((unsigned char *)name, &content, &file_len))
		{
			memcpy(http_response, ERROR_HTML_PAGE, sizeof(ERROR_HTML_PAGE));
			TCPSend(s, (u_char *)http_response, strlen((char const*)http_response));	
#ifdef WEB_DEBUG				
			printf("Unknown Page");
#endif				
		} 
		else	// if search file sucess 
		{
#ifdef WEB_DEBUG				
			printf("find file ok");
#endif				
			if(http_request->TYPE != PTYPE_CGI)			
			{
				make_http_response_head((unsigned char*)http_response, http_request->TYPE, (u_long)file_len);			
				TCPSend(s, http_response, strlen((char const*)http_response));
                               
			}
			
			

			
			while(file_len) 
			{
				if (file_len >= TX_RX_MAX_BUF_SIZE-1)
					send_len = TX_RX_MAX_BUF_SIZE-1;
				else	send_len = file_len;
			
				
				
				#if 1
				read_from_flashbuf(content, &http_response[0], send_len);
				#endif
				
				*(http_response+send_len+1) = 0;

				// Replace htmls' system environment value to real value
				if(http_request->TYPE==PTYPE_HTML)
				{
					send_len = replace_sys_env_value(http_response,send_len);
                                  
				}
				else 
				if(http_request->TYPE==PTYPE_CGI)
				{
					send_len = replace_sys_env_value(http_response,send_len);
                                 
				}
				else if(http_request->TYPE == PTYPE_TEXT)
				{
					if(strstr(name,"adc_val.xml"))
					{
						
						send_len = replace_sys_env_value(http_response,send_len);
                                          
					}						
				}
	
				TCPSend(s, http_response, send_len);
				content += send_len;
				file_len -= send_len;
			}
        }
		break;
	default :
		break;
	}
}

void RomFileTest(void)
{
	unsigned int address = 0;
	unsigned int len = 0;
	unsigned char ret;
	
	printf("index.html file search: ");	
	ret = search_file_rom(homepage_default, (unsigned long *)&address, (unsigned long *)&len);

	if(ret>0)printf("OK.\r\n");
	else printf("Fail!\r\n");
}

/**
 @brief	Replace HTML's variables to system configuration value
*/
static u_int replace_sys_env_value(u_char* base, u_int len)
{
	u_char str[18];	
	u_char *ptr = base;
	u_char *tptr = base;
	unsigned long temp;

#ifdef __DEF_W5200__
	GPIO_TypeDef* GPIOx = GPIOA;
#endif
#ifdef __DEF_W7200__
	GPIO_TypeDef* GPIOx = GPIOB;
#endif

	
	while((ptr=(u_char*)strchr((char*)tptr,'$')))
	{
		if((tptr=(u_char*)strstr((char*)ptr,EVB_NET_SIP)))
		{
			//Modified by Gang 2011-9-27
                        if(!after_cgi_flag) 
                        {
                           memcpy((unsigned char*)&temp, (netinfo).IP, 4);
                        }
                        else  memcpy((unsigned char*)&temp, (netinfonew).IP, 4);
                       
			memcpy(tptr,(unsigned char*)inet_ntoa(ntohl(temp)),15);
                       
			tptr+=15;
		}
		else if((tptr=(u_char*)strstr((char*)ptr,EVB_NET_GWIP)))
		{
			//Modified by Gang 2011-9-27
                        if(!after_cgi_flag) 
                        {
                           memcpy((unsigned char*)&temp, (netinfo).Gateway, 4);
                        }
                        else memcpy((unsigned char*)&temp, (netinfonew).Gateway, 4);
			
                        memcpy(tptr,(unsigned char*)inet_ntoa(ntohl(temp)),15);
                       
			tptr+=15;
		}
		else if((tptr=(u_char*)strstr((char*)ptr,EVB_NET_SN)))
		{
			//Modified by Gang 2011-9-27
                        if(!after_cgi_flag) 
                        {
                           memcpy((unsigned char*)&temp, (netinfo).Subnet,4);
                        }
                        else memcpy((unsigned char*)&temp, (netinfonew).Subnet,4);
			memcpy(tptr,(unsigned char*)inet_ntoa(ntohl(temp)),15);
                       
			tptr+=15;
		}
		else if((tptr=(u_char*)strstr((char*)ptr,EVB_NET_MAC)))
		{
			
                        sprintf((char*)str,"%02X:%02X:",MAC[0],MAC[1]);
			sprintf((char*)(str+6),"%02X:%02X:",MAC[2],MAC[3]);
			sprintf((char*)(str+12),"%02X:%02X",MAC[4],MAC[5]);
			memcpy(tptr,str,17);
			tptr+=17;
		}
		
				
		else if((tptr=(u_char*)strstr((char*)ptr,EVB_LED0_IMG)))
		{
			memset(tptr,0,10);
			//Modified by Gang 2011-9-27
			if(GPIO_ReadInputDataBit(GPIOx, LED3)==0x00){
				memcpy(tptr,"led_on.gif",10);
				LED3_onoff(ON);
			}
			else{
				memcpy(tptr,"led_of.gif",10);
				LED3_onoff(OFF);
			}

			tptr+=10;
		}
		
		else if((tptr=(u_char*)strstr((char*)ptr,EVB_LED1_IMG)))
		{
			memset(tptr,0,10);
                        //Modified by Gang 2011-9-27
			if(GPIO_ReadInputDataBit(GPIOx, LED4)==0x00){
				memcpy(tptr,"led_on.gif",10);
				LED4_onoff(ON);
			}
			else{
				memcpy(tptr,"led_of.gif",10);
				LED4_onoff(OFF);
			}
			tptr+=10;
		}
		
		else if((tptr=(u_char*)strstr((char*)ptr,EVB_LED0_STAT)))
		{
			memset(tptr,0x20,7);
                        //Modified by Gang 2011-9-27
			if(GPIO_ReadInputDataBit(GPIOx, LED3)==0x00) memcpy(tptr,"checked",7);
			tptr+=7;
		}			
		else if((tptr=(u_char*)strstr((char*)ptr,EVB_LED1_STAT)))
		{
			memset(tptr,0x20,7);
                        //Modified by Gang 2011-9-27
			if(GPIO_ReadInputDataBit(GPIOx, LED4)==0x00) memcpy(tptr,"checked",7);
			tptr+=7;
		}
		
		else
		{
			return len;
			
		}
	}

	if(!ptr) return len;
	
	return (u_int)(tptr-base);
}

void cgi_ipconfig(void)
{
	u_char * param;
       

	if((param = get_http_param_value(http_request->URI,"sip")))
	{
		inet_addr_((u_char*)param, (netinfo).IP);		
	}
	if((param = get_http_param_value(http_request->URI,"gwip")))
	{
		inet_addr_((u_char*)param, (netinfo).Gateway);	
	}			
	if((param = get_http_param_value(http_request->URI,"sn")))
	{
		inet_addr_((u_char*)param, (netinfo).Subnet);		
	}	
	
        /* Program the network parameters received into STM32F10x Flash */
        //Added by Gang 2011-9-27
        FLASH_Unlock(); 
        
        while(FLASH_ErasePage(FlashDestination)!= FLASH_COMPLETE);
        
        while(FLASH_ProgramWord(FlashDestination, ChipID)!= FLASH_COMPLETE);
        if (*(u32*)FlashDestination != ChipID)
        {
          printf("\r\n ChipID Flash writing error!!!");
        }
        FlashDestination += 4;
        
       
        while(FLASH_ProgramWord(FlashDestination, *(u32*)((netinfo).IP))!= FLASH_COMPLETE);
        if (*(u32*)FlashDestination != *(u32*)((netinfo).IP))
        {
          printf("\r\n IP Flash writing error!!!");
        }
        FlashDestination += 4;
        
       
        while(FLASH_ProgramWord(FlashDestination,  *(u32*)((netinfo).Gateway))!= FLASH_COMPLETE);
         if (*(u32*)FlashDestination != *(u32*)((netinfo).Gateway))
        {
          printf("\r\n G/W Flash writing error!!!");
        }
        FlashDestination += 4;
        
       
        while(FLASH_ProgramWord(FlashDestination,  *(u32*)((netinfo).Subnet))!= FLASH_COMPLETE);
        if (*(u32*)FlashDestination != *(u32*)((netinfo).Subnet))
        {
          printf("\r\n SubNet Flash writing error!!!");
        }
        
        

}

void InitNetInfo(void)
{
      (netinfo).Mac[0] = 0x00;
      (netinfo).Mac[1] = 0x08;
      (netinfo).Mac[2] = 0xDC;
      (netinfo).Mac[3] = 0x11;
      (netinfo).Mac[4] = 0x22;
      (netinfo).Mac[5] = 0x33;
     
     
      (netinfo).Gateway[0] = 192;
      (netinfo).Gateway[1] = 168;
      (netinfo).Gateway[2] = 0;
      (netinfo).Gateway[3] = 1;
      
      (netinfo).IP[0] = 192;
      (netinfo).IP[1] = 168;
      (netinfo).IP[2] = 0;
      (netinfo).IP[3] = 100;
      
      (netinfo).Subnet[0] = 255;
      (netinfo).Subnet[1] = 255;
      (netinfo).Subnet[2] = 255;
      (netinfo).Subnet[3] = 0;
      
      SetNetInfo(&netinfo);
}

void DisplayNetInfo(void)
{
        GetNetInfo(&disnetinfo);
        
	printf("============================================\r\n");
	printf("  HTTP Client Net Config Information\r\n");
	printf("============================================\r\n");
        
	printf("MAC ADDRESS      : \r\n"); //display MAC addr
	printf("%02x.%02x.%02x.%02x.%02x.%02x\r\n",(disnetinfo).Mac[0],(disnetinfo).Mac[1],(disnetinfo).Mac[2],(disnetinfo).Mac[3],(disnetinfo).Mac[4],(disnetinfo).Mac[5]);	

	printf("SUBNET MASK      : \r\n"); //display subnet
	printf("%d.%d.%d.%d\r\n",(disnetinfo).Subnet[0],(disnetinfo).Subnet[1],(disnetinfo).Subnet[2],(disnetinfo).Subnet[3]);

	printf("G/W IP ADDRESS   : \r\n"); //display W5200's G/W
	printf("%d.%d.%d.%d\r\n",(disnetinfo).Gateway[0],(disnetinfo).Gateway[1],(disnetinfo).Gateway[2],(disnetinfo).Gateway[3]);

	printf("LOCAL IP ADDRESS : \r\n"); //display W5200's IP addr
	printf("%d.%d.%d.%d\r\n",(disnetinfo).IP[0],(disnetinfo).IP[1],(disnetinfo).IP[2],(disnetinfo).IP[3]);
        
	printf("============================================\r\n");

}




