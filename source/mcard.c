/****************************************************************************
* Memory Card
*
* Supporting functions.
*
* MAXFILEBUFFER is set to 2MB as it's the largest file I've seen.
*
* CARDBACKUP FILE
*
*   0	- Copy of CardDir
*  64   - Copy of CardStatus
* 192   - Memory Card File Data
****************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "card.h"
#include "mcard.h"
#include "gci.h"
#include "freetype.h"

/*** Memory Card Work Area ***/
static u8 SysArea[CARD_WORKAREA] ATTRIBUTE_ALIGN (32);

/*** Memory File Buffer ***/
#define MAXFILEBUFFER (1024 * 2048)	/*** 2MB Buffer ***/
u8 FileBuffer[MAXFILEBUFFER] ATTRIBUTE_ALIGN (32);
u8 CommentBuffer[64] ATTRIBUTE_ALIGN (32);

u16 tlut[256] ATTRIBUTE_ALIGN (32);
u16 tlutbanner[256] ATTRIBUTE_ALIGN (32);
u8 icondata[1024] ATTRIBUTE_ALIGN (32);
u16 icondataRGB[1024] ATTRIBUTE_ALIGN (32);
/*** This array holds the 16-bit banner data for the current save
	 Needs decoding by bannerloadRGB function before we can show it ***/
u16 bannerdata[CARD_BANNER_W*CARD_BANNER_H] ATTRIBUTE_ALIGN (32);
/*** This array holds the 8-bit banner data for the current save
	 Needs decoding by bannerloadCI function before we can show it ***/
u8 bannerdataCI[CARD_BANNER_W*CARD_BANNER_H] ATTRIBUTE_ALIGN (32);
/*** This matrix will serve as our array of filenames for each file on the card
     We add 10 to filenamelen since we add on game company info***/
u8 filelist[1024][1024];
int maxfile;
extern int cancel;
extern bool offsetchanged;

/*** Card lib ***/
card_dir CardList[CARD_MAXFILES];	/*** Directory listing ***/
static card_dir CardDir;
static card_file CardFile;
static card_stat CardStatus;
static int cardcount = 0;
static u8 permission;
s32 memsize, sectsize;

GCI gci;

/*---------------------------------------------------------------------------------
	This function is called if a card is physically removed
---------------------------------------------------------------------------------*/
void card_removed(s32 chn,s32 result)
{
	if (chn == CARD_SLOTA){
		//printf("Card was removed from slot A");
	}
	else{
		//printf("Card was removed from slot B");
	}
	CARD_Unmount(chn);
}

/****************************************************************************
 * MountCard
 *
 * Mounts the memory card in the given slot.
 * CARD_Mount is called for a maximum of 10 tries
 * Returns the result of the last attempted CARD_Mount command.
 ***************************************************************************/
int MountCard(int cslot)
{
	s32 ret = -1;
	int tries = 0;
	int isMounted;

	// Mount the card, try several times as they are tricky
	while ( (tries < 10) && (ret < 0))
	{
		/*** We already initialized the Memory Card subsystem with CARD_Init() in select_memcard_slot(). Let's reset the
		EXI subsystem, just to make sure we have no problems mounting the card ***/
		EXI_ProbeReset();
		CARD_Init (NULL, NULL);

		/*** Mount the card ***/
		ret = CARD_Mount (cslot, SysArea, card_removed);
		if (ret >= 0) break;
		
		VIDEO_WaitVSync ();
		tries++;
	}
			/*** Make sure the card is really mounted ***/
	isMounted = CARD_ProbeEx(cslot, &memsize, &sectsize);
	if (memsize > 0 && sectsize > 0)//then we really mounted de card
	{
		return isMounted;
	}
	/*** If this point is reached, something went wrong ***/
	
	return ret;
}

