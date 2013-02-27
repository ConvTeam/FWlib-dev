/**
 * @file		common.h
 * @brief		Library Common Header File.
 * This header file influence on all library area directly
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *		2013/02/22 - 1.0 Release
 * @author	Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef _COMMON_H
#define _COMMON_H


#include "wizconfig.h"


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

#endif

//-------------------------------------- Platform Definition --------------------------------------
#ifdef PLATFORM_W5200_EVB
#ifndef PLATFORM_DEF_DONE
#define PLATFORM_DEF_DONE
#else
#error Duplicate definition of PLATFORM
#endif

	#define HOST_STM32F10X
	#define DEVICE_W5200
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

//------------------------------------------
#ifdef PLATFORM_W5500_EVB
#ifndef PLATFORM_DEF_DONE
#define PLATFORM_DEF_DONE
#else
#error Duplicate definition of PLATFORM
#endif

	#define HOST_STM32F10X
	#define DEVICE_W5500
	#include "host/STM32F10x/WIZ_W5500/w5500_evb.h"

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
	#define MAX_TICK_ELAPSE	0x7FFFFFFF	// Maximum elapse time you can set

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

	#define	TOTAL_SOCK_NUM	8	// Maxmium number of socket 
	#define TOTAL_SOCK_MEM	32	// Total Tx/Rx Buffer Memory (KB)
	#include "device/w5500/w5500.h"

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
#if (USE_DHCP == VAL_ENABLE)
#include "protocol/DHCP/dhcp.h"
#else
#define dhcp_init(sock, ip_update_hook, ip_conflict_hook, def) RET_NOK
#define dhcp_manual(action, renew, rebind) RET_OK
#define dhcp_get_state() DHCP_STATE_INIT
#define dhcp_set_storage(net)
#define dhcp_get_storage(net)
#define dhcp_static_mode(net)
#define dhcp_auto_start()
#endif

//------------------------------------------- LOG ---------------------------------------------
#if !defined(WIZ_LOG_LEVEL) || (WIZ_LOG_LEVEL < 0) || (WIZ_LOG_LEVEL > 3)
#define WIZ_LOG_LEVEL 2
#endif

#if (WIZ_LOG_LEVEL > 0) && defined(PRINT_TIME_LOG) && !defined(FILE_LOG_SILENCE)
#define ERR(fmt)  do { printf("### ERROR ### [%5d.%03d] %s(%d): "fmt"\r\n", \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __FUNCTION__, __LINE__); } while(0)
#define ERRA(fmt, ...)  do { printf("### ERROR ### [%5d.%03d] %s(%d): "fmt"\r\n", \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#define ERRF(fmt) do { printf("### ERROR ### [%5d.%03d] %s(%d): "fmt, \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __FUNCTION__, __LINE__); } while(0)
#define ERRFA(fmt, ...) do { printf("### ERROR ### [%5d.%03d] %s(%d): "fmt, \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#elif (WIZ_LOG_LEVEL > 0) && !defined(PRINT_TIME_LOG) && !defined(FILE_LOG_SILENCE)
#define ERR(fmt)  do { printf("### ERROR ### %s(%d): "fmt"\r\n", __FUNCTION__, __LINE__); } while(0)
#define ERRA(fmt, ...)  do { printf("### ERROR ### %s(%d): "fmt"\r\n", __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#define ERRF(fmt) do { printf("### ERROR ### %s(%d): "fmt, __FUNCTION__, __LINE__); } while(0)
#define ERRFA(fmt, ...) do { printf("### ERROR ### %s(%d): "fmt, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#else
#define ERR(fmt)
#define ERRA(fmt, ...)
#define ERRF(fmt)
#define ERRFA(fmt, ...)
#endif

#if (WIZ_LOG_LEVEL > 1) && defined(PRINT_TIME_LOG) && !defined(FILE_LOG_SILENCE)
#define LOG(fmt)  do { printf("[%5d.%03d] "fmt"\r\n", \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000); } while(0)
#define LOGA(fmt, ...)  do { printf("[%5d.%03d] "fmt"\r\n", \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __VA_ARGS__); } while(0)
#define LOGF(fmt) do { printf("[%5d.%03d] "fmt, \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000); } while(0)
#define LOGFA(fmt, ...) do { printf("[%5d.%03d] "fmt, \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __VA_ARGS__); } while(0)
#elif (WIZ_LOG_LEVEL > 1) && !defined(PRINT_TIME_LOG) && !defined(FILE_LOG_SILENCE)
#define LOG(fmt)  do { printf(fmt"\r\n"); } while(0)
#define LOGA(fmt, ...)  do { printf(fmt"\r\n", __VA_ARGS__); } while(0)
#define LOGF(fmt) do { printf(fmt); } while(0)
#define LOGFA(fmt, ...) do { printf(fmt, __VA_ARGS__); } while(0)
#else
#define LOG(fmt)
#define LOGA(fmt, ...)
#define LOGF(fmt)
#define LOGFA(fmt, ...)
#endif

#if (WIZ_LOG_LEVEL > 2) && defined(PRINT_TIME_LOG) && !defined(FILE_LOG_SILENCE)
#define DBG(fmt)  do { printf("[D] [%5d.%03d] %s(%d): "fmt"\r\n", \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __FUNCTION__, __LINE__); } while(0)
#define DBGA(fmt, ...)  do { printf("[D] [%5d.%03d] %s(%d): "fmt"\r\n", \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#define DBGF(fmt) do { printf("[D] [%5d.%03d] %s(%d): "fmt, \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __FUNCTION__, __LINE__); } while(0)
#define DBGFA(fmt, ...) do { printf("[D] [%5d.%03d] %s(%d): "fmt, \
	wizpf_get_systick()/1000, wizpf_get_systick()%1000, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#elif (WIZ_LOG_LEVEL > 2) && !defined(PRINT_TIME_LOG) && !defined(FILE_LOG_SILENCE)
#define DBG(fmt)  do { printf("[D] %s(%d): "fmt"\r\n", __FUNCTION__, __LINE__); } while(0)
#define DBGA(fmt, ...)  do { printf("[D] %s(%d): "fmt"\r\n", __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#define DBGF(fmt) do { printf("[D] %s(%d): "fmt, __FUNCTION__, __LINE__); } while(0)
#define DBGFA(fmt, ...) do { printf("[D] %s(%d): "fmt, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#else
#define DBG(fmt)
#define DBGA(fmt, ...)
#define DBGF(fmt)
#define DBGFA(fmt, ...)
#endif

#if (WIZ_LOG_LEVEL > 2) && !defined(FILE_LOG_SILENCE)
#define DBGCRTC(cond_v, fmt) do { if(cond_v) {ERR(fmt); while(1); } } while(0)
#define DBGCRTCA(cond_v, fmt, ...) do { if(cond_v) {ERRA(fmt, __VA_ARGS__); while(1); } } while(0)
#else
#define DBGCRTC(cond_v, fmt)
#define DBGCRTCA(cond_v, fmt, ...)
#endif

#if (WIZ_LOG_LEVEL > 0) && !defined(FILE_LOG_SILENCE)
#define NL1	printf("\r\n")
#define NL2	printf("\r\n\r\n")
#define NL3	printf("\r\n\r\n\r\n")
#else
#define NL1
#define NL2
#define NL3
#endif

//-------------------------------------------------------------------------------------------



#endif //_COMMON_H

