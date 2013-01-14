 /******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : hyperterminal.h
* Author             : MCD Application Team
* Date First Issued  : 05/10/2009
* Description        : This file provides all the headers of hyperterminal driver 
*                      functions.
********************************************************************************
* History:
*  09/15/2006 : Hyperterminal V1.00
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _HYPERTERMINAL_H
 #define _HYPERTERMINAL_H
 /* Includes ------------------------------------------------------------------*/
 #include "stdio.h"
 #include "string.h"
 #include "stm32f10x.h"
 /* Exported types ------------------------------------------------------------*/
 typedef  void (*pFunction)(void);
 /* Exported constants --------------------------------------------------------*/
 /* Constants used by Serial Command Line Mode */
 #define CMD_STRING_SIZE     128
 /* Exported macro ------------------------------------------------------------*/
 #define IS_AF(c)	((c >= 'A') && (c <= 'F'))
 #define IS_af(c)	((c >= 'a') && (c <= 'f'))
 #define IS_09(c)	((c >= '0') && (c <= '9'))
 #define ISVALIDHEX(c)	IS_AF(c) || IS_af(c) || IS_09(c)
 #define ISVALIDDEC(c)	IS_09(c)
 #define CONVERTDEC(c)	(c - '0')
 #define CONVERTHEX_alpha(c)	(IS_AF(c) ? (c - 'A'+10) : (c - 'a'+10))
 #define CONVERTHEX(c)   (IS_09(c) ? (c - '0') : CONVERTHEX_alpha(c))
 /* Exported functions ------------------------------------------------------- */
 void Int2Str(char *str ,u32 intnum);
 u8 Str2Int(char *inputstr,u32 *intnum);
 u8 GetIntegerInput(u32 *num);
 u8 SerialKeyPressed(char *key);
 char GetKey(void);
 void SerialPutChar(char c);
 void SerialPutString(char *s);
 void GetInputString(char * buffP);
 void Main_Menu(void);
#endif  /* _HYPERTERMINAL_H */

/*******************(C)COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
