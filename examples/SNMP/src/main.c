
#include "common/common.h"

#include "protocol/SNMP/snmp.h"
#include "appmod/usermenu/usermenu.h"


int32 main(void)
{
	if(platform_init(NULL) != RET_OK) 
		goto FAIL_TRAP;

	if(network_init(0, NULL, NULL) != RET_OK) {
		ERR("network_init fail");
		goto FAIL_TRAP;
	}

	NL1;
	LOG("-----------------------------------");
	LOG(" WIZlib Project - SNMP Example     ");
	LOG("-----------------------------------");
	NL2;

	Delay_tick(2000);	// prevent first send fail
	
	// ToDo: Modules Initialization
	// Ex) dhcp_auto_start();
	//     atc_init();
    
    SnmpXDaemon_init(); // SNMP Daemon Initialization   
    
	while(1) {
	
		// ToDo: Modules Run
		// Ex) atc_run();
		//     alarm_run();
		//     sockwatch_run();
        
        // Process SNMP Daemon : User can add the OID/Functions to snmpData[] array in snmprun.c/.h
        // [net-snmp version 5.7 package for windows] is used for this demo.
        // [Command] get WIZ_LED2 info  : snmpget -v 1 -c public 192.168.0.100 .1.3.6.1.4.1.6.1.0 
        // [Command] set WIZ_LED2 on    : snmpset -v 1 -c public 192.168.0.100 .1.3.6.1.4.1.6.2.0 i 1 
        // [Command] set WIZ_LED2 off   : snmpset -v 1 -c public 192.168.0.100 .1.3.6.1.4.1.6.2.0 i 0 
        SnmpXDaemon_process();

		wizpf_led_flicker(WIZ_LED1, 1000);	// check loop is running
	}

FAIL_TRAP:
	wizpf_led_trap(1);
	return 0;
}



























