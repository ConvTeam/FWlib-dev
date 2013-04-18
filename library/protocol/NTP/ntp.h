/**
 * @file		ntp.h
 * @brief		NTP Protocol Module Header File
 * @version	1.0
 * @date		2013/03/11
 * @par Revision
 *			2013/03/11 - 1.0 Release
 * @author	modified by HK Jung
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef	_NTP_H
#define	_NTP_H

#include "common/common.h"

/**
 * @addtogroup sntp_module
 * @{
 */

/* for ntpclient */ 
typedef signed char s_char;
typedef unsigned long long tstamp;
typedef unsigned int tdist;

typedef struct _ntpformat
{  
        uint8  dstaddr[4];      /* destination (local) address */
        char    version;        /* version number */
        char    leap;           /* leap indicator */
        char    mode;           /* mode */
        char    stratum;        /* stratum */
        char    poll;           /* poll interval */
        s_char  precision;      /* precision */
        tdist   rootdelay;      /* root delay */
        tdist   rootdisp;       /* root dispersion */
        char    refid;          /* reference ID */
        tstamp  reftime;        /* reference time */
        tstamp  org;            /* origin timestamp */
        tstamp  rec;            /* receive timestamp */
        tstamp  xmt;            /* transmit timestamp */
} ntpformat;

typedef struct _DATE_TIME
{
        uint8 year[2];
        uint8 month;
        uint8 day;
        uint8 hour;
        uint8 minute;
        uint8 second;
} DATE_TIME;

typedef struct _NTP_CONFIG
{ 
        // NTP Configinfo
        uint8 ntp_domain[4];          // Main Timeserver IP address
        uint8 ntp_domain_backup[4];   // Backup Timeserver IP address
        uint8 time_zone;              // GMT+-
        uint8 update_period[2];       // [0]: time number, [1]: time units (0-min, 1-hour)
        int8 gmt_hour;                // GMT Hour : Range [-12 ~ +13]
        uint8 gmt_min;                // GMT Minute : Range [0 ~ 59] / [0 or 30]
        DATE_TIME date;               // Manual input date time when NTP is not avaliable; 2013/01/01 01:00:00
} NTP_CONFIG;

//------------------------------ NTP Setting : User define ------------------------------
#define UTC_HOUR                    9       // SEOUL : GMT+9
#define UTC_MINUTE                  0

#define DEFAULT_NTP_DOMAIN_1	    "143.248.25.208"    // NTP Time Server IP Address, (kr.pool.ntp.org)
#define DEFAULT_NTP_DOMAIN_2	    "222.112.219.60"    // Backup NTP Time Server IP Address, (kr.pool.ntp.org)

#define MAX_NTP_RETRY_COUNT_1       3       // TimeServer1 : maximum retry count
#define NTP_RETRY_DELAY_SEC_1       2       // TimeServer1 : delay seconds 
#define MAX_NTP_RETRY_COUNT_2       3       // TimeServer2 : maximum retry count (backup server)
#define NTP_RETRY_DELAY_SEC_2       2       // TimeServer2 : delay seconds (backup server)

//------------------------------ NTP Setting : Defined ------------------------------
#define SOCK_NTP                    7       // UDP(Time)
#define NTP_PORT                    123     // NTP server port number

#define EPOCH                       1900    // NTP start year
#define SECS_PERDAY     	        86400UL // Seconds in a day = 60*60*24

#define TIMESERVER1                 1       // Default TimeServer
#define TIMESERVER2                 2       // Backup TimeServer

#define DEFAULT_NTP_YEAR            2013    // Default year
#define DEFAULT_NTP_MONTH           1       // Default month
#define DEFAULT_NTP_DAY             1       // Default day
#define DEFAULT_NTP_HOUR            1       // Default hour
#define DEFAULT_NTP_MINUTE          0       // Default minute
#define DEFAULT_NTP_SECOND          0       // Default second
//---------------------------------------------------------------------------------------

void set_ntp_default(void);
void set_ntp_client_msg(uint8 timeserver);
int8 do_ntp_client(void);

void get_seconds_from_ntp_server(uint8* buf,uint16 idx);
tstamp changedatetime_to_seconds(void);
void calcdatetime(tstamp seconds);

void ntp_time_disp(NTP_CONFIG *ConfigMsg);
void get_time_zone(NTP_CONFIG *ConfigMsg);

#endif //_NTP_H