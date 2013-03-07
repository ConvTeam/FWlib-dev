/**
 * @file		httpd.c
 * @brief		functions associated http processing
 * @version	1.0
 * @date		2013/
 * @par Revision
 *			2013/ - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#include "common/common.h"


#define HTTPD_INCLUDE
#include "protocol/HTTP/httpd.h"
#undef HTTPD_INCLUDE

extern char  homepage_default[];
char tx_buf[MAX_URI_SIZE];
char rx_buf[MAX_URI_SIZE];
uint8 BUFPUB[1024];

/**
@brief	replace the specified character in a string with new character
*/ 
void replacetochar(
	char * str, 		/**< pointer to be replaced */
	char oldchar, 	/**< old character */
	char newchar	/**< new character */
	)
{
	int x;
	for (x = 0; str[x]; x++) 
		if (str[x] == oldchar) str[x] = newchar;	
}
/**
@brief	CONVERT CHAR INTO HEX
@return	HEX
  
This function converts HEX(0-F) to a character
*/
char C2D(
	uint8 c	/**< is a character('0'-'F') to convert to HEX */
	)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 10 + c -'a';
	if (c >= 'A' && c <= 'F')
		return 10 + c -'A';

	return (char)c;
}

/**
 @brief	convert escape characters(%XX) to ascii charater 
 */ 
void unescape_http_url(
	char * url	/**< pointer to be conveted ( escape characters )*/
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


/**
 @brief	make reponse header such as html, gif, jpeg,etc.
 */ 
void make_http_response_head(
	uint8 * buf, 	/**< pointer to response header to be made */
	int8 type, 	/**< response type */
	uint32 len	/**< size of response header */
	)
{
  char * head;
  char tmp[10];
  
                  
  /*  file type*/
  if 	  (type == PTYPE_HTML)	head = RES_HTMLHEAD_OK;
  else if (type == PTYPE_CGI)	head = RES_HTMLHEAD_OK;
  else if (type == PTYPE_PL)	head = RES_HTMLHEAD_OK;
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


/**
 @brief	find MIME type of a file
 */ 
void find_http_uri_type(
	uint8 * type, 	/**< type to be returned */
	char * buf		/**< file name */
	) 
{
  /* Decide type according to extention*/
  if      (strstr(buf, ".pl"))				*type = PTYPE_PL;
  else if (strstr(buf, ".cgi")  || strstr(buf,".CGI"))	*type = PTYPE_CGI;
  else if (strstr(buf, ".html") || strstr(buf,".htm"))	*type = PTYPE_HTML;
  else if (strstr(buf, ".gif"))				*type = PTYPE_GIF;
  else if (strstr(buf, ".text") || strstr(buf,".txt"))	*type = PTYPE_TEXT;
  else if (strstr(buf, ".jpeg") || strstr(buf,".jpg"))	*type = PTYPE_JPEG;
  else if (strstr(buf, ".swf")) 			*type = PTYPE_FLASH;
  else if (strstr(buf, ".mpeg") || strstr(buf,".mpg"))	*type = PTYPE_MPEG;
  else if (strstr(buf, ".pdf")) 			*type = PTYPE_PDF;
  else if (strstr(buf, ".js")   || strstr(buf,".JS"))	*type = PTYPE_TEXT;
  else if (strstr(buf, ".xml")  || strstr(buf,".XML"))	*type = PTYPE_HTML;
  else 							*type = PTYPE_ERR;
}


/**
 @brief	parse http request from a peer
 */ 
void parse_http_request(
	st_http_request * request, 	/**< request to be returned */
	char * buf				/**< pointer to be parsed */
	)
{
  char * nexttok;
  nexttok = strtok(buf," ");
  if(!nexttok)
  {
    request->METHOD = METHOD_ERR;
    return;
  }
  if(!strcmp(nexttok, "GET") || !strcmp(nexttok,"get"))
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
    nexttok = strtok(NULL," ");
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
  nexttok = strtok(NULL,"\0");
  strcpy(request->param,nexttok);

  #ifdef HTTPD_DEBUG
  {
    uint16 i;
    printf("http_request->URI");
    for(i=0; i < strlen(request->URI);i++)
        printf("%c",request->URI[i]);
    printf("\r\n");
  }
  #endif					
	
}


/**
 @brief	get next parameter value in the request
 */ 
unsigned char* get_http_param_value(char* uri, char* param_name)
{

  char * name=0; 
  uint8 *ret=BUFPUB;
  if(!uri || !param_name) return 0;
  if((name = strstr(uri,param_name)))
  {
    name += strlen(param_name) + 1; 
    char* pos2=strstr(name,"&");
    if(!pos2) 
    {
      pos2=name+strlen(name);
    }
    uint16 len=0;
    len = pos2-name;
 
    if(len)
    {
      ret[len]=0;
      strncpy((char*)ret,name,len);
      unescape_http_url((char*)ret);
      replacetochar((char*)ret,'+',' ');
      //ret[len]=0;
      //ret[strlen((int8*)ret)]=0;
    }
    else
      ret[0]=0;
  }
  else
    return 0;
  return ret;		
}

unsigned char* get_http_uri_name(char* uri)
{
	char* uri_name;
	char* tempURI=(char*)BUFPUB;

	if(!uri) return 0;

	memset (tempURI, 0, MAX_URI_SIZE);	
	strcpy(tempURI,uri);
	
	uri_name = strtok(tempURI," ?");
	
	if(strcmp(uri_name,"/")) uri_name++;

#ifdef HTTPD_DEBUG
	PRINTLN1("uri_name=%s",uri_name);
#endif	

	return (uint8*)uri_name;
}
