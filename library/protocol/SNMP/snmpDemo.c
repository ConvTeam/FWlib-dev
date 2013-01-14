
/*
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include "stm32f10x.h"
#include "device/w5200/w5200.h"
#include "device/socket.h"
#include "config.h"
#include "common/util.h"
#endif

#include "snmpLib.h"
#include "snmpDemo.h"

time_t startTime;

dataEntryType snmpData[] =
{
	// System MIB

	// SysDescr Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 1, 0}, 
	SNMPDTYPE_OCTET_STRING, 30, {"WIZnet Embedded SNMP Agent"}, 
	NULL, NULL},

	// SysObjectID Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 2, 0}, 
	SNMPDTYPE_OBJ_ID, 8, {"\x2b\x06\x01\x02\x01\x01\x02\x00"},
	NULL, NULL},

	// SysUptime Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 3, 0}, 
	SNMPDTYPE_TIME_TICKS, 0, {""},
	currentUptime, NULL},

	// sysContact Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 4, 0}, 
	SNMPDTYPE_OCTET_STRING, 30, {"support@wiznet.co.kr"}, 
	NULL, NULL},

	// sysName Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 5, 0}, 
	SNMPDTYPE_OCTET_STRING, 30, {"http://www.wiznet.co.kr"}, 
	NULL, NULL},

	// Location Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 6, 0}, 
	SNMPDTYPE_OCTET_STRING, 30, {"4F Humax Village"},
	NULL, NULL},

	// SysServices
	{8, {0x2b, 6, 1, 2, 1, 1, 7, 0}, 
	SNMPDTYPE_INTEGER, 4, {""}, 
	NULL, NULL},

	// WIZnet LED 
	{8, {0x2b, 6, 1, 4, 1, 0, 1, 0}, 
	SNMPDTYPE_OCTET_STRING, 30, {""},
	getWIZnetLed, NULL},

	{8, {0x2b, 6, 1, 4, 1, 0, 2, 0}, 
	SNMPDTYPE_INTEGER, 4, {""},
	NULL, setWIZnetLed}
};

const int32 maxData = (sizeof(snmpData) / sizeof(dataEntryType));

void initTable()
{
	startTime = time(NULL);

	snmpData[6].u.intval = 5;

	snmpData[7].u.intval = 0;
	snmpData[8].u.intval = 0;
}

void currentUptime(void *ptr, uint8 *len)
{
	time_t curTime = time(NULL);
	*(uint32 *)ptr = (uint32)(curTime - startTime) * 100;
	*len = 4;
}


//////////////////////////////////////////////////////////////////////////////////////////
int32 wiznetLedStatus = 0;

void getWIZnetLed(void *ptr, uint8 *len)
{
	if ( wiznetLedStatus==0 )	*len = sprintf((int8 *)ptr, "LED Off");
	else						*len = sprintf((int8 *)ptr, "LED On");
}

void setWIZnetLed(int32 val)
{
	wiznetLedStatus = val;
#ifdef WIN32
#else
	if ( wiznetLedStatus==0 )	wizpf_led_act(WIZ_LED3, VAL_OFF); // LED in the W5200-EVB
	else						wizpf_led_act(WIZ_LED3, VAL_ON);
#endif
}

void UserSnmpDemo()
{
	WDEBUG("\r\n\r\nStart UserSnmpDemo");
	SnmpXInit();

	{
		dataEntryType enterprise_oid = {8, {0x2b, 6, 1, 4, 1, 0, 0x10, 0}, SNMPDTYPE_OBJ_ID, 8, {"\x2b\x06\x01\x04\x01\x00\x10\x00"},	NULL, NULL};

		dataEntryType trap_oid1 = {8, {0x2b, 6, 1, 4, 1, 0, 11, 0}, SNMPDTYPE_OCTET_STRING, 30, {""}, NULL, NULL};
		dataEntryType trap_oid2 = {8, {0x2b, 6, 1, 4, 1, 0, 12, 0}, SNMPDTYPE_INTEGER, 4, {""}, NULL, NULL};

		strcpy((int8*)trap_oid1.u.octetstring, "Alert!!!");
		trap_oid2.u.intval = 123456;
		
		//SnmpXTrapSend("222.98.173.250", "127.0.0.0", "public", enterprise_oid, 1, 0, 0);
		//SnmpXTrapSend("222.98.173.250", "127.0.0.0", "public", enterprise_oid, 6, 0, 2, &trap_oid1, &trap_oid2);
		SnmpXTrapSend("192.168.11.250", "192.168.11.251", "public", enterprise_oid, 1, 0, 0);
		SnmpXTrapSend("192.168.11.250", "127.0.0.0", "public", enterprise_oid, 6, 0, 2, &trap_oid1, &trap_oid2);
	}

	SnmpXDaemon();
}

#ifdef WIN32
int32 main(int32 argc, int8 *argv[])
{
	UserSnmpDemo();
	return 0;
}
#endif

*/
