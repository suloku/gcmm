/****************************************************************************
* ds bomb gamecube memory card manager
* based on libOGC Memory Card Backup by askot
* delete support + saves information made by dsbomb
* Gui original design by dsbomb
* Gui adds and user interaction by justb
* Banner/Icon display updates by Dronesplitter
* Uses freetype.
* libFreeType is available from the downloads sections.
*****************************************************************************/
#include <gccore.h>
#include <ogcsys.h>
#include <network.h>
#include <smb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/time.h>
#include <fat.h>

#ifdef HW_RVL
#include <wiiuse/wpad.h>
#include <sdcard/wiisd_io.h>
#else
#include <sdcard/gcsd.h>
#endif

#include "mcard.h"
#include "card.h"
#include "raw.h"
#include "sdsupp.h"
#include "freetype.h"
#include "bitmap.h"

#define PSOSDLOADID 0x7c6000a6
int mode;
int cancel;
/*** 2D Video Globals ***/
GXRModeObj *vmode;		/*** Graphics Mode Object ***/
u32 *xfb[2] = { NULL, NULL };	/*** Framebuffers ***/
int whichfb = 0;		/*** Frame buffer toggle ***/
int screenheight;
static int vmode_60hz = 0;

extern u8 filelist[1024][1024];
extern bool offsetchanged;

s32 MEM_CARD = CARD_SLOTB;
extern syssramex *sramex;
extern u8 imageserial[12];

static void updatePAD()
{
	PAD_ScanPads();
#ifdef HW_RVL
	WPAD_ScanPads();
#endif
}

// 1 for Internal SD, 0 for fat32 usb
static int initFAT(int device)
{
	ShowAction("Mounting device...");
#ifdef	HW_RVL
	if (device){//try SD card first
		__io_wiisd.startup();
		if (!__io_wiisd.isInserted())
		{
			ShowAction ("No SD card inserted! Trying USB storage.");
			sleep(1);
				__io_usbstorage.startup();
			if (!__io_usbstorage.isInserted())
			{
				ShowAction ("No USB device inserted either!");
				sleep(1);			
				return 0;
			}
			else if (!fatMountSimple ("fat", &__io_usbstorage)){
				ShowAction("Error Mounting USB fat!");
				sleep(1);
				return 0;
			}
			return 1;//usb mounted
		}
		if (!fatMountSimple ("fat", &__io_wiisd))
		{
			ShowAction("Error Mounting SD fat!");
			sleep(1);
			return 0;
		}
	}else if (!device)//try USB first
	{
		__io_usbstorage.startup();
		if (!__io_usbstorage.isInserted())
		{
			ShowAction ("No usb device inserted! Trying internal SD.");
			sleep(1);
				__io_wiisd.startup();
			if (!__io_wiisd.isInserted())
			{
				ShowAction ("No SD card inserted either!");
				sleep(1);
				return 0;
			}
			else if (!fatMountSimple ("fat", &__io_wiisd)){
				ShowAction("Error Mounting SD fat!");
				sleep(1);
				return 0;
			}
			return 1;//SD mounted
		}
		if (!fatMountSimple ("fat", &__io_usbstorage))
		{
			ShowAction("Error Mounting USB fat!");
			sleep(1);
			return 0;
		}	
	}
#else
	__io_gcsda.startup();
	if (!__io_gcsda.isInserted())
	{
		ShowAction ("No SD Gecko inserted! Insert it in slot A please");
		sleep(1);	
		return 0;
	}
	if (!fatMountSimple ("fat", &__io_gcsda))
	{
		ShowAction("Error Mounting SD fat!");
		sleep(1);
		return 0;
	}
#endif
	return 1;
}

void deinitFAT()
{
	//First unmount all the devs...
	fatUnmount ("fat");
	//...and then shutdown em!
#ifdef	HW_RVL
	__io_wiisd.shutdown();
	__io_usbstorage.shutdown();
#else
	__io_gcsda.shutdown();
#endif
}

