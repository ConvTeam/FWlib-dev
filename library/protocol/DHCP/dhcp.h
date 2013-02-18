
#ifndef _DHCP_H
#define _DHCP_H

#include "common/common.h"

#define DHCP_STATE_INIT			0	// DHCP state
#define DHCP_STATE_SEARCHING	1
#define DHCP_STATE_SELECTING	2
#define DHCP_STATE_REQUESTING	3
#define DHCP_STATE_IP_CHECK		4
#define DHCP_STATE_BOUND		5

#define DHCP_ACT_NONE			0	// DHCP action
#define DHCP_ACT_START			1
#define DHCP_ACT_RENEW			2
#define DHCP_ACT_REBIND			3

//#define DHCP_ALARM	// if not, you should handle manually	=> You can set this in wizconfig.h
//#define DHCP_ASYNC	// if not, it works using sync function	=> You can set this in wizconfig.h


int8 dhcp_init(uint8 sock, pFunc ip_update_hook, pFunc ip_conflict_hook, wiz_NetInfo *def);
int8 dhcp_manual(int8 action, uint8 *saved_ip, uint32 *renew, uint32 *rebind);		// blocking function
int8 dhcp_get_state(void);
int8 dhcp_set_storage(wiz_NetInfo *net);
int8 dhcp_get_storage(wiz_NetInfo *net);
int8 dhcp_static_mode(wiz_NetInfo *net);
int8 dhcp_alarm_start(uint8 *saved_ip);


#endif //_DHCP_H

