/**
 * @file		httputil.h
 * @brief		
 * @version	1.0
 * @date		2013/
 * @par Revision
 *			2013/ - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef	__HTTPUTIL_H__
#define	__HTTPUTIL_H__

#include "protocol/HTTP/httpd.h"
#include "common/common.h"

#define MAX_CGI_CALLBACK 10
typedef void (*cgi_func)(char *buf, uint16 *len);
struct CGI_CALLBACK {
	char *tokken;
	cgi_func get_func;
	cgi_func set_func;
};

void WebServer(uint8 s);
void HTTPProcessor(uint8 s, char * buf);
void RESTProcessor(st_http_request *http_request);
void CGIProcessor(st_http_request *http_request, char* buf);
void cgi_callback_add(char *tokken, cgi_func get_func, cgi_func set_func);

#endif
