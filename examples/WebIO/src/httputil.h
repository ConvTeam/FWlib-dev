#ifndef	__HTTPUTIL_H__
#define	__HTTPUTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "httpd.h"
#include "common/common.h"


#define MAX_CGI_CALLBACK 10
typedef void (*cgi_func)(char *buf, uint16 *len);
struct CGI_CALLBACK {
	char *tokken;
	cgi_func get_func;
	cgi_func set_func;
};

void WebServer(SOCKET s);
void HTTPProcessor(SOCKET s, char * buf);
void RESTProcessor(st_http_request *http_request);
void CGIProcessor(st_http_request *http_request, char* buf);
void cgi_callback_add(char *tokken, cgi_func get_func, cgi_func set_func);

#endif