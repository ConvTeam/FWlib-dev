
#include "common/common.h"

#include "protocol/NTP/ntp.h"
#include "appmod/usermenu/usermenu.h"

static int8 mn_disp_ntp(menu_ctrl mctrl, int8 *mbuf);
static int8 mn_get_timedata(menu_ctrl mctrl, int8 *mbuf);

extern NTP_CONFIG ConfigMsg;

int32 main(void)
{
    // Platform Initialization
	if(platform_init(NULL) != RET_OK) 
		goto FAIL_TRAP;
    
    // Network Initialization
	if(network_init(0, NULL, NULL) != RET_OK) {
		ERR("network_init fail");
		goto FAIL_TRAP;
	}
    
	NL1;
	LOG("-----------------------------------");
	LOG(" WIZlib Project - SNTP Example     ");
	LOG("-----------------------------------");
	NL2;
	
	// ToDo: Modules Initialization
	// Ex) dhcp_auto_start();
	//     atc_init();  
    
    // NTP Initialization   
    set_ntp_default();
    set_ntp_client_msg(TIMESERVER1);
        
    Delay_tick(2000);	// prevent first send fail
    
    LOG(">>> SNTP Example Start >>>");
    
    // Serial Terminal Menu Initialization
    {
        menu_init();	    	
        menu_add("NTP : Display Time Info", 0, mn_disp_ntp);
        menu_add("NTP : Get Time Data from Timeserver", 0, mn_get_timedata);    
    }
    
	while(1) {
	
		// ToDo: Modules Run
		// Ex) atc_run();
		//     alarm_run();
		//     sockwatch_run();
        
        // Configuration menu
        alarm_run();
        menu_run();
        
        //LOGF(".");

		wizpf_led_flicker(WIZ_LED1, 1000);	// check loop is running
	}

FAIL_TRAP:
	wizpf_led_trap(1);
	return 0;
}

static int8 mn_disp_ntp(menu_ctrl mctrl, int8 *mbuf)
{ 
	if(mctrl == MC_START) {
		
	} else if(mctrl == MC_END) {
        ntp_time_disp(&ConfigMsg);
	} else if(mctrl == MC_DATA) {
        
	}

	return RET_OK;
}

static int8 mn_get_timedata(menu_ctrl mctrl, int8 *mbuf)
{
    if(mctrl == MC_START) {
        do_ntp_client();                     
	} else if(mctrl == MC_END) {
        ntp_time_disp(&ConfigMsg);
	} else if(mctrl == MC_DATA) {
        
	}
	return RET_OK;
}