/****************************************************************************
* GCIMakeHeader
*
* Create a GCI compatible header from libOGC card_stat
****************************************************************************/
void GCIMakeHeader()
{
	/*** Clear out the cgi header ***/
	memset(&gci, 0xff, sizeof(GCI) );

	/*** Populate ***/
	memcpy(&gci.gamecode, &CardStatus.gamecode, 4);
	memcpy(&gci.company, &CardStatus.company, 2);
	gci.banner_fmt = CardStatus.banner_fmt;
	memcpy(&gci.filename, &CardStatus.filename, CARD_FILENAMELEN);
	gci.time = CardStatus.time;
	gci.icon_addr = CardStatus.icon_addr;
	gci.icon_fmt = CardStatus.icon_fmt;
	gci.icon_speed = CardStatus.icon_speed;

	/*** Permission key has to be gotten separately. Make it 0 for normal privileges ***/
	gci.unknown1 = permission;
	/*** Who cares about copy counter ***/
	gci.unknown2 = 0;

	/*** Block index does not matter, it won't be restored at the same spot ***/
	gci.index = 32;

	gci.filesize8 = (CardStatus.len / 8192);
	gci.comment_addr = CardStatus.comment_addr;

	/*** Copy to head of buffer ***/
	memcpy(FileBuffer, &gci, sizeof(GCI));
}

/****************************************************************************
* ExtractGCIHeader
*
* Extract a GCI Header to libOGC card_stat
****************************************************************************/
void ExtractGCIHeader()
{
	/*** Clear out the status ***/
	memset(&CardStatus, 0, sizeof(card_stat));
	memcpy(&gci, FileBuffer, sizeof(GCI));

	memcpy(&CardStatus.gamecode, &gci.gamecode, 4);
	memcpy(&CardStatus.company, &gci.company, 2);
	CardStatus.banner_fmt = gci.banner_fmt;
	memcpy(&CardStatus.filename, &gci.filename, CARD_FILENAMELEN);
	CardStatus.time = gci.time;
	CardStatus.icon_addr = gci.icon_addr;
	CardStatus.icon_fmt = gci.icon_fmt;
	CardStatus.icon_speed = gci.icon_speed;
	permission = gci.unknown1;
	CardStatus.len = gci.filesize8 * 8192;
	CardStatus.comment_addr = gci.comment_addr;
}

/****************************************************************************
* CardGetDirectory
*
* Returns number of files found on a card.
****************************************************************************/
int CardGetDirectory (int slot)
{
	int err;
	char company[4];
	char gamecode[6];

	company[2] = gamecode[4] = 0;

	/*** Clear the work area ***/
	memset (SysArea, 0, CARD_WORKAREA);

	/*** Initialise the Card system, show all ***/
	CARD_Init(NULL, NULL);

	/*** Try to mount the card ***/
	err = MountCard(slot);
	if (err < 0)
	{
		WaitCardError("CardMount", err);
		return 0;			/*** Unable to mount the card ***/
	}

	/*** Retrieve the directory listing ***/
	cardcount = 0;
	err = CARD_FindFirst (slot, &CardDir, true); //true means we want to showall
	while (err != CARD_ERROR_NOFILE)
	{
		memcpy (&CardList[cardcount], &CardDir, sizeof (card_dir));
		memset (filelist[cardcount], 0, 1024);
		memcpy (company, &CardDir.company, 2);
		memcpy (gamecode, &CardDir.gamecode, 4);
		//This array will store what will show in left window
		sprintf ((char*)filelist[cardcount], "%s-%s-%s", company, gamecode, CardDir.filename);
		cardcount++;
		err = CARD_FindNext (&CardDir);
	}

	/*** Release as soon as possible ***/
	CARD_Unmount (slot);

	maxfile = cardcount;
	return cardcount;
}

