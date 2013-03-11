
#include "common/common.h"

#include "protocol/DNS/dns.h"
#include "protocol/HTTP/httputil.h"
#include "appmod/loopback/loopback.h"
#include "appmod/usermenu/usermenu.h"

#define SOCK_DHCP		0	// UDP
#define SOCK_DNS		1	// UDP
#define SOCK_HTTP		2	// TCP


bool lb_tcp = FALSE, lb_udp = FALSE;

static int8 mn_show_network(menu_ctrl mctrl, int8 *mbuf)
{
	if(mctrl == MC_START) {
		network_disp(NULL);
	} else if(mctrl == MC_END) {

	} else if(mctrl == MC_DATA) {

	}

	return RET_OK;
}

static int8 mn_set_network(menu_ctrl mctrl, int8 *mbuf)
{
#define INPUT_GUIDE(name_v) \
	printf("Enter new "name_v" [xxx.xxx.xxx.xxx] or 'Enter key' to skip\r\n")
#define NEXT_GUIDE(name_v, addr_v) \
do {INPUT_GUIDE(name_v); \
	printf("Current "name_v" is (%d.%d.%d.%d)\r\n", \
		addr_v[0], addr_v[1], addr_v[2], addr_v[3]); \
} while(0)
#define SET_DONE_GUIDE(name_v, addr_v) \
	printf("New "name_v" is (%d.%d.%d.%d)\r\n\r\n", \
		addr_v[0], addr_v[1], addr_v[2], addr_v[3])
#define SET_STAGE(cur_name_v, next_name_v, cur_addr_v, next_addr_v) \
do {uint8 _tmp[4], _next[4]; \
	if(next_addr_v != NULL) memcpy(_next, next_addr_v, 4); \
	if(mbuf[0] == 0) { \
		stage++; \
		if(next_addr_v != NULL) NEXT_GUIDE(next_name_v, _next); \
	} else { \
		ret = ip_check((int8*)mbuf, _tmp); \
		if(ret == RET_OK) { \
			memcpy(cur_addr_v, _tmp, 4); \
			stage++; \
			SetNetInfo(&netinfo); \
			SET_DONE_GUIDE(cur_name_v, cur_addr_v); \
			if(next_addr_v != NULL) NEXT_GUIDE(next_name_v, _next); \
		} else { \
			printf("wrong input(%s)\r\n\r\n", mbuf); \
			INPUT_GUIDE(cur_name_v); \
		} \
	} \
} while(0)

	int8 ret;
	static uint8 stage = 0;
	static wiz_NetInfo netinfo;

	GetNetInfo(&netinfo);

	if(mctrl == MC_START) {
		NEXT_GUIDE("IP Address", netinfo.ip);
	} else if(mctrl == MC_END) {
		stage = 0;
	} else if(mctrl == MC_DATA) {
		switch(stage) {
		case 0:
			SET_STAGE("IP Address", "Subnet mask", netinfo.ip, netinfo.sn);
			break;
		case 1:
			SET_STAGE("Subnet mask", "Gateway Address", netinfo.sn, netinfo.gw);
			break;
		case 2:
			SET_STAGE("Gateway Address", "DNS Address", netinfo.gw, netinfo.dns);
			break;
		case 3:
			SET_STAGE("DNS Address", "", netinfo.dns, NULL);
			if(stage > 3) return RET_OK;
			break;
		}
	}

	return RET_NOK;

#undef INPUT_GUIDE
#undef NEXT_GUIDE
#undef SET_DONE_GUIDE
#undef SET_STAGE
}

static int8 mn_loopback(menu_ctrl mctrl, int8 *mbuf)
{
	if(mctrl == MC_START) {
		if(lb_tcp || lb_udp) {
			printf("Turning off (%s) Loopback\r\n", lb_tcp?"TCP":"UDP");
			lb_tcp = lb_udp = FALSE;
			return RET_OK;
		} else printf("Enter the number [1: TCP, 2: UDP]\r\n");
	} else if(mctrl == MC_END) {

	} else if(mctrl == MC_DATA) {
		if(str_check(isdigit, (int8*)mbuf) == RET_OK) {
			uint8 input = atoi((char*)mbuf);
			if(input == 1) lb_tcp = TRUE;
			else if(input == 2) lb_udp = TRUE;
			else printf("Enter the number [1: TCP, 2: UDP]\r\n");

			if(lb_tcp || lb_udp) {
				printf("Turning on (%s) Loopback\r\n", lb_tcp?"TCP":"UDP");
				return RET_OK;
			}
		} else {
			printf("It is not digit(%s) - Please try again\r\n", mbuf);
		}
	}

	return RET_NOK;
}

