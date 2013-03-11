
#include "common/common.h"

#include "appmod/usermenu/usermenu.h"



//###########################################################################
//#  
//#  DHCP Test for Comparison
//#  
//#  - Set DHCP mode you want to test in wizconfig.h
//#  - DHCP_MANUAL mode does not need loop structure, but if there is no 
//#    loop structure, you should handle renew & rebind with your own way, 
//#    or just ignore renew & rebind action
//#  - If you want to test not to use DHCP at all, you should exclude dhcp folder from project
//#  
//###########################################################################


#define DHCP_SOCK 0

static int8 show_netinfo(menu_ctrl mctrl, int8 *mbuf);
static int8 set_dhcp_mode(menu_ctrl mctrl, int8 *mbuf);
static int8 set_ip(menu_ctrl mctrl, int8 *mbuf);
static int8 set_sn(menu_ctrl mctrl, int8 *mbuf);
static int8 set_gw(menu_ctrl mctrl, int8 *mbuf);
static int8 set_dns(menu_ctrl mctrl, int8 *mbuf);

//###########################################################################
#if (DHCP_MODE == DHCP_MANUAL)
	uint32 dhcp_renew, dhcp_rebind, dhcp_time;
	uint32 dhcp_tick;
	bool dhcp_active = FALSE;
#endif
//###########################################################################

