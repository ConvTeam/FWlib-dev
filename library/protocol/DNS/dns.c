/**
 * @file		dns.c
 * @brief		DNS Protocol Module Source File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	modified by Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

//#define FILE_LOG_SILENCE
#include "protocol/DNS/dns.h"


#define	INITRTT		2000L	/* Initial smoothed response time */
#define	MAXCNAME	10	/* Maximum amount of cname recursion */

#define	TYPE_A		1	/* Host address */
#define	TYPE_NS		2	/* Name server */
#define	TYPE_MD		3	/* Mail destination (obsolete) */
#define	TYPE_MF		4	/* Mail forwarder (obsolete) */
#define	TYPE_CNAME	5	/* Canonical name */
#define	TYPE_SOA	6	/* Start of Authority */
#define	TYPE_MB		7	/* Mailbox name (experimental) */
#define	TYPE_MG		8	/* Mail group member (experimental) */
#define	TYPE_MR		9	/* Mail rename name (experimental) */
#define	TYPE_NULL	10	/* Null (experimental) */
#define	TYPE_WKS	11	/* Well-known sockets */
#define	TYPE_PTR	12	/* Pointer record */
#define	TYPE_HINFO	13	/* Host information */
#define	TYPE_MINFO	14	/* Mailbox information (experimental)*/
#define	TYPE_MX		15	/* Mail exchanger */
#define	TYPE_TXT	16	/* Text strings */
#define	TYPE_ANY	255	/* Matches any type */

#define	CLASS_IN	1	/* The ARPA Internet */

/* Round trip timing parameters */
#define	AGAIN	8		/* Average RTT gain = 1/8 */
#define	LAGAIN	3		/* Log2(AGAIN) */
#define	DGAIN	4		/* Mean deviation gain = 1/4 */
#define	LDGAIN	2		/* log2(DGAIN) */

#define	IPPORT_DOMAIN	53
#define	MAX_DNS_BUF_SIZE	512		/* maximum size of DNS buffer. */

/* Header for all domain messages */
struct dhdr
{
	uint16 id;		/* Identification */
	uint8	qr;		/* Query/Response */
#define	QUERY		0
#define	RESPONSE	1
	uint8	opcode;
#define	IQUERY		1
	uint8	aa;		/* Authoratative answer */
	uint8	tc;		/* Truncation */
	uint8	rd;		/* Recursion desired */
	uint8	ra;		/* Recursion available */
	uint8	rcode;		/* Response code */
#define	NO_ERROR	0
#define	FORMAT_ERROR	1
#define	SERVER_FAIL	2
#define	NAME_ERROR	3
#define	NOT_IMPL	4
#define	REFUSED		5
	uint16 qdcount;	/* Question count */
	uint16 ancount;	/* Answer count */
	uint16 nscount;	/* Authority (name server) count */
	uint16 arcount;	/* Additional record count */
};

static uint8 * put16(uint8 *s, uint16 i)
{
	*s++ = i >> 8;
	*s++ = i;

	return s;
}

static uint16 get16(uint8 *s)
{
	uint16 i;

	i = *s++ << 8;
	i = i + *s;

	return i;
}

static int16 dns_makequery(uint16 op, int8 *name, uint8 *buf, uint16 len)
{
	uint8 *cp;
	int8 *cp1;
	int8 sname[MAX_DNS_BUF_SIZE];
	int8 *dname;
	uint16 p;
	uint16 dlen;
	static uint16 MSG_ID = 0x1122;

	cp = buf;

	MSG_ID++;
	cp = put16(cp, MSG_ID);
	p = (op << 11) | 0x0100;			/* Recursion desired */
	cp = put16(cp, p);
	cp = put16(cp, 1);
	cp = put16(cp, 0);
	cp = put16(cp, 0);
	cp = put16(cp, 0);

	strcpy((char*)sname, (char*)name);
	dname = sname;
	dlen = strlen((char*)dname);
	for (;;)
	{
		/* Look for next dot */
		cp1 = (int8*)strchr((char*)dname, '.');

		if (cp1 != NULL) len = cp1 - dname;	/* More to come */
		else len = dlen;			/* Last component */

		*cp++ = len;				/* Write length of component */
		if (len == 0) break;

		/* Copy component up to (but not including) dot */
		strncpy((char*)cp, (char*)dname, len);
		cp += len;
		if (cp1 == NULL)
		{
			*cp++ = 0;			/* Last one; write null and finish */
			break;
		}
		dname += len+1;
		dlen -= len+1;
	}

	cp = put16(cp, 0x0001);				/* type */
	cp = put16(cp, 0x0001);				/* class */

	return ((int16)(cp - buf));
}

