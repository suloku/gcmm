/****************************************************************************
 * SD Card Support Functions
 ****************************************************************************/
#include <gccore.h>
#include <network.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include <sdcard.h>
#include <sys/dir.h>
#include <dirent.h>

#include "sdsupp.h"
#include "freetype.h"
#include "gci.h"
#include "mcard.h"

#define PAGESIZE 20
#define PADCAL 80

#define DIRENT_T_FILE 0
#define DIRENT_T_DIR 1

/*** Memory Card FileBuffer ***/
#define MAXFILEBUFFER (1024 * 2048)	/*** 2MB Buffer ***/
extern u8 FileBuffer[MAXFILEBUFFER] ATTRIBUTE_ALIGN (32);
extern u8 CommentBuffer[64] ATTRIBUTE_ALIGN (32);
extern u8 filelist[1024][1024];
extern u32 maxfile;
extern GCI gci;
extern int OFFSET;

bool file_exists(const char * filename)
{
	FILE * file = fopen(filename, "r");
	if (file)
	{
		fclose(file);
		return true;
	}
	return false;
}

int SDSaveMCImage ()
{

	char filename[1024];
	char tfile[40];
	char company[4];
	char gamecode[6];
	int bytesToWrite = 0;
	//sd_file *handle;
	FILE *handle;
	GCI thisgci;
	int check;
	int filenumber = 0;
	int retries = 0;

	/*** Make a copy of the Card Dir ***/
	memcpy (&thisgci, FileBuffer, sizeof (GCI));
	memset( tfile, 0, 40 );
	company[2] = 0;
	gamecode[4] = 0;
	memcpy (company, &thisgci.company, 2);
	memcpy (gamecode, &thisgci.gamecode, 4);
	memcpy (tfile, &thisgci.filename, CARD_FILENAMELEN);

	sprintf (filename, "fat:/%s", MCSAVES);

	mkdir(filename, S_IREAD | S_IWRITE);

	sprintf (filename, "fat:/%s/%02d-%s-%s-%s.gci", MCSAVES, filenumber, company, gamecode,tfile);
	
	//Lets try if there's already a savegame (if it exists its name is legal, so theoretically the illegal name check will pass
	//Illegal savegame should report as nonexisting...
	//We will number the files
	while (file_exists(filename)){
		sprintf (filename, "fat:/%s/%02d-%s-%s-%s.gci", MCSAVES, filenumber, company, gamecode,tfile);
		filenumber++;
	}

	//filename[128] = 0; // limit filename length, there's a bug where a file is exported as DOS-type shortname ("8P-GPO~2.GCI") which I assume happens if the name is too long...? Nope that's not it.

	while ( 1 ) // loop the writing because it sometimes fails (see above); I think this is an error in libfat but I dunno.
	{
		/*** Open SD Card file ***/
		//handle = SDCARD_OpenFile (filename, "wb");
		handle = fopen ( filename , "wb" );
		if (handle <= 0)
		{
			// couldn't open file, probably either card full or filename illegal; let's assume illegal filename and retry
			sprintf(filename, "fat:/%s/%02d-%s-%s-%s.gci", MCSAVES, filenumber, company, gamecode, "illegal_name");
			//let's see again if there aren't any saves already...
			filenumber = 1;
			while (file_exists(filename)){
				sprintf (filename, "fat:/%s/%02d-%s-%s-%s.gci", MCSAVES, filenumber, company, gamecode, "illegal_name");
				filenumber++;
			}
			//filename[128] = 0;
			handle = fopen ( filename , "wb" );
			if (handle <= 0)
			{
				char msg[100];
				sprintf(msg, "Couldn't open %s", filename);
				WaitPrompt (msg);
				return 0;
			}
		}

		bytesToWrite = (thisgci.filesize8 * 8192) + MCDATAOFFSET;
		//SDCARD_WriteFile (handle, FileBuffer, bytesToWrite);
		//SDCARD_CloseFile (handle);
		fwrite (FileBuffer , 1 , bytesToWrite , handle );
		fclose (handle);
		
		// check if file actually wrote correctly
		handle = fopen ( filename , "rb" );
		if (handle <= 0)
		{
			// file failed to open, so something failed in the write; retry
			retries ++;
			continue;
		}
		fseek (handle , 0 , SEEK_END);
		check = ftell (handle);
		fclose(handle);
		if ( check > 0 ) {
			// if filesize is bigger than 0, we can assume the file wrote correctly, break out of loop
			// ... this will end in an endless loop if the file to write is 0 bytes but that's very unlikely
			break;
		}
		
		//Try 10 times if needed, then give up. Better to avoid endless looping
		if (retries > 9)
		{
			return 0;
		}
		
	}
	
	return 1;
}