int32 main(void)
{
	if(platform_init() != RET_OK) 
		goto FAIL_TRAP;

	if(network_init(DHCP_SOCK, NULL, NULL) != RET_OK) {
		ERR("network_init fail");
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

#elif (DHCP_MODE == DHCP_AUTO_ASYNC) || (DHCP_MODE == DHCP_AUTO_SYNC)
	LOGA("DHCP Mode: Auto (%s)", DHCP_MODE==DHCP_AUTO_ASYNC? "Async": "Sync");
  #ifdef DHCP_START_AS_STATIC
	LOG("Started as Static mode");
	dhcp_static_mode(NULL);
  #else
	LOG("Started as DHCP mode");
	dhcp_auto_start();
  #endif

#elif (DHCP_MODE == DHCP_MANUAL)
{
	int8 ret;
	LOG("DHCP Mode: Manual");
  #ifdef DHCP_START_AS_STATIC
	LOG("Started as Static mode");
	dhcp_static_mode(NULL);
  #else
	LOG("Started as DHCP mode");
	do {
		ret = dhcp_manual(DHCP_ACT_START, &dhcp_renew, &dhcp_rebind);
	} while(ret != RET_OK);
	dhcp_renew = wizpf_tick_conv(FALSE, dhcp_renew);
	dhcp_rebind = wizpf_tick_conv(FALSE, dhcp_rebind);
	dhcp_time = dhcp_renew;
	dhcp_tick = wizpf_get_systick();
	dhcp_active = TRUE;
  #endif
}
#endif
//###########################################################################

	menu_init();	// to use usermenu to test DHCP mode
	menu_add("Show Current Netinfo", 0, show_netinfo);
	menu_add("Set DHCP Mode", 0, set_dhcp_mode);
	menu_add("Set IP Addr", 0, set_ip);
	menu_add("Set SN Mask", 0, set_sn);
	menu_add("Set GW Addr", 0, set_gw);
	menu_add("Set DNS Addr", 0, set_dns);

	while(1) {

//###########################################################################
#if (DHCP_MODE == DHCP_AUTO_ASYNC)
		alarm_run();
		sockwatch_run();
#elif (DHCP_MODE == DHCP_AUTO_SYNC)
		alarm_run();
#elif (DHCP_MODE == DHCP_MANUAL)
		if(dhcp_active == TRUE && wizpf_tick_elapse(dhcp_tick) > dhcp_time) {
			int8 ret;
			if(dhcp_time==dhcp_renew) DBG("start renew"); 
			else DBG("start rebind");
			ret = dhcp_manual(dhcp_time==dhcp_renew? 
				DHCP_ACT_RENEW: DHCP_ACT_REBIND, &dhcp_renew, &dhcp_rebind);
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

		menu_run();	// to use usermenu to test DHCP mode

		wizpf_led_flicker(WIZ_LED1, 1000);	// check loop is running
	}

FAIL_TRAP:
	wizpf_led_trap(1);
	return 0;
}

static int8 show_netinfo(menu_ctrl mctrl, int8 *mbuf)
{
	if(mctrl == MC_START) {
		network_disp(NULL);
	} else if(mctrl == MC_END) {

	} else if(mctrl == MC_DATA) {

	}

	return RET_OK;
}

static int8 set_dhcp_mode(menu_ctrl mctrl, int8 *mbuf)
{
	static wiz_NetInfo netinfo;

	GetNetInfo(&netinfo);

	if(mctrl == MC_START) {
	#if (DHCP_MODE == DHCP_NONE)
		printf("DHCP Disabled\r\n");
		return RET_OK;
	#else
		printf("Enter new DHCP mode [d(DHCP) or s(Static)]\r\n");
		printf("Current DHCP mode is (%s)\r\n", netinfo.dhcp==NETINFO_STATIC? "Static": "DHCP");
	#endif
	} else if(mctrl == MC_END) {
		
	} else if(mctrl == MC_DATA) {
		if(toupper(mbuf[0]) == 'D') {
			if(netinfo.dhcp == NETINFO_DHCP) printf("Already DHCP mode\r\n\r\n");
			else {
			#if (DHCP_MODE == DHCP_AUTO_ASYNC) || (DHCP_MODE == DHCP_AUTO_SYNC)
				dhcp_auto_start();
			#elif (DHCP_MODE == DHCP_MANUAL)
				int8 ret;
				do {
					ret = dhcp_manual(DHCP_ACT_START, &dhcp_renew, &dhcp_rebind);
				} while(ret != RET_OK);
				dhcp_renew = wizpf_tick_conv(FALSE, dhcp_renew);
				dhcp_rebind = wizpf_tick_conv(FALSE, dhcp_rebind);
				dhcp_time = dhcp_renew;
				dhcp_tick = wizpf_get_systick();
				dhcp_active = TRUE;
			#endif
				printf("New DHCP mode is (DHCP)\r\n");
			}
		} else if(toupper(mbuf[0]) == 'S') {
			if(netinfo.dhcp == NETINFO_STATIC) printf("Already Static mode\r\n\r\n");
			else {
				dhcp_static_mode(NULL);
				printf("New DHCP mode is (Static)\r\n");
			}
		} else printf("wrong input(%s)\r\n\r\n", mbuf);
		return RET_OK;
	}

	return RET_NOK;
}

static int8 set_ip(menu_ctrl mctrl, int8 *mbuf)
{
	uint8 addr[4];
	bool dhcp = FALSE;
	static wiz_NetInfo netinfo;

	GetNetInfo(&netinfo);
	if(netinfo.dhcp == NETINFO_DHCP) dhcp = TRUE;

	if(mctrl == MC_START) {
		printf("Enter new IP Addr [xxx.xxx.xxx.xxx]\r\n");
	} else if(mctrl == MC_END) {

	} else if(mctrl == MC_DATA) {
		if(ip_check(mbuf, addr) == RET_OK) {
			memset(&netinfo, 0, sizeof(netinfo));
			memcpy(netinfo.ip, addr, 4);
			if(dhcp == FALSE) {
				SetNetInfo(&netinfo);
				printf("Set new IP Addr (%d.%d.%d.%d)\r\n", 
					netinfo.ip[0], netinfo.ip[1], netinfo.ip[2], netinfo.ip[3]);
			}
		#if (DHCP_MODE != DHCP_NONE)
			dhcp_set_storage(&netinfo);
			printf("Store IP Addr (%d.%d.%d.%d) for Static mode\r\n", 
				netinfo.ip[0], netinfo.ip[1], netinfo.ip[2], netinfo.ip[3]);
		#endif
		} else printf("wrong input(%s)\r\n\r\n", mbuf);
		return RET_OK;
	}

	return RET_NOK;
}

static int8 set_sn(menu_ctrl mctrl, int8 *mbuf)
{
	uint8 addr[4];
	bool dhcp = FALSE;
	static wiz_NetInfo netinfo;

	GetNetInfo(&netinfo);
	if(netinfo.dhcp == NETINFO_DHCP) dhcp = TRUE;

	if(mctrl == MC_START) {
		printf("Enter new Subnet mask [xxx.xxx.xxx.xxx]\r\n");
	} else if(mctrl == MC_END) {

	} else if(mctrl == MC_DATA) {
		if(ip_check(mbuf, addr) == RET_OK) {
			memset(&netinfo, 0, sizeof(netinfo));
			memcpy(netinfo.sn, addr, 4);
			if(dhcp == FALSE) {
				SetNetInfo(&netinfo);
				printf("Set new SN Mask (%d.%d.%d.%d)\r\n", 
					netinfo.sn[0], netinfo.sn[1], netinfo.sn[2], netinfo.sn[3]);
			}
		#if (DHCP_MODE != DHCP_NONE)
			dhcp_set_storage(&netinfo);
			printf("Store SN Mask (%d.%d.%d.%d) for Static mode\r\n", 
				netinfo.sn[0], netinfo.sn[1], netinfo.sn[2], netinfo.sn[3]);
		#endif
		} else printf("wrong input(%s)\r\n\r\n", mbuf);
		return RET_OK;
	}

	return RET_NOK;
}

static int8 set_gw(menu_ctrl mctrl, int8 *mbuf)
{
	uint8 addr[4];
	bool dhcp = FALSE;
	static wiz_NetInfo netinfo;

	GetNetInfo(&netinfo);
	if(netinfo.dhcp == NETINFO_DHCP) dhcp = TRUE;

	if(mctrl == MC_START) {
		printf("Enter new Gateway Addr [xxx.xxx.xxx.xxx]\r\n");
	} else if(mctrl == MC_END) {

	} else if(mctrl == MC_DATA) {
		if(ip_check(mbuf, addr) == RET_OK) {
			memset(&netinfo, 0, sizeof(netinfo));
			memcpy(netinfo.gw, addr, 4);
			if(dhcp == FALSE) {
				SetNetInfo(&netinfo);
				printf("Set new GW Addr (%d.%d.%d.%d)\r\n", 
					netinfo.gw[0], netinfo.gw[1], netinfo.gw[2], netinfo.gw[3]);
			}
		#if (DHCP_MODE != DHCP_NONE)
			dhcp_set_storage(&netinfo);
			printf("Store GW Addr (%d.%d.%d.%d) for Static mode\r\n", 
				netinfo.gw[0], netinfo.gw[1], netinfo.gw[2], netinfo.gw[3]);
		#endif
		} else printf("wrong input(%s)\r\n\r\n", mbuf);
		return RET_OK;
	}

	return RET_NOK;
}

static int8 set_dns(menu_ctrl mctrl, int8 *mbuf)
{
	uint8 addr[4];
	bool dhcp = FALSE;
	static wiz_NetInfo netinfo;

	GetNetInfo(&netinfo);
	if(netinfo.dhcp == NETINFO_DHCP) dhcp = TRUE;

	if(mctrl == MC_START) {
		printf("Enter new DNS Addr [xxx.xxx.xxx.xxx]\r\n");
	} else if(mctrl == MC_END) {

	} else if(mctrl == MC_DATA) {
		if(ip_check(mbuf, addr) == RET_OK) {
			memset(&netinfo, 0, sizeof(netinfo));
			memcpy(netinfo.dns, addr, 4);
			if(dhcp == FALSE) {
				SetNetInfo(&netinfo);
				printf("Set new DNS Addr (%d.%d.%d.%d)\r\n", 
					netinfo.dns[0], netinfo.dns[1], netinfo.dns[2], netinfo.dns[3]);
			}
		#if (DHCP_MODE != DHCP_NONE)
			dhcp_set_storage(&netinfo);
			printf("Store DNS Addr (%d.%d.%d.%d) for Static mode\r\n", 
				netinfo.dns[0], netinfo.dns[1], netinfo.dns[2], netinfo.dns[3]);
		#endif
		} else printf("wrong input(%s)\r\n\r\n", mbuf);
		return RET_OK;
	}

	return RET_NOK;
}



