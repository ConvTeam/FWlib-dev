/**
 * @file		ntp.c
 * @brief		SNTP Protocol Module Source File
 * @version	    1.0
 * @date		2013/03/11
 * @par Revision
 *			2013/02/11 - 1.0 Release
 * @author	modified by HK Jung
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#include "protocol/NTP/ntp.h"

extern uint8 msgbuf[128];

#define TX_RX_MAX_BUF_SIZE	2048
//--------------------------------- NTP TimeServer Setting ---------------------------------
//uint8 DEFAULT_NTP_DOMAIN_1[4] = {143,248,25,208}; // NTP Time Server, kr.pool.ntp.org
//uint8 DEFAULT_NTP_DOMAIN_2[4] = {222,112,219,60}; // Backup NTP Time Server, kr.pool.ntp.org
//------------------------------------------------------------------------------------------
//uint8 DEFAULT_NTP_DOMAIN_1[4];
//uint8 DEFAULT_NTP_DOMAIN_2[4];

NTP_CONFIG ConfigMsg;
uint8 NTPBUF[TX_RX_MAX_BUF_SIZE];

ntpformat NTPformat;
uint8 ntpmessage[48];
tstamp totalseconds = 0;

/**
 * @addtogroup sntp_module
 * @{
 */

/**
 * Initialize SNTP module.
 * This should be called just one time at first time
 *
 * @param none
 * @return none
 */
void set_ntp_default(void)
{ 
  // Date & Time & NTP   
  if(ip_check(DEFAULT_NTP_DOMAIN_1, ConfigMsg.ntp_domain) != RET_OK) {
    ERR("TimeServer1 IP Address set fail");
  }
  if(ip_check(DEFAULT_NTP_DOMAIN_2, ConfigMsg.ntp_domain_backup) != RET_OK) {
    ERR("TimeServer2 IP Address set fail");
  }  
  ConfigMsg.gmt_hour = UTC_HOUR;
  ConfigMsg.gmt_min = UTC_MINUTE;
  get_time_zone(&ConfigMsg);        // Set ConfigMsg.time_zone using UTC_HOUR and UTC_MINUTE
  
  // Manual input date & time; 2013/01/01 01:00:00
  ConfigMsg.date.year[0] = (DEFAULT_NTP_YEAR & 0xff00)>>8;
  ConfigMsg.date.year[1] = DEFAULT_NTP_YEAR & 0xff;
  ConfigMsg.date.month = DEFAULT_NTP_MONTH;
  ConfigMsg.date.day = DEFAULT_NTP_DAY;
  ConfigMsg.date.hour = DEFAULT_NTP_HOUR;
  ConfigMsg.date.minute = DEFAULT_NTP_MINUTE;
  ConfigMsg.date.second = DEFAULT_NTP_SECOND;
}  

/**
 * Make NTP Client Message.
 * This function makes NTP Client message.
 * 
 * @param TimeServer Selecter(1: Default server, 2: Backup server)
 */
void set_ntp_client_msg(uint8 timeserver)
{
   if(timeserver != 2)
   {
       NTPformat.dstaddr[0] = ConfigMsg.ntp_domain[0];
       NTPformat.dstaddr[1] = ConfigMsg.ntp_domain[1];
       NTPformat.dstaddr[2] = ConfigMsg.ntp_domain[2];
       NTPformat.dstaddr[3] = ConfigMsg.ntp_domain[3];   
   }
   else 
   {
       NTPformat.dstaddr[0] = ConfigMsg.ntp_domain_backup[0];
       NTPformat.dstaddr[1] = ConfigMsg.ntp_domain_backup[1];
       NTPformat.dstaddr[2] = ConfigMsg.ntp_domain_backup[2];
       NTPformat.dstaddr[3] = ConfigMsg.ntp_domain_backup[3];
   }  
  
   uint8 Flag;
   NTPformat.leap = 0;           /* leap indicator */ 
   NTPformat.version = 4;        /* version number */
   NTPformat.mode = 3;           /* mode */
   NTPformat.stratum = 0;        /* stratum */
   NTPformat.poll = 0;           /* poll interval */
   NTPformat.precision = 0;      /* precision */
   NTPformat.rootdelay = 0;      /* root delay */
   NTPformat.rootdisp = 0;       /* root dispersion */
   NTPformat.refid = 0;          /* reference ID */
   NTPformat.reftime = 0;        /* reference time */
   NTPformat.org = 0;            /* origin timestamp */
   NTPformat.rec = 0;            /* receive timestamp */
   NTPformat.xmt = 1;            /* transmit timestamp */
   
   Flag = (NTPformat.leap<<6)+(NTPformat.version<<3)+NTPformat.mode; //one byte Flag
   memcpy(ntpmessage,(void const*)(&Flag),1);
}
/**
 * NTP Client process handler.
 * - Blocking Function
 * - do_ntp_client() function does not need a loop structure
 *
 * @param none 
 * @return RET_OK: Success
 * @return RET_NOK: Failed
 */
