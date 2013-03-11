/**
 * @file		usermenu.h
 * @brief		User Menu (Terminal) Module Header File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

#ifndef	_USERMENU_H
#define	_USERMENU_H

#include "common/common.h"


/**
 * @ingroup usermenu_module
 * Maximum number of User Menu Definition.
 * - If not set in the wizconfig.h, default value is 10.
 * - The more number, the more memory use.
 * - If there is not enough number, menu_add will fail
 */
#ifndef MAX_MENU_COUNT		// If you want different value, define this at wizconfig.h
#define MAX_MENU_COUNT	10
#endif

/**
 * @ingroup usermenu_module
 * Maximum Terminal Input Buffer Size.
 * - If not set in the wizconfig.h, default value is 100 Byte.
 * - The more size, the more memory use.
 * - If there is not enough size, user cannot input all they want
 */
#ifndef CMD_BUF_SIZE		// If you want different value, define this at wizconfig.h
#define CMD_BUF_SIZE	100
#endif

/**
 * @ingroup usermenu_module
 * Menu Control Signal.
 * With this signal, Callback function can determine user action
 * @see @ref menu_func
 */
typedef enum {
	MC_START, 	///< When user entered into a menu
	MC_END, 	///< When user exited out of a menu
	MC_DATA		///< While user is in a menu
} menu_ctrl;

/**
 * @ingroup usermenu_module
 * User Menu Callback Function Definition.
 * The function form with which a user menu function will be added to the menu
 * @param mctrl With this signal, Callback function can determine user action (@ref menu_ctrl)
 * @param mbuf The string buffer which user input
 * @return RET_OK: Return to parent menu
 * @return RET_NOK: Stay current menu
 */
typedef int8 (*menu_func)(menu_ctrl mctrl, int8 *mbuf);

void menu_init(void);
void menu_print_tree(void);
int8 menu_add(int8 *desc, int8 parent, menu_func mfunc);
void menu_run(void);


#endif	//_USERMENU_H