/****************************************************************************
* Initialise Video
*
* Before doing anything in libogc, it's recommended to configure a video
* output.
****************************************************************************/
static void
Initialise (void)
{
	VIDEO_Init ();		/*** ALWAYS CALL FIRST IN ANY LIBOGC PROJECT!
				     Not only does it initialise the video
				     subsystem, but also sets up the ogc os
				***/

	PAD_Init ();			/*** Initialise pads for input ***/
#ifdef HW_RVL
	WPAD_Init ();
#endif

	// get default video mode
	vmode = VIDEO_GetPreferredMode(NULL);

	switch (vmode->viTVMode >> 2)
	{
	case VI_PAL:
		// 576 lines (PAL 50Hz)
		// display should be centered vertically (borders)
		//Make all video modes the same size so menus doesn't screw up
		vmode = &TVPal576IntDfScale;
		vmode->xfbHeight = 480;
		vmode->viYOrigin = (VI_MAX_HEIGHT_PAL - 480)/2;
		vmode->viHeight = 480;

		vmode_60hz = 0;
		break;

	case VI_NTSC:
		// 480 lines (NTSC 60hz)
		vmode_60hz = 1;
		break;

	default:
		// 480 lines (PAL 60Hz)
		vmode_60hz = 1;
		break;
	}

#ifdef HW_DOL
	/* we have component cables, but the preferred mode is interlaced
	 * why don't we switch into progressive?
	 * (user may not have progressive compatible display but component input)
	 * on the Wii, the user can do this themselves on their Wii Settings */
	if(VIDEO_HaveComponentCable())
		vmode = &TVNtsc480Prog;
#endif

	/*	// check for progressive scan // bool progressive = FALSE;
		if (vmode->viTVMode == VI_TVMODE_NTSC_PROG)
			progressive = true;
	*/

#ifdef HW_RVL
	// widescreen fix
	if(CONF_GetAspectRatio())
	{
		vmode->viWidth = 678;
		vmode->viXOrigin = (VI_MAX_WIDTH_PAL - 678) / 2;
	}
#endif

	// configure VI
	VIDEO_Configure (vmode);

	// always 480 lines /*** Update screen height for font engine ***/
	screenheight = vmode->xfbHeight;

	/*** Now configure the framebuffer.
	     Really a framebuffer is just a chunk of memory
	     to hold the display line by line.
	***/
	// Allocate the video buffers
	xfb[0] = (u32 *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer (vmode));
	/*** I prefer also to have a second buffer for double-buffering.
	     This is not needed for the console demo.
	***/
	xfb[1] = (u32 *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer (vmode));

	/*** Define a console ***/
	console_init (xfb[0], 20, 64, vmode->fbWidth, vmode->xfbHeight, vmode->fbWidth * 2);

	/*** Clear framebuffer to black ***/
	VIDEO_ClearFrameBuffer (vmode, xfb[0], COLOR_BLACK);
	VIDEO_ClearFrameBuffer (vmode, xfb[1], COLOR_BLACK);

	/*** Set the framebuffer to be displayed at next VBlank ***/
	VIDEO_SetNextFramebuffer (xfb[0]);

	/*** Get the PAD status updated by libogc ***/
	VIDEO_SetPostRetraceCallback (updatePAD);
	VIDEO_SetBlack (0);

	/*** Update the video for next vblank ***/
	VIDEO_Flush ();

	VIDEO_WaitVSync ();		/*** Wait for VBL ***/
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync ();

}



/****************************************************************************
* BackupMode -SD Mode
*
* Perform backup of a memory card file to a SD Card.
****************************************************************************/
void SD_BackupMode ()
{
	int memitems;
	int selected = 0;
	int bytestodo;

	clearRightPane();
	DrawText(390,130,"B a c k u p   M o d e");
	writeStatusBar("Pick a file using UP or DOWN ", "Press A to backup to SD Card ") ;
	/*** Get the directory listing from the memory card ***/
	memitems = CardGetDirectory (MEM_CARD);

	/*** If it's a blank card, get out of here ***/
	if (!memitems)
	{
		WaitPrompt ("No saved games to backup!");
	}
	else
	{
		selected = ShowSelector (1);
		if (cancel)
		{
			WaitPrompt ("Backup action cancelled!");
		}
		else
		{
			/*** Backup the file ***/
			ShowAction ("Reading File From MC SLOT B");
			bytestodo = CardReadFile (MEM_CARD, selected);
			if (bytestodo)
			{
				ShowAction ("Saving to SD CARD");
				if (SDSaveMCImage())
				{
					WaitPrompt ("Backup complete");
				}
				else
				{
					WaitPrompt ("Backup failed");
				}
			}
			else
			{
				WaitPrompt ("Error reading MC file");
			}

		}
	}


}



