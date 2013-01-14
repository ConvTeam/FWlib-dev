
#ifndef _DHCP_H
#define _DHCP_H

#include "common/common.h"

#define DHCP_STATE_INIT			0	// DHCP state
#define DHCP_STATE_SEARCHING	1
#define DHCP_STATE_SELECTING	2
#define DHCP_STATE_REQUESTING	3
#define DHCP_STATE_IP_CHECK		4
#define DHCP_STATE_BOUND		5


int8 dhcp_init(SOCKET sock, pFunc ip_update, pFunc ip_conflict, uint8 *my_mac);
int8 dhcp_get_state(void);
void dhcp_run(void);

#endif //_DHCP_H

