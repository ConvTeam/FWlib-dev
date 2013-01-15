
#include "httputil.h"
#include "webpage.h"
#include "at24c32.h"

#include "ntp.h"

extern CONFIG_MSG  ConfigMsg;
extern char tx_buf[MAX_URI_SIZE];
extern char rx_buf[MAX_URI_SIZE];

uint8 *homepage_default = "index.html";
uint8 bchannel_start;
uint8 *user_data;
//web page password session; check web page password is received or not
static uint8 pwd_rxd=0;

//timer
extern vu16 cgi_post_wait_time;
extern vu8 http_time;
//
extern uint32 totalseconds;
//


extern uint16 ka_interval_val;
extern uint16 ka_count_val;

extern uint16 nagle_val;
extern uint16 inact_val;
extern uint16 recon_val;

extern uint16 lport_val;
extern uint16 rport_val;
extern uint16 locport_val;
//processing http protocol , and excuting the followed fuction.
void do_http(void)
{
  u_char ch=SOCK_HTTP;
  int len;

  st_http_request *http_request;
  memset(rx_buf,0x00,MAX_URI_SIZE);
  http_request = (st_http_request*)rx_buf;		// struct of http request
  
  /* http service start */
  switch(getSn_SR(ch))
  {
    case SOCK_INIT:
      listen(ch);
      break;
    case SOCK_LISTEN:
      if ((len = getSn_RX_RSR(ch)) > 0)
        if(ConfigMsg.debug) printf("listen: %d\r\n",len);
      break;
    case SOCK_ESTABLISHED:
    //case SOCK_CLOSE_WAIT:
      if(getSn_IR(ch) & Sn_IR_CON)
      {
        setSn_IR(ch, Sn_IR_CON);
      }
      if ((len = getSn_RX_RSR(ch)) > 0)		
      {
        
        //if ((u_int)len > MAX_URI_SIZE) len = MAX_URI_SIZE;				
        len = recv(ch, (uint8*)http_request, len);       
        *(((u_char*)http_request)+len) = 0;
        proc_http(ch, (u_char*)http_request); // request is processed
        disconnect(ch);
      }
   
      break;
   
    case SOCK_CLOSE_WAIT:   
      if ((len = getSn_RX_RSR(ch)) > 0)
      {
        //printf("close wait: %d\r\n",len);
        len = recv(ch, (uint8*)http_request, len);       
        *(((u_char*)http_request)+len) = 0;
        proc_http(ch, (u_char*)http_request); // request is processed
      }
      disconnect(ch);
      break;
   
    case SOCK_CLOSED:                   
      socket(ch, Sn_MR_TCP, DEFAULT_HTTP_PORT, 0x00);    /* reinitialize the socket */
      break;
    default:
    break;
  }// end of switch 
  //check http timeout
  if(pwd_rxd)
  {
    if(http_time>=HTTP_PWD_TIMEOUT)
    {
      http_time=HTTP_PWD_TIMEOUT;
      //pwd_rxd=0;
    }
  }
}


