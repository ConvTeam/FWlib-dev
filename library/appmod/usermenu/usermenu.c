/**
 * @file		usermenu.c
 * @brief		User Menu (Terminal) Module Source File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

//#define FILE_LOG_SILENCE
#include "appmod/usermenu/usermenu.h"


struct menu_item {
	//uint8 index;
	int8 parent;
	int8 *desc;
	menu_func mfunc;
};

struct menu_info {
	uint8 total;
	uint8 cur;
};

struct menu_item mtree[MAX_MENU_COUNT];
struct menu_info mi;


/**
 * @ingroup usermenu_module
 * Initialize Usermenu Module.
 */
void menu_init(void)
{
	memset(mtree, 0, sizeof(mtree));
	mi.total = 0;
	mi.cur = 0;
}

/**
 * @ingroup usermenu_module
 * Add Usermenu.
 *
 * @param desc Brief which will be displayed
 * @param parent Parent menu index. \n - Root index is 0 
 *		\n - A return value of this function can be used
 * @param mfunc The Callback function which will be called when user input enter key
 * @return >0: Registered menu index (this can be used as parent number) 
 * @return RET_NOK: Error
 */
int8 menu_add(int8 *desc, int8 parent, menu_func mfunc)
{
	int32 len;

	if(desc == NULL) {
		ERR("description string is NULL");
		return RET_NOK;
	} else if(parent != 0 && 
		(parent > mi.total || mtree[parent-1].mfunc != NULL))
	{
		if(parent > mi.total) ERR("wrong parent num");
		else ERR("parent menu should be NULL function");
		return RET_NOK;
	} else if(mi.total >= MAX_MENU_COUNT) {
		ERR("not enough menu space");
		return RET_NOK;
	}

	//mtree[mi.total].index = mi.total+1;
	len = strlen((char*)desc);
	mtree[mi.total].desc = malloc(len+1);
	if(mtree[mi.total].desc == NULL) {
		ERRA("malloc fail - size(%d)", len+1);
		return RET_NOK;
	}
	strcpy((char*)mtree[mi.total].desc, (char*)desc);
	//mtree[mi.total].desc = strdup(desc);
	mtree[mi.total].parent = parent;
	mtree[mi.total].mfunc = mfunc;
	mi.total++;

	return mi.total;
}

/**
 * @ingroup usermenu_module
 * Print Current Registered Menu.
 * This is for Debug or Check
 */
void menu_print_tree(void)
{
	uint8 i;

	printf("=== Menu Tree ===========================\r\n");
	printf(" Idx Exp Par Desc\r\n");
	printf("-----------------------------------------\r\n");
	for(i=0; i<MAX_MENU_COUNT; i++) {
		printf(" %-3d %c   %-3d %s\r\n", 
			i+1, mtree[i].mfunc==NULL?'v':' ', mtree[i].parent, mtree[i].desc);
	}
	printf("=========================================\r\n");
}

/**
 * @ingroup usermenu_module
 * Usermenu Handler.
 * This function should be run under main loop.
 */
