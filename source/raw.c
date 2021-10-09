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
#include <ogc/libversion.h>
#if (_V_MAJOR_ <= 2) && (_V_MINOR_ <= 2)
#ifndef ogc_card_sync
#define ogc_card_sync __card_sync
#endif
#ifndef ogc_card_read
#define ogc_card_read __card_read
#endif
#ifndef ogc_card_sectorerase
#define ogc_card_sectorerase __card_sectorerase
#endif
#ifndef ogc_card_write
#define ogc_card_write __card_write
#endif
#include "card.h"
extern s32 __card_write(s32 chn,u32 address,u32 block_len,void *buffer,cardcallback callback);
extern s32 __card_sectorerase(s32 chn,u32 sector,cardcallback callback);
extern s32 __card_sync(s32 chn);
extern s32 __card_read(s32 chn,u32 address,u32 block_len,void *buffer,cardcallback callback);
s32 CARD_GetFreeBlocks(s32 chn, u16* freeblocks);
s32 CARD_GetSerialNo(s32 chn,u32 *serial1,u32 *serial2);
#else
#include <ogc/card.h>
#endif

#include "sdsupp.h"
#include "mcard.h"
#include "gci.h"
#include "freetype.h"
#include "raw.h"

#define mem_free(x) {free(x);x=NULL;}
//Enabling this definition will enable additional checks to ensure raw restore worked
#define DEBUGRAW

u8 *CardBuffer = 0;
#ifdef DEBUGRAW
u8 *CheckBuffer = 0;
u8 *EraseCheckBuffer = 0;
#endif
s8 read_data = 0;
s8 write_data = 0;
s8 erase_data = 0;
Header cardheader;

extern u8 currFolder[260];
extern int folderCount;

extern syssram* __SYS_LockSram();
extern syssramex* __SYS_LockSramEx();
extern u32 __SYS_UnlockSram(u32 write);
extern u32 __SYS_UnlockSramEx(u32 write);
#if (_V_MAJOR_ >= 2) && (_V_MINOR_ >= 3)
extern s32 ogc_card_write(s32 chn,u32 address,u32 block_len,void *buffer,cardcallback callback);
extern s32 ogc_card_sectorerase(s32 chn,u32 sector,cardcallback callback);
extern s32 ogc_card_sync(s32 chn);
extern s32 ogc_card_read(s32 chn,u32 address,u32 block_len,void *buffer,cardcallback callback);
#endif

syssramex *sramex;
u8 imageserial[12];

void freecardbuf(){
	if(CardBuffer)
	{
		if(CardBuffer != NULL)
		{
			mem_free(CardBuffer);
			CardBuffer = NULL;
		}
	}
}

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
	write_data =1;
}
void _erase_callback(s32 chn,s32 result)
{
	erase_data =1;
}