int SDLoadMCImage(char *sdfilename)
{

	//sd_file *handle;
	FILE *handle;
	char filename[1024];
	char msg[256];
	//int offset = 0;
	//int bytesToRead = 0;
	long bytesToRead = 0;

	/*** Clear the work buffers ***/
	memset (FileBuffer, 0, MAXFILEBUFFER);
	memset (CommentBuffer, 0, 64);

	/*** Make fullpath filename ***/
	//sprintf (filename, "dev0:\\%s\\%s", MCSAVES, sdfilename);
	sprintf (filename, "fat:/%s/%s", MCSAVES, sdfilename);

	//SDCARD_Init ();

	/*** Does this file exist ? ***/
	/*if (SDCARD_SeekFile(filename, 0, SDCARD_SEEK_SET) != SDCARD_ERROR_READY){
	   return 0;
	}*/

	/*** Open the SD Card file ***/
	//handle = SDCARD_OpenFile (filename, "rb");
	handle = fopen ( filename , "rb" );
	if (handle <= 0)
	{
		sprintf(msg, "Couldn't open %s", filename);
		WaitPrompt (msg);
		return 0;
	}


	/*    if (handle == NULL){
	      WaitPrompt ("Unable to open file!");
	      return 0;
	    }*/

	// obtain file size:
	fseek (handle , 0 , SEEK_END);
	bytesToRead = ftell (handle);
	rewind (handle);

	//bytesToRead = SDCARD_GetFileSize(handle);
	if (bytesToRead <= 0)
	{
		sprintf(msg, "Incorrect file size %ld .", bytesToRead);
		WaitPrompt (msg);
		//WaitPrompt("Incorrect file size.");
		return 0;
	}
	fseek (handle , OFFSET , SEEK_SET);
	/*** Read the file ***/
	//SDCARD_ReadFile (handle, FileBuffer, bytesToRead);
	fread (FileBuffer,1,bytesToRead,handle);

	if(OFFSET == 0x80)
	{
		// swap byte pairs
		// 0x06 and 0x07
		u8 temp = FileBuffer[0x06];
		FileBuffer[0x06] = FileBuffer[0x07];
		FileBuffer[0x07] = temp;

		// swap byte pairs
		// 0x2C and 0x2D, 0x2E and 0x2F, 0x30 and 0x31, 0x32 and 0x33,
		// 0x34 and 0x35, 0x36 and 0x37, 0x38 and 0x39, 0x3A and 0x3B,
		// 0x3C and 0x3D,0x3E and 0x3F.
		int i = 0;
		while(i<10)
		{
			u8 temp = FileBuffer[0x2C + i*2];
			FileBuffer[0x2C + i*2] = FileBuffer[0x2C + i*2+1];
			FileBuffer[0x2C + i*2+1] = temp;
			i++;
		}
	}
	//sprintf(msg, "Offset: %d", bytesToRead);
	//WaitPrompt (msg);
	/*** Close the file ***/
	//SDCARD_CloseFile (handle);
	fclose (handle);

	return bytesToRead;
}

