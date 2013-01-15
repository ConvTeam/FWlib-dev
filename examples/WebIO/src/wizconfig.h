
#ifndef _CONFIG_H_
#define _CONFIG_H_

//------------------------------ Environment Setting ------------------------------
#define COMPILER_IAR_EWARM
//#define COMPILER_GCC_ARM

#define PLATFORM_W7200_EVB
//#define PLATFORM_W5200_EVB

//#define HOST_STM32F10X	// Host will be set automatically by platform
//#define HOST_8051

//#define DEVICE_W5200	// Device will be set automatically by platform
//#define DEVICE_W5100

#define SYSTEM_LITTLE_ENDIAN	// ( Default )
//#define SYSTEM_BIG_ENDIAN

//------------------------------ Network Setting ------------------------------
#define USE_DHCP	VAL_ENABLE

#define MAC_ADDR_0	0x00
#define MAC_ADDR_1	0x08
#define MAC_ADDR_2	0xDC
#define MAC_ADDR_3	0x11
#define MAC_ADDR_4	0x22
#define MAC_ADDR_5	0x33

#define IP_ADDR_0	192
#define IP_ADDR_1	168
#define IP_ADDR_2	0
#define IP_ADDR_3	100

#define SN_ADDR_0	255
#define SN_ADDR_1	255
#define SN_ADDR_2	255
#define SN_ADDR_3	0

#define GW_ADDR_0	192
#define GW_ADDR_1	168
#define GW_ADDR_2	0
#define GW_ADDR_3	1

#define DNS_ADDR_0	168
#define DNS_ADDR_1	126
#define DNS_ADDR_2	63
#define DNS_ADDR_3	1

//------------------------------ Etc ------------------------------
#define WIZ_LOG_LEVEL 2		// 0: No print,  1: Error,  2: Error+Log,  3: Error+Log+Debug
//#define PRINT_TIME_LOG

#endif