static int32 parse_name(uint8 *msg, uint8 *compressed, int8 *buf, int16 len)
{
	uint16 slen;		/* Length of current segment */
	uint8 * cp;
	int32 clen = 0;		/* Total length of compressed name */
	int32 indirect = 0;	/* Set if indirection encountered */
	int32 nseg = 0;		/* Total number of segments in name */

	cp = compressed;

	for (;;)
	{
		slen = *cp++;	/* Length of this segment */

		if (!indirect) clen++;

		if ((slen & 0xc0) == 0xc0)
		{
			if (!indirect)
				clen++;
			indirect = 1;
			/* Follow indirection */
			cp = &msg[((slen & 0x3f)<<8) + *cp];
			slen = *cp++;
		}

		if (slen == 0)	/* zero length == all done */
			break;

		len -= slen + 1;

		if (len < 0) return -1;

		if (!indirect) clen += slen;

		while (slen-- != 0) *buf++ = (int8)*cp++;
		*buf++ = '.';
		nseg++;
	}

	if (nseg == 0)
	{
		/* Root name; represent as single dot */
		*buf++ = '.';
		len--;
	}

	*buf++ = '\0';
	len--;

	return clen;	/* Length of compressed message */
}

static uint8 * dns_question(uint8 *msg, uint8 *cp)
{
	int32 len;
	int8 name[MAX_DNS_BUF_SIZE];

	len = parse_name(msg, cp, name, MAX_DNS_BUF_SIZE);

	DBGA("dns_question : %s", name);

	if (len == -1) return 0;

	cp += len;
	cp += 2;		/* type */
	cp += 2;		/* class */

	return cp;
}

static uint8 * dns_answer(uint8 *msg, uint8 *cp, uint8 *pSip)
{
	int32 len, type;
	int8 name[MAX_DNS_BUF_SIZE];

	len = parse_name(msg, cp, name, MAX_DNS_BUF_SIZE);

	if (len == -1) return 0;

	cp += len;
	type = get16(cp);
	cp += 2;		/* type */
	cp += 2;		/* class */
	cp += 4;		/* ttl */
	cp += 2;		/* len */

	DBGA("dns_answer : %s : %u : %.4x", name, len, type);


	switch (type)
	{
	case TYPE_A:
		/* Just read the address directly into the structure */
		//Server_IP_Addr[0] = *cp++;
		//Server_IP_Addr[1] = *cp++;
		//Server_IP_Addr[2] = *cp++;
		//Server_IP_Addr[3] = *cp++;
		//Config_Msg.Sip[0] = *cp++;
		//Config_Msg.Sip[1] = *cp++;
		//Config_Msg.Sip[2] = *cp++;
		//Config_Msg.Sip[3] = *cp++;
		pSip[0] = *cp++;
		pSip[1] = *cp++;
		pSip[2] = *cp++;
		pSip[3] = *cp++;
		break;
	case TYPE_CNAME:
	case TYPE_MB:
	case TYPE_MG:
	case TYPE_MR:
	case TYPE_NS:
	case TYPE_PTR:
		/* These types all consist of a single domain name */
		/* convert it to ascii format */
		len = parse_name(msg, cp, name, MAX_DNS_BUF_SIZE);
		if (len == -1) return 0;

		cp += len;
		break;
	case TYPE_HINFO:
		len = *cp++;
		cp += len;

		len = *cp++;
		cp += len;
		break;
	case TYPE_MX:
		cp += 2;
		/* Get domain name of exchanger */
		len = parse_name(msg, cp, name, MAX_DNS_BUF_SIZE);
		if (len == -1) return 0;

		cp += len;
		break;
	case TYPE_SOA:
		/* Get domain name of name server */
		len = parse_name(msg, cp, name, MAX_DNS_BUF_SIZE);
		if (len == -1) return 0;

		cp += len;

		/* Get domain name of responsible person */
		len = parse_name(msg, cp, name, MAX_DNS_BUF_SIZE);
		if (len == -1) return 0;

		cp += len;

		cp += 4;
		cp += 4;
		cp += 4;
		cp += 4;
		cp += 4;
		break;
	case TYPE_TXT:
		/* Just stash */
		break;
	default:
		/* Ignore */
		break;
	}

	return cp;
}