int SDLoadMCImageHeader(char *sdfilename)
{

	FILE *handle;
	char filename[1024];
	char msg[256];
	//int offset = 0;
	//int bytesToRead = 0;
	long bytesToRead = 0;

	/*** Clear the work buffers ***/
	memset (FileBuffer, 0, MAXFILEBUFFER);
	memset (CommentBuffer, 0, 64);

	/*** Make fullpath filename ***/
	sprintf (filename, "fat:/%s/%s", MCSAVES, sdfilename);


	/*** Open the SD Card file ***/
	//handle = SDCARD_OpenFile (filename, "rb");
	handle = fopen ( filename , "rb" );
	if (handle <= 0)
	{
		sprintf(msg, "Couldn't open %s", filename);
		WaitPrompt (msg);
		return 0;
	}

	// obtain file size:
	fseek (handle , 0 , SEEK_END);
	bytesToRead = ftell (handle);
	rewind (handle);
	if (bytesToRead < 64) //We don't want to read something smaller than the header
	{
		sprintf(msg, "Incorrect file size %ld . Not GCI File", bytesToRead);
		WaitPrompt (msg);
		//WaitPrompt("Incorrect file size.");
		return 0;
	}

	OFFSET = 0;
	char tmp[0xD];
	char fileType[1024];

	char * dot;
	int pos = 4;
	dot = strrchr(filename,'.');
	strncpy(fileType, dot+1,pos);
	fileType[pos]='\0';

	if(strcasecmp(fileType, "gci"))
	{
		fread(&tmp, 1, 0xD, handle);
		rewind(handle);
		if (!strcasecmp(fileType, "gcs"))
		{
			if (!memcmp(&tmp, "GCSAVE", 6))	// Header must be uppercase
			{
				OFFSET = 0x110;
			}
			else
			{
				WaitPrompt ("Incorrect Header. Not GCS File");
				return 0;
			}
		}
		else
		{
			if (!strcasecmp(fileType, "sav"))
			{
				if (!memcmp(tmp, "DATELGC_SAVE", 0xC)) // Header must be uppercase
				{
					OFFSET = 0x80;
				}
				else
				{
					WaitPrompt ("Incorrect Header. Not SAV File");
					return 0;
				}
			}
		}
	}
	else OFFSET = 0;
	fseek(handle, OFFSET, SEEK_SET);

	/*** Read the file header ***/
	fread (FileBuffer,1,sizeof(GCI),handle);

	u16 length = (bytesToRead - OFFSET - sizeof(GCI)) / 0x2000;
	switch(OFFSET)
	{
	case 0x110:
	{
		// field containing the Block count as displayed within
		// the GameSaves software is not stored in the GCS file.
		// It is stored only within the corresponding GSV file.
		// If the GCS file is added without using the GameSaves software,
		// the value stored is always "1"
		FileBuffer[0x38] = (u8)(length >> 8);
		FileBuffer[0x39] = (u8)length;
	}
	break;
	case 0x80:
	{
		// swap byte pairs
		// 0x06 and 0x07

		u8 temp = FileBuffer[0x06];
		FileBuffer[0x06] = FileBuffer[0x07];
		FileBuffer[0x07] = temp;

		// swap byte pairs
		// 0x2C and 0x2D, 0x2E and 0x2F, 0x30 and 0x31, 0x32 and 0x33,
		// 0x34 and 0x35, 0x36 and 0x37, 0x38 and 0x39, 0x3A and 0x3B,
		// 0x3C and 0x3D,0x3E and 0x3F.
		int i = 0;
		while(i<10)
		{
			u8 temp = FileBuffer[0x2C + i*2];
			FileBuffer[0x2C + i*2] = FileBuffer[0x2C + i*2+1];
			FileBuffer[0x2C + i*2+1] = temp;
			i++;
		}
		break;
	}
	default:
		break;
	}
	u16 l2 =(u16)(FileBuffer[0x38] << 8) | FileBuffer[0x39];
	if (length !=  l2)
	{
		sprintf(msg, "l1 = %x l2 = %x", length,l2);
		WaitPrompt (msg);//"File Length does not equal filesize");
		return 0;
	}

	ExtractGCIHeader();
	GCIMakeHeader();

	//Get the comment
	rewind(handle);
	fseek(handle, MCDATAOFFSET + OFFSET + gci.comment_addr, SEEK_SET);
	fread (CommentBuffer, 1, MCDATAOFFSET, handle);

	/*** Close the file ***/
	fclose (handle);

	return bytesToRead;
}

int isdir_sd(char *path)
{
	DIR* dir = opendir(path);
	if(dir == NULL)
		return 0;

	closedir(dir);

	return 1;
}

/****************************************************************************
* SDGetFileList
*
* Get the directory listing from SD Card
****************************************************************************/
int SDGetFileList()
{
	int filecount = 0;
	DIR *dir;
	struct dirent *dit;
	static char namefile[256*4]; // reserva espacio suficiente para UTF-8
	int dirtype;
	//static struct stat filestat;

	char filename[1024];
	sprintf (filename, "fat:/%s/", MCSAVES);

	if ((dir = opendir(filename)) == NULL)
	{
		return -1;
	}

	while ((dit = readdir(dir)) != NULL)
	{
		if(strncmp(dit->d_name, ".", 1) != 0 && strncmp(dit->d_name, "..", 2) != 0)
		{
			strcpy((char *)filelist[filecount], dit->d_name);
			sprintf(namefile, "%s%s", filename, dit->d_name);
			dirtype = ((isdir_sd(namefile) == 1) ? DIRENT_T_DIR : DIRENT_T_FILE);

			filecount++;
		}
	}

	/* while(1)
	{
		if(readdir(dir)!=0) break; // si no hay mas entradas en el directorio, sal

		if(filestat.st_mode & S_IFDIR) // es el nombre de un directorio
		{
			// namefile contiene el nombre del directorio en formato UTF-8,que puede ser "." o ".." tambien
		} else {
			// namefile contiene el nombre del fichero en formato UTF-8
			strcpy ((char *)filelist[filecount], namefile);
			filecount++;
		}
	}*/

	closedir(dir); // cierra el directorio
	maxfile = filecount;
	return filecount;

	/*  int entries = 0;
	  int filecount = 0;
	  char filename[1024];
	  //DIR *sddir = NULL;
	  DIR_ITER *sddir = NULL;

	  SDCARD_Init ();

	  sprintf (filename, "dev0:\\%s\\", MCSAVES);

	  entries = SDCARD_ReadDir (filename, &sddir);

	  while (entries){
	      strcpy (filelist[filecount], sddir[filecount].fname);
	      filecount++;
	      entries--;
	  }

	  free(sddir);

	  maxfile = filecount;
	  return filecount;
	  */
}
