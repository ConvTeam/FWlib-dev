
//#define FILE_LOG_SILENCE
#include "appmod/usermenu/usermenu.h"


struct menu_item {
	//uint8 index;
	int8 parent;
	char *desc;
	menu_func mfunc;
};

struct menu_info {
	uint8 total;
	uint8 cur;
};

struct menu_item mtree[MAX_MENU_COUNT];
struct menu_info mi;

void menu_init(void)
{
	memset(mtree, 0, sizeof(mtree));
	mi.total = 0;
	mi.cur = 0;
}

int8 menu_add(char *desc, int8 parent, menu_func mfunc)
{
	// null 함수인 parent에 추가 하면 하위메뉴가 생김, parent가 0이면 최상위임, 그러므로 index는 1부터 시작임
	int len;

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
	len = strlen(desc);
	mtree[mi.total].desc = malloc(len+1);
	strcpy(mtree[mi.total].desc, desc);
	//mtree[mi.total].desc = strdup(desc);
	mtree[mi.total].parent = parent;
	mtree[mi.total].mfunc = mfunc;
	mi.total++;

	return mi.total; // 메뉴 인덱스를 리턴
}

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

void menu_run(void)
{
	int8 recv_char;
	static char buf[CMD_BUF_SIZE];
	bool disp = FALSE;
	int8 ret;
	uint8 i, cnt, tmp8;
	static uint8 depth = 1, buf_len = 0;

	recv_char = (char)getchar_nonblk();
	if(recv_char == RET_NOK) return; // 입력 값 없는 경우	printf("RECV: 0x%x\r\n", recv_char);

	//PUTCHAR('\n');PUTCHAR('-');PUTCHAR('-');PUTCHAR('-');PUTCHAR('-');PUTCHAR('-');PUTCHAR('-');PUTCHAR('\n');

	if(isgraph(recv_char) == 0) {	// 제어 문자 처리
//printf("ctrl\r\n");
		switch(recv_char) {
		case 0x0a:
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

	} else if(buf_len < CMD_BUF_SIZE-1){	// -1 이유 : 0 이 하나 필요하므로 
		buf[buf_len++] = (uint8_t)recv_char;
//buf[buf_len] = 0;
		PUTCHAR(recv_char);
//printf(" buf(%c, %s)\r\n", recv_char, buf);
	} else {
		printf("input buffer stuffed\r\n");
	}

	if(recv_char != 0x0a && recv_char != 0x0d && recv_char != 0x7f) return;		//LOGA("Command: %s", buf);

//printf("\r\n~~~~~\r\n");
	if(mi.cur == 0 || mtree[mi.cur-1].mfunc == NULL) {	// 루트거나 NULL Func(폴더)인 경우
		if(buf_len != 0) {
			if(str_check(isdigit, buf) == RET_OK) {
//printf("----------digit(%d)\r\n", atoi(buf));
				tmp8 = atoi(buf);
				for(i=0; i<mi.total; i++) {
					if(mi.cur == mtree[i].parent) {
//printf("----------i(%d)\r\n", i);	
						if(tmp8 == 1) break;
						else tmp8--;
					}
				}

				if(i < mi.total) {			//DBGA("----------set cur(%d)", tmp8);
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
			} else printf("not digit(%s)\r\n", buf);
		}

		if(buf_len == 0 || disp) {
			printf("\r\n=== MENU ================================\r\n");
			for(i=0, cnt=0; i<mi.total; i++) {
				if(mi.cur == mtree[i].parent) {
					cnt++;
					printf("%c%2d: %s\r\n", mtree[i].mfunc==NULL?'+':' ', cnt, mtree[i].desc);
				}
			}
			printf("=========================================\r\n");
		}
	} else {
		ret = mtree[mi.cur-1].mfunc(MC_DATA, buf);
		if(ret != RET_OK) {
			//printf("process continue\r\n");
		} else {
			//printf("process done\r\n");
			mtree[mi.cur-1].mfunc(MC_END, buf);
			mi.cur = mtree[mi.cur-1].parent;
			depth--;
		}		
	}

	memset(buf, 0, CMD_BUF_SIZE);
	buf_len = 0;
	printf("\r\n");
	for(i=0; i<depth; i++) PUTCHAR('>');
	PUTCHAR(' ');

}