/****************************************************************************
* CardListFiles
****************************************************************************/
void CardListFiles ()
{
	int i;
	char company[4];
	char gamecode[6];

	company[2] = 0;
	gamecode[4] = 0;

	for (i = 0; i < cardcount; i++)
	{
		memcpy (company, &CardList[i].company, 2);
		memcpy (gamecode, &CardList[i].gamecode, 4);
		printf ("%s %s %s\n", company, gamecode, CardList[i].filename);
	}
}
/****************************************************************************
* CardReadFileHeader
*
* Retrieve a file header from the previously populated list.
* Reads in banner and icon data now
****************************************************************************/
//TODO: get icon and banner settings
int CardReadFileHeader (int slot, int id)
{
	int bytesdone = 0;
	int err;
	u32 SectorSize;
	char company[4];
	char gamecode[6];
	int filesize;
	int numicons;
	int i;
	u16 check;

	if (id >= cardcount)
	{
		WaitPrompt("Bad id");
		return 0;			/*** Bad id ***/
	}

	/*** Clear the work buffers ***/
	memset (FileBuffer, 0, MAXFILEBUFFER);
	memset (CommentBuffer, 0, 64);
	memset (SysArea, 0, CARD_WORKAREA);
	company[2] = 0;
	gamecode[4] = 0;

	memcpy (company, &CardList[id].company, 2);
	memcpy (gamecode, &CardList[id].gamecode, 4);

	/*** Initialise for this company ***/
	CARD_Init (gamecode, company);

	/*** Mount the card ***/
	err = MountCard(slot);
	if (err < 0)
	{
		WaitCardError("CardMount", err);
		return 0;			/*** Unable to mount the card ***/
	}

	/*** Retrieve sector size ***/
	CARD_GetSectorSize (slot, &SectorSize);

	/*** Open the file ***/
	err = CARD_Open (slot, (char *) &CardList[id].filename, &CardFile);
	if (err < 0)
	{
		CARD_Unmount (slot);
		WaitCardError("CardOpen", err);
		return 0;
	}

#ifdef STATUSOGC
	/*** Get card status info ***/
	CARD_GetStatus (slot, CardFile.filenum, &CardStatus);
	CARD_GetAttributes(slot,CardFile.filenum, &permission);

	GCIMakeHeader();
#else
	//get directory entry (same as gci header, but with all the data)
	memset(&gci,0,sizeof(GCI));
	__card_getstatusex(slot,CardFile.filenum,&gci);
	/*** Copy to head of buffer ***/
	memcpy(FileBuffer, &gci, sizeof(GCI));
#endif

	/*** Copy the file contents to the buffer ***/
	filesize = CardFile.len;

	while (bytesdone < filesize)
	{
		CARD_Read (&CardFile, FileBuffer + MCDATAOFFSET + bytesdone, SectorSize, bytesdone);
		bytesdone += SectorSize;
	}

	//Find how many icons are present
	numicons = 8;
	check = gci.icon_fmt;
	for (i = 0; i < 8; i++) {
		if (check & 0xC000)
			break;
		else
			numicons--;
		check = check << 2;
	}

	/***
		Get the Banner/Icon Data from the memory card file.
		Very specific if/else setup to minimize data copies.
	***/
	if ((gci.banner_fmt&CARD_BANNER_MASK) == CARD_BANNER_RGB) {
		//RGB banners are 96*32*2 in size
		memcpy(bannerdata, FileBuffer + MCDATAOFFSET + gci.icon_addr, 6144);
		//this checks for CI icon format 8 bit icon
		if (gci.icon_fmt&0x01) {
			memcpy(icondata, FileBuffer + MCDATAOFFSET + gci.icon_addr+6144, 1024);
			if ((gci.icon_fmt&CARD_ICON_MASK) == 1) {
				memcpy(tlut, FileBuffer + MCDATAOFFSET + gci.icon_addr+6144+1024*numicons, 512);
			}
			else if ((gci.icon_fmt&CARD_ICON_MASK) == 3) {
				memcpy(tlut, FileBuffer + MCDATAOFFSET + gci.icon_addr+6144+1024, 512);
			}
		}
		//if not CI, read in RGB 16 bit icon
		else {
			memcpy(icondataRGB, FileBuffer + MCDATAOFFSET + gci.icon_addr+6144, 2048);
		}
	}
	else if ((gci.banner_fmt&CARD_BANNER_MASK) == CARD_BANNER_CI) {
		memcpy(bannerdataCI, FileBuffer + MCDATAOFFSET + gci.icon_addr, 3072);
		memcpy(tlutbanner, FileBuffer + MCDATAOFFSET + gci.icon_addr+3072, 512);
		if (gci.icon_fmt&0x01) {
			memcpy(icondata, FileBuffer + MCDATAOFFSET + gci.icon_addr+3072+512, 1024);
			if ((gci.icon_fmt&CARD_ICON_MASK) == 1) {
				memcpy(tlut, FileBuffer + MCDATAOFFSET + gci.icon_addr+3072+512+1024*numicons, 512);
			}
			else if ((gci.icon_fmt&CARD_ICON_MASK) == 3) {
				memcpy(tlut, FileBuffer + MCDATAOFFSET + gci.icon_addr+3072+512+1024, 512);
			}
		}
		else {
			memcpy(icondataRGB, FileBuffer + MCDATAOFFSET + gci.icon_addr+3072+512, 2048);
		}
	}
	else {
		if (gci.icon_fmt&0x01) {
			memcpy(icondata, FileBuffer + MCDATAOFFSET + gci.icon_addr, 1024);
			if ((gci.icon_fmt&CARD_ICON_MASK) == 1) {
				memcpy(tlut, FileBuffer + MCDATAOFFSET + gci.icon_addr+1024*numicons, 512);
			}
			else {
				memcpy(tlut, FileBuffer + MCDATAOFFSET + gci.icon_addr+1024, 512);
			}
		}
		else {
			memcpy(icondataRGB, FileBuffer + MCDATAOFFSET + gci.icon_addr, 2048);
		}
	}

	/*** Get the comment (two 32 byte strings) into buffer ***/
	memcpy(CommentBuffer, FileBuffer + MCDATAOFFSET + gci.comment_addr, 64);

	/*** Close the file ***/
	CARD_Close (&CardFile);

	/*** Unmount the card ***/
	CARD_Unmount (slot);

	return filesize + MCDATAOFFSET;
}