int8 do_ntp_client(void)
{    
    uint16 RSR_len;
    uint8 * data_buf = NTPBUF;
    uint32 destip = 0;
    uint16 destport;
    uint16 startindex = 40; // last 8-byte of data_buf[size is 48 byte] is xmt, so the startindex should be 40
    uint8 ntp_endflag = 0;       
    int8 ret = RET_OK;
    
    uint8 ntp_retry_cnt_1 = 0; // counting the ntp retry number for Server 1 (Default time server)
    uint8 ntp_retry_cnt_2 = 0; // counting the ntp retry number for Server 2 (Backup time server)
    
    static uint32 tick = 0; // Timeout handler implementation using [systick]
    
    do
    {        
        //switch(getSn_SR(SOCK_NTP))
        switch(GetUDPSocketStatus(SOCK_NTP))
        {
          //case SOCK_UDP:            
          case SOCKSTAT_UDP:
            if(ntp_retry_cnt_1 < MAX_NTP_RETRY_COUNT_1 + 1)
            {
                if(ntp_retry_cnt_1 == 0) // first send request, no need to wait
                { 
                    LOGA("Try to connect to TimeServer1 [%d.%d.%d.%d] ...", ConfigMsg.ntp_domain[0], ConfigMsg.ntp_domain[1], ConfigMsg.ntp_domain[2], ConfigMsg.ntp_domain[3]);
                    
                    UDPSend(SOCK_NTP, (int8*)ntpmessage, sizeof(ntpmessage), ConfigMsg.ntp_domain, NTP_PORT);
                    ntp_retry_cnt_1++;
                    
                    // Timeout handler implementation using [Systick]
                    tick = wizpf_get_systick();
                }
                // Retry connect to TimeServer1 (default time server)
                else // send request again? it should wait for a while
                {
                    // Timeout handler implementation using [Systick]                    
                    if(wizpf_tick_elapse(tick) > (NTP_RETRY_DELAY_SEC_1 * 1000))
                    {
                        UDPSend(SOCK_NTP, (int8*)ntpmessage, sizeof(ntpmessage), ConfigMsg.ntp_domain, NTP_PORT);                        
                        
                        LOGA("NTP Retry: %d", ntp_retry_cnt_1);
                        ntp_retry_cnt_1++;
                        tick = wizpf_get_systick();
                    }  
                }      
            }
            else if(ntp_retry_cnt_2 < MAX_NTP_RETRY_COUNT_2 + 1)// TimeServer2
            {                
                if(ntp_retry_cnt_2 == 0) // first send request to backup timeserver, no need to wait
                { 
                    LOGA("Try to connect to TimeServer2 [%d.%d.%d.%d] ...", ConfigMsg.ntp_domain_backup[0], ConfigMsg.ntp_domain_backup[1], ConfigMsg.ntp_domain_backup[2], ConfigMsg.ntp_domain_backup[3]);
                    
                    set_ntp_client_msg(TIMESERVER2);
                    
                    UDPSend(SOCK_NTP, (int8*)ntpmessage, sizeof(ntpmessage), ConfigMsg.ntp_domain_backup, NTP_PORT);
                    ntp_retry_cnt_2++;
                    
                    // Timeout handler implementation using [Systick]
                    tick = wizpf_get_systick();                   
                }                 
                // Retry connect to TimeServer2 (backup time server)
                else 
                {
                    // Timeout handler implementation using [Systick]                    
                    if(wizpf_tick_elapse(tick) > (NTP_RETRY_DELAY_SEC_2 * 1000))
                    {
                        UDPSend(SOCK_NTP, (int8*)ntpmessage, sizeof(ntpmessage), ConfigMsg.ntp_domain, NTP_PORT);                        
                        
                        LOGA("NTP Retry: %d", ntp_retry_cnt_2);
                        ntp_retry_cnt_2++;
                        tick = wizpf_get_systick();
                    }                   
                }     
            }  
            else //ntp retry fail
            { 
                ntp_retry_cnt_1 = 0;
                ntp_retry_cnt_2 = 0;
                
                LOG("NTP Retry failed");
             
                ntp_endflag = 1; // NTP endflag
                set_ntp_client_msg(TIMESERVER1);
                
                UDPClose(SOCK_NTP); 
                
                ret = RET_NOK;
            }
            
            //if ((RSR_len = getSn_RX_RSR(SOCK_NTP)) > 0) 		
            if((RSR_len = GetSocketRxRecvBufferSize(SOCK_NTP)) > 0)
            {
                if (RSR_len > TX_RX_MAX_BUF_SIZE) RSR_len = TX_RX_MAX_BUF_SIZE;	// if Rx data size is lager than TX_RX_MAX_BUF_SIZE 
                UDPRecv(SOCK_NTP, (int8*)data_buf, RSR_len, (uint8*)&destip, &destport);	
                
                get_seconds_from_ntp_server(data_buf, startindex);
                
                LOG("NTP Success");
                ntp_endflag = 1; // NTP endflag
                
                UDPClose(SOCK_NTP);                
                
                set_ntp_client_msg(TIMESERVER1);
                ntp_retry_cnt_1 = 0;   
                ntp_retry_cnt_2 = 0;
            }            
            break;
          
          case SOCKSTAT_CLOSED:
            UDPOpen(SOCK_NTP, NTP_PORT);
            break;
        }
    } while(!ntp_endflag);  
    
    return ret;
}