void proc_http(SOCKET s, u_char * buf)
{
  char* name;
  unsigned long file_len;
  uint16 send_len;
  //uint32 content = 0;
  
  uint8* http_response;
  st_http_request *http_request;
  
  memset(tx_buf,0x00,MAX_URI_SIZE);
  http_response = (u_char*)rx_buf;

  http_request = (st_http_request*)tx_buf;
    
  parse_http_request(http_request, buf);    // After analyze request, convert into http_request
  
  //method Analyze
  switch (http_request->METHOD)				
  {
    case METHOD_ERR :	
      memcpy(http_response, ERROR_REQUEST_PAGE, sizeof(ERROR_REQUEST_PAGE));
      send(s, (u_char *)http_response, strlen((char const*)http_response),0);
      break;
    case METHOD_HEAD:
    case METHOD_GET:
    case METHOD_POST:
      //get file name from uri
      name=http_request->URI;
      if(strcmp(name,"/"))
          name++;
     
      //protect the config.html page. if corrected password is not received, then the page will not display.
      if(!strcmp(name,"config.html"))
      {
        //printf("time=%d\r\n",http_time);
        if(pwd_rxd!=1)
        {
          make_pwd_response(0,2,tx_buf,0);//no pwd, do NOT care about pwd timeout
          sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
          send(s, (u_char *)http_response, strlen((char const*)http_response),0);
          return;
        }
        else if(http_time>=HTTP_PWD_TIMEOUT)
        {
          make_pwd_response(0,2,tx_buf,1);//pwd timeout
          sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
          send(s, (u_char *)http_response, strlen((char const*)http_response),0);
          return;
        }
        else
          http_time=0;//reset timer
      }
      if (!strcmp(name, "/")) strcpy(name, (char const*)homepage_default);	// If URI is "/", respond by index.htm 

      find_http_uri_type(&http_request->TYPE, name);	//Check file type (HTML, TEXT, GIF, JPEG are included)
      
      if(http_request->TYPE == PTYPE_PL)
      {
        //printf("typepl=%s\r\n",name);
        if(strstr(name,NET_CONFIG_CGI))
        {
          if(*(((u_char*)http_request)+strlen((char*)http_request)-1) == 0x0a)//post data in 2 packets
          {
            uint16 len=0;
            uint16 content_len=0;
            int8 sub[10];
            //get Content-Length  
            mid((char*)http_request,"Content-Length: ","\r\n",sub);
            content_len=ATOI(sub,10);
            //printf("content len=%d\r\n",content_len);
            while(len!=content_len)
            {
              len=getSn_RX_RSR(SOCK_HTTP);
              
              if(len>=content_len)
                len=recv(SOCK_HTTP, (u_char*)(http_request)+strlen((char*)http_request), content_len);
              
              //a timeout is needed!
              if(cgi_post_wait_time>=1)//if post data does not come with in 5 minutes
              {
                memcpy(http_response, ERROR_HTML_PAGE, sizeof(ERROR_REQUEST_PAGE));
                send(s, (u_char *)http_response, strlen((char const*)http_response),0);
                //disconnect(s);
                //pwd_rxd=0;//clear password session flag
                return;
              }
            }
          }
          if((pwd_rxd==1) && (http_time<HTTP_PWD_TIMEOUT))
          {
            //
            //printf("http config\r\n");
            cgi_ipconfig(http_request);
            //printf("http config ok\r\n");
            //
            //build response cgi page html
            make_cgi_response(5,(int8*)ConfigMsg.lip,tx_buf);         
            sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
            send(s, (u_char *)http_response, strlen((char *)http_response),0);		
            //disconnect(s);
            reboot();
            return;
          }
          else
          {
            if(pwd_rxd==0)
              make_pwd_response(0,2,tx_buf,0);//pwd wrong; do NOT care timeout
            else
              make_pwd_response(0,2,tx_buf,1);//timeout
            sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
            send(s, (u_char *)http_response, strlen((char *)http_response),0);
            return;
          }
        }
        else if(strstr(name,NET_PWD_CGI))
        {
          if(*(((u_char*)http_request)+strlen((char*)http_request)-1) == 0x0a)//post data in 2 packets
          {
            uint16 len=0;
            uint16 content_len=0;
            int8 sub[10];
            //get Content-Length  
            mid((char*)http_request,"Content-Length: ","\r\n",sub);
            content_len=ATOI(sub,10);
            while(len!=content_len)
            {
              len=getSn_RX_RSR(SOCK_HTTP);
              if(len>=content_len)
                len=recv(SOCK_HTTP, (u_char*)(http_request)+strlen((char*)http_request), content_len);
              //printf("\r\nrx_len=%d %s\r\n",len,(char*) http_request);
              //a timeout is needed!
              if(cgi_post_wait_time>=1)//if post data does not come with in 5 minutes
              {
                if(ConfigMsg.debug) printf("pwd page response timeout %d\r\n", cgi_post_wait_time);
                memcpy(http_response, ERROR_HTML_PAGE, sizeof(ERROR_REQUEST_PAGE));
                send(s, (u_char *)http_response, strlen((char const*)http_response),0);
                //disconnect(s);
                //printf("pwd page response timeout\r\n");
                cgi_post_wait_time=0;
                return;
              }
              
            }

          }
               
          //check password
          uint8* param;
          if((param = get_http_param_value(name,"pwd")))
          {
            
            uint8 digest[16];
            uint8 isRight=1;
            //md5(param) and compare with saved md5 digest
            //make response
            md5(digest,param);
            for(uint16 i=0;i<16;i++)
            {
              if(digest[i]!=ConfigMsg.pwd[i])
              {
                isRight=0;
                break;
              }
            }  
            pwd_rxd=isRight;//set the pwd session flag
            http_time=0;//reset the http timer
            make_pwd_response(isRight,3,tx_buf,0);//do NOT care about the pwd timeout
            sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
            send(s, (u_char *)http_response, strlen((char const*)http_response),0);
          }  
          else
          {
            if(strlen((int8*)param)==0)//if no pwd input, it is the same mean that the password is incorrect!
            {
              make_pwd_response(0,2,tx_buf,0);//do NOT care about the pwd timeout
              sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
              send(s, (u_char *)http_response, strlen((char const*)http_response),0);
            }
          }
        }
        else if(strstr(name,"widget.pl"))//widget response
        {
          if((pwd_rxd==1) && (http_time<HTTP_PWD_TIMEOUT))
          {
            http_time=0;//reset timer
            //printf("send widget.pl\r\n");
            
            memset(tx_buf,0,MAX_URI_SIZE);
            //update datetime
            if(totalseconds) calcdatetime(totalseconds);
            if(ConfigMsg.dns_flag==0)
              //                                        devicname      serialnumber                  mac address                  local ip address       local port     gateway ip address         subnet mask          dns server ipaddress    remote host ip         remote port     locating server        loc port       ka interval    ka count        ntp server ip addr         manual input datetime                 nagle         inactivity    reconnection    dhcp         rs232       mode                 baudrate    databit    parity    stopbit     flow      timezon
              sprintf(tx_buf,"WidgetCallback({\"txt\":[{\"v\":\"%s\"},{\"v\":\"%s\"},{\"v\":\"%02X:%02X:%02X:%02X:%02X:%02X\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d\"},{\"v\":\"%d\"},{\"v\":\"%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d-%02d-%02d %02d:%02d:%02d\"},{\"v\":\"%d\"},{\"v\":\"%d\"},{\"v\":\"%d\"}],\"dhcp\":%d,\"rs232\":%d,\"mode\":%d,\"sel\":[{\"v\":%d},{\"v\":%d},{\"v\":%d},{\"v\":%d},{\"v\":%d},{\"v\":%d}],\"loc\":%d,\"ka\":%d,\"ntp\":%d,\"pwd\":\"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\"});",
                                             ConfigMsg.device_name, ConfigMsg.device_serial,ConfigMsg.mac[0],ConfigMsg.mac[1],ConfigMsg.mac[2],ConfigMsg.mac[3],ConfigMsg.mac[4],ConfigMsg.mac[5],ConfigMsg.lip[0],ConfigMsg.lip[1], ConfigMsg.lip[2], ConfigMsg.lip[3],lport_val,ConfigMsg.gw[0],ConfigMsg.gw[1],ConfigMsg.gw[2],ConfigMsg.gw[3],ConfigMsg.sub[0],ConfigMsg.sub[1],ConfigMsg.sub[2],ConfigMsg.sub[3],ConfigMsg.dns[0],ConfigMsg.dns[1],ConfigMsg.dns[2],ConfigMsg.dns[3],ConfigMsg.rip[0],ConfigMsg.rip[1],ConfigMsg.rip[2],ConfigMsg.rip[3],rport_val, 
                                             ConfigMsg.loc_server_ip[0],ConfigMsg.loc_server_ip[1],ConfigMsg.loc_server_ip[2],ConfigMsg.loc_server_ip[3],locport_val,ka_interval_val,ka_count_val,
                                             ConfigMsg.ntp_domain[0],ConfigMsg.ntp_domain[1],ConfigMsg.ntp_domain[2],ConfigMsg.ntp_domain[3],toUint16(ConfigMsg.date.year),ConfigMsg.date.month,ConfigMsg.date.day,ConfigMsg.date.hour,ConfigMsg.date.minute,ConfigMsg.date.second,nagle_val,inact_val,recon_val,ConfigMsg.dhcp,ConfigMsg.type,ConfigMsg.mode,ConfigMsg.baudrate,ConfigMsg.datasize,ConfigMsg.parity,ConfigMsg.stopbit,ConfigMsg.flowcontrol,ConfigMsg.time_zone,ConfigMsg.loc_flag,ConfigMsg.ka_flag,ConfigMsg.ntp_flag,ConfigMsg.pwd[0],ConfigMsg.pwd[1],ConfigMsg.pwd[2],ConfigMsg.pwd[3],ConfigMsg.pwd[4],ConfigMsg.pwd[5],ConfigMsg.pwd[6],ConfigMsg.pwd[7],ConfigMsg.pwd[8],ConfigMsg.pwd[9],ConfigMsg.pwd[10],ConfigMsg.pwd[11],ConfigMsg.pwd[12],ConfigMsg.pwd[13],ConfigMsg.pwd[14],ConfigMsg.pwd[15]); 
            else
              //                                        devicname      serialnumber                  mac address                  local ip address       local port     gateway ip address         subnet mask          dns server ipaddress    remote host name         remote port     locating server        loc port          ka interval    ka count        ntp server ip addr         manual input datetime                 nagle         inactivity    reconnection    dhcp         rs232       mode                 baudrate    databit    parity    stopbit     flow      timezon
              sprintf(tx_buf,"WidgetCallback({\"txt\":[{\"v\":\"%s\"},{\"v\":\"%s\"},{\"v\":\"%02X:%02X:%02X:%02X:%02X:%02X\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%s\"},{\"v\":\"%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d\"},{\"v\":\"%d\"},{\"v\":\"%d\"},{\"v\":\"%d.%d.%d.%d\"},{\"v\":\"%d-%02d-%02d %02d:%02d:%02d\"},{\"v\":\"%d\"},{\"v\":\"%d\"},{\"v\":\"%d\"}],\"dhcp\":%d,\"rs232\":%d,\"mode\":%d,\"sel\":[{\"v\":%d},{\"v\":%d},{\"v\":%d},{\"v\":%d},{\"v\":%d},{\"v\":%d}],\"loc\":%d,\"ka\":%d,\"ntp\":%d,\"pwd\":\"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\"});",
                                             ConfigMsg.device_name, ConfigMsg.device_serial,ConfigMsg.mac[0],ConfigMsg.mac[1],ConfigMsg.mac[2],ConfigMsg.mac[3],ConfigMsg.mac[4],ConfigMsg.mac[5],ConfigMsg.lip[0],ConfigMsg.lip[1], ConfigMsg.lip[2], ConfigMsg.lip[3],lport_val,ConfigMsg.gw[0],ConfigMsg.gw[1],ConfigMsg.gw[2],ConfigMsg.gw[3],ConfigMsg.sub[0],ConfigMsg.sub[1],ConfigMsg.sub[2],ConfigMsg.sub[3],ConfigMsg.dns[0],ConfigMsg.dns[1],ConfigMsg.dns[2],ConfigMsg.dns[3],ConfigMsg.domain,rport_val, 
                                             ConfigMsg.loc_server_ip[0],ConfigMsg.loc_server_ip[1],ConfigMsg.loc_server_ip[2],ConfigMsg.loc_server_ip[3],locport_val,ka_interval_val,ka_count_val,
                                             ConfigMsg.ntp_domain[0],ConfigMsg.ntp_domain[1],ConfigMsg.ntp_domain[2],ConfigMsg.ntp_domain[3],toUint16(ConfigMsg.date.year),ConfigMsg.date.month,ConfigMsg.date.day,ConfigMsg.date.hour,ConfigMsg.date.minute,ConfigMsg.date.second,nagle_val,inact_val,recon_val,ConfigMsg.dhcp,ConfigMsg.type,ConfigMsg.mode,ConfigMsg.baudrate,ConfigMsg.datasize,ConfigMsg.parity,ConfigMsg.stopbit,ConfigMsg.flowcontrol,ConfigMsg.time_zone,ConfigMsg.loc_flag,ConfigMsg.ka_flag,ConfigMsg.ntp_flag,ConfigMsg.pwd[0],ConfigMsg.pwd[1],ConfigMsg.pwd[2],ConfigMsg.pwd[3],ConfigMsg.pwd[4],ConfigMsg.pwd[5],ConfigMsg.pwd[6],ConfigMsg.pwd[7],ConfigMsg.pwd[8],ConfigMsg.pwd[9],ConfigMsg.pwd[10],ConfigMsg.pwd[11],ConfigMsg.pwd[12],ConfigMsg.pwd[13],ConfigMsg.pwd[14],ConfigMsg.pwd[15]); 
          
            
            sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
            send(s, (u_char *)http_response, strlen((char const*)http_response),0);
            
          }
          else
          {
            if(pwd_rxd==0)
              make_pwd_response(0,2,tx_buf,0);//pwd wrong; do NOT care timeout
            else
              make_pwd_response(0,2,tx_buf,1);//timeout
            sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
            send(s, (u_char *)http_response, strlen((char *)http_response),0);
            return;
          }
        }
      }
      else
      {
        //printf("type=%s\r\n",name);
        //download log message
        if(strstr(name,"log_msg.bin"))//
        {
          if((pwd_rxd==1) && (http_time<HTTP_PWD_TIMEOUT))
          {
            http_time=0;//reset timer
            if(ConfigMsg.debug) printf("send log_msg.bin\r\n");
            memset(tx_buf,0,MAX_URI_SIZE);
            uint8 log_cnt = at24c32_read(log_msg_count_addr);
            //check the real log message count
            if((log_cnt!=200) && (log_cnt>0))
            {
              if(at24c32_read(log_msg_addr+log_cnt*12)!=0xff)
                log_cnt=200;
            }
            
            if(log_cnt>0)
            {  
              uint16 real_total_log_len=log_cnt*12;
              sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: application/octet\r\nContent-Length:%d\r\n\r\n",real_total_log_len);
              send(s, (uint8 *)http_response, strlen((char const*)http_response),0);
              //todo... read EEPROM
              for(uint8 i=0;i<log_cnt;i++)
              {
                int8 log[12];
                //read_log_msg(log,log_cnt*12,12);
                for(uint8 j=0;j<12;j++)
                {
                  log[j]=at24c32_read(log_msg_addr+j+i*log_msg_len);
                }
                send(s,(uint8*)log,12,0);
              }
              /*
              read_log_msg(tx_buf,0,log_msg_total_len/2);
              //
              send(s, (uint8*)tx_buf,log_msg_total_len/2,0);
              
              read_log_msg(tx_buf,log_msg_total_len/2,log_msg_total_len/2);
              
              send(s, (uint8*)tx_buf,log_msg_total_len/2,0);
              */
            }
            else
            {
              sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",34,"There is no any log message saved!");
              send(s, (uint8 *)http_response, strlen((char const*)http_response),0);
            }
          }
          else
          {
            if(pwd_rxd==0)
              make_pwd_response(0,2,tx_buf,0);//pwd wrong; do NOT care timeout
            else
              make_pwd_response(0,2,tx_buf,1);//timeout
            sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
            send(s, (u_char *)http_response, strlen((char *)http_response),0);
            return;
          }
        }
        //view log message in text
        if(strstr(name,"log_msg.txt"))//
        {
          if((pwd_rxd==1) && (http_time<HTTP_PWD_TIMEOUT))
          {
            http_time=0;//reset timer
            if(ConfigMsg.debug) printf("send log_msg.txt\r\n");
            memset(tx_buf,0,MAX_URI_SIZE);
            //text format: 37 bytes in total
            //001. 2012.01.02 00:00:00 Case Close/r/n
            uint8 log_cnt = at24c32_read(log_msg_count_addr);
            //check the real log message count
            if((log_cnt!=200) && (log_cnt>0))
            {
              if(at24c32_read(log_msg_addr+log_cnt*12)!=0xff)
              {
                printf("%02x ", at24c32_read(log_msg_addr+log_cnt*12));
                log_cnt=200;
              }
                
            }
            //printf(" log_cnt=%d\r\n",log_cnt);
            //so total length = 37*log_cnt
            if(log_cnt>0)
            {
              uint16 real_total_log_len=37*log_cnt;
              sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length:%d\r\n\r\n",real_total_log_len);
              
              send(s, (uint8 *)http_response, strlen((char const*)http_response),0);
              for(uint8 i=0;i<log_cnt;i++)
              {
                int8 log[12];
                //read_log_msg(log,log_cnt*12,12);
                for(uint8 j=0;j<12;j++)
                {
                  log[j]=at24c32_read(log_msg_addr+j+i*log_msg_len);
                }
                uint16 year=log[0]*256+log[1];
                sprintf((int8*)http_response, "%03d. %d.%02d.%02d %02d:%02d:%02d Case %s\r\n", i+1, year, log[2], log[3], log[4], log[5], log[6], ((log[8]==0) ? "Close":"Open "));
                send(s, (uint8 *)http_response, strlen((char const*)http_response),0);
              }
             
            }
            else
            {
              sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",34,"There is no any log message saved!");
              send(s, (uint8 *)http_response, strlen((char const*)http_response),0);
            }
          }
          else
          {
            if(pwd_rxd==0)
              make_pwd_response(0,2,tx_buf,0);//pwd wrong; do NOT care timeout
            else
              make_pwd_response(0,2,tx_buf,1);//timeout
            sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
            send(s, (u_char *)http_response, strlen((char *)http_response),0);
            return;
          }
        }
        
      }
      
      if(strcmp(name,"index.html")==0)
      {
        file_len=strlen(INDEX_HTML);
        make_http_response_head((unsigned char*)http_response, http_request->TYPE, (u_long)file_len);
        send(s, http_response, strlen((char const*)http_response),0);
        send(s, (u_char *)INDEX_HTML, strlen((char const*)INDEX_HTML),0);
      }
      else if(strcmp(name,"config.html")==0)
      {
        file_len=strlen(CONFIG_HTML);
        make_http_response_head((unsigned char*)http_response, http_request->TYPE, (u_long)file_len);
        send(s, http_response, strlen((char const*)http_response),0);
        send_len=0;
        while(file_len)
        {
          if(file_len>1024)
          {
            if(getSn_SR(s)!=SOCK_ESTABLISHED)
            {
              return;
            }
            send(s, (u_char *)CONFIG_HTML+send_len, 1024,0);
            send_len+=1024;
            file_len-=1024;
          }
          else
          {
            send(s, (u_char *)CONFIG_HTML+send_len, file_len,0);
            send_len+=file_len;
            file_len-=file_len;
          }
        }
      }
        
      else
      {
        memcpy(http_response, ERROR_HTML_PAGE, sizeof(ERROR_HTML_PAGE));
        send(s, (u_char *)http_response, strlen((char const*)http_response),0);
      }
      
      break;
    default :
      break;
  }
}