/****************************************************************************
* CardReadFile
*
* Retrieve a file from the previously populated list.
* Place in filebuffer space, for collection by SMB write.
****************************************************************************/
int CardReadFile (int slot, int id)
{
	int bytesdone = 0;
	int err;
	u32 SectorSize;
	char company[4];
	char gamecode[6];
	int filesize;

	if (id >= cardcount)
	{
		WaitPrompt("Bad id");
		return 0;			/*** Bad id ***/
	}

	/*** Clear the work buffers ***/
	memset (FileBuffer, 0, MAXFILEBUFFER);
	memset (SysArea, 0, CARD_WORKAREA);
	company[2] = 0;
	gamecode[4] = 0;

	memcpy (company, &CardList[id].company, 2);
	memcpy (gamecode, &CardList[id].gamecode, 4);

	/*** Initialise for this company ***/
	CARD_Init (gamecode, company);

	/*** Mount the card ***/
	err = MountCard(slot);
	if (err < 0)
	{
		WaitCardError("CardMount", err);
		return 0;			/*** Unable to mount the card ***/
	}

	/*** Retrieve sector size ***/
	CARD_GetSectorSize (slot, &SectorSize);

	/*** Open the file ***/
	err = CARD_Open (slot, (char *) &CardList[id].filename, &CardFile);
	if (err < 0)
	{
		CARD_Unmount (slot);
		WaitCardError("CardOpen", err);
		return 0;
	}

#ifdef STATUSOGC
	/*** Get card status info ***/
	CARD_GetStatus (slot, CardFile.filenum, &CardStatus);
	CARD_GetAttributes(slot,CardFile.filenum, &permission);

	GCIMakeHeader();
#else
	//get directory entry (same as gci header, but with all the data)
	memset(&gci,0,sizeof(GCI));
	__card_getstatusex(slot,CardFile.filenum,&gci);
	/*** Copy to head of buffer ***/
	memcpy(FileBuffer, &gci, sizeof(GCI));
#endif

	/*** Copy the file contents to the buffer ***/
	filesize = CardFile.len;

	while (bytesdone < filesize)
	{
		CARD_Read (&CardFile, FileBuffer + MCDATAOFFSET + bytesdone, SectorSize, bytesdone);
		bytesdone += SectorSize;
	}

	/*** Close the file ***/
	CARD_Close (&CardFile);

	/*** Unmount the card ***/
	CARD_Unmount (slot);

	return filesize + MCDATAOFFSET;
}

