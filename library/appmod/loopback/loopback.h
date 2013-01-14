
#ifndef _LOOPBACK_H
#define _LOOPBACK_H

#include "common/common.h"


void loopback_tcps(SOCKET s, uint16 port);
void loopback_tcpc(SOCKET s, uint16 port);
void loopback_udp(SOCKET s, uint16 port);

#endif //_LOOPBACK_H



