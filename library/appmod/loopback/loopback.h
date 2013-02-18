
#ifndef _LOOPBACK_H
#define _LOOPBACK_H

#include "common/common.h"


void loopback_tcps(uint8 s, uint16 port);
void loopback_tcpc(uint8 s, uint16 port);
void loopback_udp(uint8 s, uint16 port);

#endif //_LOOPBACK_H