/****************************************************************************
* BackupModeAllFiles - SD Mode
* Copy all files on the Memory Card to the SD card
****************************************************************************/
void SD_BackupModeAllFiles ()
{
	int memitems;
	int selected = 0;
	int bytestodo;
	
	char buffer[128];

	clearRightPane();
	DrawText(390,130," B a c k u p   A l l ");
	writeStatusBar("Backing up all files.", "This may take a while.");
	/*** Get the directory listing from the memory card ***/
	memitems = CardGetDirectory (MEM_CARD);

	/*** If it's a blank card, get out of here ***/
	if (!memitems)
	{
		WaitPrompt ("No saved games to backup!");
	}
	else
	{
		for ( selected = 0; selected < memitems; selected++ ) {
			/*** Backup files ***/
			sprintf(buffer, "[%d/%d] Reading from MC slot B", selected+1, memitems);
			ShowAction(buffer);
			bytestodo = CardReadFile(MEM_CARD, selected);
			if (bytestodo)
			{
				sprintf(buffer, "[%d/%d] Saving to SD card", selected+1, memitems);
				ShowAction(buffer);
				SDSaveMCImage();
			}
			else
			{
				WaitPrompt ("Error reading MC file");
				return;
			}
		}
		
		WaitPrompt("Full card backup done!");
	}
}



/****************************************************************************
* RestoreMode
*
* Restore a file to Memory Card from SD Card
****************************************************************************/
void SD_RestoreMode ()
{
	int files;
	int selected;

	clearRightPane();
	DrawText(390,130,"R e s t o r e   M o d e");
	writeStatusBar("Pick a file using UP or DOWN", "Press A to restore to Memory Card ") ;
	files = SDGetFileList (1);
	if (!files)
	{
		WaitPrompt ("No saved games in SD Card to restore !");
	}

	else
	{
		selected = ShowSelector (1);

		if (cancel)
		{
			WaitPrompt ("Restore action cancelled !");
		}
		else
		{
			ShowAction ("Reading from SD Card");
			if (SDLoadMCImage ((char*)filelist[selected]))
			{
				ShowAction ("Updating Memory Card");
				if (CardWriteFile (MEM_CARD))
				{
					WaitPrompt ("Restore Complete");
				}
				else
				{
					WaitPrompt ("Error during restore");
				}
			}
			else
			{
				WaitPrompt ("Error reading image");
			}
		}

	}

}

/****************************************************************************
* RawBackupMode -SD Mode
*
* Perform backup of full memory card (in raw format) to a SD Card.
****************************************************************************/
void SD_RawBackupMode ()
{
	s32 writen = 0;
	char msg[64];
	clearRightPane();
	DrawText(50, 230, "R A W   B a c k u p   M o d e");
	writeStatusBar("Reading memory card... ", "");

	if (BackupRawImage(MEM_CARD, &writen) == 1)
	{
		sprintf(msg, "Backup complete! Wrote %d bytes to SD",writen);
		WaitPrompt(msg);
	}else{
		
		WaitPrompt("Backup failed!");
	}

}

/****************************************************************************
* RawRestoreMode
*
* Restore a full raw backup to Memory Card from SD Card
****************************************************************************/
void SD_RawRestoreMode ()
{
	int files;
	int selected;
	char msg[64];
	s32 writen = 0;
	int i;

	clearRightPane();
	DrawText(400,130,"R A W   R e s t o r e");
	DrawText(450,150,"M o d e");
	writeStatusBar("Pick a file using UP or DOWN", "Press A to restore to Memory Card ");
	
	files = SDGetFileList (0);
	if (!files)
	{
		WaitPrompt ("No raw backups in SD Card to restore !");
	}else
	{
		selected = ShowSelector (0);

		if (cancel)
		{
			WaitPrompt ("Restore action cancelled !");
			return;
		}
		else
		{
			//Now imageserial and sramex.flash_id[MEM_CARD] variables should hold the proper information
			for (i=0;i<12;i++){
				if (imageserial[i] != sramex->flash_id[MEM_CARD][i]){
					WaitPrompt ("Card and image flash ID don't match !");
					return;
				}
			}
			ShowAction ("Reading from SD Card...");
			if (RestoreRawImage(MEM_CARD, (char*)filelist[selected], &writen) == 1)
			{
				sprintf(msg, "Restore complete! Wrote %d bytes to card",writen);
				WaitPrompt(msg);
			}else
			{
				WaitPrompt("Restore failed!");
			}
		}
	}
}

