
//#define FILE_LOG_SILENCE
#include "protocol/SMTP/smtp.h"


static char buf[255];
static uint16 client_port = 2000;

/*
********************************************************************************
*              MAKE MIME MESSAGE
*
* Description : This function makes MIME message.
* Arguments   : MIME - is a pointer to the destination.
                sender - is a pointer to the sender.
                recipient - is a pointer to the recipient.
                subject - is a pointer to the subject of mail.
                content - is a pointer to the content of mail.
* Returns     : none.
* Note        :
********************************************************************************
*/
void MakeMIME(uint8 * MIME, uint8 * sender, uint8 * recipient, uint8 * subject, uint8 * content)
{
	sprintf((void *)MIME,   "From: %s\r\n"\
				"To: %s\r\n"\
				"Subject: %s\r\n"\
				"\r\n%s\r\n"\
				"\r\n."\
				, sender, recipient, subject, content);
}

/*
********************************************************************************
*              SEND MESSAGE AND RECEIVE THE REPLY
*
* Description : This function makes DNS query message and parses the reply from DNS server.
* Arguments   : s - is a socket number.
                data - is a pointer to the data for send.
                pSip - is a pointer to the server ip.
* Returns     : none.
* Note        :
********************************************************************************
*/
void send_receive(uint8 s, uint8 * data, uint8 * pSip)
{
	int16 n;

	DBGA("Send------>%s", data);

	n = sprintf(buf, "%s\r\n", data);
	while(TCPSend(s, (void *)buf, n) <= 0);

	//recv
	while((n = TCPRecv(s, (void *)buf, 255)) <= 0);
	buf[n]='\0';

	DBGA("Receive------>%s", buf);
}

/*
********************************************************************************
*              SEND MAIL
*
* Description : This function send mail and parses the reply from mail server.
* Arguments   : s - is a socket number.
                sender - is a pointer to the sender.
                passwd - is a pointer to the password.
                recipient - is a pointer to the recipient.
                subject - is a pointer to the subject of mail.
                content - is a pointer to the content of mail.
                pSip - is a pointer to the server ip.
* Returns     : if succeeds : 1, fails : 0
* Note        :
********************************************************************************
*/
uint8 send_mail(uint8 s, uint8 * sender, uint8 * passwd, uint8 * recipient, uint8 * subject, uint8 * content, uint8 * pSip)
{
	uint8 MIME[256]={'\0'}, encode[16]={'\0'}, ret=1;
	int16 n;

	// connnect to server
	if (TCPClientOpen(s, client_port++, pSip, 25) == 1) {
		do{

			//recv
			while((n = TCPRecv(s, (void *)buf, 255)) <= 0);
			buf[n]='\0';
			DBGA("Receive(%d)------>%s", n, buf);
			if(strncmp(buf, "220", 3)) {
				ERR("This is not SMTP Server");
				ret = 0;
				break;
			}

			// Send HELO
			send_receive(s, "EHLO", pSip);
			if(strncmp(buf, "250", 3)) {
				ERR("Fail HELO");
				ret = 0;
				break;
			}

			// Send AUTH LOGIN
			send_receive(s, "AUTH LOGIN", pSip);
			if(strncmp(buf, "334", 3)) {
				ERR("Fail AUTH LOGIN");
				ret = 0;
				break;
			}

			// Send ID
			base64_encode((void *)sender, strlen((void *)sender)+1, (void *)encode);
			send_receive(s, encode, pSip);
			if(strncmp(buf, "334", 3)) {
				ERR("Fail ID");
				ret = 0;
				break;
			}

			// Send PW
			base64_encode((void *)passwd, strlen((void *)passwd)+1, (void *)encode);
			send_receive(s, encode, pSip);
			if(strncmp(buf, "235", 3)) {
				ERR("Fail PW");
				ret = 0;
				break;
			}

			// Send MAIL FROM
			sprintf(buf, "MAIL FROM:%s", sender);
			send_receive(s, (void *)buf, pSip);
			if(strncmp(buf, "250", 3)) {
				ERR("Fail MAIL FROM");
				ret = 0;
				break;
			}

			// Send RCPT
			sprintf(buf, "RCPT TO:%s", recipient);
			send_receive(s, (void *)buf, pSip);
			if(strncmp(buf, "250", 3)) {
				ERR("Fail RCPT TO");
				ret = 0;
				break;
			}

			// Send DATA
			send_receive(s, "DATA", pSip);
			if(strncmp(buf, "354", 3)) {
				ERR("Fail DATA");
				ret = 0;
				break;
			}

			// Send content
			MakeMIME(MIME, sender, recipient, subject, content);
			send_receive(s, MIME, pSip);
			if(strncmp(buf, "250", 3)) {
				ERR("Fail Send Content");
				ret = 0;
				break;
			}

			// Send QUIT
			send_receive(s, "QUIT", pSip);
			NL1;
			LOG("Send ok");

		}while(0);

	}else {
		NL1;
		ERR("connection error");
		return(0);
	}

	TCPClose(s);
	while(getSn_SR(s) != SOCK_CLOSED);

	return(ret);
}
