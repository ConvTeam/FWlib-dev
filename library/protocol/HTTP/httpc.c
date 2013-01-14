
//#define FILE_LOG_SILENCE
#include "protocol/HTTP/httpc.h"

/*
pFunction Jump_To_Application;
unsigned long JumpAddress;
extern wiz_NetInfo  httpclient_NetInfo;

extern uint8 done_dns;
#define	MAX_BUF_SIZE	1024*4		// Maximum receive buffer size

uint16 wrt_idx = 0;
uint8 flag = 1;

int16 http_client( uint8 s, uint8 *HTTPs_IP, uint8 *url_path, uint8 *url_dn, uint8 * data_buf)
{
	unsigned long len;
	uint8 http_req[1024];

	static uint16 any_port = 5000;	
       
	switch (getSn_SR(s))
	{
		case SOCK_ESTABLISHED:				
                                                
                                                if(flag){
		            			printf("< Connect OK >\r\n");
		            			
                                                sprintf((char*)http_req, "GET %s HTTP/1.1\r\nAccept: *"
                                                		"/*\r\nHost: %s\r\nUser-Agent: Mozilla/4.0 \r\n\r\n", url_path, url_dn);
								
                                                                TCPSend(s, http_req, strlen((char *)http_req));
                                                                
                                                               
		           				flag = 0;							
						}
		         
						if ((len = getSn_RX_RSR(s)) > 0){

                                                                if(wrt_idx>= TX_RX_MAX_BUF_SIZE)
                                                                      {
                                                                                            
                                                                                   JumpAddress = *(volatile unsigned long*) (ApplicationAddress + 4); //reset app 
                                                                                   Jump_To_Application = (pFunction) JumpAddress;
                                                                                   Jump_To_Application();
                                                                                   while(1);
                                                                      }	
                                                               
								if (len > MAX_BUF_SIZE) len = MAX_BUF_SIZE;
								len = TCPRecv(s, (data_buf+wrt_idx) , len);
                                                                wrt_idx += len;

						}
					
					
						if(strstr((char const*)data_buf, "</html>")){
                                                                TCPClose(s);
						                printf("< Disconnect >\r\n");
								return (wrt_idx);
						}
                                                
                                              
			break;
		
		 case SOCK_CLOSE_WAIT:                           		
						
                                                //Modified by Gang 2011-10-04
                                                if ((len = getSn_RX_RSR(s)) > 0){
                                                               
							if(wrt_idx>= TX_RX_MAX_BUF_SIZE)
                                                        {
                                                                             
                                                                     JumpAddress = *(volatile unsigned long*) (ApplicationAddress + 4); //reset app 
	                                                             Jump_To_Application = (pFunction) JumpAddress;
	                                                             Jump_To_Application();
                                                                     while(1);
                                                        }	
                                                        
                                                        if (len > MAX_BUF_SIZE) len = MAX_BUF_SIZE;
								len = TCPRecv(s, (data_buf+wrt_idx) , len);
                                                                wrt_idx += len;
                                                                  
                                                                 
						}
						
				              
					
						if(strstr((char const*)data_buf, "</html>")){
                                                                TCPClose(s);
						                printf("< Disconnect >\r\n");
								return (wrt_idx);
						}	
                                                
                                                

						
				break;
				
		 case SOCK_CLOSED:                                            
		     	//close(s);                           
		     //if ( (socket(s,Sn_MR_TCP,any_port++, 0) )!=1 ){
						//printf("< Fail : socket > \r\n");
		      //}else{
				  	//printf("\r\n<socket init OK! >\r\n");		
					//}
					//
			 //break;
			//	
		 //case SOCK_INIT:	
			//
                        //if( (connect(s,(uint8*)HTTPs_IP, 80) )!= 1 ){
					//printf("< Fail : connect >\r\n");
				//}
                        ////Modified by Gang 2011-10-06
                        //else 
                        //{
                        //  flag=1; 
                        //  wrt_idx = 0;
                        //  memset(data_buf,0,sizeof(data_buf));
                        //}
                   if (TCPClientOpen(s,any_port++,(uint8*)httpclient_NetInfo.HTTPs_IP,80)==1)
                   {
                      flag=1; 
                      wrt_idx = 0;
                      memset(data_buf,0,sizeof(data_buf));
                   }
                   
		   break;
	
	}
		
		
	return 0;	
}
*/

