#ifndef	__HTTPUTIL_H__
#define	__HTTPUTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f10x.h"
#include "httpd.h"
#include "common/common.h"


void WebServer(SOCKET s);
void HTTPProcessor(SOCKET s, char * buf);
void RESTProcessor(st_http_request *http_request);
int32 CGIProcessor(char* name, char* buf);

#endif