/****************************************************************************
* CardWriteFile
*
* Relies on *GOOD* data being placed in the FileBuffer prior to calling.
* See ReadSMBImage
****************************************************************************/
int CardWriteFile (int slot)
{
	char company[4];
	char gamecode[6];
	char filename[CARD_FILENAMELEN];
	int err;
	u32 SectorSize;
	int offset;
	int written;
	int filelen;

	company[2] = gamecode[4] = 0;

	memset (SysArea, 0, CARD_WORKAREA);
	memset(filename, 0, CARD_FILENAMELEN);
	ExtractGCIHeader();
	memcpy(company, &gci.company, 2);
	memcpy(gamecode, &gci.gamecode, 4);
	memcpy(filename, &gci.filename, CARD_FILENAMELEN);
	filelen = gci.filesize8 * 8192;

	/*** Init system ***/
	CARD_Init(gamecode, company);

	/*** Mount the card ***/
	err = MountCard(slot);
	if (err < 0)
	{
		WaitCardError("CardMount", err);
		return 0;			/*** Unable to mount the card ***/
	}

	CARD_GetSectorSize (slot, &SectorSize);

	/*** If this file exists, abort ***/
	err = CARD_FindFirst (slot, &CardDir, false);
	while (err != CARD_ERROR_NOFILE)
	{
		if (((u32)CardDir.gamecode == (u32)gamecode) && (strcmp ((char *) CardDir.filename, (char *)filename) == 0))
		{
			/*** Found the file - abort ***/
			CARD_Unmount (slot);
			WaitCardError("File already exists", err);
			return 0;
		}

		err = CARD_FindNext (&CardDir);
	}

	//This is needed for propper restoring
	CARD_SetCompany(company);
	CARD_SetGamecode(gamecode);

	/*** Now restore the file from backup ***/
	err = CARD_Create (slot, (char *) filename, filelen, &CardFile);
	if (err < 0)
	{
		//Return To show all so we don't have errors
		CARD_SetCompany(NULL);
		CARD_SetGamecode(NULL);
		CARD_Unmount (slot);
		WaitCardError("CardCreate", err);
		return 0;
	}

	/*** Now write the file data, in sector sized chunks ***/
	offset = 0;
	while (offset < filelen)
	{
		if ((offset + SectorSize) <= filelen)
		{
			written = CARD_Write (&CardFile, FileBuffer + MCDATAOFFSET + offset + OFFSET, SectorSize, offset);
		}
		else
		{
			written = CARD_Write (&CardFile, FileBuffer + MCDATAOFFSET + offset + OFFSET, ((offset + SectorSize) - filelen), offset);
		}

		offset += SectorSize;
	}

	OFFSET = 0;

#ifdef STATUSOGC
	/*** Finally, update the status ***/
	CARD_SetStatus (slot, CardFile.filenum, &CardStatus);
#else
	__card_setstatusex(slot, CardFile.filenum, &gci);
#endif

	//For some reason this sets the file to Move->allowed, Copy->not allowed, Public file instead of the actual permission value
	//CARD_SetAttributes(slot, CardFile.filenum, &permission);

	//Return To show all so we don't have errors
	CARD_SetCompany(NULL);
	CARD_SetGamecode(NULL);

	CARD_Close (&CardFile);
	CARD_Unmount (slot);

	return 1;
}