u64 Card_SerialNo(s32 slot)
{
	int err;
	u32 SectorSize = 0;

	CARD_Init(NULL,NULL);
	EXI_ProbeReset();
		
	err = MountCard(slot);
	if (err < 0)
	{
		WaitCardError("SerialNo", err);
		return -1;			/*** Unable to mount the card ***/
	}
	
	usleep(10*1000);
	
	err = CARD_GetSectorSize(slot,&SectorSize);
	if(err < 0 )
	{
		WaitCardError("SerialNo Sectsize", err);
		CARD_Unmount(slot);
		return -1;
	}

	CardBuffer = (u8*)memalign(32,SectorSize);
	if(CardBuffer == NULL)
	{
		WaitPrompt("SerialNo: Failed to malloc memory.");
		CARD_Unmount(slot);
		return -1;
	}
	s32 current_block = 0;
	int read = 0;

	while( 1 )
	{
		read_data = 0;

		if( (err != 0) || current_block >= 1)
			break;
		DCInvalidateRange(CardBuffer+SectorSize*current_block,SectorSize);
		err = ogc_card_read(slot,0,SectorSize,CardBuffer+SectorSize*current_block,_read_callback);
		if(err == 0)
		{
			while(read_data == 0)
				usleep(1*1000); //sleep untill the read is done
			read = read + SectorSize;
			current_block++;

		}
		else
		{
			mem_free(CardBuffer);
			WaitCardError("BakRaw __read", err);
			CARD_Unmount(slot);
			return -1;
		}
	}
	
	//get the true serial
	u64 serial1[4];
	int i;
    for (i=0;i<4;i++){
        memcpy(&serial1[i], CardBuffer+(8*i), sizeof(u64));
    }
    u64 serialA = serial1[0]^serial1[1]^serial1[2]^serial1[3];

	mem_free(CardBuffer);
	CARD_Unmount(slot);
	return serialA;
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
	char filename[256];
	char msg[128+256];
	char msg2[128+256];
	msg2[0] = '\0';
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
		CARD_Unmount(slot);
		return -1;
	}

	err = CARD_GetBlockCount(slot,(u32*)&BlockCount);
	if(err < 0 )
	{
		WaitCardError("BakRaw Blockcount", err);
		CARD_Unmount(slot);
		return -1;
	}
	CardBuffer = (u8*)memalign(32,SectorSize);
	if(CardBuffer == NULL)
	{
		//printf("failed to malloc memory. fail\n");
		WaitPrompt("BackRaw: Failed to malloc memory.");
		CARD_Unmount(slot);
		return -1;
	}
	s32 current_block = 0;
	int read = 0;
	s32 writen = 0;
	char name[64];
	int filenumber = 1;
	
	sprintf (filename, "fat:/%s", MCSAVES);
	mkdir(filename, S_IREAD | S_IWRITE);	
	
	time2name(name);
	sprintf (filename, "fat:/%s/%04db_%s.raw", MCSAVES, BlockCount-5, name);
	//not really needed because the filename has seconds in it and the same filename will "never" happen
	while (file_exists(filename)){
		sprintf (filename, "fat:/%s/%04db_%s_%02d.raw", MCSAVES, BlockCount-5, name, filenumber);
		filenumber++;
	}	
	dumpFd = fopen(filename,"wb");
	if (dumpFd == NULL)
	{
		mem_free(CardBuffer);
		//printf("can not create file on SD\n");
		WaitPrompt("BackRaw: cannot create file on SD.");
		CARD_Unmount(slot);
		return -1;
	}
	//printf("dumping...\n");
	int card_addr = SectorSize*current_block;
	while(current_block < BlockCount)
	{
		read_data = 0;//Reset read_callback
		card_addr = SectorSize*current_block;
		//printf("\rReading : %u bytes of %u (block %d)...",read,BlockCount*SectorSize,current_block);
		sprintf(msg, "Reading...: Block %d of %d (%u bytes of %u)",current_block,BlockCount,read,BlockCount*SectorSize);
		writeStatusBar(msg, msg2);
		//memset(CardBuffer, 0xFF, SectorSize);//Reset buffer. In GC memory card bytes are set to 0xFF when erasing.
		DCInvalidateRange(CardBuffer,SectorSize);
		if( (err != 0) || current_block >= BlockCount)
			break;
		err = ogc_card_read(slot,card_addr,SectorSize,CardBuffer,_read_callback);//Read 1 block
		if(err == 0)//Correct read
		{
			while(read_data == 0)
				usleep(1*1000); //sleep untill the read is done
			read = read + SectorSize;
			current_block++;
		}
		else //card_read failed
		{
			mem_free(CardBuffer);
			//printf("\nerror reading data : %d...\n",err);
			fclose(dumpFd);
			WaitCardError("BakRaw __read", err);
			CARD_Unmount(slot);
			return -1;
		}
		
		//Write Block to fat storage
			//printf("\n");
			//u8* ptr = (u8*)CardBuffer;
			//fwrite((u8*)ptr,SectorSize,1,dumpFd);
			fwrite(CardBuffer,SectorSize,1,dumpFd);
			//writen += SectorSize;
			if(bytes_writen != NULL)
				*bytes_writen += SectorSize;
			//printf("\rWriting : %u bytes of %u",writen+SectorSize,read);
			sprintf(msg2, "Writing...: Block %d of %d (%u bytes of %u)",current_block,BlockCount,*bytes_writen,BlockCount*SectorSize);
			writeStatusBar(msg, msg2);
	}

	fclose(dumpFd);

	mem_free(CardBuffer);
	CARD_Unmount(slot);
	return 1;
}

