#include <stdio.h>
#include <string.h>
#include "Types.h"
#include "Apps\romfile.h"
#include "sockutil.h"

unsigned int m_WriteAddress = FLASH_ROMFILE_START_ADDRESS;

unsigned int read_from_flashbuf(unsigned long Address, unsigned char *Buffer, unsigned int Size)
{
	if (((Address + Size) > MAX_WRITABLE_FLASH_ADDRESS)||(Address < FLASH_ROMFILE_START_ADDRESS)||(Size == 0))
    		return 0;
	flash_read(Address, Buffer, Size);
	return Size;
}

// Added by Gang 2011-9-26 for W5200 HTTPs application note
void flash_read( unsigned short flash_addr, unsigned char *user_data, int len )
{
	int i;
       
	for (i=0 ; i < len ; i++)
	{
		user_data[i] = *((const unsigned char*)(flash_addr + i));
                
	}
        
      
}


unsigned char search_file_rom(unsigned char *FileName, unsigned long *Address, unsigned long *Size)
{
	unsigned char fileidx = 1;

	ROM_DIR file;
	unsigned int addr = FLASH_ROMFILE_START_ADDRESS;
	unsigned int read_result = 0;

	while(1)
	{
		read_result = read_from_flashbuf((unsigned long)addr, (unsigned char*)&file.b, sizeof(file));

		if((file.b[3]==0xff && file.b[2]==0xff)||(read_result == 0)) //Endian adjust : 0 -> 3 , 1 -> 2
		{
			break;
		}
		
		//search file
		if(!strcmp((char*)FileName, file.f.name))
		{
			//*Size = swapl(file.f.len);
			//*Address = swapl(file.f.start)+FLASH_ROMFILE_START_ADDRESS;
                        //Modified by Gang 2011-9-27
                        *Size = file.f.len;
			*Address = file.f.start+FLASH_ROMFILE_START_ADDRESS;

			return fileidx;
		}
		addr += FILE_HEAD_SIZE;
		fileidx++;
		
	}
	
	fileidx = 0;
	
	return fileidx;
}