void WaitCardError(char *src, int error)
{
	char msg[1024], err[256];
	
	//Error message possibilites
	switch (error)
	{
	case CARD_ERROR_BUSY:
		sprintf(err, "Busy");
		break;
	case CARD_ERROR_WRONGDEVICE:
		sprintf(err, "Wrong device in slot");
		break;
	case CARD_ERROR_NOCARD:
		sprintf(err, "No Card");
		break;
	case CARD_ERROR_NOFILE:
		sprintf(err, "No File");
		break;
	case CARD_ERROR_IOERROR:
		sprintf(err, "Internal EXI I/O error");
		break;
	case CARD_ERROR_BROKEN:
		sprintf(err, "File/Dir entry broken");
		break;
	case CARD_ERROR_EXIST:
		sprintf(err, "File already exists");
		break;
	case CARD_ERROR_NOENT:
		sprintf(err, "No empty blocks");
		break;
	case CARD_ERROR_INSSPACE:
		sprintf(err, "Not enough space");
		break;
	case CARD_ERROR_NOPERM:
		sprintf(err, "No permission");
		break;
	case CARD_ERROR_LIMIT:
		sprintf(err, "Card size limit reached");
		break;
	case CARD_ERROR_NAMETOOLONG:
		sprintf(err, "Filename too long");
		break;
	case CARD_ERROR_ENCODING:
		sprintf(err, "Font encoding mismatch");
		break;
	case CARD_ERROR_CANCELED:
		sprintf(err, "Card operation cancelled");
		break;
	case CARD_ERROR_FATAL_ERROR:
		sprintf(err, "Fatal error");
		break;
	case CARD_ERROR_READY:
		sprintf(err, "Ready");
		break;
	default:
		sprintf(err, "Unknown");
		break;
	}
	//Here we build the full error message
	sprintf(msg, "MemCard Error: %s - %d %s", src,error, err);

	WaitPrompt(msg);
}

void MC_DeleteMode(int slot)
{
	int memitems, err;
	int selected = 0;
	int erase;
	clearRightPane();
	DrawText(390,130,"D e l e t e   M o d e");
	writeStatusBar("Choose a file with UP button or DOWN button ", "Press A button to delete ") ;
	char msg[1024];

	/*** Get the directory listing from the memory card ***/
	memitems = CardGetDirectory (slot);

	/*** If it's a blank card, get out of here ***/
	if (!memitems)
	{
		WaitPrompt ("No saved games to delete !");
	}
	else
	{
		while(1)
		{
			// TODO: implement showselector
			selected = ShowSelector(1);
			if (cancel)
			{
				WaitPrompt ("Delete action cancelled !");
				return;
			}

			//0 = B wass pressed -> delete the file
			erase = WaitPromptChoice("Are you sure you want to delete the file?", "Delete", "Cancel");
			if (!erase)
			{
				// selected = 1;

				/*** Delete the file ***/
				sprintf(msg, "Deleting \"%s\"", CardList[selected].filename);
				writeStatusBar(msg,"");
				//WaitPrompt(msg);

				CARD_Init((const char*)CardList[selected].gamecode, (const char*)CardList[selected].company);

				/*** Try to mount the card ***/
				err = MountCard(slot);
				if (err < 0)
				{
					WaitCardError("MCDel Mount", err);
					return; /*** Unable to mount the card ***/
				}

				err = CARD_Delete(slot, (char *) &CardList[selected].filename);
				if (err < 0)
				{
					WaitCardError("MCDel", err);
				}
				else
				{
					WaitPrompt("Delete complete");
				}

				CARD_Unmount(slot);
				return;
			}
			offsetchanged = true;
		}
	}
}

void MC_FormatMode(s32 slot)
{
	int erase = 1;
	int err;
	
	//0 = B wass pressed -> ask again
	if (!slot){
		erase = WaitPromptChoice("Are you sure you want to format memory card in slot A?", "Format", "Cancel");
	}else{
		erase = WaitPromptChoice("Are you sure you want to format memory card in slot B?", "Format", "Cancel");
	}
	
	if (!erase){
		if (!slot){
			erase = WaitPromptChoiceAZ("All contents of memory card in slot A will be erased, continue?", "Format", "Cancel");
		}else{
			erase = WaitPromptChoiceAZ("All contents of memory card in slot B will be erased, continue?", "Format", "Cancel");
		}

		if (!erase)
		{

			/*** Try to mount the card ***/
			err = MountCard(slot);
			if (err < 0)
			{
				WaitCardError("MCFormat Mount", err);
				return; /*** Unable to mount the card ***/
			}
			ShowAction("Formatting card...");
			/*** Format the card ***/
			CARD_Format(slot);
			usleep(1000*1000);

			/*** Try to mount the card ***/
			err = MountCard(slot);
			if (err < 0)
			{
				WaitCardError("MCFormat Mount", err);
				return; /*** Unable to mount the card ***/
			}else
			{
				WaitPrompt("Format completed successfully");
			}

				CARD_Unmount(slot);
				return;
		}
	}

	WaitPrompt("Format operation cancelled");
	return;
}