static int8 mn_set_led(menu_ctrl mctrl, int8 *mbuf)
{
	if(mctrl == MC_START) {
		printf("Enter the number [1: ON, 2: OFF]\r\n");
	} else if(mctrl == MC_END) {

	} else if(mctrl == MC_DATA) {
		if(str_check(isdigit, (int8*)mbuf) == RET_OK) {
			uint8 input = atoi((char*)mbuf);
			if(input == 1) {
				wizpf_led_set(WIZ_LED1, VAL_ON);
				wizpf_led_set(WIZ_LED2, VAL_ON);
				printf("LED On\r\n");
			} else if(input == 2) {
				wizpf_led_set(WIZ_LED1, VAL_OFF);
				wizpf_led_set(WIZ_LED2, VAL_OFF);
				printf("LED Off\r\n");
			} else {
				printf("wrong number(%d) - try again\r\n", input);
				return RET_NOK;
			}
			return RET_OK;
		} else {
			printf("not digit(%s) - try again\r\n", mbuf);
			return RET_NOK;
		}
	}

	return RET_NOK;
}

static int8 mn_dns(menu_ctrl mctrl, int8 *mbuf)
{
	uint8 domain_ip[4];

	if(mctrl == MC_START) {
		printf("Enter the Domain name you want to check (ex: www.abc.com)\r\n");
	} else if(mctrl == MC_END) {

	} else if(mctrl == MC_DATA) {	//printf("start dns\r\n");
		if(mbuf[0] == 0 || strchr((char*)mbuf, '.') == NULL) {
			printf("wrong input(%s)\r\n", mbuf);
			return RET_NOK;
		}

		if(dns_query(SOCK_DNS, (uint8*)mbuf, domain_ip) == RET_OK) {
			printf("IP Address of (%s) is (%d.%d.%d.%d)\r\n", mbuf,
				domain_ip[0], domain_ip[1], domain_ip[2], domain_ip[3]);
		}else {
			printf("DNS fail\r\n");
		}
		return RET_OK;
	}

	return RET_NOK;
}

static int8 mn_base64(menu_ctrl mctrl, int8 *mbuf)
{
	static uint8 stage = 0;
	static char encodedText[256];

	if(mctrl == MC_START) {
		printf("Enter the number [1: Encode, 2: Decode]\r\n");
	} else if(mctrl == MC_END) {
		stage = 0;
	} else if(mctrl == MC_DATA) {
		switch(stage) {
		case 0:
			if(mbuf[0] == '1') {
				stage = 1;
				printf("Type Plain Text\r\n");
			} else if(mbuf[0] == '2') {
				stage = 2;
				printf("Type Plain Text\r\n");
			} else printf("wrong number(%s) - try again\r\n", mbuf);
			break;
		case 1:
			memset(encodedText, 0, sizeof(encodedText));
			base64_encode((int8*)mbuf, strlen((char*)mbuf)+1, (int8*)encodedText);
			printf("Encoded Text:\r\n%s\r\n", encodedText);
			return RET_OK;
		case 2:
			memset(encodedText, 0, sizeof(encodedText));
			base64_decode((int8*)mbuf, (void *)encodedText, strlen((char*)mbuf));
			printf("Decoded Text:\r\n%s\r\n", encodedText);
			return RET_OK;
		default: printf("wrong stage(%d)\r\n", stage);
		}
	}

	return RET_NOK;
}


void inet_addr_(unsigned char* addr,unsigned char *ip)
{
	int i;
	char taddr[30];
	char * nexttok;
	char num;
	strcpy(taddr,(char *)addr);

	nexttok = taddr;
	for(i = 0; i < 4 ; i++)
	{
		nexttok = strtok(nexttok,".");
		if(nexttok[0] == '0' && nexttok[1] == 'x') num = atoi(nexttok+2);
		else num = atoi(nexttok);

		ip[i] = num;
		nexttok = NULL;
	}
}

void IP_get_func(char *buf, uint16 *len)
{
	wiz_NetInfo netinfo;
	GetNetInfo(&netinfo);
	*len = sprintf(buf, "%d.%d.%d.%d", netinfo.ip[0], netinfo.ip[1], netinfo.ip[2], netinfo.ip[3]);
}
void IP_set_func(char *buf, uint16 *len)
{
	wiz_NetInfo netinfo;
	GetNetInfo(&netinfo);
	inet_addr_((uint8*)buf, netinfo.ip);
	SetNetInfo(&netinfo);
}

void GW_get_func(char *buf, uint16 *len)
{
	wiz_NetInfo netinfo;
	GetNetInfo(&netinfo);
	*len = sprintf(buf, "%d.%d.%d.%d", netinfo.gw[0], netinfo.gw[1], netinfo.gw[2], netinfo.gw[3]);
}
void GW_set_func(char *buf, uint16 *len)
{
	wiz_NetInfo netinfo;
	GetNetInfo(&netinfo);
	inet_addr_((uint8*)buf, netinfo.gw);
	SetNetInfo(&netinfo);
}

