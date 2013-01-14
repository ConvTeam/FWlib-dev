
//#define FILE_LOG_SILENCE
#include "protocol/HTTP/https.h"

/*
static char tempURI[MAX_URI_SIZE];
extern char  homepage_default[];


// @brief	convert escape characters(%XX) to ascii charater 
 
char C2D(uint8 c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 10 + c -'a';
	if (c >= 'A' && c <= 'F')
		return 10 + c -'A';

	return (char)c;
}

void unescape_http_url(
	char * url	//< pointer to be conveted ( escape characters )
	)
{
	int x, y;

	for (x = 0, y = 0; url[y]; ++x, ++y) {
		if ((url[x] = url[y]) == '%') {
			url[x] = C2D(url[y+1])*0x10+C2D(url[y+2]);
			y+=2;
		}
	}
	url[x] = '\0';
}


// @brief	make reponse header such as html, gif, jpeg,etc.
 
void make_http_response_head(
	unsigned char * buf, 	//< pointer to response header to be made
	char type, 	//< response type
	uint32 len	//< size of response header
	)
{
	char * head;
	char tmp[10];

			
	//  file type
	if 	(type == PTYPE_HTML) head = RES_HTMLHEAD_OK;
	else if (type == PTYPE_GIF)	head = RES_GIFHEAD_OK;
	else if (type == PTYPE_TEXT)	head = RES_TEXTHEAD_OK;
	else if (type == PTYPE_JPEG)	head = RES_JPEGHEAD_OK;
	else if (type == PTYPE_FLASH)	head = RES_FLASHHEAD_OK;
	else if (type == PTYPE_MPEG)	head = RES_MPEGHEAD_OK;
	else if (type == PTYPE_PDF)	head = RES_PDFHEAD_OK;
#ifdef HTTPD_DEBUG	
	else	PRINTLN("\r\n\r\n-MAKE HEAD UNKNOWN-\r\n");
#endif	

	sprintf(tmp,"%ld", len);	
	strcpy((char*)buf, head);
	strcat((char*)buf, tmp);
	strcat((char*)buf, "\r\n\r\n");
}


// @brief	find MIME type of a file
 
void find_http_uri_type(
	uint8 * type, 	//< type to be returned
	char * buf		//< file name
	) 
{
	// Decide type according to extention
	if 	(strstr(buf, ".htm"))				*type = PTYPE_HTML;
	else if (strstr(buf, ".gif"))				*type = PTYPE_GIF;
	else if (strstr(buf, ".text") || strstr(buf,".txt"))	*type = PTYPE_TEXT;
	else if (strstr(buf, ".jpeg") || strstr(buf,".jpg"))	*type = PTYPE_JPEG;
	else if (strstr(buf, ".swf")) 				*type = PTYPE_FLASH;
	else if (strstr(buf, ".mpeg") || strstr(buf,".mpg"))	*type = PTYPE_MPEG;
	else if (strstr(buf, ".pdf")) 				*type = PTYPE_PDF;
	else if (strstr(buf, ".cgi") || strstr(buf,".CGI"))	*type = PTYPE_CGI;
	else if (strstr(buf, ".js") || strstr(buf,".JS"))	*type = PTYPE_TEXT;	
	else if (strstr(buf, ".xml") || strstr(buf,".XML"))	*type = PTYPE_HTML;		
	else 							*type = PTYPE_ERR;
}


// @brief	parse http request from a peer
 
void parse_http_request(
	st_http_request * request, 	//< request to be returned
	uint8 * buf				//< pointer to be parsed
	)
{
	char * nexttok;
	nexttok = strtok((char*)buf," ");
	if(!nexttok)
	{
		request->METHOD = METHOD_ERR;
		return;
	}
	if 	(!strcmp(nexttok, "GET") || !strcmp(nexttok,"get"))
	{
		request->METHOD = METHOD_GET;
		nexttok = strtok(NULL," ");
#ifdef HTTPD_DEBUG
		PRINTLN("METHOD_GET");
#endif				
	}
	else if (!strcmp(nexttok, "HEAD") || !strcmp(nexttok,"head"))	
	{
		request->METHOD = METHOD_HEAD;
		nexttok = strtok(NULL," ");
#ifdef HTTPD_DEBUG
		PRINTLN("METHOD_HEAD");
#endif				
		
	}
	else if (!strcmp(nexttok, "POST") || !strcmp(nexttok,"post"))
	{
		nexttok = strtok(NULL,"\0");
		request->METHOD = METHOD_POST;
#ifdef HTTPD_DEBUG
		PRINTLN("METHOD_POST");
#endif				
	}
	else
	{
		request->METHOD = METHOD_ERR;
#ifdef HTTPD_DEBUG
		PRINTLN("METHOD_ERR");
#endif				
}	
	
	if(!nexttok)
	{
		request->METHOD = METHOD_ERR;
#ifdef HTTPD_DEBUG
		PRINTLN("METHOD_ERR");
#endif				
		return;
	}
	strcpy(request->URI,nexttok);
#ifdef HTTPD_DEBUG
{
		uint16 i;
		PRINTLN("http_request->URI");
		for(i=0; i < strlen(request->URI);i++)
			PRINT1("%c",request->URI[i]);
		PRINTLN("");
}
#endif					
	
}


// @brief	get next parameter value in the request
 
unsigned char* get_http_param_value(
	char* uri, 
	char* param_name
	)
{
	uint8 * name=0;
	

	if(!uri || !param_name) return 0;
	
	strcpy((char*)tempURI,uri);
	if((name = (uint8*)strstr(tempURI,param_name)))
	{
		name += strlen(param_name) + 1; // strlen(para_name) + strlen("=")
		if((name = (uint8*)strtok((char *)name,"& \r\n\t\0")))
		{
			unescape_http_url((char *)name);
			replacetochar((char *)name,'+',' ');
		}
	}
#ifdef HTTPD_DEBUG
	PRINTLN2("%s=%s",param_name,name);
#endif	
	return name;		
}


unsigned char* get_http_uri_name(char* uri)
{
	uint8* uri_name;
	
	if(!uri) return 0;

	memset (tempURI, 0, MAX_URI_SIZE);	
	strcpy((char*)tempURI,uri);
	
	uri_name = (uint8*)strtok(tempURI," ?");
	
	if(strcmp((char *)uri_name,"/")) uri_name++;

#ifdef HTTPD_DEBUG
	PRINTLN1("uri_name=%s",uri_name);
#endif	

	return uri_name;
}
*/






