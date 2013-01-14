
//#define FILE_LOG_SILENCE
#include "protocol/FTP/ftpc.h"

/*
extern uint8	IP[4];
extern uint8	FTP_SERVER_IP[4];
extern uint8	FTP_CLIENT_PI_SOCK;
extern uint8 	FTP_CLIENT_DTP_SOCK;

static uint8	bRunDTP;
static uint8	bRunPutFile;

static uint32   DTP_Port = FTP_Client_TCPS_Port;


uint8		cmd_buf[TX_RX_MAX_BUF_SIZE];
uint8		data_buf[TX_RX_MAX_BUF_SIZE];

pFunction	Jump_To_Application;
unsigned long	JumpAddress;



void ftp_client_PI (uint8 sock)
{
        int   ret;
        int   idx;
	char  msg[STR_SIZE];
	uint16 status;
	static uint16 any_port = 50000;
	
        memset(cmd_buf, 0, sizeof(cmd_buf));
        ret = TCPRecv(sock, cmd_buf, TX_RX_MAX_BUF_SIZE);

        if(ret > 0)							// Received
	{
        	for( idx=0; idx < ret; idx++ )
                {
			if( idx == 0 )
				printf("\r\n");
                 
			printf("%c",cmd_buf[idx]);
                }
          
                switch( status = Parse_FTPServer_Statuscode(cmd_buf) )
		{
			case R_220:					// Service ready for new user.
                                printf("\r\nInput your User ID > ");
				GetInputString(msg);
                                Send_USER_ID(sock,msg);
				break;
			
				
			case R_331:					// User name okay, need password.
				printf("\r\nInput your Password > ");
				GetInputString(msg);
				Send_PW(sock,msg);
				break;
			

			case R_230:					// User logged in, proceed
			  	printf("\r\nUser logged in, proceed");
			  	TCPClose(FTP_CLIENT_DTP_SOCK);
				Send_Port(sock, IP, DTP_Port);
				break;


			case R_200:					// 'Generic' command ok
				printf("\r\nInput FTP CMD > ");
				GetInputString(msg);
				Send_cmd(sock, msg);
				break;
			
				
			case R_150:					// File Status ok: opening data conn
				bRunDTP = 1;
				break;
			  	
				
			case R_250:
				TCPClose(FTP_CLIENT_DTP_SOCK);
				Send_Port(sock, IP, DTP_Port);
				break;
				
			
			case R_226:					// Closing data connection.  File transfer/abort successful 
				TCPClose(FTP_CLIENT_DTP_SOCK);
				Send_Port(sock, IP, DTP_Port);
				break;
				

			case R_425:
				TCPClose(FTP_CLIENT_DTP_SOCK);
				Send_Port(sock, IP, DTP_Port);
				break;
				
			default:
			  	printf("\r\nDefault Status = %d",(uint16)status);
								
				JumpAddress = *(volatile unsigned long*) (ApplicationAddress + 4);	// reset app
				Jump_To_Application = (pFunction) JumpAddress;
				Jump_To_Application();
				while(1);
		}
	}
	else if(ret == ERROR_CLOSED || ret == ERROR_CLOSE_WAIT )
	{
		  printf("\r\n%d : FTP Client Start.\r\n",(uint16)sock);
		  if( TCPClientOpen( sock, any_port++, FTP_SERVER_IP, FTP_Server_CMD_Port ) == 0 )
			  printf("< Fail : Socket Connect >\r\n");
	}
}


void ftp_client_DTP(uint8 sock)
{
	int   len;
        int   idx;
	
	
	switch( GetTCPSocketStatus(sock) )
	{
		case STATUS_ESTABLISHED:
			if(bRunDTP)
			{
				bRunDTP = 0;
#ifdef Debug				
				printf("< FTP_DTP Connect OK>\r\n");
#endif
			}
                        
                        len = TCPRecv(sock, data_buf, TX_RX_MAX_BUF_SIZE);
                        
                        if( len > 0 )
                        {
				for( idx = 0; idx < len; idx++ )
				{
					if( idx == 0 )
						printf("\r\n");
					
					printf("%c",data_buf[idx]);
				}
                        }
			
			if( bRunPutFile )
			{
				bRunPutFile = 0;
				TCPSend(sock, (void *)PUTFILE, strlen(PUTFILE));
				TCPClose(sock);
			}
			break;
			
		case STATUS_CLOSE_WAIT:
                        
			len = TCPRecv(sock, data_buf, TX_RX_MAX_BUF_SIZE);
                        
                        if( len > 0 )
                        {
				for( idx = 0; idx < len; idx++ )
				{
					if( idx == 0 )
						printf("\r\n");
					
					printf("%c",data_buf[idx]);
				}
                        }
			
			if( bRunPutFile )
			{
				bRunPutFile = 0;
				TCPSend(sock, (void *)PUTFILE, strlen(PUTFILE));
			}

			TCPClose(sock);
			break;
			
		case STATUS_CLOSED:
			
			TCPClose(sock);
			if( TCPServerOpen( sock, DTP_Port++ ) == 0 )
			{
				printf("\r\nDTP Socket Open Error");
				break;
			}
                        
                        if( DTP_Port > 70000 )
                                DTP_Port = FTP_Client_TCPS_Port;
                        
			bRunDTP = 1;
			break;
	}	
}
	

uint16 Parse_FTPServer_Statuscode(uint8* dat)
{
	uint16 Statuscode =(dat[0]-'0')*100+(dat[1]-'0')*10+(dat[2]-'0');
#ifdef Debug  
	printf("\r\n dat[0]=%x dat[1]=%x dat[2]=%x",dat[0],dat[1],dat[2]);
	printf("\r\n Statuscode= %ld",Statuscode);
#endif
	return Statuscode;
}


void Send_USER_ID(uint8 sock,char* msg)
{
	char dat[STR_SIZE];
	sprintf(dat,"USER %s\r\n",msg);
	TCPSend(sock,(uint8*)dat,strlen(dat));
}


void Send_PW(uint8 sock,char* msg)
{
 	char dat[STR_SIZE];
	sprintf(dat,"PASS %s\r\n",msg);
	TCPSend(sock,(uint8*)dat,strlen(dat));
}


void Send_Port(uint8 sock, uint8* ip, uint32 port)
{
	char 	dat[STR_SIZE];
	uint8	h_val, l_val;

	h_val = port/256;
	l_val = port%256;
	sprintf(dat,"PORT %d,%d,%d,%d,%d,%d\r\n",ip[0],ip[1],ip[2],ip[3],h_val,l_val);
	TCPSend(sock,(uint8*)dat,strlen(dat));
}


void Send_cmd(uint8 sock, char* msg)
{
	uint16	len;
	char	new_msg[STR_SIZE];
	char    dat[STR_SIZE];


	if( strstr(msg,"ls") != NULL )
	{
		TCPSend(sock,ls,strlen(ls));
	}
	else if( strstr(msg,"get") != NULL )
	{
		memset(dat, 0, sizeof(dat));
		memset(new_msg, 0, sizeof(new_msg));
		memcpy(new_msg, msg+4, strlen(msg+4));
		len = sprintf(dat, "RETR %s\r\n",new_msg);
		TCPSend(sock, (uint8*)dat, len);
		
	}
	else if( strstr(msg, "put") != NULL )
	{
		memset(dat, 0, sizeof(dat));
		memset(new_msg, 0, sizeof(new_msg));
		memcpy(new_msg, msg+4, strlen(msg+4));
		len = sprintf(dat, "STOR %s\r\n",new_msg);
		TCPSend(sock, (uint8*)dat, len);
                bRunPutFile = 1;
                
	}
	else if( strstr(msg, "cd") != NULL )
	{
		memset(dat, 0, sizeof(dat));
		memset(new_msg, 0, sizeof(new_msg));
		memcpy(new_msg, msg+3, strlen(msg+3));
		len = sprintf(dat, "CWD %s\r\n", new_msg);
		TCPSend(sock, (uint8*)dat, len);
	}
	else if( strstr(msg, "bye") != NULL )
	{
		TCPSend(sock, bye, strlen(bye) );
		TCPClose(FTP_CLIENT_DTP_SOCK);
		TCPClose(FTP_CLIENT_PI_SOCK);
		JumpAddress = *(volatile unsigned long*) (ApplicationAddress + 4);
		Jump_To_Application = (pFunction) JumpAddress;
		Jump_To_Application();
		while(1);
	}
	else
	{
		printf("\r\n Sorry, this FTP client only support ls, put and get commands!");
		TCPClose(FTP_CLIENT_DTP_SOCK);
		Send_Port(sock, IP, DTP_Port);
	}
}

*/

