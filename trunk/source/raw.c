#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <malloc.h>
#include <sys/dir.h>
#include <dirent.h>
#include <ogcsys.h>

#include "sdsupp.h"
#include "card.h"
#include "mcard.h"
#include "gci.h"
#include "freetype.h"
#include "raw.h"

u8 *CardBuffer = 0;
s8 read_data = 0;
Header cardheader;

extern syssram* __SYS_LockSram();
extern syssramex* __SYS_LockSramEx();
extern u32 __SYS_UnlockSram(u32 write);
extern u32 __SYS_UnlockSramEx(u32 write);

syssramex *sramex;
u8 imageserial[12];

//code from tueidj's Devolution
void getserial(u8 *serial)
{
	int i;
	uint64_t rand;
	// rand() taken from K&R, lol
	rand = cardheader.formatTime; // seed
 
	for (i=0; i < 12; i++)
	{
		rand = (rand * 1103515245 + 12345) >> 16;
		cardheader.serial[i] -= rand;
		rand = ((uint32_t)rand * 1103515245 + 12345) >> 16;
		rand &= 0x7FFF;
		
		serial[i] = cardheader.serial[i];
	}
	serial[13]="\0";

}
	
void ClearFlashID(s32 chn){
	int i;
	
	sramex = __SYS_LockSramEx();
	for (i=0; i<12; i++){
		sramex->flash_id[chn][i] = 0x00;
	}
	__SYS_UnlockSramEx(1);
}

void _read_callback(s32 chn,s32 result)
{
	read_data =1;
}
void _write_callback(s32 chn,s32 result)
{
	read_data =1;
}

//output is 29 char long
void time2name(char *name)
{
	int month, day, year, hour, min, sec;
	month = day = year = hour = min = sec = 0;
	char monthstr[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
	                         "Aug", "Sep", "Oct", "Nov", "Dec"
	                       };

	// Taken from void SecondsToDate(int seconds, int *pYear, int *pMonth, int *pDay)
	// Calculates year month and day since jan 1, 1970 from (t) seconds
	// Reference: Fliegel, H. F. and van Flandern, T. C. (1968).
	// Communications of the ACM, Vol. 11, No. 10 (October, 1968).
	// Original code in Fortran
		int I, J, K, L, N;
		
		u32 t = time(NULL);

		L = t / 86400 + 2509157;
		N = 4 * L / 146097;
		L = L - (146097 * N + 3) / 4;
		I = 4000 * (L + 1) / 1461001;
		L = L - 1461 * I / 4 + 31;
		J = 80 * L / 2447;
		K = L - 2447 * J / 80;
		L = J / 11;
		J = J + 2 - 12 * L;
		I = 100 * (N - 49) + I + L;
		year = I;
		month = J;
		day = K;
		
		sec = t % 60;
		t /= 60;
		min = t % 60;
		t /= 60;
		hour = t % 24;
	
	sprintf(name, "%04d_%02d%s_%02d_%02d-%02d-%02d", year, month, monthstr[month-1], day, hour, min, sec);
}

s8 BackupRawImage(s32 slot, s32 *bytes_writen )
{
	int err;
	char filename[1024];
	char msg[1024];
	FILE* dumpFd = 0;
	u32 SectorSize = 0;
	s32 BlockCount = 0;
	
	CARD_Init(NULL,NULL);
	EXI_ProbeReset();
		
	err = MountCard(slot);
	if (err < 0)
	{
		WaitCardError("BakRaw", err);
		return -1;			/*** Unable to mount the card ***/
	}
	
	usleep(10*1000);
	
	err = CARD_GetSectorSize(slot,&SectorSize);
	if(err < 0 )
	{
		WaitCardError("BakRaw Sectsize", err);
		return -1;
	}

	err = CARD_GetBlockCount(slot,(u32*)&BlockCount);
	if(err < 0 )
	{
		WaitCardError("BakRaw Blockcount", err);
		return -1;
	}	
	CardBuffer = (u8*)memalign(32,SectorSize*BlockCount);
	if(CardBuffer == NULL)
	{
		//printf("failed to malloc memory. fail\n");
		WaitPrompt("BackRaw: Failed to malloc memory.");
		return -1;
	}
	s32 current_block = 0;
	int read = 0;
	s32 writen = 0;
	char name[64];
	int filenumber = 1;
	time2name(name);
	sprintf (filename, "fat:/%s/Backup_%s.raw", MCSAVES, name);
	//not really needed because the filename has seconds in it and the same filename will "never" happen
	while (file_exists(filename)){
		sprintf (filename, "fat:/%s/Backup_%s_%02d.raw", MCSAVES, name, filenumber);
		filenumber++;
	}	
	dumpFd = fopen(filename,"wb");
	if (dumpFd == NULL)
	{
		//printf("can not create file on SD\n");
		WaitPrompt("BackRaw: cannot create file on SD.");
		return -1;
	}
	//printf("dumping...\n");
	while( 1 )
	{
		read_data = 0;
		//printf("\rReading : %u bytes of %u (block %d)...",read,BlockCount*SectorSize,current_block);
		sprintf(msg, "Reading : %u bytes of %u (block %d)...",read,BlockCount*SectorSize,current_block);
		writeStatusBar(msg, "");
		if( (err != 0) || current_block >= BlockCount)
			break;
		err = __card_read(slot,SectorSize*current_block,SectorSize,CardBuffer+SectorSize*current_block,_read_callback);
		if(err == 0)
		{
			while(read_data == 0)
				usleep(1*1000); //sleep untill the read is done
			read = read + SectorSize;
			current_block++;

		}
		else
		{
			//printf("\nerror reading data : %d...\n",err);
			fclose(dumpFd);
			WaitCardError("BakRaw __read", err);
			return -1;
		}
	}
	//printf("\n");
	u8* ptr = (u8*)CardBuffer;
	for(writen = 0;writen < read;writen=writen+16384)
	{
		fwrite((u8*)ptr+writen,16384,1,dumpFd);
		//printf("\rWriting : %u bytes of %u",writen+16384,read);
		sprintf(msg, "Writing : %u bytes of %u",writen+16384,read);
		writeStatusBar(msg, "");
	}
	fclose(dumpFd);
	if(bytes_writen != NULL)
		*bytes_writen = writen;
	return 1;
}

