#include "sd.h"
#include "SDPrint.c"


	while(f_readdir(&fp, &info) == FR_OK)
	{
			if(info.fname[0] == 0)
				break;
			if(strstr(info.lfname, "gcode"))
			LISTBOX_AddString(hItem, info.lfname);
	}
    LISTBOX_SetFont(hItem, GUI_FONT_24_1);

	for(i = 0; i < 128; i++)
	{
			buf[i] = '\0';
	}


FATFS fs;
DIR fp;

char buf[128];
FILINFO info;
info.lfsize = 64;
info.lfname = buf;		
bool StartRead = false;
char getword;
unsigned int num_getword;
FIL fil;
char *file;
int Num_Get;
static char data_buf[272];
bool StopRead = false;

char *SD_Opreations(void)
{
	char err;
	
	static char ERR[2];
	static enum
	{
		SD_INIT = 0,
		FS_MOUNT,
		FS_OPENDIR,
		FS_READDIR,
		FS_WAIT,
		FS_OPEN_FILE,
		FS_READ_FILE,
	}SD_STATUS = SD_INIT;
	
	ERR[0] = 0;
	ERR[1] = 0;
	
	switch(SD_STATUS)
	{
		case SD_INIT:
			err = SD_Init();
			if(!err)
				break;
			SD_STATUS = FS_MOUNT;
			
		case FS_MOUNT:
			err = f_mount(&fs, "0:", 1);
			if(!err)
				break;
			SD_STATUS = FS_OPENDIR;
			
		case FS_OPENDIR:
			err = f_opendir(&fp, "0:");
			if(!err)
				break;
			SD_STATUS = FS_READDIR;
			
		case FS_READDIR:
			err = f_readdir(&fp, &info);
			if(!err)
				break;
			else
			{
				if(info.fname[0] == 0)
					SD_STATUS = FS_WAIT;
				if(strstr(info.lfname, "uvf"))
					return info.lfname;
			}
			
		case FS_WAIT:
			if(StartRead)
				SD_STATUS = FS_OPEN_FILE;
			else
				break;
		
		case FS_OPEN_FILE:
			err = f_open(&fil, file, FA_READ);	
			if(!err)
				break;
			else
				SD_STATUS = FS_READ_FILE;
					
		case FS_READ_FILE:
			err = f_gets(data_buff, Num_Get, &fil);
			if(!err)
				break;
			else
			{
				if(StopRead)
					SD_STATUS = FS_CLOSE_FILE;
				else
					return data_buff;
			}
			
		case FS_CLOSE_FILE:
			err = f_close(&fil);
			if(!err)
				break;
			else
			{
				StartRead = false;
				StopRead = false;
			}
			
		default:
			break;
	}
	
	ERR[0] = (char)SD_STATUS;
	ERR[1] = err;
	return ERR;
}