/****************************************************************************
* Main
****************************************************************************/
int main ()
{

	int have_sd = 0;


#ifdef HW_DOL
	int *psoid = (int *) 0x80001800;
	void (*PSOReload) () = (void (*)()) 0x80001800;
#endif

	Initialise ();	/*** Start video ***/
	FT_Init ();		/*** Start FreeType ***/

#ifdef HW_RVL
	initialise_power();
	have_sd = initFAT(WaitPromptChoice ("Use internal SD or FAT 32 USB device?", "USB", "SD"));
#else
	have_sd = initFAT(0);
#endif


	for (;;)
	{
		/*** Select Mode ***/
		ClearScreen();
		setfontsize (FONT_SIZE);
		freecardbuf();
		cancel = 0;/******a global value to track action aborted by user pressing button B********/
		mode = SelectMode ();
#ifdef HW_RVL		
		if ((mode != 500 ) && (mode != 100)){
			if (WaitPromptChoice ("Please select a memory card slot", "Slot B", "Slot A") == 1)
			{
				MEM_CARD = CARD_SLOTA;
			}else
			{
				MEM_CARD = CARD_SLOTB;
			}
		}
#endif

		/*** Mode == 100 for backup, 200 for restore ***/
		switch (mode)
		{
		case 100 : //User pressed A so keep looping
			//SMB_BackupMode();
			//WaitPrompt ("Inactive");
			break;
		case 200 : //User wants to delete
			MC_DeleteMode(MEM_CARD);
			break;
		case 300 : //User wants to backup
			if (have_sd) SD_BackupMode();
			else WaitPrompt("Reboot aplication with an SD card");
			break;
		case 400 : //User wants to restore
			if (have_sd) SD_RestoreMode();
			else WaitPrompt("Reboot aplication with an SD card");
			break;
		case 500 ://exit
			ShowAction ("Exiting...");
			deinitFAT();
#ifdef HW_RVL
			//if there's a loader stub load it, if not return to wii menu.
			if (!!*(u32*)0x80001800) exit(1);
			else SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
#else
			if (psoid[0] == PSOSDLOADID) PSOReload ();
			else SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
#endif
			break; //PSO_Reload
		case 600 : //User wants to backup full card
			if (have_sd) SD_BackupModeAllFiles();
			else WaitPrompt("Reboot aplication with an SD card");
			break;
		case 700 : //Raw backup mode
			if (have_sd)
			{
				DrawText(50, 230, "R A W   B a c k u p   M o d e");
				SD_RawBackupMode();
			}else
			{
				WaitPrompt("Reboot aplication with an SD card");
			}
			break;
		case 800 : //Raw restore mode
			//These two lines are a work around for the second call of CARD_Probe to detect a newly inserted memory card
			CARD_Probe(MEM_CARD);
			VIDEO_WaitVSync (); 		
			if (CARD_Probe(MEM_CARD) > 0)
			{
				if (have_sd) SD_RawRestoreMode();
				else WaitPrompt("Reboot aplication with an SD card");
				
			}else if (MEM_CARD)
			{
				WaitPrompt("Please insert a memory card in slot B");
			}else
			{
				WaitPrompt("Please insert a memory card in slot A");
			}				
			break;
		case 900 : //Format card mode
			//These two lines are a work around for the second call of CARD_Probe to detect a newly inserted memory card
			CARD_Probe(MEM_CARD);
			VIDEO_WaitVSync (); 
			if (CARD_Probe(MEM_CARD) > 0)
			{
				DrawText(50, 230, "F o r m a t   C a r d   M o d e");
				clearRightPane();
				MC_FormatMode(MEM_CARD);
				
			}else if (MEM_CARD)
			{
				WaitPrompt("Please insert a memory card in slot B");
			}else
			{
				WaitPrompt("Please insert a memory card in slot A");
			}
			break;
		}

		offsetchanged = true;
	}
	while (1);
	return 0;
}