s8 RestoreRawImage( s32 slot, char *sdfilename, s32 *bytes_writen )
{
	FILE *dumpFd = 0;
	char filename[256];
	char msg[128+256];
	int err;
	u32 SectorSize = 0;
	s32 BlockCount = 0;
	s32 current_block = 0;
	s32 writen = 0;
	s32 write_addr = 0;
	
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
		CARD_Unmount(slot);
		return -1;
	}

	err = CARD_GetBlockCount(slot,(u32*)&BlockCount);
	if(err < 0 )
	{
		WaitCardError("RestRaw Blockcount", err);
		CARD_Unmount(slot);
		return -1;
	}	
	CardBuffer = (u8*)memalign(32,SectorSize);
	if(CardBuffer == NULL)
	{
		//printf("failed to malloc memory. fail\n");
		WaitPrompt("RestRaw: Failed to malloc memory.");
		CARD_Unmount(slot);
		return -1;
	}
#ifdef DEBUGRAW
	CheckBuffer = (u8*)memalign(32,SectorSize);
	if(CheckBuffer == NULL)
	{
		mem_free(CardBuffer);
		WaitPrompt("RestRaw: Failed to malloc memory (CheckBuffer)");
		CARD_Unmount(slot);
		return -1;
	}
	EraseCheckBuffer = (u8*)memalign(32,SectorSize);
	if(EraseCheckBuffer == NULL)
	{
		mem_free(CardBuffer);
		mem_free(CheckBuffer);
		WaitPrompt("RestRaw: Failed to malloc memory (EraseCheckBuffer)");
		CARD_Unmount(slot);
		return -1;
	}
	memset(EraseCheckBuffer, 0xFF, SectorSize);
