
#ifndef	_USERMENU_H
#define	_USERMENU_H

#include "common/common.h"


#define MAX_MENU_COUNT	10
#define CMD_BUF_SIZE	100

typedef enum {MC_START, MC_END, MC_DATA} menu_ctrl;

typedef int8 (*menu_func)(menu_ctrl mctrl, char *mbuf);

void menu_init(void);
void menu_print_tree(void);
int8 menu_add(char *desc, int8 parent, menu_func mfunc);
void menu_run(void);

#endif	//_USERMENU_H



