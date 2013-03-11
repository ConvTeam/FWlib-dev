
#include "common/common.h"

#include "protocol/DNS/dns.h"
#include "appmod/atcmd/atcmd.h"

#define ASSIST_SOCK		0

int32 main(void)
{
	if(platform_init() != RET_OK) 
		goto FAIL_TRAP;

	if(network_init(ASSIST_SOCK, NULL, NULL) != RET_OK) {
		ERR("network_init fail");
		goto FAIL_TRAP;
	}

	NL1;
	LOG("-----------------------------------");
	LOG("Serial to Ethernet Using AT Command");
	LOG("-----------------------------------");
	NL2;

	Delay_tick(2000);	// prevent first send fail
	dhcp_auto_start();
	atc_init();

	while(1) {
		atc_run();
		alarm_run();
		sockwatch_run();
		wizpf_led_flicker(WIZ_LED1, 1000);	// check loop is running
	}

FAIL_TRAP:
	wizpf_led_trap(1);
	return 0;
}