void ntp_time_disp(NTP_CONFIG *ConfigMsg)
{
	uint16 year; 
    year = ((ConfigMsg->date.year[0] << 8) & 0xff00) | ConfigMsg->date.year[1];
    
	LOG("---------------------------------------");
	LOG(" NTP : Current Network Time Information  ");
    LOGA(" TimeServer1: %d.%d.%d.%d", ConfigMsg->ntp_domain[0], ConfigMsg->ntp_domain[1], ConfigMsg->ntp_domain[2], ConfigMsg->ntp_domain[3]);
    LOGA(" TimeServer2: %d.%d.%d.%d", ConfigMsg->ntp_domain_backup[0], ConfigMsg->ntp_domain_backup[1], ConfigMsg->ntp_domain_backup[2], ConfigMsg->ntp_domain_backup[3]);
	LOG("---------------------------------------");
    if(ConfigMsg->gmt_hour >= 0) {
    	LOGA("  GMT +%.2d:%.2d", ConfigMsg->gmt_hour, ConfigMsg->gmt_min);
    } else {
        LOGA("  GMT %.2d:%.2d", ConfigMsg->gmt_hour, ConfigMsg->gmt_min);
    }
    LOGA("  %.4d / %.2d / %.2d", year, ConfigMsg->date.month, ConfigMsg->date.day); // Year / Month / Day
    LOGA("  %.2d : %.2d : %.2d", ConfigMsg->date.hour, ConfigMsg->date.minute, ConfigMsg->date.second); // Hour : Minute : Second
    LOG("---------------------------------------");
}


