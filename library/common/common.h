
#ifndef _COMMON_H
#define _COMMON_H

#include "wizconfig.h"
#include "common/types.h"

//-------------------------------------- Compiler Definition --------------------------------------
#ifdef COMPILER_IAR_EWARM
#ifndef COMPILER_DEF_DONE
#define COMPILER_DEF_DONE
#else
#error Duplicate definition of COMPILER
#endif

	//Todo

#endif

//------------------------------------------
#ifdef COMPILER_GCC_ARM
#ifndef COMPILER_DEF_DONE
#define COMPILER_DEF_DONE
#else
#error Duplicate definition of COMPILER
#endif

	//Todo
	//#define __GNUC__		// ??

#endif

//-------------------------------------- Platform Definition --------------------------------------
#ifdef PLATFORM_W5200_EVB
#ifndef PLATFORM_DEF_DONE
#define PLATFORM_DEF_DONE
#else
#error Duplicate definition of PLATFORM
#endif

	#define HOST_STM32F10X	// ??
	#define DEVICE_W5200	// ??
	#include "host/STM32F10x/WIZ_W5200/w5200_evb.h"

#endif

//------------------------------------------
#ifdef PLATFORM_W7200_EVB
#ifndef PLATFORM_DEF_DONE
#define PLATFORM_DEF_DONE
#else
#error Duplicate definition of PLATFORM
#endif

	#define HOST_STM32F10X
	#define DEVICE_W5200
	#include "host/STM32F10x/WIZ_W7200/w7200_evb.h"

#endif

//-------------------------------------- Host Definition --------------------------------------
#ifdef HOST_8051
#ifndef HOST_DEF_DONE
#define HOST_DEF_DONE
#else
#error Duplicate definition of HOST
#endif

	//Todo

#endif

//------------------------------------------
#ifdef HOST_STM32F10X
#ifndef HOST_DEF_DONE
#define HOST_DEF_DONE
#else
#error Duplicate definition of HOST
#endif

	#include "stm32f10x.h"
	#include "host/wizspi.h"

#endif

//-------------------------------------- Device Definition --------------------------------------
#ifdef DEVICE_W5100
#ifndef DEVICE_DEF_DONE
#define DEVICE_DEF_DONE
#else
#error Duplicate definition of DEVICE
#endif

	//Todo

#endif

//------------------------------------------
#ifdef DEVICE_W5200
#ifndef DEVICE_DEF_DONE
#define DEVICE_DEF_DONE
#else
#error Duplicate definition of DEVICE
#endif

	#define	TOTAL_SOCK_NUM	8	// Maxmium number of socket 
	#define TOTAL_SOCK_MEM	32	// Total Tx/Rx Buffer Memory (KB)
	#include "device/w5200/w5200.h"

#endif

//------------------------------------------
#ifdef DEVICE_W5300
#ifndef DEVICE_DEF_DONE
#define DEVICE_DEF_DONE
#else
#error Duplicate definition of DEVICE
#endif

	//Todo

#endif

//------------------------------------------
#ifdef DEVICE_W5500
#ifndef DEVICE_DEF_DONE
#define DEVICE_DEF_DONE
#else
#error Duplicate definition of DEVICE
#endif

	//Todo

#endif

//-------------------------------------- Definition Check ----------------------------------------
#ifndef COMPILER_DEF_DONE
#error COMPILER Not defined
#endif

#ifndef PLATFORM_DEF_DONE
#error PLATFORM Not defined
#endif

#ifndef HOST_DEF_DONE
#error HOST Not defined
#endif

#ifndef DEVICE_DEF_DONE
#error DEVICE Not defined
#endif

#ifndef USE_DHCP
#define USE_DHCP	VAL_DISABLE
#endif

#if (defined(SYSTEM_LITTLE_ENDIAN) && defined(SYSTEM_BIG_ENDIAN)) || \
	(!defined(SYSTEM_LITTLE_ENDIAN) && !defined(SYSTEM_BIG_ENDIAN))
#error Endian define error
#endif

//-------------------------------------- Common Includes ----------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common/util.h"
#include "device/socket.h"
#include "device/sockutil.h"
#include "host/wizplatform.h"

//------------------------------------------- LOG ---------------------------------------------
#if !defined(WIZ_LOG_LEVEL) || (WIZ_LOG_LEVEL < 0) || (WIZ_LOG_LEVEL > 3)
#define WIZ_LOG_LEVEL 2
#endif

