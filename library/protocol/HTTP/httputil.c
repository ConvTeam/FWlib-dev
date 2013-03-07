/**
 * @file		httputil.c
 * @brief		
 * @version	1.0
 * @date		2013/
 * @par Revision
 *			2013/ - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#include "protocol/HTTP/httputil.h"
#include "webpage.h"
#include "romfile.h"

#define DEFAULT_HTTP_PORT 80
extern char rx_buf[MAX_URI_SIZE];
extern char tx_buf[MAX_URI_SIZE];
extern uint8 BUFPUB[1024];

uint8 *homepage_default = "/ipconfig.htm";


void mid(char* src, char* s1, char* s2, char* sub)
{
	char* sub1;
	char* sub2;
	uint16 n;

	sub1=strstr((char*)src,(char*)s1);
	sub1+=strlen((char*)s1);
	sub2=strstr((char*)sub1,(char*)s2);

	n=sub2-sub1;
	strncpy((char*)sub,(char*)sub1,n);
	sub[n]='\0';
}

//processing http protocol , and excuting the followed fuction.
void WebServer(uint8 s)
{
	int ret;
	uint32 header_len=0, content_len=0, received_len=0;
	char sub[10];

	/* http service start */
	ret = TCPRecv(s, (int8*)rx_buf, MAX_URI_SIZE);

	if(ret > 0){					// If Received..
		*(((uint8*)rx_buf)+ret) = '\0';

		if(strstr(rx_buf, "Content-Length: ")){
			mid((char*)rx_buf, "Content-Length: ", "\r\n", sub);
			content_len=atoi(sub);
			header_len = (uint32)(strstr(rx_buf, "\r\n\r\n") - rx_buf + 4);

			received_len = ret;
			while(received_len!=(content_len+header_len))
			{
				ret = TCPRecv(s, (int8*)rx_buf+received_len, MAX_URI_SIZE);
				received_len+=ret;
			}

			*(((uint8*)rx_buf)+received_len) = '\0';
		}

		HTTPProcessor(s, (char*)rx_buf);	// request is processed
		memset(rx_buf,0x00,MAX_URI_SIZE);

		IINCHIP_WRITE(Sn_CR(s),Sn_CR_DISCON);
		while( IINCHIP_READ(Sn_CR(s)) ) ;

		//TCPClose(s);

	} else if(ret == SOCKERR_NOT_TCP){		// Not TCP Socket, It's UDP Socket
		DBG("UDP Socket Close");
		UDPClose(s);

	} else if(ret == SOCKERR_CLOSED){			// Socket Closed
		LOGA("HTTP Server Started - ch(%d)",(uint16)s);
		TCPServerOpen(s, DEFAULT_HTTP_PORT);
	}

	if(GetTCPSocketStatus(s) == SOCKERR_CLOSE_WAIT){// Close waiting
		TCPClose(s);
	}
}

/**
@brief	This function used to send the data in TCP mode
@return	1 for success else 0.
*/ 
struct CGI_CALLBACK cgi_callback[MAX_CGI_CALLBACK];
void cgi_callback_add(char *tokken, cgi_func get_func, cgi_func set_func)
{
	static uint16 total=0;
	int len;

	if(tokken == NULL) {
		ERR("description string is NULL");
		return;

	} else if(total >= MAX_CGI_CALLBACK) {
		ERR("not enough space");
		return;
	}

	len = strlen(tokken);
	cgi_callback[total].tokken = malloc(len+1);
	strcpy(cgi_callback[total].tokken, tokken);
	cgi_callback[total].get_func = get_func;
	cgi_callback[total].set_func = set_func;
	total++;
}

int32 HTTPSend(uint8 s, char *src, char *dest, uint16 len, uint8 mode)
{
	int32 ret=0;
	char *oldtmp=0, *newtmp=0;
	char sub[32];
	uint16 i, mlen=0;
	char *tmp = (char*)BUFPUB;

	oldtmp=src;
	newtmp=oldtmp;
	while((newtmp = strstr(oldtmp, "<="))){
		if(mode == 0)
			ret += TCPSend(s, (int8*)oldtmp, (uint16)(newtmp-oldtmp));
		else if(mode == 1)
		{
			strncat(dest, oldtmp, (uint16)(newtmp-oldtmp));
			ret += (uint16)(newtmp-oldtmp);
		}
		else if(mode == 2)
			ret += (uint16)(newtmp-oldtmp);

		mid(newtmp, "<=", ">", sub);// mid 함수의 리턴값에 따라 "<=" 까지만 읽었고, ">" 까지 읽지 않았는지 판단한 후 다음 파일을 읽고나서 처리 하도록 구현해야 함.
		for(i=0; i<MAX_CGI_CALLBACK; i++){
			if(!strcmp(sub, cgi_callback[i].tokken)){
				if(cgi_callback[i].get_func == NULL){
					i = MAX_CGI_CALLBACK;
					break;
				}

				cgi_callback[i].get_func(tmp, &mlen);

				if(mode == 0)
					ret += TCPSend(s, (int8*)tmp, mlen);
				else if(mode == 1)
				{
					strncat(dest, tmp, mlen);
					ret += mlen;
				}
				else if(mode == 2)
					ret += mlen;
				break;
			}
		}
		if(i==MAX_CGI_CALLBACK){
			mlen = sprintf(tmp, "<=%s>", sub);

			if(mode == 0)
				ret += TCPSend(s, (int8*)tmp, mlen);
			else if(mode == 1)
			{
				strncat(dest, tmp, mlen);
				ret += mlen;
			}
			else if(mode == 2)
				ret += mlen;
		}
		oldtmp = newtmp + strlen(sub)+3;
	}

	if(mode == 0)
		ret += TCPSend(s, (int8*)oldtmp, (len-(uint16)(oldtmp-src)));
	else if(mode == 1)
	{
		strncat(dest, oldtmp, (len-(uint16)(oldtmp-src)));
		ret += (len-(uint16)(oldtmp-src));
	}
	else if(mode == 2)
		ret += (len-(uint16)(oldtmp-src));

	return ret;
}

