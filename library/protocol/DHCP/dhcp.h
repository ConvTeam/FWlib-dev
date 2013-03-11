/**
 * @file		dhcp.h
 * @brief		DHCP Protocol Module Header File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	modified by Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef _DHCP_H
#define _DHCP_H

//#include "common/common.h"

/**
 * @addtogroup dhcp_module
 * @{
 */

/**
 * @brief DHCP State
 */
typedef enum {
	DHCP_STATE_INIT,		///< INIT State - Send Discover Packet
	DHCP_STATE_SEARCHING,	///< SEARCHING State - Waiting for Offer
	DHCP_STATE_SELECTING,	///< SELECTING State - Send Request
	DHCP_STATE_REQUESTING,	///< REQUESTING State - Waiting for ACK
	DHCP_STATE_IP_CHECK,	///< IP_CHECK State - Check IP Validity
	DHCP_STATE_BOUND,		///< BOUND State - Set Alarm for next step
	DHCP_STATE_FAILED		///< FAILED State - Set Static addresses temporally
} dhcp_state;

/**
 * @brief DHCP Action
 */
typedef enum {
	DHCP_ACT_NONE,			///< Do nothing (maybe previous DHCP action was failed)
	DHCP_ACT_START,			///< Start IP Assign only when INIT state
	DHCP_ACT_RENEW,			///< Start Renew only when BOUND state
	DHCP_ACT_REBIND,		///< Start Rebind only when BOUND state
} dhcp_action;

/* @} */


//#define DHCP_AUTO	// if not, you should handle manually	=> You can set this in wizconfig.h
//#define DHCP_ASYNC	// if not, it works using sync function	=> You can set this in wizconfig.h

#undef dhcp_init
#undef dhcp_manual
#undef dhcp_get_state
#undef dhcp_set_storage
#undef dhcp_get_storage
#undef dhcp_static_mode
#undef dhcp_auto_start

int8 dhcp_init(uint8 sock, void_func ip_update_hook, void_func ip_conflict_hook, wiz_NetInfo *def);
int8 dhcp_manual(dhcp_action action, uint32 *renew, uint32 *rebind);		// blocking function
dhcp_state dhcp_get_state(void);
void dhcp_set_storage(wiz_NetInfo *net);
void dhcp_get_storage(wiz_NetInfo *net);
void dhcp_static_mode(wiz_NetInfo *net);
void dhcp_auto_start(void);
#if !defined(DHCP_AUTO) && defined(DHCP_ASYNC)
	#error DHCP_ASYNC define without DHCP_AUTO is not allowed
#endif


#endif //_DHCP_H