#ifndef FILE_LOG_SILENCE
#ifdef PRINT_TIME_LOG
#define ERR(fmt)  do { if(WIZ_LOG_LEVEL > 0) printf("### ERROR ### [%5d.%03d] %s(%d): "fmt"\r\n", \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __FUNCTION__, __LINE__); } while(0)
#define ERRA(fmt, ...)  do { if(WIZ_LOG_LEVEL > 0) printf("### ERROR ### [%5d.%03d] %s(%d): "fmt"\r\n", \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#define ERRF(fmt) do { if(WIZ_LOG_LEVEL > 0) printf("### ERROR ### [%5d.%03d] %s(%d): "fmt, \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __FUNCTION__, __LINE__); } while(0)
#define ERRFA(fmt, ...) do { if(WIZ_LOG_LEVEL > 0) printf("### ERROR ### [%5d.%03d] %s(%d): "fmt, \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)

#define LOG(fmt)  do { if(WIZ_LOG_LEVEL > 1) printf("[%5d.%03d] "fmt"\r\n", \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000); } while(0)
#define LOGA(fmt, ...)  do { if(WIZ_LOG_LEVEL > 1) printf("[%5d.%03d] "fmt"\r\n", \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __VA_ARGS__); } while(0)
#define LOGF(fmt) do { if(WIZ_LOG_LEVEL > 1) printf("[%5d.%03d] "fmt, \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000); } while(0)
#define LOGFA(fmt, ...) do { if(WIZ_LOG_LEVEL > 1) printf("[%5d.%03d] "fmt, \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __VA_ARGS__); } while(0)

#define DBG(fmt)  do { if(WIZ_LOG_LEVEL > 2) printf("[D] [%5d.%03d] %s(%d): "fmt"\r\n", \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __FUNCTION__, __LINE__); } while(0)
#define DBGA(fmt, ...)  do { if(WIZ_LOG_LEVEL > 2) printf("[D] [%5d.%03d] %s(%d): "fmt"\r\n", \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#define DBGF(fmt) do { if(WIZ_LOG_LEVEL > 2) printf("[D] [%5d.%03d] %s(%d): "fmt, \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __FUNCTION__, __LINE__); } while(0)
#define DBGFA(fmt, ...) do { if(WIZ_LOG_LEVEL > 2) printf("[D] [%5d.%03d] %s(%d): "fmt, \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)

#else
#define ERR(fmt)  do { if(WIZ_LOG_LEVEL > 0) printf("### ERROR ### %s(%d): "fmt"\r\n", __FUNCTION__, __LINE__); } while(0)
#define ERRA(fmt, ...)  do { if(WIZ_LOG_LEVEL > 0) printf("### ERROR ### %s(%d): "fmt"\r\n", __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#define ERRF(fmt) do { if(WIZ_LOG_LEVEL > 0) printf("### ERROR ### %s(%d): "fmt, __FUNCTION__, __LINE__); } while(0)
#define ERRFA(fmt, ...) do { if(WIZ_LOG_LEVEL > 0) printf("### ERROR ### %s(%d): "fmt, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)

#define LOG(fmt)  do { if(WIZ_LOG_LEVEL > 1) printf(fmt"\r\n"); } while(0)
#define LOGA(fmt, ...)  do { if(WIZ_LOG_LEVEL > 1) printf(fmt"\r\n", __VA_ARGS__); } while(0)
#define LOGF(fmt) do { if(WIZ_LOG_LEVEL > 1) printf(fmt); } while(0)
#define LOGFA(fmt, ...) do { if(WIZ_LOG_LEVEL > 1) printf(fmt, __VA_ARGS__); } while(0)

#define DBG(fmt)  do { if(WIZ_LOG_LEVEL > 2) printf("[D] %s(%d): "fmt"\r\n", __FUNCTION__, __LINE__); } while(0)
#define DBGA(fmt, ...)  do { if(WIZ_LOG_LEVEL > 2) printf("[D] %s(%d): "fmt"\r\n", __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#define DBGF(fmt) do { if(WIZ_LOG_LEVEL > 2) printf("[D] %s(%d): "fmt, __FUNCTION__, __LINE__); } while(0)
#define DBGFA(fmt, ...) do { if(WIZ_LOG_LEVEL > 2) printf("[D] %s(%d): "fmt, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#endif

#define NL1	printf("\r\n");
#define NL2	printf("\r\n\r\n");
#define NL3	printf("\r\n\r\n\r\n");

#else
#define ERR(fmt)
#define ERRA(fmt, ...)
#define ERRF(fmt)
#define ERRFA(fmt, ...)

#define LOG(fmt)
#define LOGA(fmt, ...)
#define LOGF(fmt)
#define LOGFA(fmt, ...)

#define DBG(fmt)
#define DBGA(fmt, ...)
#define DBGF(fmt)
#define DBGFA(fmt, ...)

#define NL1
#define NL2
#define NL3
#endif

//-------------------------------------------------------------------------------------------








#endif //_COMMON_H

