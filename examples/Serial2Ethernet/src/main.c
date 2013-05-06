
#include "common/common.h"

#include "protocol/DNS/dns.h"
#include "appmod/atcmd/atcmd.h"

#define ASSIST_SOCK		0

void dhcp_trigger(int8 arg)
{
	dhcp_auto_start();
}

int32 main(void)
{
	if(platform_init(NULL) != RET_OK) 
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

	atc_init();
	alarm_set(2000, dhcp_trigger, 0);	//dhcp_auto_start();

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