void FILESend(uint8 s, st_http_request *http_request, char* buf)
{
	uint32 file_len=0, file_len_tmp=0;
	uint32 send_len=0, content_len=0;
	uint32 content = 0;
	char* name=(char*)http_request->URI;
	if(strcmp(name,"/"))
		name++;

	/* Search the specified file in stored binaray html image */
	if(!search_file_rom((unsigned char *)name, &content, &file_len))
	{
		memcpy(buf, ERROR_HTML_PAGE, sizeof(ERROR_HTML_PAGE));
		TCPSend(s, (int8*)buf, strlen((char const*)buf));
	}
	else
	{
		file_len_tmp = file_len;
		send_len=0;
		while(file_len_tmp)
		{
			if(file_len_tmp>1024)
			{
				read_from_flashbuf(content+send_len, (uint8*)buf, 1024);

				// Replace html's system environment value to real value and check size
				content_len += HTTPSend(NULL, buf, NULL, 1024, 2);

				send_len+=1024;
				file_len_tmp-=1024;
			}
			else
			{
				read_from_flashbuf(content+send_len, (uint8*)buf, file_len_tmp);
				buf[file_len_tmp] = '\0';

				// Replace html's system environment value to real value and check size
				content_len += HTTPSend(NULL, buf, NULL, file_len_tmp, 2);

				send_len+=file_len_tmp;
				file_len_tmp-=file_len_tmp;
			}
		}
		make_http_response_head((unsigned char*)buf, http_request->TYPE, content_len);
		TCPSend(s, (int8*)buf, strlen((char const*)buf));

		send_len=0;
		while(file_len)
		{
			if(file_len>1024)
			{
				read_from_flashbuf(content+send_len, (uint8*)buf, 1024);

				// Replace html's system environment value to real value and send
				if(HTTPSend(s, buf, NULL, 1024, 0)<0)
				{
					return;
				}

				send_len+=1024;
				file_len-=1024;
			}
			else
			{
				read_from_flashbuf(content+send_len, (uint8*)buf, file_len);
				buf[file_len] = '\0';

				// Replace html's system environment value to real value and send
				HTTPSend(s, buf, NULL, (uint16)file_len, 0);

				send_len+=file_len;
				file_len-=file_len;
			}
		}
	}
}

void HTTPProcessor(uint8 s, char * buf)
{
	uint8* http_response;
	st_http_request *http_request;

	http_response = (uint8*)rx_buf;
	http_request = (st_http_request*)tx_buf;

	memset(tx_buf,0x00,MAX_URI_SIZE);
	parse_http_request(http_request, buf);		// After analyze request, convert into http_request
	memset(rx_buf,0x00,MAX_URI_SIZE);

	//method Analyze
	switch (http_request->METHOD)				
	{
		case METHOD_ERR :	
			memcpy(http_response, ERROR_REQUEST_PAGE, sizeof(ERROR_REQUEST_PAGE));
			TCPSend(s, (int8 *)http_response, strlen((char*)http_response));
			break;
		case METHOD_HEAD:
		case METHOD_GET:
		case METHOD_POST:
			if (!strcmp((char*)http_request->URI, "/")) strcpy(http_request->URI, (char const*)homepage_default);	// If URI is "/", respond by index.htm 

			RESTProcessor(http_request);

			//get http type from type
			find_http_uri_type(&http_request->TYPE, http_request->URI);	//Check file type (HTML, TEXT, GIF, JPEG are included)

			if(http_request->TYPE == PTYPE_PL || http_request->TYPE == PTYPE_CGI)
			{
				CGIProcessor(http_request, (char*)http_response);
			}

			FILESend(s, http_request, (char*)http_response);

			break;

		default:
			break;
	}
}

void RESTProcessor(st_http_request *http_request)
{
	return;
}

void CGIProcessor(st_http_request *http_request, char* buf)
{
	uint32 file_len=0;
	uint32 content = 0;
	char *oldtmp=0, *newtmp=0;
	char sub[32], *subtmp=0;
	uint16 i;
	char* name=(char*)http_request->URI;
	if(strcmp(name,"/"))
		name++;

	/* Search the specified file in stored binaray html image */
	if(search_file_rom((unsigned char *)name, &content, &file_len))
	{
		if(file_len > 1024)// cgi파일은 최대 1024 Bytes 까지 제한
			return;

		read_from_flashbuf(content, (uint8*)buf, file_len);
		buf[file_len] = '\0';
                HTTPSend(NULL, buf, (char*)BUFPUB, file_len, 1);

		oldtmp=(char*)BUFPUB;
		newtmp=oldtmp;
		while((newtmp = strstr(oldtmp, "<?"))){
			mid(newtmp, "<?", "?>", sub);
			oldtmp = newtmp + strlen(sub) + 4;

			subtmp = strstr(sub, "SetValue");
			mid(subtmp, "(", ")", sub);

			for(i=0; i<MAX_CGI_CALLBACK; i++){
				if(!strcmp(sub, cgi_callback[i].tokken)){
					char *tmp;
					uint16 len;

					if(cgi_callback[i].set_func == NULL){
						i = MAX_CGI_CALLBACK;
						break;
					}

					tmp = (char*)get_http_param_value(http_request->param,cgi_callback[i].tokken);
					len = strlen(tmp);

					cgi_callback[i].set_func(tmp, &len);
					break;
				}
			}
			if(i==MAX_CGI_CALLBACK){
			}
		}
	}
}