void get_seconds_from_ntp_server(uint8* buf,uint16 idx)
{
    tstamp seconds = 0;
    uint8 i=0;
    for (i = 0; i < 4; i++)
    {
      seconds = (seconds << 8) | buf[idx + i];
    }
    switch (ConfigMsg.time_zone)
    {
      case 0:
        seconds -=  12*3600;
        break;
      case 1:
        seconds -=  11*3600;
        break;
      case 2:
        seconds -=  10*3600;
        break;
      case 3:
        seconds -=  (9*3600+30*60);
        break;
      case 4:
        seconds -=  9*3600;
        break;
      case 5:
      case 6:
        seconds -=  8*3600;
        break;
      case 7:
      case 8:
        seconds -=  7*3600;
        break;
      case 9:
      case 10:
        seconds -=  6*3600;
        break;
      case 11:
      case 12:
      case 13:
        seconds -= 5*3600;
        break;
      case 14:
        seconds -=  (4*3600+30*60);
        break;
      case 15:
      case 16:
        seconds -=  4*3600;
        break;
      case 17:
        seconds -=  (3*3600+30*60);
        break;
      case 18:
        seconds -=  3*3600;
        break;
      case 19:
        seconds -=  2*3600;
        break;
      case 20:
        seconds -=  1*3600;
        break;
      case 21:                            //?
      case 22:
        break;
      case 23:
      case 24:
      case 25:
        seconds +=  1*3600;
        break;
      case 26:
      case 27:
        seconds +=  2*3600;
        break;
      case 28:
      case 29:
        seconds +=  3*3600;
        break;
      case 30:
        seconds +=  (3*3600+30*60);
        break;
      case 31:
        seconds +=  4*3600;
        break;
      case 32:
        seconds +=  (4*3600+30*60);
        break;
      case 33:
        seconds +=  5*3600;
        break;
      case 34:
        seconds +=  (5*3600+30*60);
        break;
      case 35:
        seconds +=  (5*3600+45*60);
        break;
      case 36:
        seconds +=  6*3600;
        break;
      case 37:
        seconds +=  (6*3600+30*60);
        break;
      case 38:
        seconds +=  7*3600;
        break;
      case 39:
        seconds +=  8*3600;
        break;
      case 40:
        seconds +=  9*3600;
        break;
      case 41:
        seconds +=  (9*3600+30*60);
        break;
      case 42:
        seconds +=  10*3600;
        break;
      case 43:
        seconds +=  (10*3600+30*60);
        break;
      case 44:
        seconds +=  11*3600;
        break;
      case 45:
        seconds +=  (11*3600+30*60);
        break;
      case 46:
        seconds +=  12*3600;
        break;
      case 47:
        seconds +=  (12*3600+45*60);
        break;
      case 48:
        seconds +=  13*3600;
        break;
      case 49:
        seconds +=  14*3600;
        break;   
    }
    
    totalseconds=seconds;
    //calculation for date
    calcdatetime(seconds); 
}



void calcdatetime(tstamp seconds)
{
    uint8 yf=0;
    tstamp n=0,d=0,total_d=0,rz=0;
    uint16 y=0,r=0,yr=0;
    signed long long yd=0;
    
    n = seconds;
    total_d = seconds/(SECS_PERDAY);
    d=0;
    uint32 p_year_total_sec=SECS_PERDAY*365;
    uint32 r_year_total_sec=SECS_PERDAY*366;
    while(n>=p_year_total_sec) 
    {
      if((EPOCH+r)%400==0 || ((EPOCH+r)%100!=0 && (EPOCH+r)%4==0))
      {
        n = n -(r_year_total_sec);
        d = d + 366;
      }
      else
      {
        n = n - (p_year_total_sec);
        d = d + 365;
      }
      r+=1;
      y+=1;
    
    }
    
    y += EPOCH;
 
    ConfigMsg.date.year[0] = (uint8)((y & 0xff00)>>8);
    ConfigMsg.date.year[1] = (uint8)(y & 0xff);
    
    yd=0;
    yd = total_d - d;
    
    yf=1;
    while(yd>=28) 
    {
        
        if(yf==1 || yf==3 || yf==5 || yf==7 || yf==8 || yf==10 || yf==12)
        {
          yd -= 31;
          if(yd<0)break;
          rz += 31;
        }
    
        if (yf==2)
        {
          if (y%400==0 || (y%100!=0 && y%4==0)) 
          {
            yd -= 29;
            if(yd<0)break;
            rz += 29;
          }
          else 
          {
            yd -= 28;
            if(yd<0)break;
            rz += 28;
          }
        } 
        if(yf==4 || yf==6 || yf==9 || yf==11 )
        {
          yd -= 30;
          if(yd<0)break;
          rz += 30;
        }
        yf += 1;
        
    }
    ConfigMsg.date.month=yf;
    yr = total_d-d-rz;
   
    yr += 1;
    
    ConfigMsg.date.day=yr;
    
    //calculation for time
    seconds = seconds%SECS_PERDAY;
    ConfigMsg.date.hour = seconds/3600;
    ConfigMsg.date.minute = (seconds%3600)/60;
    ConfigMsg.date.second = (seconds%3600)%60;    
}

