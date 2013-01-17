
#ifndef	__ROMEFILE_H__
#define	__ROMEFILE_H__

#define ROM_FNAMELEN    20  /* Maximum filename size */
#define FILE_HEAD_SIZE			(ROM_FNAMELEN +11)

typedef struct          /* Filename block structure */
{
    unsigned long len;               /* Length of file in bytes */
    unsigned long start;             /* Start address of file data in ROM */
    unsigned short check;             /* TCP checksum of file */
    unsigned char flags;            /* Embedded Gateway Interface (EGI) flags */
    char name[ROM_FNAMELEN];		/* Lower-case filename with extension */
} ROM_FNAME;

#define HTTP_FAIL       "HTTP/ 200 OK\r\n\r\nNo Web pages!\r\n"
#define MAXFILES    	100 		// Limit on ROM file count (to stop runaway)

typedef union               		// ROM file directory entry format
{
    ROM_FNAME f;                	// Union of filename..
    unsigned char b[sizeof(ROM_FNAME)];  	// ..with byte values for i2c transfer
} ROM_DIR;

/*Very important settings*/
#define FLASH_ROMFILE_START_ADDRESS		0x08007829 //This value is determined by the size of f/w
#define MAX_WRITABLE_FLASH_ADDRESS		0x0801FFFF //STM32F103C8 has 64K flash (0x08000000~0x0801FFFF)


//unsigned int  write_to_flash(unsigned char *flash_data, unsigned int len);
unsigned char search_file_rom(unsigned char *FileName, unsigned long *Address, unsigned long *Size);
unsigned int read_from_flashbuf(unsigned long Address, unsigned char *Buffer, unsigned int Size);
void flash_read( unsigned short flash_addr, unsigned char *user_data, int len );

#endif