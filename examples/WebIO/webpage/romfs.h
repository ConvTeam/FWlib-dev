#ifndef _ROMFS_H_
#define _ROMFS_H_

#include "../mcu/types.h"

typedef struct _ROMFILE ROMFILE;

struct _ROMFILE
{
	ROMFILE *romfs_next;	/* Link to next ROMFILE structure. */
	u_char  *romfs_name;	/* Filename */
	u_int    romfs_size;	/* File size. */
	code_area *romfs_data;	/* File contents. */
};

extern ROMFILE* romfs_list;


u_char search_file(u_char * name, code_area ** buf, u_int * len);	// Search a file from ROM FILE

#endif /* _ROMFS_H_ */