tstamp changedatetime_to_seconds(void) 
{
  tstamp seconds=0;
  uint32 total_day=0;
  uint16 i=0,run_year_cnt=0,l=0;
  
  l = ConfigMsg.date.year[0];//high
  
  l = (l<<8);
  
  l = l + ConfigMsg.date.year[1];//low
 
  
  for(i=EPOCH;i<l;i++)
  {
    if((i%400==0) || ((i%100!=0) && (i%4==0))) 
    {
      run_year_cnt += 1;
    }
  }
  
  total_day=(l-EPOCH-run_year_cnt)*365+run_year_cnt*366;

  for(i=1;i<=ConfigMsg.date.month;i++)
  {
    if(i==5 || i==7 || i==10 || i==12)
    {
      total_day += 30;
    }
    if (i==3)
    {
      if (l%400==0 && l%100!=0 && l%4==0) 
      {
        total_day += 29;
      }
      else 
      {
        total_day += 28;
      }
    } 
    if(i==2 || i==4 || i==6 || i==8 || i==9 || i==11)
    {
      total_day += 31;
    }
  }
 
  seconds = (total_day+ConfigMsg.date.day-1)*24*3600;  
  seconds += ConfigMsg.date.second;//seconds
  seconds += ConfigMsg.date.minute*60;//minute
  seconds += ConfigMsg.date.hour*3600;//hour
  
  return seconds;
}


void get_time_zone(NTP_CONFIG *ConfigMsg)
{
      
    switch (ConfigMsg->gmt_hour)
    {
      case -12:
        ConfigMsg->time_zone = 0;
        break;
      case -11:
        ConfigMsg->time_zone = 1;
        break;
      case -10:
        ConfigMsg->time_zone = 2;
        break;
      case -9:
        if(ConfigMsg->gmt_min > 0) ConfigMsg->time_zone = 3;
        else ConfigMsg->time_zone = 4;
        break;
      case -8:
        ConfigMsg->time_zone = 5;   // 5, 6
        break;
      case -7:
        ConfigMsg->time_zone = 7;   // 7, 8
        break;
      case -6:
        ConfigMsg->time_zone = 9;   // 9, 10
        break;
      case -5:
        ConfigMsg->time_zone = 11;  // 11, 12, 13
        break;
      case -4:
        if(ConfigMsg->gmt_min > 0) ConfigMsg->time_zone = 14;
        else ConfigMsg->time_zone = 15; // 15, 16
        break;
      case -3:
        if(ConfigMsg->gmt_min > 0) ConfigMsg->time_zone = 17;
        else ConfigMsg->time_zone = 18;
        break;
      case -2:
        ConfigMsg->time_zone = 19;
        break;
      case -1:
        ConfigMsg->time_zone = 20;
        break;
      case 0:
        ConfigMsg->time_zone = 21;  // 21, 22
        break;
      case 1:
        ConfigMsg->time_zone = 23;  // 23, 24, 25
        break;
      case 2:
        ConfigMsg->time_zone = 26;  // 26, 27
        break;
      case 3:
        if(ConfigMsg->gmt_min == 0) ConfigMsg->time_zone = 28;  // 28, 29        
        else ConfigMsg->time_zone = 30;
        break;
      case 4:
        if(ConfigMsg->gmt_min == 0) ConfigMsg->time_zone = 31;
        else ConfigMsg->time_zone = 32;
        break;
      case 5:
        if(ConfigMsg->gmt_min == 0) ConfigMsg->time_zone = 33;
        else if(ConfigMsg->gmt_min == 30) ConfigMsg->time_zone = 34;
        else ConfigMsg->time_zone = 35;
        break;
      case 6:
        if(ConfigMsg->gmt_min == 0) ConfigMsg->time_zone = 36;
        else ConfigMsg->time_zone = 37;
        break;
      case 7:
        ConfigMsg->time_zone = 38; 
        break;
      case 8:
        ConfigMsg->time_zone = 39; 
        break;
      case 9:
        if(ConfigMsg->gmt_min == 0) ConfigMsg->time_zone = 40;
        else ConfigMsg->time_zone = 41;
        break;
      case 10:
        if(ConfigMsg->gmt_min == 0) ConfigMsg->time_zone = 42;
        else ConfigMsg->time_zone = 43;
        break;
      case 11:
        if(ConfigMsg->gmt_min == 0) ConfigMsg->time_zone = 44;
        else ConfigMsg->time_zone = 45;
        break;
      case 12:
        if(ConfigMsg->gmt_min == 0) ConfigMsg->time_zone = 46;
        else ConfigMsg->time_zone = 47;
        break;
      case 13:
        ConfigMsg->time_zone = 48; 
        break;
      case 14:
        ConfigMsg->time_zone = 49; 
        break;
      default :
        ConfigMsg->time_zone = 40;  // Seoul, GMT +9:00
        break;
    }
}