void cgi_ipconfig(st_http_request *http_request)
{
  
  u_char * param;
  u_char * ntpptr;
  //Device setting
  if(param = get_http_param_value(http_request->URI,"devicename"))//device name
  {
    memset(ConfigMsg.device_name,0x00,16);
    memcpy(ConfigMsg.device_name,param,strlen((char*)param));    
  }
  if(param = get_http_param_value(http_request->URI,"dhcp"))
  {
    if(strstr((char const*)param,"static")!= NULL)
    {
      ConfigMsg.dhcp = STATIC_MODE;
    }
    if(strstr((char const*)param,"dhcp")!= NULL)
    {
     ConfigMsg.dhcp = DHCP_MODE;
    }
  }
  if((param = get_http_param_value(http_request->URI,"sip")))
  {
    inet_addr_((u_char*)param, ConfigMsg.lip);	
  }
  if((param = get_http_param_value(http_request->URI,"gwip")))
  {
    inet_addr_((u_char*)param, ConfigMsg.gw);	
  }
  if((param = get_http_param_value(http_request->URI,"sn")))
  {
    inet_addr_((u_char*)param, ConfigMsg.sub);		
  }
  //COM setting
  if((param = get_http_param_value(http_request->URI,"rs232")))
  {
    if(strstr((char const*)param,"rs232")!= NULL)
    {
      ConfigMsg.type=RS232;
    }
    if(strstr((char const*)param,"rs485")!= NULL)
    {
      ConfigMsg.type=RS485;
    }		
  }
  if((param = get_http_param_value(http_request->URI,"baudrate")))
  {
    if(strstr((char const*)param,"1200")!= NULL)
    {
       ConfigMsg.baudrate = BR_1200_INDEX;
    }
    if(strstr((char const*)param,"2400")!= NULL)
    {
      ConfigMsg.baudrate = BR_2400_INDEX;
    }
    if(strstr((char const*)param,"4800")!= NULL)
    {
      ConfigMsg.baudrate = BR_4800_INDEX;
    }
    if(strstr((char const*)param,"9600")!= NULL)
    {
       ConfigMsg.baudrate = BR_9600_INDEX;
    }
    if(strstr((char const*)param,"19200")!= NULL)
    {
       ConfigMsg.baudrate = BR_19200_INDEX;
    }
    if(strstr((char const*)param,"38400")!= NULL)
    {
       ConfigMsg.baudrate = BR_38400_INDEX;
    }	
    if(strstr((char const*)param,"57600")!= NULL)
    {
       ConfigMsg.baudrate = BR_57600_INDEX;
    }
    if(strstr((char const*)param,"115200")!= NULL)
    {
       ConfigMsg.baudrate = BR_115200_INDEX;
    }	
  }
  if((param = get_http_param_value(http_request->URI,"databits")))
  {
    if(strstr((char const*)param,"7")!= NULL)
    {
      ConfigMsg.datasize = DATABIT_7_INDEX;
    }
    else
      ConfigMsg.datasize = DATABIT_8_INDEX;
  }
  if((param = get_http_param_value(http_request->URI,"parity")))
  {
    if(strstr((char const*)param,"NONE")!= NULL)
    {
       ConfigMsg.parity = PARITY_NONE;
    }
    if(strstr((char const*)param,"ODD")!= NULL)
    {
      ConfigMsg.parity = PARITY_ODD;
    }
     if(strstr((char const*)param,"EVEN")!= NULL)
    {
      ConfigMsg.parity = PARITY_EVEN;
    }
  }
  if((param = get_http_param_value(http_request->URI,"stopbits")))
  {
    if(strstr((char const*)param,"1")!= NULL)
    {
         ConfigMsg.stopbit = STOPBIT_1_INDEX;
    }
  }
  if((param = get_http_param_value(http_request->URI,"flowcontrol")))
  {
    if(strstr((char const*)param,"NONE")!= NULL)
    {
         ConfigMsg.flowcontrol = FLOW_NONE;
    }
    if(strstr((char const*)param,"CTS/RTS")!= NULL)
    {
         ConfigMsg.flowcontrol = FLOW_RTSCTS;
    }
  }
  //Communication mode setting
  if((param = get_http_param_value(http_request->URI,"mode")))
  {
    if(strstr((char const*)param,"TCPC")!= NULL)
    {
       ConfigMsg.mode = TCP_CLIENT_MODE;
    }
    if(strstr((char const*)param,"TCPS")!= NULL)
    {
       ConfigMsg.mode = TCP_SERVER_MODE;
    }	
    if(strstr((char const*)param,"TCPM")!= NULL)
    {
      ConfigMsg.mode = TCP_MIXED_MODE;
    }
    if(strstr((char const*)param,"UDP")!= NULL)
    {
       ConfigMsg.mode = UDP_MODE;
    }	
  }
  if((param = get_http_param_value(http_request->URI,"localport")))
  {
    *(param+5)=0x00;
    uint16 l=ATOI((char*)param,10);
    ConfigMsg.lport[0]=(uint8)((l & 0xff00)>>8);
    ConfigMsg.lport[1]=(uint8)(l & 0xff);
  }
  if((param = get_http_param_value(http_request->URI,"remoteport")))
  {
    *(param+5)=0x00;
    uint16 l=ATOI((char*)param,10);
    ConfigMsg.rport[0]=(uint8)((l & 0xff00)>>8);
    ConfigMsg.rport[1]=(uint8)(l & 0xff);
  }
  if((param = get_http_param_value(http_request->URI,"remotehost")))
  {
    //remote host can be URL or IP address; it should be check if it is IP address or not
    if(is_ipaddr(param))
    {
      inet_addr_((u_char*)param, ConfigMsg.rip);
      ConfigMsg.dns_flag=0;
    }
    else
    {
      memset(ConfigMsg.domain,0x00,32);
      memcpy(ConfigMsg.domain,param,strlen((char*)param));
      ConfigMsg.dns_flag=1;
    }
  }
  if((param = get_http_param_value(http_request->URI,"enlocating")))
  {
    if(strstr((char const*)param,"enable")!= NULL)
    {
       ConfigMsg.loc_flag=1;
    }
  }
  else
      ConfigMsg.loc_flag=0;
  if((param = get_http_param_value(http_request->URI,"serverportname")))
  {
    *(param+5)=0x00;
    uint16 l=ATOI((char*)param,10);
    ConfigMsg.loc_server_port[0]=(uint8)((l & 0xff00)>>8);
    ConfigMsg.loc_server_port[1]=(uint8)(l & 0xff);
  }
  if((param = get_http_param_value(http_request->URI,"serveripname")))
  {
    inet_addr_((u_char*)param, ConfigMsg.loc_server_ip);
  }
  if((param = get_http_param_value(http_request->URI,"enkeepalive")))
  {
    if(strstr((char const*)param,"enable")!= NULL)
    {
       ConfigMsg.ka_flag=1;
    }
    //printf("ka\r\n");
  }
  else
  {   
    //printf("no ka\r\n");
    ConfigMsg.ka_flag=0;
  }
  if((param = get_http_param_value(http_request->URI,"keepinterval")))
  {
    *(param+5)=0x00;
    uint16 l=ATOI((char*)param,10);
    ConfigMsg.ka_interval[0]=(uint8)((l & 0xff00)>>8);
    ConfigMsg.ka_interval[1]=(uint8)(l & 0xff);
    //printf("ka interval: %s\r\n", param);
  }
 
  //not used from ver0.9
  /*
  if((param = get_http_param_value(http_request->URI,"keepinterval1")))
  {
    *(param+5)=0x00;
    uint16 l=ATOI((char*)param,10);
    ConfigMsg.ka_period[0]=(uint8)((l & 0xff00)>>8);
    ConfigMsg.ka_period[1]=(uint8)(l & 0xff);
  }
  if((param = get_http_param_value(http_request->URI,"keepinterval2")))
  {
    *(param+5)=0x00;
    uint16 l=ATOI((char*)param,10);
    ConfigMsg.ka_count[0]=(uint8)((l & 0xff00)>>8);
    ConfigMsg.ka_count[1]=(uint8)(l & 0xff);
  }
  */
  //2012 revised
  if((param = get_http_param_value(http_request->URI,"password")))
  {
    if(strlen((int8*)param)>0)
    {
      //printf("old pwd=%s\r\n",param);
      uint8 pwd[16];
      md5(pwd,(uint8*)param);
      uint8 isOldPwdRight=1;
      for(uint8 i=0;i<16;i++)
      {
        if(pwd[i]!=ConfigMsg.pwd[i])
        {
          isOldPwdRight=0;
          break;
        }
      }
      //sprintf(pwd,"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",ConfigMsg.pwd[0],ConfigMsg.pwd[1],ConfigMsg.pwd[2],ConfigMsg.pwd[3],ConfigMsg.pwd[4],ConfigMsg.pwd[5],ConfigMsg.pwd[6],ConfigMsg.pwd[7],ConfigMsg.pwd[8],ConfigMsg.pwd[9],ConfigMsg.pwd[10],ConfigMsg.pwd[11],ConfigMsg.pwd[12],ConfigMsg.pwd[13],ConfigMsg.pwd[14],ConfigMsg.pwd[15]);
      if(isOldPwdRight==1)//check the old password is input correctly
      {
        param=get_http_param_value(http_request->URI,"newpwd");
        memset(pwd,0x00,16);
        strncpy((int8*)pwd,(int8*)param,strlen((int8*)param));
        param=get_http_param_value(http_request->URI,"newpwd2");
        if(strcmp((int8*)pwd,(int8*)param)==0)//if newpwd==newpwd2, then change the password; else do nothing
        {
          memset(ConfigMsg.pwd,0x00,16);
          md5(ConfigMsg.pwd,(uint8*)param);
          if(ConfigMsg.debug) printf("pwd changed\r\n");
        }
      }
      //else
        //if(ConfigMsg.debug) printf("old pwd is wrong\r\n");
    }
    //else
      //if(ConfigMsg.debug) printf("no old pwd get\r\n");
  }
 
  //printf("mac3=%02X:%02X:%02X:%02X:%02X:%02X",ConfigMsg.mac[0],ConfigMsg.mac[1],ConfigMsg.mac[2],ConfigMsg.mac[3],ConfigMsg.mac[4],ConfigMsg.mac[5]);
  if((param = get_http_param_value(http_request->URI,"ntpenable")))
  {
    if(strstr((char const*)param,"enable")!= NULL)
    {
       ConfigMsg.ntp_flag=1;
    }
  }
  else
    ConfigMsg.ntp_flag=0;
  if((param = get_http_param_value(http_request->URI,"ntpserver")))
  {
    memset(ConfigMsg.ntp_domain,0x00,4);
    //memcpy(ConfigMsg.ntp_domain,param,strlen((char*)param));
    inet_addr_((u_char*)param, ConfigMsg.ntp_domain);
  }
  if((param = get_http_param_value(http_request->URI,"timezone")))
  {
    if(strstr((char const*)param,")UTC")!= NULL)
    {
      ntpptr = (uint8*)strstr((char const*)param,")UTC");
      ConfigMsg.time_zone =(uint8)((*(ntpptr-2)-'0')*10)+(uint8)(*(ntpptr-1)-'0');
    }
  }
  if(param = get_http_param_value(http_request->URI,"datetime"))//parsing date time: yyyy-MM-dd hh:mm:ss
  {
    if(strlen((int8*)param)==19)
    {
      int8 tmp[5];
      memset(tmp,0,5);
      strncpy(tmp,(int8*)param,4);
      uint16 l=ATOI((char*)tmp,10);
      ConfigMsg.date.year[0]=(uint8)((l & 0xff00)>>8);
      ConfigMsg.date.year[1]=(uint8)(l & 0xff);
      
      memset(tmp,0,5);
      param+=5;
      strncpy(tmp,(int8*)param,2);
      l=ATOI((char*)tmp,10);
      ConfigMsg.date.month=(uint8)(l & 0xff);
      
      memset(tmp,0,5);
      param+=3;
      strncpy(tmp,(int8*)param,2);
      l=ATOI((char*)tmp,10);
      ConfigMsg.date.day=(uint8)(l & 0xff);
      
      memset(tmp,0,5);
      param+=3;
      strncpy(tmp,(int8*)param,2);
      l=ATOI((char*)tmp,10);
      ConfigMsg.date.hour=(uint8)(l & 0xff);
      
      memset(tmp,0,5);
      param+=3;
      strncpy(tmp,(int8*)param,2);
      l=ATOI((char*)tmp,10);
      ConfigMsg.date.minute=(uint8)(l & 0xff);
      
      memset(tmp,0,5);
      param+=3;
      strncpy(tmp,(int8*)param,2);
      l=ATOI((char*)tmp,10);
      ConfigMsg.date.second=(uint8)(l & 0xff);
    }
    else
      if(ConfigMsg.debug) printf("date time format is wrong.\r\n");
  }
  //timer options
  if((param = get_http_param_value(http_request->URI,"nagle")))
  {
    *(param+5)=0x00;
    uint16 l=ATOI((char*)param,10);
    ConfigMsg.nagle[0]=(uint8)((l & 0xff00)>>8);
    ConfigMsg.nagle[1]=(uint8)(l & 0xff);
  }
  if((param = get_http_param_value(http_request->URI,"inact")))
  {
    *(param+5)=0x00;
    uint16 l=ATOI((char*)param,10);
    ConfigMsg.inactivity[0]=(uint8)((l & 0xff00)>>8);
    ConfigMsg.inactivity[1]=(uint8)(l & 0xff);
  }
  if((param = get_http_param_value(http_request->URI,"recon")))
  {
    *(param+5)=0x00;
    uint16 l=ATOI((char*)param,10);
    ConfigMsg.reconn[0]=(uint8)((l & 0xff00)>>8);
    ConfigMsg.reconn[1]=(uint8)(l & 0xff);
  }
  
  /* Program the network parameters received into eeprom */
  write_config_to_eeprom();
}