static int8 parseMSG(struct dhdr *pdhdr, uint8 *pbuf, uint8 *pSip)
{
	uint16 tmp;
	uint16 i;
	uint8 * msg;
	uint8 * cp;

	msg = pbuf;
	memset(pdhdr, 0, sizeof(pdhdr));

	pdhdr->id = get16(&msg[0]);
	tmp = get16(&msg[2]);
	if (tmp & 0x8000) pdhdr->qr = 1;

	pdhdr->opcode = (tmp >> 11) & 0xf;

	if (tmp & 0x0400) pdhdr->aa = 1;
	if (tmp & 0x0200) pdhdr->tc = 1;
	if (tmp & 0x0100) pdhdr->rd = 1;
	if (tmp & 0x0080) pdhdr->ra = 1;

	pdhdr->rcode = tmp & 0xf;
	pdhdr->qdcount = get16(&msg[4]);
	pdhdr->ancount = get16(&msg[6]);
	pdhdr->nscount = get16(&msg[8]);
	pdhdr->arcount = get16(&msg[10]);

	DBGA("dhdr->qdcount : %x", (uint16)pdhdr->qdcount);
	DBGA("dhdr->ancount : %x", (uint16)pdhdr->ancount);
	DBGA("dhdr->nscount : %x", (uint16)pdhdr->nscount);
	DBGA("dhdr->arcount : %x", (uint16)pdhdr->arcount);

	/* Now parse the variable length sections */
	cp = &msg[12];

	/* Question section */
	for (i = 0; i < pdhdr->qdcount; i++)
	{
		cp = dns_question(msg, cp);
	}

	/* Answer section */
	for (i = 0; i < pdhdr->ancount; i++)
	{
		cp = dns_answer(msg, cp, pSip);
	}

	/* Name server (authority) section */
	for (i = 0; i < pdhdr->nscount; i++)
	{
		;
	}

	/* Additional section */
	for (i = 0; i < pdhdr->arcount; i++)
	{
		;
	}

	if(pdhdr->rcode == 0) return RET_OK;
	else return RET_NOK;
}

/**
 * @ingroup dns_module
 * Perform DNS Query.
 *
 * @param sock Socket number to use
 * @param domain Domain name string to resolve
 * @param ip The variable in which resolved IP address will be returned
 * @return RET_OK: Success
 * @return RET_NOK: Error
 */
int8 dns_query(uint8 sock, uint8 *domain, uint8 *ip)
{
	struct dhdr dhp;
	wiz_NetInfo netinfo;
	uint8 ip_tmp[4], i = 0;
	uint16 port;
	int32 len, cnt;
	uint32 tick;
	static uint8 dns_buf[MAX_DNS_BUF_SIZE];

	tick = wizpf_get_systick();
	srand(tick);
	while(UDPOpen(sock, rand() % 5535 + 60000) != RET_OK) {
		if(i++ > 3) {
			DBGA("UDPOpen fail (%d)times", i);
			return RET_NOK;
		}
	}

	i = 0;
	GetNetInfo(&netinfo);
	len = dns_makequery(0, (int8 *)domain, dns_buf, MAX_DNS_BUF_SIZE);
	while((cnt = UDPSend(sock, (int8*)dns_buf, len, netinfo.dns, IPPORT_DOMAIN)) < 0) {
		if(i++ > 3) {
			DBGA("UDPSend fail (%d)times", i);
			return RET_NOK;
		}
	}
	if (cnt == 0) return RET_NOK; // dns fail

	cnt = 0;
	while(1) {
		if((len = UDPRecv(sock, (int8*)dns_buf, MAX_DNS_BUF_SIZE, ip_tmp, &port)) > 0) {
			UDPClose(sock);
			break;
		}
		Delay_ms(10);
		if(cnt++ == 100) {
			UDPClose(sock);
			return RET_NOK;
		}
	}

	return parseMSG(&dhp, dns_buf, ip);	/* Convert to local format */
}





