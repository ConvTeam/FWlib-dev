
#include "common/common.h"

#include "protocol/DNS/dns.h"
//#include "protocol/HTTP/httputil.h"
#include "protocol/SMTP/smtp.h"
#include "appmod/atcmd/atcmd.h"
#include "appmod/loopback/loopback.h"
#include "appmod/usermenu/usermenu.h"


int32 main(void)
{
	int8 ret;
	uint32 tick = 0;

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
	LOG(" WIZlib Project - Title            ");
	LOG("-----------------------------------");
	NL2;

	Delay_tick(2000);	// prevent first send fail
	
	// ToDo: Modules Initialization
	// ex) dhcp_auto_start();
	//     atc_init();

	while(1) {
	
		// ToDo: Modules Run
		// ex) atc_run();
		//     alarm_run();
		//     sockwatch_run();

		if(wizpf_tick_elapse(tick) > 1000) {	// running check
			wizpf_led_set(WIZ_LED3, VAL_TOG);
			tick = wizpf_get_systick();
		}
	}

FAIL_TRAP:
	wizpf_led_trap(1);
	return 0;
}



