void SUB_get_func(char *buf, uint16 *len)
{
	wiz_NetInfo netinfo;
	GetNetInfo(&netinfo);
	*len = sprintf(buf, "%d.%d.%d.%d", netinfo.sn[0], netinfo.sn[1], netinfo.sn[2], netinfo.sn[3]);
}
void SUB_set_func(char *buf, uint16 *len)
{
	wiz_NetInfo netinfo;
	GetNetInfo(&netinfo);
	inet_addr_((uint8*)buf, netinfo.sn);
	SetNetInfo(&netinfo);
}

void DNS_get_func(char *buf, uint16 *len)
{
	wiz_NetInfo netinfo;
	GetNetInfo(&netinfo);
	*len = sprintf(buf, "%d.%d.%d.%d", netinfo.dns[0], netinfo.dns[1], netinfo.dns[2], netinfo.dns[3]);
}
void DNS_set_func(char *buf, uint16 *len)
{
	wiz_NetInfo netinfo;
	GetNetInfo(&netinfo);
	inet_addr_((uint8*)buf, netinfo.dns);
	SetNetInfo(&netinfo);
}

void MAC_get_func(char *buf, uint16 *len)
{
	wiz_NetInfo netinfo;
	GetNetInfo(&netinfo);
	*len = sprintf(buf, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", netinfo.mac[0], netinfo.mac[1], netinfo.mac[2], netinfo.mac[3], netinfo.mac[4], netinfo.mac[5]);
}

void LED1_get_func(char *buf, uint16 *len)
{
	if(wizpf_led_get(WIZ_LED1) == VAL_ON){	// led on
		*len = sprintf(buf, "<input name=\"LED1\" type=\"radio\" value=\"ON\" checked>ON<input name=\"LED1\" type=\"radio\" value=\"OFF\">OFF");
	}else{						// led off
		*len = sprintf(buf, "<input name=\"LED1\" type=\"radio\" value=\"ON\">ON<input name=\"LED1\" type=\"radio\" value=\"OFF\" checked>OFF");
	}
}
void LED1_set_func(char *buf, uint16 *len)
{
	if(!strcmp(buf, "ON")){
		wizpf_led_set(WIZ_LED1, VAL_ON);
	}else{
		wizpf_led_set(WIZ_LED1, VAL_OFF);
	}
}

void LED2_get_func(char *buf, uint16 *len)
{
	if(wizpf_led_get(WIZ_LED2) == VAL_ON){	// led on
		*len = sprintf(buf, "<input name=\"LED2\" type=\"radio\" value=\"ON\" checked>ON<input name=\"LED2\" type=\"radio\" value=\"OFF\">OFF");
	}else{						// led off
		*len = sprintf(buf, "<input name=\"LED2\" type=\"radio\" value=\"ON\">ON<input name=\"LED2\" type=\"radio\" value=\"OFF\" checked>OFF");
	}
}
void LED2_set_func(char *buf, uint16 *len)
{
	if(!strcmp(buf, "ON")){
		wizpf_led_set(WIZ_LED2, VAL_ON);
	}else{
		wizpf_led_set(WIZ_LED2, VAL_OFF);
	}
}


int main(void)
{
#define TCP_LISTEN_PORT	5000
#define UDP_LISTEN_PORT	5000

	int8 root;

	if(platform_init() != RET_OK) 
		goto FAIL_TRAP;

	if(network_init(SOCK_DHCP, NULL, NULL) != RET_OK) {
		ERR("network_init fail");
		goto FAIL_TRAP;
	}

	printf("\r\n-----------------------------------\r\n");
	printf("Web Server\r\n");
	printf("-----------------------------------\r\n\r\n");

	menu_init();
	root = menu_add("Network setting", 0, NULL);
	menu_add("Show", root, mn_show_network);
	menu_add("Static Set", root, mn_set_network);
	menu_add("Loopback", 0, mn_loopback);
	menu_add("LED Test", 0, mn_set_led);
	root = menu_add("App Test", 0, NULL);
	menu_add("DNS", root, mn_dns);
	menu_add("BASE64", root, mn_base64);

	menu_print_tree();

	cgi_callback_add("IP", IP_get_func, IP_set_func);
	cgi_callback_add("GW", GW_get_func, GW_set_func);
	cgi_callback_add("SUB", SUB_get_func, SUB_set_func);
	cgi_callback_add("DNS", DNS_get_func, DNS_set_func);
	cgi_callback_add("SRC_MAC_ADDRESS", MAC_get_func, NULL);

	cgi_callback_add("LED1", LED1_get_func, LED1_set_func);
	cgi_callback_add("LED2", LED2_get_func, LED2_set_func);

	Delay_tick(2000);	// prevent first send fail
	dhcp_auto_start();

	while(1) {
		alarm_run();
		menu_run();
		if(lb_tcp) loopback_tcps(7, (uint16)TCP_LISTEN_PORT);
		if(lb_udp) loopback_udp(7, (uint16)UDP_LISTEN_PORT);

		WebServer(SOCK_HTTP);
	}

FAIL_TRAP:
	wizpf_led_trap(1);
}



























