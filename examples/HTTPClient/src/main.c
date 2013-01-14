/********************************************************************
*	2011 WIZnet technology all rights reserved
*	HTTP Client Application Note code for W5200E01-M3
*	--------------------------------------------------
*	Ver 0.9 Release Sep. 2011	First release						
********************************************************************/

#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_flash.h"
#include "stdio.h"
#include "config.h"
#include "common/util.h"
#include "common/sockutil.h"
#include "device/w5200.h"
#include "device/socket.h"
#include "host/wizspi.h"
#include "protocols/HTTP/http_client.h"
#include "protocols/DNS/dns.h"
#include <string.h>
#include <stdlib.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

// Configuration Network Information of W5200

wiz_NetInfo  httpclient_NetInfo;
wiz_NetInfo  display_NetInfo;



uint8 ch_status[MAX_SOCK_NUM] = { 0, };	/** 0:close, 1:ready, 2:connected */


static char tx_buf[TX_RX_MAX_BUF_SIZE];


#define TX_BUF	tx_buf
#define RX_BUF	rx_buf

uint8 * data_buf = (uint8*) TX_BUF;	// Position of receive buffer

uint16 READ_SN_IR =0;

__IO uint32_t Timer2_Counter;

/* Network parameters---------------------------------------------------------*/
#define DNS_SOCK 1
#define HTTPC_SOCK 0



/* HTTPc----------------------------------------------------------------------*/

#define MAX_URL_SIZE                            128

uint8 str_start[6]= "<html>";
uint8 str_end[7]= "</html>";
uint8 done_dns = 0;


/* Private function prototypes -----------------------------------------------*/
void RCC_Configuration(void);
void NVIC_Configuration(void);
void GPIO_Configuration(void);
void Timer_Configuration(void);
extern void EXTI_IMR_EMR_enable(void);

void InitNetInfo(void);
void DisplayNetInfo(void);


/* Private functions ---------------------------------------------------------*/

void Timer2_ISR(void) // Timer2 interrupt service routine
{
	if (Timer2_Counter++ > 1000) { // 1m x 1000 = 1sec
		Timer2_Counter = 0;		
		       
	}
	
}

