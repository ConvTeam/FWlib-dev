
#ifndef _SOCKUTIL_H
#define _SOCKUTIL_H

//#include "common/common.h"


#define DEFAULT_MAC_SET(dst_v) do { \
	dst_v[0] = MAC_ADDR_0; \
	dst_v[1] = MAC_ADDR_1; \
	dst_v[2] = MAC_ADDR_2; \
	dst_v[3] = MAC_ADDR_3; \
	dst_v[4] = MAC_ADDR_4; \
	dst_v[5] = MAC_ADDR_5; \
	DBGA("Static MAC Addr(%02x:%02x:%02x:%02x:%02x:%02x)", \
		dst_v[0], dst_v[1], dst_v[2], dst_v[3], dst_v[4], dst_v[5]); \
} while(0)

#define STATIC_IP_SET(dst_v) do { \
	dst_v[0] = IP_ADDR_0; \
	dst_v[1] = IP_ADDR_1; \
	dst_v[2] = IP_ADDR_2; \
	dst_v[3] = IP_ADDR_3; \
	DBGA("Static IP Addr(%d.%d.%d.%d)", \
		dst_v[0], dst_v[1], dst_v[2], dst_v[3]); \
} while(0)

#define STATIC_SN_SET(dst_v) do { \
	dst_v[0] = SN_ADDR_0; \
	dst_v[1] = SN_ADDR_1; \
	dst_v[2] = SN_ADDR_2; \
	dst_v[3] = SN_ADDR_3; \
	DBGA("Static SN Mask(%d.%d.%d.%d)", \
		dst_v[0], dst_v[1], dst_v[2], dst_v[3]); \
} while(0)

#define STATIC_GW_SET(dst_v) do { \
	dst_v[0] = GW_ADDR_0; \
	dst_v[1] = GW_ADDR_1; \
	dst_v[2] = GW_ADDR_2; \
	dst_v[3] = GW_ADDR_3; \
	DBGA("Static GW Addr(%d.%d.%d.%d)", \
		dst_v[0], dst_v[1], dst_v[2], dst_v[3]); \
} while(0)

#define STATIC_DNS_SET(dst_v) do { \
	dst_v[0] = DNS_ADDR_0; \
	dst_v[1] = DNS_ADDR_1; \
	dst_v[2] = DNS_ADDR_2; \
	dst_v[3] = DNS_ADDR_3; \
	DBGA("Static DNS Addr(%d.%d.%d.%d)", \
		dst_v[0], dst_v[1], dst_v[2], dst_v[3]); \
} while(0)


int8 network_init(SOCKET dhcp_sock, pFunc ip_update, pFunc ip_conflict);
void network_disp(wiz_NetInfo *netinfo);
int8 ip_check(char *str, uint8 *ip);
char* inet_ntoa(unsigned long addr);				/* Convert 32bit Address into Dotted Decimal Format */
char* inet_ntoa_pad(unsigned long addr);
unsigned long inet_addr(unsigned char* addr);	/* Converts a string containing an (Ipv4) Internet Protocol decimal dotted address into a 32bit address */
unsigned short htons( unsigned short hostshort);	/* htons function converts a unsigned short from host to TCP/IP network byte order (which is big-endian).*/
unsigned long htonl(unsigned long hostlong);		/* htonl function converts a unsigned long from host to TCP/IP network byte order (which is big-endian). */
unsigned long ntohs(unsigned short netshort);		/* ntohs function converts a unsigned short from TCP/IP network byte order to host byte order (which is little-endian on Intel processors). */
unsigned long ntohl(unsigned long netlong);		/* ntohl function converts a uint32 from TCP/IP network order to host byte order (which is little-endian on Intel processors). */
unsigned short checksum(unsigned char * src, unsigned int len);		/* Calculate checksum of a stream */

#endif	//_SOCKUTIL_H



