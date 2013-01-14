

#ifndef _FTPC_H
#define _FTPC_H

#include "common/common.h"

/*
// Responses 

#define R_110	110		// Restart marker reply (MARK yyyy = mmmm) 
#define R_120	120		// Svc ready in nnn minutes 
#define R_125	125		// Data connection already open; starting 
#define R_150	150		// File status ok; opening data conn 
#define R_200	200		// 'Generic' command ok 
#define R_202	202		// Command not implemented, superflous at this site 
#define R_211	211		// System status or system help reply 
#define R_212	212		// Directory status 
#define R_213	213		// File status 
#define R_214	214		// Help message (how to use server or non-standard command) 
#define R_215	215		// NAME system type.  NAME == Official system name 
#define R_220	220		// Service ready for new user. 
#define R_221	221		// Service closing control connection, as per normal 
#define R_225	225		// Data connection open; no transfer in progress 
#define R_226	226		// Closing data connection.  File transfer/abort successful 
#define R_227	227		// Entering passive mode (h1,h2,h3,h4,p1,p2) 
#define	R_229	229		// Entering extended passive mode (|||p|) 
#define R_230	230		// User logged in, proceed 
#define R_232   232		// User logged in, authorized by security data 
#define R_234   234		// Security data exchange complete 
#define R_235   235		// Security exchange successful 

#define R_250	250		// Requested file action okay, completed. 
#define R_257	257		// "PATHNAME" created. 
#define R_331	331		// User name okay, need password. 
#define R_332	332		// Need account for login. 
#define R_334   334		// Security data required 
#define R_335   335		// Additional security data required 
#define R_336   336		// Username OK, need password; presenting challenge 
#define R_350	350		// Requested file action pending further info 
#define R_421	421		// Service not available, closing control connection (service is about to be shutdown) 
#define R_425	425		// Can't open data connection 
#define R_426	426		// Connection closed; transfer aborted 
#define R_431   431		// Necessary security resource is unavailable 
#define R_450	450		// Requested file action not taken (file unavailable; busy) 
#define R_451	451		// Requested action aborted; local error in processing 
#define R_452	452		// Requested action not taken; insufficient storage space 
#define	R_500	500		// Syntax error, command unrecognized 
#define R_501	501		// Syntax error in parameters or arguments 
#define R_502	502		// Command not implemented 
#define R_503	503		// Bad sequence of commands 
#define R_504	504		// Command not implemented for that parameter 
#define	R_522	522		// Extended port failure: unknown network protocol 
#define R_530	530		// Not logged in 
#define R_532	532		// Need account for storing files 
#define R_533   533		// Integrity protected command required by policy 
#define R_534   534		// Unwilling to accept security arguments 
#define R_535   535		// Data failed security check 
#define R_536   536		// Unsupported data channel protection level 
#define R_537   537		// Unsupported command protection by security mechanism 
#define R_550	550		// Requested action not taken. No access, etc 
#define R_551	551		// Requested action not taken, page type unknown 
#define R_552	552		// Requested file action aborted, exceeding storage allocation 
#define	R_553	553		// Requested action not taken, file name not allowed 
#define R_554   554           // Requested action not taken, invalid REST parameter (RFC 1123) 
#define R_631	631		// Integrity protected response (RFC 2228) 
#define R_632	632		// Privacy protected response (RFC 2228) 
#define R_633	633		// Confidentiality protected response (RFC 2228) 
#define R_DUP	NULL		// Duplicate last numeric in ml response 

#define FTP_Server_CMD_Port 21
#define FTP_Client_TCPS_Port 60001

#define STR_SIZE 100

#define PUTFILE  "This is the txt file for put cmd testing!!!!WIZnet!!!"
#define ls "NLST\r\n"
#define bye "QUIT\r\n"


void ftp_client_PI       	(uint8 sock);
void ftp_client_DTP    	  	(uint8 sock);


uint16  Parse_FTPServer_Statuscode	(uint8* dat);
void    Send_USER_ID       	        (uint8 sock,char* msg);
void	Send_PW				(uint8 sock,char* msg);
void 	Send_Port			(uint8 sock, uint8* ip, uint32 port);
void 	Send_cmd			(uint8 sock, char* msg);
*/

#endif //_FTPC_H
