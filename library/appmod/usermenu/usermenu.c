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


enum disp_mode_e {
	umdm_none,
	umdm_min,
	umdm_max
};

struct menu_item {
	//uint8 index;
	int8 parent;
	int8 *desc;
	menu_func mfunc;
};

struct menu_info {
	uint8 total;
	uint8 cur;
	uint8 depth;
	int8 buf[CMD_BUF_SIZE];
	uint8 buf_len;
	bool software_input;
};

#if   defined(MENU_DISP_MODE) && (MENU_DISP_MODE == 0)
static const int8 disp_mode = umdm_none;
#elif defined(MENU_DISP_MODE) && (MENU_DISP_MODE == 1)
static const int8 disp_mode = umdm_min;
#else
static const int8 disp_mode = umdm_max;
#endif
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
	mi.depth = 1;
	mi.buf_len = 0;
	if(disp_mode == umdm_max) mi.software_input = TRUE;
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

	if(disp_mode == umdm_none) return;

	printf("=== Menu Tree ===========================\r\n");
	printf(" Idx Exp Par Desc\r\n");
	printf("-----------------------------------------\r\n");
	for(i=0; i<MAX_MENU_COUNT; i++) {
		printf(" %-3d %c   %-3d %s\r\n", 
			i+1, mtree[i].mfunc==NULL?'v':' ', mtree[i].parent, mtree[i].desc);
	}
	printf("=========================================\r\n");
}

#define SET_ROOT_MENU() do { \
	mi.cur = 0; \
	mi.depth = 1; \
	if(disp_mode == umdm_max) menu_disp(); \
} while(0)

#define CMDLINE_RESET(newline_b) do { \
	uint8 i; \
	if(disp_mode != umdm_none) { \
		if(newline_b) { \
			printf("\r\n"); \
			for(i=0; i<mi.depth; i++) putc('>', WIZ_USART1); \
			putc(' ', WIZ_USART1); \
		} else { \
			for(i=0; i<mi.buf_len; i++) putc('\b', WIZ_USART1); \
			for(i=0; i<mi.buf_len; i++) putc(' ', WIZ_USART1); \
			for(i=0; i<mi.buf_len; i++) putc('\b', WIZ_USART1); \
		} \
	} \
	mi.buf_len = 0; \
} while(0)

#define SET_UPPER_MENU(endfunc_b, cond_v) do { \
	if(mi.cur != 0) { \
		if(endfunc_b && mtree[mi.cur-1].mfunc) \
			mtree[mi.cur-1].mfunc(MC_END, mi.buf); \
		mi.cur = mtree[mi.cur-1].parent; \
		mi.depth--; \
	} \
	if(cond_v) { \
		printf("\r\n"); \
		menu_disp(); \
	} \
} while(0)

/**
 * @ingroup usermenu_module
 * Usermenu Handler.
 * This function should be run under main loop.
 */
