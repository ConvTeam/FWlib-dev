
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "common/types.h"


//### DHCP Compare Define #####################################################
#define DHCP_NONE			0	// Disable DHCP, If you test this mode, you should exclude dhcp folder from project
#define DHCP_AUTO_ASYNC		1	// DHCP Auto (Alarm) mode, use non-blocking function
#define DHCP_AUTO_SYNC		2	// DHCP Auto (Alarm) mode, use blocking function
#define DHCP_MANUAL			3	// DHCP Manual mode
//---------------------------------------------------------------------------

  /* Select one of upper mode */
  #define DHCP_MODE DHCP_AUTO_ASYNC

  /* Uncomment if you want to start DHCP as STATIC mode */
  //#define DHCP_START_AS_STATIC

//###########################################################################


//------------------------------ Environment Setting ------------------------------
#define COMPILER_IAR_EWARM
//#define COMPILER_GCC_ARM

//#define PLATFORM_W7200_EVB  // Defined in Project Option
//#define PLATFORM_W5200_EVB

#define SYSTEM_LITTLE_ENDIAN	// ( Default )
//#define SYSTEM_BIG_ENDIAN

//------------------------------ Network Setting ------------------------------

//###########################################################################
#if (DHCP_MODE != DHCP_NONE)
#define USE_DHCP	VAL_ENABLE
#endif

#if (DHCP_MODE == DHCP_AUTO_ASYNC) || (DHCP_MODE == DHCP_AUTO_SYNC)
#define DHCP_AUTO	// if not set, you should handle dhcp manually, if set, need to run 'alarm_run()' funtion in main loop
#endif
#if (DHCP_MODE == DHCP_AUTO_ASYNC)
#define DHCP_ASYNC	// if not set, it works using sync function, if set, need to run 'sockwatch_run()' funtion in main loop
#endif
//###########################################################################

#define DEFAULT_MAC_ADDR	"00:08:DC:11:22:33"
#define DEFAULT_IP_ADDR		"192.168.0.100"
#define DEFAULT_SN_MASK		"255.255.255.0"
#define DEFAULT_GW_ADDR		"192.168.0.1"
#define DEFAULT_DNS_ADDR	"168.126.63.1"

//------------------------------ Etc ------------------------------
#define WIZ_LOG_LEVEL	2		// 0: No print,  1: Error,  2: Error+Log,  3: Error+Log+Debug
//#define PRINT_TIME_LOG

#endif