uint16 pased_idx(uint8 * st_buf, uint16  size, uint16 data_len){
	uint16 i,j,idx;
	idx = 0; i=0;

			do{
				idx = 0;
				for(j=0; j<size; j++){					
					if(data_buf[i+j] == st_buf[j] ){
					  idx++;		
					}				
				}
				i++;
				if(i>data_len)
					break;
			}while( idx != (size) );
			
			idx  = i-1;					
			return idx;
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
	uint16 i; 
        RCC_Configuration(); // Configure the system clocks
	NVIC_Configuration(); // NVIC Configuration
	GPIO_Configuration();
	USART1_Init(115200);
	Timer_Configuration();
        
        printf("============================================\r\n");
	printf("   HTTP client for W5200 \r\n");
	printf("============================================\r\n");
       
	wizspi_init(WIZ_SPI_1);	
        Reset_W5200();
        wizInit();
        InitNetInfo();
        DisplayNetInfo();
	              
        // LED3 and LED4 On!
	LED3_onoff(ON);
	LED4_onoff(ON);
     
       
        uint8  url[MAX_URL_SIZE];			//full url
	uint8  url_dn[MAX_URL_SIZE];			//domain name url
	uint8  url_path[MAX_URL_SIZE];		//local path in html server
	char * split;				//string split pointer 
	
	
	uint16 done_http = 0;
	uint16 start_idx = 0 ;
	uint16 end_idx = 0;
	uint16 tmp_start;
	uint16 tmp_end;
	uint8 BR [4]= "<BR>";
	uint8 rBR [4]= "\r\n";
	uint8 TITLE [7]= "<TITLE>";
	uint8 bTITLE [8]= "</TITLE>";
	

	uint16 tmp_idx;
	uint8 no_pr = 0;
	uint8 str[17];
       
	
            while (1)
            {
                          
              sprintf((char*)str,"%.3d.%.3d.%.3d.%.3d ",
				IINCHIP_READ (SIPR0+0), IINCHIP_READ (SIPR0+1), 
				IINCHIP_READ (SIPR0+2), IINCHIP_READ (SIPR0+3));
                            
                           
                            /* Get Http Address  */
                            printf("\r\n Please enter a HTTP Address without 'http://' \n\r"); 
                            printf("http://");
                            memset(url,0,sizeof(url));	
                            //Modified by Gang 2011-10-04
                            zScanf_s(1, url);
                            printf("\r\n Your HTTP address is: %s \r\n",url);		
                            
                            /* Parse URL Path  */
                            split = strchr((char const*)url,'/');			
                            strcpy((char*)url_path,split); 			
                            printf("Domain path: %s \r\n",url_path);
                    
                            /* Parse URL Domain  */
                            split = strtok((char*)url,"/");			
                            strcpy((char*)url_dn,split);				
                            printf("Domain name: %s \r\n",url_dn);
                            
               
                            /* Do DNS Client */
                            memset(httpclient_NetInfo.HTTPs_IP,0,sizeof(httpclient_NetInfo.HTTPs_IP));
                            //Delay_ms(10);
                            done_dns = dns_query(DNS_SOCK, url_dn, httpclient_NetInfo.HTTPs_IP);
                            printf("\r\n HTTPs_IP= %d.%d.%d.%d",httpclient_NetInfo.HTTPs_IP[0],httpclient_NetInfo.HTTPs_IP[1],httpclient_NetInfo.HTTPs_IP[2],httpclient_NetInfo.HTTPs_IP[3]);
                          
                            while(done_dns) {
                            
                            /* Do HTTP Client */
                            done_http = http_client(HTTPC_SOCK, httpclient_NetInfo.HTTPs_IP, url_path, url_dn,data_buf);
                           
                            if(done_http) { // on success, done_dns is not  '0'
                                            
    #define Recieved_DATA
    #ifdef Recieved_DATA
                                            printf("\r\n<< Recieved Data -- START>> \r\n");
                                           
                                            for(i=0; i<done_http; i++){
                                                  
                                                  printf("%c",(uint8)data_buf[i]);
                                              }   
                                            printf("\r\n");
                                            printf("<< Recieved Data -- END>> \r\n");
                                           
    #endif
                                        /* parsed index */
                                            //All other HTML elements are nested between the opening <html> and </html> tags.
                                            start_idx = pased_idx((uint8 *)str_start , sizeof(str_start), done_http );
                                            end_idx   = pased_idx((uint8 *)str_end , sizeof(str_end), done_http );			
    
                                            /* printf get <html> ...</html> */							
                                            for(i=start_idx; i<(end_idx+7); i++){						
                                                    /* remove header */
                                                    data_buf[i-start_idx] = data_buf[i];
                                             }   
                                            printf("\r\n");
                                            
    
                                            /* replace <br> tag to \r\n */
                                            //The br tag is used for specifying a line break.
                                            do{
                                                    tmp_idx = pased_idx((uint8*)BR, sizeof(BR) , end_idx-start_idx) ;
                                                    if(tmp_idx == 0 ) 
                                                            break;
                                                 
                                                    memcpy((uint8 *)data_buf+tmp_idx, (uint8*)rBR, sizeof(rBR)) ;
                                            }while(tmp_idx!=end_idx-start_idx);
    
    #define Parsed_DATA
    #ifdef Parsed_DATA
                                            /* parsed DATA */
                                            printf("\r\n<< Parsed Data -- START >>");
                                            printf("\r\nTITLE : \r\n");
                                            /* parse <TITLE> and </TITLE> tags */
                                            tmp_start = pased_idx((uint8 *)TITLE , sizeof(TITLE),(end_idx-start_idx) ) + sizeof(TITLE);
                                            tmp_end   = pased_idx((uint8 *)bTITLE , sizeof(bTITLE),(end_idx-start_idx) );												
                                            for(i=tmp_start; i<tmp_end; i++){
                                                   
                                                    printf("%c",(uint8)data_buf[i]); // printf title
                                            }   
    
                                            printf("\r\n BODY : \r\n");
                                            /*DO NOT PRINT TAG COMMAND: between '<' with '>' */
                                            for(i=tmp_end; i<(end_idx-start_idx); i++){
                                                    //Tag command - ex.)<PRE>
                                                    //'<' is a start point of tag command.
                                                    if((uint8)data_buf[i]=='<'){ 
                                                            no_pr = 0;
                                                    }
                                                    //'>' is a end point of Tag command.
                                                    //To avoid in row tags -> ex.) <PRE><H1>						
                                                    if((uint8)data_buf[i]=='>' && (uint8)data_buf[i+1] !='<' ) {
                                                            no_pr = 1;
                                                            i++;
                                                    }					
                                                    if(no_pr){
                                                        Delay_ms(1);    
                                                        printf("%c",(uint8)data_buf[i]);
                                                    }
                                            }   
                                            printf("\r\n<< Parsed Data -- END >>\r\n");
    #endif
                                            /* Init. parameter */
                                            start_idx= 0;
                                            end_idx= 0;				
                                    done_dns = 0;  
                                    break;
    
                            }
                           //done_http
                            //done_dns=0;
                    }//while : done_dns == 1		
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


void InitNetInfo(void)
{
       httpclient_NetInfo.Mac[0] = 0x00;
       httpclient_NetInfo.Mac[1] = 0x08;
       httpclient_NetInfo.Mac[2] = 0xDC;
       httpclient_NetInfo.Mac[3] = 0x11;
       httpclient_NetInfo.Mac[4] = 0x22;
       httpclient_NetInfo.Mac[5] = 0x33;
     
     
       httpclient_NetInfo.Gateway[0] = 192;
       httpclient_NetInfo.Gateway[1] = 168;
       httpclient_NetInfo.Gateway[2] = 0;
       httpclient_NetInfo.Gateway[3] = 1;
      
       httpclient_NetInfo.IP[0] = 192;
       httpclient_NetInfo.IP[1] = 168;
       httpclient_NetInfo.IP[2] = 0;
       httpclient_NetInfo.IP[3] = 100;
      
       httpclient_NetInfo.Subnet[0] = 255;
       httpclient_NetInfo.Subnet[1] = 255;
       httpclient_NetInfo.Subnet[2] = 255;
       httpclient_NetInfo.Subnet[3] = 0;
      
      SetNetInfo(&httpclient_NetInfo);
}

void DisplayNetInfo(void)
{
        GetNetInfo(&display_NetInfo);
        
	printf("============================================\r\n");
	printf("  HTTP Client Net Config Information\r\n");
	printf("============================================\r\n");
        
	printf("MAC ADDRESS      : \r\n"); //display MAC addr
	printf("%02x.%02x.%02x.%02x.%02x.%02x\r\n",display_NetInfo.Mac[0],display_NetInfo.Mac[1],display_NetInfo.Mac[2],display_NetInfo.Mac[3],display_NetInfo.Mac[4],display_NetInfo.Mac[5]);	

	printf("SUBNET MASK      : \r\n"); //display subnet
	printf("%d.%d.%d.%d\r\n",display_NetInfo.Subnet[0],display_NetInfo.Subnet[1],display_NetInfo.Subnet[2],display_NetInfo.Subnet[3]);

	printf("G/W IP ADDRESS   : \r\n"); //display W5200's G/W
	printf("%d.%d.%d.%d\r\n",display_NetInfo.Gateway[0],display_NetInfo.Gateway[1],display_NetInfo.Gateway[2],display_NetInfo.Gateway[3]);

	printf("LOCAL IP ADDRESS : \r\n"); //display W5200's IP addr
	printf("%d.%d.%d.%d\r\n",display_NetInfo.IP[0],display_NetInfo.IP[1],display_NetInfo.IP[2],display_NetInfo.IP[3]);
        
	printf("============================================\r\n");

}








