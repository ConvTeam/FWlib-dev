
//#define FILE_LOG_SILENCE
#include "common/common.h"
//#include "device/sockutil.h"

#if (USE_DHCP == VAL_ENABLE)
#include "protocol/DHCP/dhcp.h"
#endif


int8 network_init(SOCKET dhcp_sock, pFunc ip_update, pFunc ip_conflict)
{
	wiz_NetInfo netinfo;

	DEFAULT_MAC_SET(netinfo.Mac);

#if (USE_DHCP == VAL_ENABLE)
	dhcp_init(dhcp_sock, ip_update, ip_conflict, netinfo.Mac);
	while(1) {
		dhcp_run();
		if(dhcp_get_state() == DHCP_STATE_BOUND) break;		
	}
#else
	STATIC_IP_SET(netinfo.IP);
	STATIC_SN_SET(netinfo.Subnet);
	STATIC_GW_SET(netinfo.Gateway);
	STATIC_DNS_SET(netinfo.DNSServerIP);
	SetNetInfo(&netinfo);
	network_disp(&netinfo);
#endif

	return RET_OK;
}

void network_disp(wiz_NetInfo *netinfo)
{
	GetNetInfo(netinfo);
	printf("\r\n---------------------------------------\r\n");
	printf("W5200 Network Configuration Information\r\n");
	printf("---------------------------------------\r\n");
	printf("MAC : %02X:%02X:%02X:%02X:%02X:%02X\r\n", netinfo->Mac[0], netinfo->Mac[1],
		netinfo->Mac[2], netinfo->Mac[3], netinfo->Mac[4], netinfo->Mac[5]);
	printf("IP  : %d.%d.%d.%d\r\n", netinfo->IP[0], netinfo->IP[1], 
		netinfo->IP[2], netinfo->IP[3]);
	printf("SN  : %d.%d.%d.%d\r\n", netinfo->Subnet[0], netinfo->Subnet[1], 
		netinfo->Subnet[2], netinfo->Subnet[3]);
	printf("GW  : %d.%d.%d.%d\r\n", netinfo->Gateway[0], netinfo->Gateway[1], 
		netinfo->Gateway[2], netinfo->Gateway[3]);
	printf("DNS : %d.%d.%d.%d\r\n", netinfo->DNSServerIP[0], netinfo->DNSServerIP[1], 
		netinfo->DNSServerIP[2], netinfo->DNSServerIP[3]);
	printf("---------------------------------------\r\n");
}

int8 ip_check(char *str, uint8 *ip)
{
	uint8_t cnt=0;
	char tmp[50], *split;
	int digit;

	if(ip && sizeof(ip)/sizeof(uint8) < 4) {
		ERR("not enough ip variable size");
		return RET_NOK;
	}

	strcpy(tmp, str);
	split = strtok(tmp, ".");
	while(split != NULL && str_check(isdigit, split) == RET_OK) {
		digit = atoi(split);
		if(digit > 255 || digit < 0) return RET_NOK;
		if(ip) ip[cnt] = digit;
		cnt++;
		split = strtok(NULL, ".");
	}

	if(cnt != 4) {
		//printf("not 4 digit (%d)\r\n", cnt);
		return RET_NOK;
	}

	return RET_OK;
}

char* inet_ntoa(unsigned long addr)
{
	static char addr_str[16];
	memset(addr_str,0,16);
	sprintf(addr_str,"%d.%d.%d.%d",(int)(addr>>24 & 0xFF),(int)(addr>>16 & 0xFF),(int)(addr>>8 & 0xFF),(int)(addr & 0xFF));
	return addr_str;
}

char* inet_ntoa_pad(unsigned long addr)
{
	static char addr_str[16];
	memset(addr_str,0,16);
	sprintf(addr_str,"%03d.%03d.%03d.%03d",(int)(addr>>24 & 0xFF),(int)(addr>>16 & 0xFF),(int)(addr>>8 & 0xFF),(int)(addr & 0xFF));
	return addr_str;
}

unsigned long inet_addr(unsigned char* addr)
{
	char i;
	uint32 inetaddr = 0;
	char taddr[30];
	char * nexttok;
	int num;
	strcpy(taddr,(char*)addr);
	
	nexttok = taddr;
	for(i = 0; i < 4 ; i++)
	{
		nexttok = strtok(nexttok,".");
		if(nexttok[0] == '0' && nexttok[1] == 'x') num = strtol(nexttok+2, NULL, 16);
		else num = strtol(nexttok, NULL, 10);
		inetaddr = inetaddr << 8;		
		inetaddr |= (num & 0xFF);
		nexttok = NULL;
	}
	return inetaddr;	
}	

uint16 swaps(uint16 i)
{
	uint16 ret=0;
	ret = (i & 0xFF) << 8;
	ret |= ((i >> 8)& 0xFF);
	return ret;	
}

uint32 swapl(uint32 l)
{
	uint32 ret=0;
	ret = (l & 0xFF) << 24;
	ret |= ((l >> 8) & 0xFF) << 16;
	ret |= ((l >> 16) & 0xFF) << 8;
	ret |= ((l >> 24) & 0xFF);
	return ret;
}

/**
@brief	htons function converts a unsigned short from host to TCP/IP network byte order (which is big-endian).
@return 	the value in TCP/IP network byte order
*/ 
uint16 htons(uint16 hostshort)
{
#ifdef SYSTEM_LITTLE_ENDIAN
	return swaps(hostshort);
#else
	return hostshort;
#endif		
}


/**
@brief	htonl function converts a unsigned long from host to TCP/IP network byte order (which is big-endian).
@return 	the value in TCP/IP network byte order
*/ 
uint32 htonl(uint32 hostlong)
{
#ifdef SYSTEM_LITTLE_ENDIAN
	return swapl(hostlong);
#else
	return hostlong;
#endif	
}


/**
@brief	ntohs function converts a unsigned short from TCP/IP network byte order to host byte order (which is little-endian on Intel processors).
@return 	a 16-bit number in host byte order
*/ 
uint32 ntohs(uint16 netshort)
{
#ifdef SYSTEM_LITTLE_ENDIAN
	return htons(netshort);
#else
	return netshort;
#endif		
}


/**
@brief	converts a unsigned long from TCP/IP network byte order to host byte order (which is little-endian on Intel processors).
@return 	a 16-bit number in host byte order
*/ 
uint32 ntohl(uint32 netlong)
{
#ifdef SYSTEM_LITTLE_ENDIAN
	return swapl(netlong);
#else
	return netlong;
#endif		
}

/**
@brief	Calculate checksum of a stream
@return 	checksum
*/ 
unsigned short checksum(unsigned char * src, unsigned int len)
{
	uint16 sum, tsum, i, j;
	uint32 lsum;

	j = len >> 1;

	lsum = 0;

	for (i = 0; i < j; i++) 
	{
		tsum = src[i * 2];
		tsum = tsum << 8;
		tsum += src[i * 2 + 1];
		lsum += tsum;
	}

	if (len % 2) 
	{
		tsum = src[i * 2];
		lsum += (tsum << 8);
	}


	sum = lsum;
	sum = ~(sum + (lsum >> 16));
	return (uint16) sum;	
}





