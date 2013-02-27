
#include "common/common.h"

#include "protocol/DNS/dns.h"
#include "appmod/atcmd/atcmd.h"

#define ASSIST_SOCK		0

int32 main(void)
{
#define TCP_LISTEN_PORT	5000
#define UDP_LISTEN_PORT	5000

	int8 ret;
	uint32 tick = 0;

	ret = platform_init();
	if(ret != RET_OK) {
		goto FAIL_TRAP;
	}

	ret = network_init(ASSIST_SOCK, NULL, NULL);
	if(ret != RET_OK) {
		ERRA("network_init fail - ret(%d)", ret);
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

		if(wizpf_tick_elapse(tick) > 1000) {	// running check
			wizpf_led_set(WIZ_LED1, VAL_TOG);
			tick = wizpf_get_systick();
		}
	}

FAIL_TRAP:
	wizpf_led_trap(1);
	return 0;
}



























