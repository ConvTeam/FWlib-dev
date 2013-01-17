
#include "httputil.h"
#include "webpage.h"

#define DEFAULT_HTTP_PORT 80
extern char rx_buf[MAX_URI_SIZE];
extern char tx_buf[MAX_URI_SIZE];

uint8 *homepage_default = "/config.html";
uint8 bchannel_start;
uint8 *user_data;

//timer
extern vu16 cgi_post_wait_time;
extern vu8 http_time;
//
extern uint32 totalseconds;
//


extern uint16 ka_interval_val;
extern uint16 ka_count_val;

extern uint16 nagle_val;
extern uint16 inact_val;
extern uint16 recon_val;

extern uint16 lport_val;
extern uint16 rport_val;
extern uint16 locport_val;
//processing http protocol , and excuting the followed fuction.
void WebServer(SOCKET s)
{
	int ret;

	/* http service start */
	ret = TCPRecv(s, (uint8*)rx_buf, MAX_URI_SIZE);

	if(ret > 0){					// If Received..
		*(((uint8*)rx_buf)+ret) = 0;

		HTTPProcessor(s, (char*)rx_buf);		// request is processed
		memset(rx_buf,0x00,ret);

		IINCHIP_WRITE(Sn_CR(s),Sn_CR_DISCON);
		while( IINCHIP_READ(Sn_CR(s)) );	// wait to process the command...

	} else if(ret == ERROR_NOT_TCP_SOCKET){		// Not TCP Socket, It's UDP Socket
		DBG("UDP Socket Close");
		UDPClose(s);

	} else if(ret == ERROR_CLOSED){			// Socket Closed
		LOGA("HTTP Server Started - ch(%d)",(uint16)s);
		TCPServerOpen(s, DEFAULT_HTTP_PORT);
	}

	if(GetTCPSocketStatus(s) == STATUS_CLOSE_WAIT){// Close waiting
		TCPClose(s);
	}
}


void HTTPProcessor(SOCKET s, char * buf)
{
	char* name;
	uint8 type;
	int32 file_len=0;
	int32 send_len=0;

	uint8* http_response;
	st_http_request *http_request;

	memset(tx_buf,0x00,MAX_URI_SIZE);
	http_response = (uint8*)rx_buf;

	http_request = (st_http_request*)tx_buf;

	parse_http_request(http_request, buf);		// After analyze request, convert into http_request

	//method Analyze
	switch (http_request->METHOD)				
	{
		case METHOD_ERR :	
			memcpy(http_response, ERROR_REQUEST_PAGE, sizeof(ERROR_REQUEST_PAGE));
			TCPSend(s, (uint8 *)http_response, strlen((char*)http_response));
			break;
		case METHOD_HEAD:
		case METHOD_GET:
		case METHOD_POST:
			//get file name from uri
			name=(char*)http_request->URI;

			if (!strcmp(name, "/")) strcpy(name, (char const*)homepage_default);	// If URI is "/", respond by index.htm 

			RESTProcessor(http_request);

			//get http type from type
			find_http_uri_type(&http_request->TYPE, name);	//Check file type (HTML, TEXT, GIF, JPEG are included)
			type=http_request->TYPE;

			if(http_request->TYPE == PTYPE_PL || http_request->TYPE == PTYPE_CGI)
			{
				file_len = CGIProcessor(name, tx_buf);
			}

			if(file_len >= 0)
			{
				make_http_response_head((unsigned char*)http_response, type, file_len);
				TCPSend(s, http_response, strlen((char const*)http_response));
				send_len=0;
				while(file_len)
				{
					if(file_len>1024)
					{
						if(TCPSend(s, (uint8*)tx_buf+send_len, 1024)<0)
						{
							return;
						}
						TCPSend(s, (uint8*)tx_buf+send_len, 1024);
						send_len+=1024;
						file_len-=1024;
					}
					else
					{
						TCPSend(s, (uint8*)tx_buf+send_len, file_len);
						send_len+=file_len;
						file_len-=file_len;
					}
				}
			}
			else
			{
				if(strcmp(name,"/config.html")==0)
				{
					file_len=strlen(CONFIG_HTML);
					make_http_response_head((unsigned char*)http_response, type, file_len);
					TCPSend(s, http_response, strlen((char const*)http_response));
					send_len=0;
					while(file_len)
					{
						if(file_len>1024)
						{
							if(TCPSend(s, (uint8*)CONFIG_HTML+send_len, 1024)<0)
							{
								return;
							}
							send_len+=1024;
							file_len-=1024;
						}
						else
						{
							TCPSend(s, (uint8*)CONFIG_HTML+send_len, file_len);
							send_len+=file_len;
							file_len-=file_len;
						}
					}
				}
				else
				{
					memcpy(http_response, ERROR_HTML_PAGE, sizeof(ERROR_HTML_PAGE));
					TCPSend(s, http_response, strlen((char const*)http_response));
				}
			}
			break;

			default :
			break;
	}
}

void RESTProcessor(st_http_request *http_request)
{
	return;
}

int32 CGIProcessor(char* name, char* buf)
{
	int32 file_len=0;
#if 1
	if(strstr(name,"/widget.pl"))//widget response
	{
		memset(buf,0,MAX_URI_SIZE);
		//                                        devicname      serialnumber                  mac address                  local ip address       local port     gateway ip address         subnet mask          dns server ipaddress    remote host ip         remote port     locating server        loc port       ka interval    ka count        ntp server ip addr         manual input datetime                 nagle         inactivity    reconnection    dhcp         rs232       mode                 baudrate    databit    parity    stopbit     flow      timezon
		sprintf(buf,"WidgetCallback({\"txt\":[{\"v\":\"%s\"},{\"v\":\"%s\"},{\"v\":\"%02X:%02X:%02X:%02X:%02X:%02X\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d\"},{\"v\":\"%d\"},{\"v\":\"%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d-%02d-%02d %02d:%02d:%02d\"},{\"v\":\"%d\"},{\"v\":\"%d\"},{\"v\":\"%d\"}],\"dhcp\":%d,\"rs232\":%d,\"mode\":%d,\"sel\":[{\"v\":%d},{\"v\":%d},{\"v\":%d},{\"v\":%d},{\"v\":%d},{\"v\":%d}],\"loc\":%d,\"ka\":%d,\"ntp\":%d,\"pwd\":\"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\"});",
                                           "W7200_EVB", "iMCU7200EVB", 0x00, 0x08, 0xDC, 0x11, 0x22, 0x33, 192, 168, 10, 137, 80, 191, 168, 10, 1, 255, 255, 255, 0, 168, 126, 63, 1, 192, 168, 10, 237, 8080, 
                                           0,0,0,0,0,0,0,
                                           0,0,0,0,2013,1,17,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0); 

		file_len=strlen(buf);
	}
	else
	{
		file_len = -1;
	}
#else
	uint32 content = 0;

	/* Search the specified file in stored binaray html image */
	if(!search_file_rom((unsigned char *)name, &content, &file_len))
	{
		file_len = -1;
	} 
	else	// if search file sucess 
	{
		read_from_flashbuf(content, buf, file_len);
		*(buf+file_len+1) = '\0';

		// Replace htmls' system environment value to real value
		file_len = replace_sys_env_value(http_response,file_len);
	}
#endif
	return file_len;
}