void trimp(uint8* src, uint8* dst, uint16 len)
{
  uint16 i;
  for(i=0;i<len;i++)
  {
    if(*(src+i)!=0x00) 
      *(dst+i)=*(src+i);
    else 
      *(dst+i)=0x20;
  }
}

void make_cgi_response(uint16 delay, int8* url,int8* cgi_response_content)
{
  sprintf(cgi_response_content,"<html><head><title>Nuri - Configuration</title><script language=javascript>j=%d;function func(){document.getElementById('delay').innerText=' '+j + ' ';j--;setTimeout('func()',1000);if(j==0)location.href='http://%d.%d.%d.%d/';}</script></head><body onload='func()'>please wait for a while, the module will boot in<span style='color:red;' id='delay'></span> seconds.</body></html>",delay,url[0],url[1],url[2],url[3]);
  return;
}

void make_pwd_response(int8 isRight,uint16 delay,int8* cgi_response_content, int8 isTimeout)
{
  if (isRight==1)
    sprintf(cgi_response_content,"<html><head><title>Password - Nuri</title><script language=javascript>j=%d;function func(){document.getElementById('delay').innerText=' '+j+' ';j--;setTimeout('func()',1000);if(j==0)location.href='config.html';}</script></head><body onload='func()'>Please wait for a while, the configuration page will be loaded automatically in<span style='color:red;' id='delay'></span> seconds.</body></html>",delay);
  else
  {
    if (isTimeout==0)
      sprintf(cgi_response_content,"<html><head><title>Password - Nuri</title><script language=javascript>j=%d;function func(){document.getElementById('delay').innerText=' '+j+' ';j--;setTimeout('func()',1000);if(j==0)location.href='index.html';}</script></head><body onload='func()'>The password you input is incorrect, please check it again. </br> This page will go back in<span style='color:red;' id='delay'></span> seconds.</body></html>",delay);
  }
  if(isTimeout==1)
    sprintf(cgi_response_content,"<html><head><title>Password - Nuri</title><script language=javascript>j=%d;function func(){document.getElementById('delay').innerText=' '+j+' ';j--;setTimeout('func()',1000);if(j==0)location.href='index.html';}</script></head><body onload='func()'>You have NOT do any operation more than 5 minutes, please input your password again. </br> This page will go back in<span style='color:red;' id='delay'></span> seconds.</body></html>",delay);
}