s8 RestoreRawImage( s32 slot, char *sdfilename, s32 *bytes_writen )
{
	FILE *dumpFd = 0;
	char filename[1024];
	char msg[256];
	int err;
	u32 SectorSize = 0;
	s32 BlockCount = 0;
	s32 current_block = 0;
	s32 writen = 0;	

	CARD_Init(NULL,NULL);
	EXI_ProbeReset();
	
	err = MountCard(slot);
	if (err < 0)
	{
		WaitCardError("RestRaw", err);
		return -1;			/*** Unable to mount the card ***/
	}
	
	usleep(10*1000);
	
	err = CARD_GetSectorSize(slot,&SectorSize);
	if(err < 0 )
	{
		WaitCardError("RestRaw Sectsize", err);
		return -1;
	}

	err = CARD_GetBlockCount(slot,(u32*)&BlockCount);
	if(err < 0 )
	{
		WaitCardError("RestRaw Blockcount", err);
		return -1;
	}	
	CardBuffer = (u8*)memalign(32,SectorSize*BlockCount);
	if(CardBuffer == NULL)
	{
		//printf("failed to malloc memory. fail\n");
		WaitPrompt("RestRaw: Failed to malloc memory.");
		return -1;
	}


	/*** Make fullpath filename ***/
	sprintf (filename, "fat:/%s/%s", MCSAVES, sdfilename);

	/*** Open the SD Card file ***/
	dumpFd = fopen ( filename , "rb" );
	if (dumpFd <= 0)
	{
		free(CardBuffer);
		sprintf(msg, "Couldn't open %s", filename);		
		WaitPrompt (msg);
		return -1;
	}

	//first read the content of the SD dump into the buffer.
	u32 lSize;
	fseek (dumpFd , 0 , SEEK_END);
	lSize = ftell (dumpFd);
	rewind (dumpFd);
	if(lSize != BlockCount*SectorSize)
	{
		//check for mci raw image
		if((lSize-64) != BlockCount*SectorSize){
			//incorrect dump size D:<
			fclose(dumpFd);
			free(CardBuffer);
			WaitPrompt ("Card and image sizes differ");
			return -1;
		}
	}
	//printf("are you -SURE- you want to recover? a faulty backup or even a program\nfailure will corrupt the memory card!\n");

	int erase = 1;
	//0 = B wass pressed -> ask again
	if (!slot){
		erase = WaitPromptChoice("Are you -SURE- you want to restore to memory card in slot A?", "Restore", "Cancel");
	}else{
		erase = WaitPromptChoice("Are you -SURE- you want to restore to memory card in slot B?", "Restore", "Cancel");
	}
	
	if (!erase)
	{
		if (!slot){
			erase = WaitPromptChoiceAZ("All contents of memory card in slot A will be overwritten, continue?", "Restore", "Cancel");
		}else{
			erase = WaitPromptChoiceAZ("All contents of memory card in slot B will be erased, continue?", "Restore", "Cancel");
		}

		if (!erase)
		{
			if ((lSize-64) == BlockCount*SectorSize) fseek(dumpFd, 64, SEEK_SET);
			fread(CardBuffer,BlockCount*0x2000,1,dumpFd);
			fclose(dumpFd);
			
			// Test code to see if raw image is correctly read
			/*FILE *test = 0;
			test = fopen ( "fat:/test.bin" , "wb" );
			fwrite (CardBuffer , 1 , BlockCount*0x2000 , test );
			fclose (test);
			*/
			
			//printf("writing data to memory card...\n");
			ShowAction ("Writing data to memory card...");
			s32 upblock = 0;
			s32 write_len = SectorSize;
			//s32 write_len = 0x80;//pagesize o memory card
			while( 1 )
			{
				//printf("\rWriting... : %d of %d (block %d)",writen,BlockCount*SectorSize,current_block);
				sprintf(msg, "Writing... : Block %d (%d of %d)",current_block, writen,BlockCount*SectorSize);
				ShowAction (msg);
				//gprintf("\rWriting... : %d of %d (block %d of %d)",writen,BlockCount*SectorSize,current_block,BlockCount);
				read_data = 0;
				if( (err != 0) || current_block >= BlockCount || writen == BlockCount*SectorSize)
					break;
				//s32 __card_write(s32 chn,u32 address,u32 block_len,void *buffer,cardcallback callback)
				err = __card_write(slot,writen,write_len,CardBuffer+writen,_read_callback);
				if(err == 0)
				{
					while(read_data == 0)
						usleep(1*1000); //sleep untill the write is done which sadly takes alot longer then read
					__card_sync(slot);

					writen = writen + write_len;
					upblock=upblock+write_len;
					if(upblock == SectorSize)
					{
						current_block++;
						upblock = 0;
					}
				}
				else
				{
					fclose(dumpFd);
					free(CardBuffer);
					//printf("\nerror writing data(%d) Memory card could be corrupt now!!!\n",err);
					sprintf(msg, "error writing data(%d) Memory card could be corrupt now!!!",err);
					WaitPrompt (msg);
					return -1;
				}
			}
			//printf("\n");
			//gprintf("\n");
			if(bytes_writen != NULL)
				*bytes_writen = writen;
			return 1;		
		}
	}
	
	return -1;

}
