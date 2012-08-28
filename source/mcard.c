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

#include "mcard.h"
#include "gci.h"
#include "freetype.h"

/*** Memory Card Work Area ***/
static u8 SysArea[CARD_WORKAREA] ATTRIBUTE_ALIGN (32);

/*** Memory File Buffer ***/
#define MAXFILEBUFFER (1024 * 2048)	/*** 2MB Buffer ***/
u8 FileBuffer[MAXFILEBUFFER] ATTRIBUTE_ALIGN (32);
u8 CommentBuffer[64] ATTRIBUTE_ALIGN (32);
u8 filelist[1024][1024];
int maxfile;
extern int cancel;
/*** Card lib ***/
card_dir CardList[CARD_MAXFILES];	/*** Directory listing ***/
static card_dir CardDir;
static card_file CardFile;
static card_stat CardStatus;
static int cardcount = 0;
static u8 permission;


GCI gci;


/*---------------------------------------------------------------------------------
	This function is called if a card is physically removed
---------------------------------------------------------------------------------*/
void card_removed(s32 chn,s32 result)
{
//---------------------------------------------------------------------------------
	printf("card was removed from slot %c\n",(chn==0)?'A':'B');
	CARD_Unmount(chn);
}

/****************************************************************************
 * MountCard
 *
 * Mounts the memory card in the given slot.
 * Returns the result of the last attempted CARD_Mount command.
 ***************************************************************************/
int MountCard(int cslot)
{
	s32 ret = -1;
	int tries = 0;
	int isMounted, memsize, sectsize;
	char msg[64];

	// Mount the card, try several times as they are tricky
	while ( (tries < 10) && (ret < 0))
	{
		/*** We already initialized the Memory Card subsystem with CARD_Init() in select_memcard_slot(). Let's reset the
		EXI subsystem, just to make sure we have no problems mounting the card ***/
		EXI_ProbeReset();

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
		/*** It was a false positive. Weird ***/
		return isMounted;
	}
	sprintf(msg, "%d blocks (%d sectorsize)", memsize, sectsize);
	writeStatusBar("Card mounted",msg);
	ShowScreen();	
	sleep(1);
	writeStatusBar("","");	
	/*** If this point is reached, everything went fine ***/
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
	err = CARD_FindFirst (slot, &CardDir, true);
	while (err != CARD_ERROR_NOFILE)
	{
		memcpy (&CardList[cardcount], &CardDir, sizeof (card_dir));
		memset (filelist[cardcount], 0, 1024);
		memcpy (company, &CardDir.company, 2);
		memcpy (gamecode, &CardDir.gamecode, 4);
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
	if (err)
	{
		CARD_Unmount (slot);
		WaitCardError("CardOpen", err);
		return 0;
	}

	/*** Get card status info ***/
	CARD_GetStatus (slot, CardFile.filenum, &CardStatus);
	CARD_GetAttributes(slot,CardFile.filenum, &permission);

	GCIMakeHeader();

	/*** Copy the file contents to the buffer ***/
	filesize = CardFile.len;

	while (bytesdone < filesize)
	{
		CARD_Read (&CardFile, FileBuffer + MCDATAOFFSET + bytesdone, SectorSize, bytesdone);
		bytesdone += SectorSize;
	}

	//get the comment (two 32 byte strings) into buffer
	memcpy(CommentBuffer, FileBuffer + MCDATAOFFSET + CardStatus.comment_addr, 64);

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
	if (err)
	{
		CARD_Unmount (slot);
		WaitCardError("CardOpen", err);
		return 0;
	}

	/*** Get card status info ***/
	CARD_GetStatus (slot, CardFile.filenum, &CardStatus);
	CARD_GetAttributes(slot,CardFile.filenum, &permission);

	GCIMakeHeader();

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
	if (err)
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

	/*** Finally, update the status ***/
	CARD_SetStatus (slot, CardFile.filenum, &CardStatus);

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
delete_loop:
		// TODO: implement showselector
		selected = ShowSelector();
		if (cancel)
		{
			WaitPrompt ("Delete action cancelled !");
			return;
		}

		//0 = B wass pressed -> delete the file
		erase = WaitPromptChoice("Are you sure to delete the file?", "Delete", "Cancel");
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
			if (err)
			{
				WaitCardError("MCDel", err);
			}
			else
			{
				WaitPrompt("Delete complete");
			}

			CARD_Unmount(slot);
		}
		else
		{
			goto delete_loop;
		}
	}
}