/**
 * @file		wizconfig.h
 * @brief		User Configuration Header File Templete.
 * Copy this to your source folder and modify for your project.
 * 
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *		2013/02/22 - 1.0 Release
 * @author	modified by Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "common/types.h"

//------------------------------ Environment Setting ------------------------------
/**
 * @def SYSTEM_LITTLE_ENDIAN
 * Define Endianness - Little Endian (Default).
 * If you want to set to Big Endian, 
 * remove this and define SYSTEM_BIG_ENDIAN
 * @see @ref SYSTEM_BIG_ENDIAN
 * @def SYSTEM_BIG_ENDIAN
 * Define Endianness - Big Endian.
 * If you want to set to Little Endian, 
 * remove this and define SYSTEM_LITTLE_ENDIAN
 * @see @ref SYSTEM_LITTLE_ENDIAN
 */
#define SYSTEM_LITTLE_ENDIAN	// ( Default )
#define SYSTEM_BIG_ENDIAN

/**
 * @def COMPILER_IAR_EWARM
 * Indicate your complier is IAR EWARM.
 * If you are using IAR EWARM, define this.
 * @see @ref COMPILER_GCC_ARM
 * @def COMPILER_GCC_ARM
 * Indicate your complier is GCC ARM.
 * If you are using GCC ARM, define this.
 * @see @ref COMPILER_IAR_EWARM
 * @remarks @b Not implemented yet.
 */
#define COMPILER_IAR_EWARM
#define COMPILER_GCC_ARM

/**
 * @def PLATFORM_W5200_EVB
 * Indicate your platform is W5200 EVB.
 * If you are using W5200 EVB Board, define this.
 * @see @ref PLATFORM_W7200_EVB,  @ref PLATFORM_W5500_EVB
 * @def PLATFORM_W7200_EVB
 * Indicate your platform is W7200 EVB.
 * If you are using W7200 EVB Board, define this.
 * @see @ref PLATFORM_W5200_EVB,  @ref PLATFORM_W5500_EVB
 * @def PLATFORM_W5500_EVB
 * Indicate your platform is W5500 EVB.
 * If you are using W5500 EVB Board, define this.
 * @see @ref PLATFORM_W7200_EVB,  @ref PLATFORM_W5200_EVB
 */
#define PLATFORM_W5200_EVB
#define PLATFORM_W7200_EVB
#define PLATFORM_W5500_EVB

/**
 * @def HOST_8051
 * Indicate your Host Device(MCU) is 8051.
 * Normally, this is defined automatically by platform definition.
 * but if you does not use defined platform, this definition will be needed.
 * @see @ref HOST_STM32F10X
 * @def HOST_STM32F10X
 * Indicate your Host Device(MCU) is STM32F10x.
 * Normally, this is defined automatically by platform definition.
 * but if you does not use defined platform, this definition will be needed.
 * @see @ref HOST_8051
 */
#define HOST_STM32F10X
#define HOST_8051

/**
 * @def DEVICE_W5100
 * Indicate your Network Device(IINCHIP) is W5100.
 * Normally, this is defined automatically by platform definition.
 * but if you does not use defined platform, this definition will be needed.
 * @see @ref DEVICE_W5200,  @ref DEVICE_W5300,  @ref DEVICE_W5500
 * @def DEVICE_W5200
 * Indicate your Network Device(IINCHIP) is W5200.
 * Normally, this is defined automatically by platform definition.
 * but if you does not use defined platform, this definition will be needed.
 * @see @ref DEVICE_W5100,  @ref DEVICE_W5300,  @ref DEVICE_W5500
 * @def DEVICE_W5300
 * Indicate your Network Device(IINCHIP) is W5300.
 * Normally, this is defined automatically by platform definition.
 * but if you does not use defined platform, this definition will be needed.
 * @see @ref DEVICE_W5100,  @ref DEVICE_W5200,  @ref DEVICE_W5500
 * @def DEVICE_W5500
 * Indicate your Network Device(IINCHIP) is W5500.
 * Normally, this is defined automatically by platform definition.
 * but if you does not use defined platform, this definition will be needed.
 * @see @ref DEVICE_W5100,  @ref DEVICE_W5200,  @ref DEVICE_W5300
 */
#define DEVICE_W5100
#define DEVICE_W5200
#define DEVICE_W5300
#define DEVICE_W5500

/**
 * @def TOTAL_SOCK_NUM
 * Total Socket number you can use.
 * This is defined automatically by Network Device definition.
 * @def TOTAL_SOCK_MEM
 * Total Socket memory you can use.
 * This is defined automatically by Network Device definition.
 * @see @ref device_mem_init
 */
#define TOTAL_SOCK_NUM
#define TOTAL_SOCK_MEM

//------------------------------ Network Setting ------------------------------
/**
 * @def USE_DHCP
 * Include DHCP Module.
 * If you want to use DHCP function, define this.
 * and add library/protocol/DHCP files to your project.
 * and finally, define DHCP_AUTO, DHCP_ASYNC properly.
 * Ex) If you want to set Auto Async DHCP mode
 * @code
 * #define USE_DHCP	VAL_ENABLE
 * #define DHCP_AUTO
 * #define DHCP_ASYNC
 * @endcode
 * @def DHCP_AUTO
 * Set DHCP as Auto (using alarm module).
 * - You need to define USE_DHCP as VAL_ENABLE before define this.
 * - Alarm module is used to calculate timing and to run.
 * - By using @ref dhcp_auto_start, you can start DHCP action.
 * - If you define this, you should call @ref alarm_run
 *   at main loop continuously.
 * - If this is not defined and you want to use DHCP, 
 *   you have to handle it manually by using @ref dhcp_manual.
 * @def DHCP_ASYNC
 * Use async socket function which does not be blocked.
 * - You need to define USE_DHCP as VAL_ENABLE before define this.
 * - If you define this, you should call @ref sockwatch_run 
 *   at main loop continuously.
 * - If this is not defined, it works as sync(blocking) function.
 */
#define USE_DHCP	VAL_ENABLE
#define DHCP_AUTO
#define DHCP_ASYNC

/**
 * @def DEFAULT_MAC_ADDR
 * Define default MAC Address.
 * @def DEFAULT_IP_ADDR
 * Define default IP Address.
 * @def DEFAULT_SN_MASK
 * Define default Subnet Mask.
 * @def DEFAULT_GW_ADDR
 * Define default Gateway Address.
 * @def DEFAULT_DNS_ADDR
 * Define default DNS Server Address.
 */
#define DEFAULT_MAC_ADDR	"00:08:DC:11:22:33"
#define DEFAULT_IP_ADDR		"192.168.0.100"
#define DEFAULT_SN_MASK		"255.255.255.0"
#define DEFAULT_GW_ADDR		"192.168.0.1"
#define DEFAULT_DNS_ADDR	"168.126.63.1"

//------------------------------ Etc ------------------------------
/**
 * @def WIZ_LOG_LEVEL
 * Log level which is set at wizconfig.h. default value is 2.
 * - 0: Log Disabled
 * - 1: Error Log
 * - 2: Error Log + Normal Log
 * - 3: Error Log + Normal Log + Debug Log
 * @def PRINT_TIME_LOG
 * Define Add systick time stamp to every log print
 */
#define WIZ_LOG_LEVEL	2	// 0: No print,  1: Error,  2: Error+Log,  3: Error+Log+Debug
#define PRINT_TIME_LOG

 


 
 
 






 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