void menu_run(void)
{
	int8 ret = RET_NOK;
	int8 recv_char;

	if(mi.software_input == TRUE) {
		mi.software_input = FALSE;
		recv_char = 0x0d;
	} else {
		recv_char = (int8)getc_nonblk(WIZ_USART1);
		if(recv_char == RET_NOK) return;	//printf("RECV: 0x%x\r\n", recv_char);
	}

	if(isgraph(recv_char) == 0) {	//printf("ctrl\r\n");
		switch(recv_char) {
		case 0x0a:
			break;
		case 0x0d:			//printf("<ENT>\r\n");
			mi.buf[mi.buf_len] = 0;
			if(disp_mode != umdm_none) printf("\r\n");
			break;
		case 0x20:			//printf("<SP>\r\n");
			CMDLINE_RESET(FALSE);
			break;
		case 0x08:			//printf("<BS>\r\n");
			if(mi.buf_len != 0) {
				mi.buf_len--;
				if(disp_mode != umdm_none) printf("\b \b");
			}
			break;
		case 0x7f:			//printf("<DEL>\r\n");
			mi.buf_len = 0;					//printf("DEL: cur(%d), mfunc(%c), depth(%d)\r\n", mi.cur, mtree[mi.cur-1].mfunc==NULL?'N':'Y', mi.depth);
			SET_UPPER_MENU(TRUE, disp_mode == umdm_max);
			CMDLINE_RESET(TRUE);
			break;
		case 0x1b:			//printf("<ESC>\r\n");
			break;
		}

	} else if(mi.buf_len < CMD_BUF_SIZE-1){
		mi.buf[mi.buf_len++] = (uint8_t)recv_char;	//mi.buf[mi.buf_len] = 0;
		if(disp_mode != umdm_none) putc(recv_char, WIZ_USART1);	//printf(" buf(%c, %s)\r\n", recv_char, mi.buf);
	} else {
		if(disp_mode != umdm_none) printf("input buffer stuffed\r\n");
	}

	if(recv_char != 0x0d) return;		//LOGA("Command: %s", mi.buf);

	if(mi.cur == 0 || mtree[mi.cur-1].mfunc == NULL)	// Out of the Item
	{
		if(mi.buf_len != 0) {
			if(str_check(isdigit, mi.buf) == RET_OK) {		//printf("digit(%d)\r\n", atoi(mi.buf));
				uint8 tmp8 = atoi((char*)mi.buf);

				if(mi.cur != 0 && tmp8 == 0) { // If 0 entered, return to upper menu
					SET_UPPER_MENU(TRUE, disp_mode != umdm_none);
				} else if(tmp8 != 0) {	// If not 0, search that menu
					uint8 i;

					for(i=0; i<mi.total; i++) {
						if(mi.cur == mtree[i].parent) {//printf("-i(%d)\r\n", i);	
							if(tmp8 == 1) break;
							else tmp8--;
						}
					}

					if(i < mi.total) {		//DBGA("-set cur(%d)", tmp8);
						mi.cur = i+1;
						mi.depth++;
						if(mtree[mi.cur-1].mfunc) {
							ret = mtree[mi.cur-1].mfunc(MC_START, mi.buf);
							if(ret == RET_OK) SET_UPPER_MENU(TRUE, disp_mode == umdm_max);
							else if(ret == RET_ROOT) SET_ROOT_MENU();
						} else {
							if(disp_mode != umdm_none) menu_disp();
						}
					} else if(disp_mode != umdm_none) 
						printf("wrong number(%s)\r\n", mi.buf);
				}  else if(disp_mode != umdm_none) 
					printf("wrong number(%s)\r\n", mi.buf);
			} else if(disp_mode != umdm_none) 
				printf("not digit(%s)\r\n", mi.buf);
		} else if(disp_mode != umdm_none) menu_disp();
	}
	else	// In the Item
	{
		if(mi.buf_len == 0) {
			ret = mtree[mi.cur-1].mfunc(MC_END, mi.buf);
			if(ret == RET_ROOT) SET_ROOT_MENU();
			else SET_UPPER_MENU(FALSE, disp_mode == umdm_max);
		} else {
			ret = mtree[mi.cur-1].mfunc(MC_DATA, mi.buf);
			if(ret == RET_OK) {				//printf("process done\r\n");
				SET_UPPER_MENU(TRUE, disp_mode == umdm_max);
			} else if(ret == RET_ROOT) {	//printf("process done & return to root\r\n");
				mtree[mi.cur-1].mfunc(MC_END, mi.buf);
				SET_ROOT_MENU();
			} //else							//printf("process continue\r\n");
		}
	}

	CMDLINE_RESET(TRUE);

}

void menu_disp(void)
{
	uint8 cnt, i;

	printf("\r\n=== MENU ================================\r\n");
	if(mi.depth > 1) printf("  0: Back\r\n");
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