#endif	


	/*** Make fullpath filename ***/
	sprintf (filename, "fat:/%s/%s", currFolder, sdfilename);

	/*** Open the SD Card file ***/
	dumpFd = fopen ( filename , "rb" );
	if (dumpFd <= 0)
	{
		mem_free(CardBuffer);
#ifdef DEBUGRAW
		mem_free(CheckBuffer);
		mem_free(EraseCheckBuffer);
#endif
		sprintf(msg, "Couldn't open %s", filename);		
		WaitPrompt (msg);
		CARD_Unmount(slot);
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
			mem_free(CardBuffer);
#ifdef DEBUGRAW
			mem_free(CheckBuffer);
			mem_free(EraseCheckBuffer);
#endif
			WaitPrompt ("Card and image sizes differ");
			CARD_Unmount(slot);
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

		if (!erase)//Actual restore starts here
		{
			//ShowAction ("Reading data...");
			//If it's a MCI image, skip the header
			if ((lSize-64) == BlockCount*SectorSize) fseek(dumpFd, 64, SEEK_SET);
			
			s32 upblock = 0;
			s32 write_len = SectorSize;
			//s32 write_len = 0x80;//pagesize of memory card
/*
//Test code to skip blocks
			int blockSkip = 1023;
			if ((lSize-64) == BlockCount*SectorSize) fseek(dumpFd, 64+blockSkip*SectorSize, SEEK_SET);
			else fseek(dumpFd, blockSkip*SectorSize, SEEK_SET);
			current_block = blockSkip;
			BlockCount = BlockCount; //Change to how many blocks you'd like to write
			write_addr = current_block*SectorSize;
//Testcode end
*/
			while (1)
			{
				if( (err != 0) || current_block >= BlockCount || writen == BlockCount*SectorSize)
					break;
			
				fread(CardBuffer,SectorSize,1,dumpFd);
				//fclose(dumpFd);
/*
//Test code to see if raw image is correctly read
				FILE *test = 0;
				test = fopen ( "fat:/fatread.bin" , "wb" );
				fwrite (CardBuffer , 1 , SectorSize , test);
				fclose (test);
//Testcode end
*/
				//printf("writing data to memory card...\n");
				//ShowAction ("Writing data to memory card...");
				
				//printf("\rWriting... : %d of %d (block %d)",writen,BlockCount*SectorSize,current_block);
				sprintf(msg, "Writing...: Block %d of %d (%d of %d)",current_block,BlockCount,writen,BlockCount*SectorSize);
				ShowAction (msg);
				//gprintf("\rWriting... : %d of %d (block %d of %d)",writen,BlockCount*SectorSize,current_block,BlockCount);
				
				write_data = 0;//Write callback
				erase_data = 0;//Erase callback

				//DCStoreRange is called in card.c before actual writing to the card, let's copy it
				DCStoreRange(CardBuffer,write_len);
				
				//Data has to be erased before writing for propper restore (for some reason)
				if(writen == 0 || !(writen%SectorSize))//We can only erase full sectors (blocks), but we can write in pages or other sizes
				{
					
					//s32 ogc_card_sectorerase(s32 chn,u32 sector,cardcallback callback);
					err = ogc_card_sectorerase(slot,write_addr, _erase_callback);
					if(err == 0)
					{
						while(erase_data == 0)
							usleep(1*1000); //sleep until the erase is done which sadly takes alot longer then read
						ogc_card_sync(slot);
	#ifdef DEBUGRAW //This code section is broken right now
	
						//At this point the sector (block) should be erased
						//Check if the sector was erased (all set to 0xFF).

						//Some unofficial memory cards apparently don't erase sectors when using ogc_card_sectorerase(),
						//instead they can directly properly store the data calling just ogc_card_write(),
						//so the following check will always fail, preventing restore for those cards, so I'm disabling
						//it and relying only on the ogc_card_read() check done after writing the data.
/*
							memset(CheckBuffer,0,write_len);
							DCInvalidateRange(CheckBuffer,write_len);
							read_data = 0;
							err = ogc_card_read(slot,writen,write_len,CheckBuffer,_read_callback);
							while(read_data == 0) //sleep until reading is done
								usleep(1*1000);
							
							if(err == 0) //Read OK
							{
								if(!memcmp(EraseCheckBuffer,CheckBuffer,write_len))
								{
									//printf("read and write are the same y0\n");
								}
								else
								{
									//Uncomment to dump contents that where read
									
									FILE* dump = fopen("fat:/sectcheck_dump.bin","wb");
									fwrite(CheckBuffer,1,write_len,dump);
									fclose(dump);
									
									fclose(dumpFd);
									mem_free(CardBuffer);
									mem_free(CheckBuffer);
									mem_free(EraseCheckBuffer);
									sprintf(msg, "Error! _sectorerase failed for unknown reasons at sector %d!", current_block);
									WaitPrompt (msg);
									CARD_Unmount(slot);
									return -1;
								}
							}
							else //Read failed
							{
								fclose(dumpFd);
								mem_free(CardBuffer);
								mem_free(CheckBuffer);
								mem_free(EraseCheckBuffer);
								sprintf(msg, "Error reading data(%d) Memory card could be corrupt now!!!",err);
								WaitPrompt (msg);
								CARD_Unmount(slot);
								return -1;
							}
*/
	#endif
					}else //sector_erase failed
					{
						//Cleanup
						fclose(dumpFd);
						mem_free(CardBuffer);
	#ifdef DEBUGRAW
						mem_free(CheckBuffer);
						mem_free(EraseCheckBuffer);
	#endif					
						//printf("\nerror writing data(%d) Memory card could be corrupt now!!!\n",err);
						sprintf(msg, "error %d erasing sector. Memory card could be corrupt now!!!",err);
						WaitPrompt (msg);
						CARD_Unmount(slot);
						return -1;
					}
				}
				
				//s32 ogc_card_write(s32 chn,u32 address,u32 block_len,void *buffer,cardcallback callback)
				err = ogc_card_write(slot,write_addr,write_len,CardBuffer,_write_callback);
				if(err == 0)
				{
					while(write_data == 0)
						usleep(1*1000); //sleep untill the write is done which sadly takes alot longer then read
					ogc_card_sync(slot);

	#ifdef DEBUGRAW
					//Check the written data against the buffer
					memset(CheckBuffer,0,write_len);
					DCInvalidateRange(CheckBuffer,write_len);
					read_data = 0;
					//err = ogc_card_read(slot,SectorSize*current_block,SectorSize,CheckBuffer,_read_callback);
					err = ogc_card_read(slot,write_addr,write_len,CheckBuffer,_read_callback);
					while(read_data == 0)
						usleep(1*1000);
					ogc_card_sync(slot);
	
					if(err == 0)//Read OK
					{
						if(!memcmp(CardBuffer,CheckBuffer,write_len))
						{
							//printf("read and write are the same y0\n");
						}
						else //Written and read data differ
						{
							//printf("different data. writing to SD (0x%08X), error %d\n",*check,CardError);

							//Uncomment to dump contents that where read
							FILE* dump = fopen("fat:/check_dump.bin","wb");
							fwrite(CheckBuffer,1,write_len,dump);
							fclose(dump);
							
							//Cleanup
							fclose(dumpFd);
							mem_free(CardBuffer);
							mem_free(CheckBuffer);
							mem_free(EraseCheckBuffer);
							sprintf(msg, "Error! Writing failed for unknown reasons at block %d!", current_block);
							WaitPrompt (msg);
							CARD_Unmount(slot);
							return -1;
						}
					}
					else //Reading failed
					{
						//Cleanup
						fclose(dumpFd);
						mem_free(CardBuffer);
						mem_free(CheckBuffer);
						mem_free(EraseCheckBuffer);
						sprintf(msg, "Error %d reading data). Memory card could be corrupt now!!!",err);
						WaitPrompt (msg);
						CARD_Unmount(slot);
						return -1;
					}
	#endif
					/*At this point:
						1. Data was read from fat device
						2. Sector was erased (+- checked if properly erased)
						3. Data was written to memory card
						4. Written data was checked to be correctly written
					
					  So, next block it is
					*/
					
					writen += write_len;
					upblock+=upblock+write_len;
					write_addr += write_len;
					if(upblock == SectorSize) //Advance block if we are writing in sizes different than SectorSize (i.e. a page, which is 0x80)
					{
						current_block++;
						upblock = 0;
					}
				}
				else //card_write failed
				{
					//Cleanup
					fclose(dumpFd);
					mem_free(CardBuffer);
	#ifdef DEBUGRAW
					mem_free(CheckBuffer);
					mem_free(EraseCheckBuffer);
	#endif
					//printf("\nerror writing data(%d) Memory card could be corrupt now!!!\n",err);
					sprintf(msg, "Error %d writing data. Memory card could be corrupt now!!!",err);
					WaitPrompt (msg);
					CARD_Unmount(slot);
					return -1;
				}
			} // END while lopp

		//Restore finished

			fclose(dumpFd);

			//printf("\n");
			//gprintf("\n");
			if(bytes_writen != NULL)
				*bytes_writen = writen;
			mem_free(CardBuffer);
	#ifdef DEBUGRAW
			mem_free(CheckBuffer);
			mem_free(EraseCheckBuffer);
	#endif		
			CARD_Unmount(slot);
			return 1;		
		}
	//User canceled restore (2nd prompt)
	}
//User canceled restore (1st prompt)

//Clean
	mem_free(CardBuffer);
#ifdef DEBUGRAW
	mem_free(CheckBuffer);
	mem_free(EraseCheckBuffer);
#endif
	CARD_Unmount(slot);
	return -1;

}
