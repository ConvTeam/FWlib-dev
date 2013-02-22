
#include "common/common.h"

#include "protocol/DHCP/dhcp.h"
//#include "protocol/DNS/dns.h"
//#include "protocol/HTTP/httputil.h"
//#include "protocol/SMTP/smtp.h"
//#include "appmod/atcmd/atcmd.h"
//#include "appmod/loopback/loopback.h"
//#include "appmod/usermenu/usermenu.h"



//###########################################################################
// #  
// #  DHCP Test for Comparison
// #  
// #  - Set DHCP mode you want to test in wizconfig.h
// #  - DHCP_MANUAL mode does not need loop structure, but if there is no 
// #    loop structure, you should handle renew & rebind with different way, 
// #    or just ignore renew & rebind action
// #  
//###########################################################################



int32 main(void)
{
	int8 ret;
	uint32 tick = 0;
//###########################################################################
#if (DHCP_MODE == DHCP_MANUAL)
	uint32 dhcp_renew, dhcp_rebind, dhcp_time;
	uint32 dhcp_tick;
#endif
//###########################################################################

	ret = platform_init();
	if(ret != RET_OK) {
		goto FAIL_TRAP;
	}

	ret = network_init(0, NULL, NULL);
	if(ret != RET_OK) {
		ERRA("network_init fail - ret(%d)", ret);
		goto FAIL_TRAP;
	}

	NL1;
	LOG("-----------------------------------");
	LOG(" DHCP Test for Comparison          ");
	LOG("-----------------------------------");
	NL2;

	Delay_tick(2000);	// prevent first send fail

//###########################################################################
#if (DHCP_MODE == DHCP_NONE)
	LOG("Static Mode");
#elif (DHCP_MODE == DHCP_ALARM_ASYNC)
	LOG("DHCP Mode: Alarm Async");
	dhcp_alarm_start(NULL);
#elif (DHCP_MODE == DHCP_ALARM_SYNC)
	LOG("DHCP Mode: Alarm Sync");
	dhcp_alarm_start(NULL);
#elif (DHCP_MODE == DHCP_MANUAL)
	LOG("DHCP Mode: Manual");
	do {
		ret = dhcp_manual(DHCP_ACT_START, NULL, &dhcp_renew, &dhcp_rebind);
	} while(ret != RET_OK);
	dhcp_renew = wizpf_tick_conv(FALSE, dhcp_renew);
	dhcp_rebind = wizpf_tick_conv(FALSE, dhcp_rebind);
	dhcp_time = dhcp_renew;
	dhcp_tick = wizpf_get_systick();
#endif
//###########################################################################

	while(1) {

//###########################################################################
#if (DHCP_MODE == DHCP_ALARM_ASYNC)
		alarm_run();
		sockwatch_run();
#elif (DHCP_MODE == DHCP_ALARM_SYNC)
		alarm_run();
#elif (DHCP_MODE == DHCP_MANUAL)
		if(wizpf_tick_elapse(dhcp_tick) > dhcp_time) {
			if(dhcp_time==dhcp_renew) DBG("start renew"); else DBG("start rebind");
			ret = dhcp_manual(dhcp_time==dhcp_renew? DHCP_ACT_RENEW: DHCP_ACT_REBIND, 
				NULL, &dhcp_renew, &dhcp_rebind);
			dhcp_tick = wizpf_get_systick();
			if(ret == RET_OK) {	// renew success
				dhcp_renew = wizpf_tick_conv(FALSE, dhcp_renew);
				dhcp_rebind = wizpf_tick_conv(FALSE, dhcp_rebind);
				dhcp_time = dhcp_renew;
			} else {
				if(dhcp_time == dhcp_renew) dhcp_time = dhcp_rebind; // renew fail, try rebind
				else dhcp_time = 60000; // retry after 1 min
			}
		}
#endif
//###########################################################################

		if(wizpf_tick_elapse(tick) > 1000) {	// running check
			wizpf_led_set(WIZ_LED3, VAL_TOG);
			tick = wizpf_get_systick();
		}
	}

FAIL_TRAP:
	wizpf_led_trap(1);
	return 0;
}





