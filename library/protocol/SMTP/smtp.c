/**
 * @file		smtp.c
 * @brief		SMTP (Simple Mail Transfer Protocol) Module Source File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

//#define FILE_LOG_SILENCE
#include "protocol/SMTP/smtp.h"


static int8 buf[255];
static uint16 client_port = 2000;


/*
 * Make MIME Message.
 * This function makes MIME message.
 * 
 * @param MIME pointer to the destination
 * @param sender pointer to the sender
 * @param recipient pointer to the recipient
 * @param subject pointer to the subject of mail
 * @param content pointer to the content of mail
 */
static void MakeMIME(uint8 *MIME, uint8 *sender, uint8 *recipient, uint8 *subject, uint8 *content)
{
	sprintf((void *)MIME,   "From: %s\r\n"\
				"To: %s\r\n"\
				"Subject: %s\r\n"\
				"\r\n%s\r\n"\
				"\r\n."\
				, sender, recipient, subject, content);
}

/*
 * Send Message and Receive The Reply.
 * This function makes DNS query message and parses the reply from DNS server.
 * 
 * @param s a socket number
 * @param data pointer to the data for send
 * @param pSip pointer to the server ipr
 */
static void send_receive(uint8 s, uint8 * data, uint8 * pSip)
{
	int16 n;

	DBGA("Send------>%s", data);

	n = sprintf((char*)buf, "%s\r\n", data);
	while(TCPSend(s, (void *)buf, n) <= 0);

	//recv
	while((n = TCPRecv(s, (void *)buf, 255)) <= 0);
	buf[n]='\0';

	DBGA("Receive------>%s", buf);
}

/**
 * @ingroup smtp_module
 * Send email.
 * This function send mail and parses the reply from mail server.
 * 
 * @param s a socket number
 * @param sender pointer to the sender
 * @param passwd pointer to the password
 * @param recipient pointer to the recipient
 * @param subject pointer to the subject of mail
 * @param content pointer to the content of mail
 * @param pSip pointer to the server ip
 * @return RET_OK: Success
 * @return RET_NOK: Error
 */
int8 send_mail(uint8 s, uint8 *sender, uint8 *passwd, 
	uint8 *recipient, uint8 *subject, uint8 *content, uint8 *pSip)
{
	int8 ret = RET_OK;
	uint8 MIME[256]={'\0'}, encode[16]={'\0'};
	int16 n;

	if (TCPClientOpen(s, client_port++, pSip, 25) == 1)		// connnect to server
	{
		do{
			while((n = TCPRecv(s, (void *)buf, 255)) <= 0);	//recv
			buf[n]='\0';
			DBGA("Receive(%d)------>%s", n, buf);
			if(strncmp((char*)buf, "220", 3)) {
				ERR("This is not SMTP Server");
				ret = RET_NOK;
				break;
			}
			send_receive(s, "HELO", pSip);	// Send HELO
			if(strncmp((char*)buf, "250", 3)) {
				ERR("Fail HELO");
				ret = RET_NOK;
				break;
			}
			send_receive(s, "AUTH LOGIN", pSip);	// Send AUTH LOGIN
			if(strncmp((char*)buf, "334", 3)) {
				ERR("Fail AUTH LOGIN");
				ret = RET_NOK;
				break;
			}
			base64_encode((void *)sender, strlen((void *)sender)+1, (void *)encode);	// Send ID
			send_receive(s, encode, pSip);
			if(strncmp((char*)buf, "334", 3)) {
				ERR("Fail ID");
				ret = RET_NOK;
				break;
			}
			base64_encode((void *)passwd, strlen((void *)passwd)+1, (void *)encode);	// Send PW
			send_receive(s, encode, pSip);
			if(strncmp((char*)buf, "235", 3)) {
				ERR("Fail PW");
				ret = RET_NOK;
				break;
			}
			sprintf((char*)buf, "MAIL FROM:%s", sender);	// Send MAIL FROM
			send_receive(s, (void *)buf, pSip);
			if(strncmp((char*)buf, "250", 3)) {
				ERR("Fail MAIL FROM");
				ret = RET_NOK;
				break;
			}

			sprintf((char*)buf, "RCPT TO:%s", recipient);	// Send RCPT
			send_receive(s, (void *)buf, pSip);
			if(strncmp((char*)buf, "250", 3)) {
				ERR("Fail RCPT TO");
				ret = RET_NOK;
				break;
			}
			send_receive(s, "DATA", pSip);	// Send DATA
			if(strncmp((char*)buf, "354", 3)) {
				ERR("Fail DATA");
				ret = RET_NOK;
				break;
			}
			MakeMIME(MIME, sender, recipient, subject, content);	// Send content
			send_receive(s, MIME, pSip);
			if(strncmp((char*)buf, "250", 3)) {
				ERR("Fail Send Content");
				ret = RET_NOK;
				break;
			}
			send_receive(s, "QUIT", pSip);	// Send QUIT
			NL1;
			LOG("Send ok");
		}while(0);

	}
	else
	{
		NL1;
		ERR("connection error");
		return(RET_NOK);
	}

	TCPClose(s);
	while(getSn_SR(s) != SOCK_CLOSED);

	return(ret);
}
