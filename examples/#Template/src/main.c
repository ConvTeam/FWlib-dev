
#include "common/common.h"

#include "protocol/DNS/dns.h"
//#include "protocol/HTTP/httputil.h"
#include "protocol/SMTP/smtp.h"
#include "appmod/atcmd/atcmd.h"
#include "appmod/loopback/loopback.h"
#include "appmod/usermenu/usermenu.h"


int32 main(void)
{
	if(platform_init() != RET_OK) 
		goto FAIL_TRAP;

	if(network_init(0, NULL, NULL) != RET_OK) {
		ERR("network_init fail");
		goto FAIL_TRAP;
	}

	NL1;
	LOG("-----------------------------------");
	LOG(" WIZlib Project - Title            ");
	LOG("-----------------------------------");
	NL2;

	Delay_tick(2000);	// prevent first send fail
	
	// ToDo: Modules Initialization
	// Ex) dhcp_auto_start();
	//     atc_init();

	while(1) {
	
		// ToDo: Modules Run
		// Ex) atc_run();
		//     alarm_run();
		//     sockwatch_run();

		wizpf_led_flicker(WIZ_LED1, 1000);	// check loop is running
	}

FAIL_TRAP:
	wizpf_led_trap(1);
	return 0;
}



