void menu_run(void)
{
	int8 recv_char;
	static int8 buf[CMD_BUF_SIZE];
	bool disp = FALSE;
	int8 ret;
	uint8 i, cnt, tmp8;
	static uint8 depth = 1, buf_len = 0;

	recv_char = (int8)getchar_nonblk();
	if(recv_char == RET_NOK) return;	//printf("RECV: 0x%x\r\n", recv_char);

	if(isgraph(recv_char) == 0) {	//printf("ctrl\r\n");
		switch(recv_char) {
		case 0x0a:
			break;
		case 0x0d:			//printf("<ENT>\r\n");
			printf("\r\n");
			break;
		case 0x20:			//printf("<SP>\r\n");
			buf[buf_len] = 0;
			printf("\r\n");
			for(i=0; i<depth; i++) PUTCHAR('>');
			printf(" %s", buf);
			break;
		case 0x08:			//printf("<BS>\r\n");
			if(buf_len != 0) {
				buf_len--;
				buf[buf_len] = 0;
				printf("\b \b");
			}
			break;
		case 0x7f:			//printf("<DEL>\r\n"); 
			memset(buf, 0, CMD_BUF_SIZE);
			buf_len = 0;					//printf("DEL: cur(%d), mfunc(%c), depth(%d)\r\n", mi.cur, mtree[mi.cur-1].mfunc==NULL?'N':'Y', depth);
			if(mi.cur != 0) {
				if(mtree[mi.cur-1].mfunc)
					mtree[mi.cur-1].mfunc(MC_END, buf);
				mi.cur = mtree[mi.cur-1].parent;
				depth--;
			}
			printf("\r\n");
			break;
		case 0x1b:			//printf("<ESC>\r\n");
			break;
		}

	} else if(buf_len < CMD_BUF_SIZE-1){
		buf[buf_len++] = (uint8_t)recv_char;	//buf[buf_len] = 0;
		PUTCHAR(recv_char);	//printf(" buf(%c, %s)\r\n", recv_char, buf);
	} else {
		printf("input buffer stuffed\r\n");
	}

	if(recv_char != 0x0d && recv_char != 0x7f) return;		//LOGA("Command: %s", buf);

	if(mi.cur == 0 || mtree[mi.cur-1].mfunc == NULL)	// Out of the tem
	{
		if(buf_len != 0) {
			if(str_check(isdigit, buf) == RET_OK) {//printf("digit(%d)\r\n", atoi(buf));
				tmp8 = atoi((char*)buf);
				if(depth > 1 && tmp8 == 0) { // If 0 entered, return to upper menu
					if(mi.cur != 0) {
						if(mtree[mi.cur-1].mfunc)
							mtree[mi.cur-1].mfunc(MC_END, buf);
						mi.cur = mtree[mi.cur-1].parent;
						depth--;
						disp = TRUE;
					} else printf("return tried despite root");
				} else {	// If not 0, search that menu
					for(i=0; i<mi.total; i++) {
						if(mi.cur == mtree[i].parent) {//printf("-i(%d)\r\n", i);	
							if(tmp8 == 1) break;
							else tmp8--;
						}
					}

					if(i < mi.total) {		//DBGA("-set cur(%d)", tmp8);
						cnt = mi.cur;
						mi.cur = i+1;
						if(mtree[mi.cur-1].mfunc) {
							ret = mtree[mi.cur-1].mfunc(MC_START, buf);
							if(ret == RET_OK) {
								mtree[mi.cur-1].mfunc(MC_END, buf);
								mi.cur = cnt;
							} else {
								depth++;
							}
						} else {
							depth++;
							disp = TRUE;
						}
					} else printf("wrong number(%s)\r\n", buf);
				}
			} else printf("not digit(%s)\r\n", buf);
		} else disp = TRUE;
	}
	else	// In the Item
	{
		if(buf_len == 0) {
			mtree[mi.cur-1].mfunc(MC_END, buf);
			mi.cur = mtree[mi.cur-1].parent;
			depth--;
			disp = TRUE;
		} else {
			ret = mtree[mi.cur-1].mfunc(MC_DATA, buf);
			if(ret != RET_OK) {	//printf("process continue\r\n");
			} else {			//printf("process done\r\n");
				mtree[mi.cur-1].mfunc(MC_END, buf);
				mi.cur = mtree[mi.cur-1].parent;
				depth--;
			}
		}
	}

	if(disp) {
		printf("\r\n=== MENU ================================\r\n");
		if(depth > 1) printf("  0: Back\r\n");
		for(i=0, cnt=0; i<mi.total; i++) {
			if(mi.cur == mtree[i].parent) {
				cnt++;
				if(mtree[i].mfunc == NULL) {
					if(cnt < 10) 
						 printf(" +%d: %s\r\n", cnt, mtree[i].desc);
					else printf("+%2d: %s\r\n", cnt, mtree[i].desc);
				} else {
					if(cnt < 10) 
						 printf("  %d: %s\r\n", cnt, mtree[i].desc);
					else printf(" %2d: %s\r\n", cnt, mtree[i].desc);
				}
			}
		}
		printf("=========================================\r\n");
	}

	memset(buf, 0, CMD_BUF_SIZE);
	buf_len = 0;
	printf("\r\n");
	for(i=0; i<depth; i++) PUTCHAR('>');
	PUTCHAR(' ');

}


