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
#include <sdcard/gcsd.h>

#ifdef HW_RVL
#include <wiiuse/wpad.h>
#include <sdcard/wiisd_io.h>
#endif


#include "mcard.h"
#include "raw.h"
#include "sdsupp.h"
#include "freetype.h"
#include "bitmap.h"

#ifndef HW_RVL
#include "aram/sidestep.h"
#endif

#define PSOSDLOADID 0x7c6000a6
//Comment FLASHIDCHECK to allow writing any image to any mc. This will corrupt official cards.
#define FLASHIDCHECK

#define SDGECKOA_PATH "sda"
#define SDGECKOB_PATH "sdb"
#define SD2SP2_PATH "sdc"
#define GCLOADER_PATH "gcl"
#define WIISD_PATH "wsd"
#define WIIUSB_PATH "usb"
char fatpath[4];

const char appversion[] = "v1.5.1b2";
int mode;
int cancel;
int doall;

/*** 2D Video Globals ***/
GXRModeObj *vmode;		/*** Graphics Mode Object ***/
u32 *xfb[2] = { NULL, NULL };	/*** Framebuffers ***/
int whichfb = 0;		/*** Frame buffer toggle ***/
int screenheight;
int vmode_60hz = 0;
u32 retraceCount;

extern u8 filelist[1024][1024];
extern bool offsetchanged;
extern u8 currFolder[260];
extern int folderCount;
extern int displaypath;
u8 selector_flag;

s32 MEM_CARD = CARD_SLOTB;
extern syssramex *sramex;
extern u8 imageserial[12];

#define NOFAT_MSG "No FAT device detected. You may run device selector again."

static void updatePAD(u32 retrace)
{
    retraceCount = retrace;
	PAD_ScanPads();
#ifdef HW_RVL
	WPAD_ScanPads();
#endif
}

/* DEfinitions in sdsupp.h
#define DEV_NUM 	0
#define DEV_GCSDA 	1
#define DEV_GCSDB 	2
#define DEV_GCSDC 	3
#define DEV_GCODE 	4
#define DEV_WIISD 	5
#define DEV_WIIUSB	6

#define DEV_TOTAL 6

#define DEV_ND		0
#define DEV_AVAIL	1
#define DEV_MOUNTED 2
*/
u8 DEVICES [DEV_TOTAL+1];
bool have_sd;
u8 CUR_DEVICE; //Current device index

static void detect_devices(){

	int i = 0;
	DEVICES[DEV_NUM]=0;
	for (i=0;i<=DEV_TOTAL;i++) DEVICES[i]=0;

		if (__io_gcsda.startup()&&__io_gcsda.shutdown()){
			DEVICES[DEV_GCSDA] = DEV_AVAIL;
			DEVICES[DEV_NUM] +=1;
		}
		if (__io_gcsdb.startup()&&__io_gcsdb.shutdown()){
			DEVICES[DEV_GCSDB] = DEV_AVAIL;
			DEVICES[DEV_NUM] +=1;
		}
#ifdef HW_DOL
		if (__io_gcsd2.startup()&&__io_gcsd2.shutdown()){
			DEVICES[DEV_GCSDC] = DEV_AVAIL;
			DEVICES[DEV_NUM] +=1;
		}
		if (__io_gcode.startup()&&__io_gcode.shutdown()){
			DEVICES[DEV_GCODE] = DEV_AVAIL;
			DEVICES[DEV_NUM] +=1;
		}
#endif
#ifdef HW_RVL
		if (__io_wiisd.startup()&&__io_wiisd.shutdown()){
			DEVICES[DEV_WIISD] = DEV_AVAIL;
			DEVICES[DEV_NUM] +=1;
		}
		if (__io_usbstorage.startup())
		{
			if (__io_usbstorage.isInserted())
			{
				DEVICES[DEV_WIIUSB] = DEV_AVAIL;
				DEVICES[DEV_NUM] +=1;
				__io_usbstorage.shutdown();
			}
		}
#endif
}

/*
1 GC SD Gecko slot A
2 GC SD Gecko slot B
3 GC SD2SP2
4 GC Loader
5 Wii SD
6 Wii uSB
*/
static bool initFAT(int device)
{
	ShowAction("Mounting device...");
	char msg[128];
	if (DEVICES[device] != DEV_AVAIL){
		sprintf (msg, "Failed to mount device %d (device not available %d)", device, DEVICES[device]);
		WaitPrompt(msg);
		return false;
	}
	//sprintf (msg, "Mounting device... %d", device);
	//WaitPrompt(msg);
	switch (device)
	{
		case DEV_GCSDA:
			__io_gcsda.startup();
			if (!__io_gcsda.isInserted())
			{
				WaitPrompt("No SD Gecko inserted in SLOT A!");
				return false;
			}
			if (!fatMountSimple (SDGECKOA_PATH, &__io_gcsda))
			{
				WaitPrompt("Error Mounting SD fat!");
				return false;
			}
			DEVICES[DEV_GCSDA] = DEV_MOUNTED;
			sprintf(fatpath, "%s", SDGECKOA_PATH);
			break;
		
		case DEV_GCSDB:
			__io_gcsdb.startup();
			if (!__io_gcsdb.isInserted())
			{
				WaitPrompt("No SD card inserted in SLOT B!");
				return false;
			}
			if (!fatMountSimple (SDGECKOB_PATH, &__io_gcsdb))
			{
				WaitPrompt("Error Mounting SD fat!");
				return false;
			}
			DEVICES[DEV_GCSDB] = DEV_MOUNTED;
			sprintf(fatpath, "%s", SDGECKOB_PATH);
			break;
#ifdef HW_DOL
		case DEV_GCSDC:
			__io_gcsd2.startup();
			if (!__io_gcsd2.isInserted())
			{
				WaitPrompt("No SD card inserted in SD2SP2!");
				return false;
			}
			if (!fatMountSimple (SD2SP2_PATH, &__io_gcsd2))
			{
				WaitPrompt("Error Mounting SD fat!");
				return false;
			}
			DEVICES[DEV_GCSDC] = DEV_MOUNTED;
			sprintf(fatpath, "%s", SD2SP2_PATH);
			break;

		case DEV_GCODE:
			__io_gcode.startup();
			if (!__io_gcode.isInserted())
			{
				WaitPrompt("No SD card inserted in GCLoader!");
				return false;
			}
			if (!fatMountSimple (GCLOADER_PATH, &__io_gcode))
			{
				WaitPrompt("Error Mounting SD fat!");
				return false;
			}
			DEVICES[DEV_GCODE] = DEV_MOUNTED;
			sprintf(fatpath, "%s", GCLOADER_PATH);
			break;
#elif HW_RVL
		case DEV_WIISD:
			__io_wiisd.startup();
			if (!__io_wiisd.isInserted())
			{
				WaitPrompt("No SD card inserted!");
				return false;
			}
			if (!fatMountSimple (WIISD_PATH, &__io_wiisd))
			{
				WaitPrompt("Error Mounting SD fat!");
				return false;
			}
			DEVICES[DEV_WIISD] = DEV_MOUNTED;
			sprintf(fatpath, "%s", WIISD_PATH);
			break;
		
		case DEV_WIIUSB:
			__io_usbstorage.startup();
			if (!__io_usbstorage.isInserted())
			{
				WaitPrompt("No usb device inserted!");
				return false;
			}
			if (!fatMountSimple (WIIUSB_PATH, &__io_usbstorage))
			{
				WaitPrompt("Error Mounting SD fat!");
				return false;
			}
			DEVICES[DEV_WIIUSB] = DEV_MOUNTED;
			sprintf(fatpath, "%s", WIIUSB_PATH);
			break;
#endif

		default:
			WaitPrompt("Unwknown error mounting device");
			return false;
			break;
	}
	
	return true;
}

/*
// 1 for Internal SD, 0 for fat32 usb
static int initFAT2(int device)
{
	SD2SP2 = 0;
	ShowAction("Mounting device...");
#ifdef	HW_RVL
	if (device){//try SD card first
		__io_wiisd.startup();
		if (!__io_wiisd.isInserted())
		{
			ShowAction ("No SD card inserted! Trying USB storage.");
			sleep(2);
				__io_usbstorage.startup();
			if (!__io_usbstorage.isInserted())
			{
				WaitPrompt ("No USB device inserted either!");
				return 0;
			}
			else if (!fatMountSimple ("fat", &__io_usbstorage)){
				WaitPrompt("Error Mounting USB fat!");
				return 0;
			}
			return 1;//usb mounted
		}
		if (!fatMountSimple ("fat", &__io_wiisd))
		{
			WaitPrompt("Error Mounting SD fat!");
			return 0;
		}
	}else if (!device)//try USB first
	{
		__io_usbstorage.startup();
		if (!__io_usbstorage.isInserted())
		{
			ShowAction ("No usb device inserted! Trying internal SD.");
			sleep(2);
				__io_wiisd.startup();
			if (!__io_wiisd.isInserted())
			{
				WaitPrompt ("No SD card inserted either!");
				return 0;
			}
			else if (!fatMountSimple ("fat", &__io_wiisd)){
				WaitPrompt("Error Mounting SD fat!");
				return 0;
			}
			return 1;//SD mounted
		}
		if (!fatMountSimple ("fat", &__io_usbstorage))
		{
			WaitPrompt("Error Mounting USB fat!");
			return 0;
		}
	}
#else
	int i =0;
	s32 ret;
	if (device)
	{//Memcard in SLOT B, SD gecko in SLOT A
		//This will ensure SD gecko is recognized if inserted or changed to another slot after GCMM is executed
		for(i=0;i<10;i++){
			ret = CARD_Probe(CARD_SLOTA);
			if (ret == CARD_ERROR_WRONGDEVICE)
				break;
		}
		__io_gcsda.startup();
		if (!__io_gcsda.isInserted())
		{
			WaitPrompt ("No SD Gecko inserted! Insert it in slot A and restart");
			return 0;
		}
		if (!fatMountSimple ("fat", &__io_gcsda))
		{
			WaitPrompt("Error Mounting SD fat!");
			return 0;
		}
	}else //Memcard in SLOT A, SD gecko in SLOT B
	{
		//This will ensure SD gecko is recognized if inserted or changed to another slot after GCMM is executed
		for(i=0;i<10;i++){
			ret = CARD_Probe(CARD_SLOTB);
			if (ret == CARD_ERROR_WRONGDEVICE)
				break;
		}	
		__io_gcsdb.startup();
		if (!__io_gcsdb.isInserted())
		{
			WaitPrompt ("No SD Gecko inserted! Insert it in slot B and restart");
			return 0;
		}
		if (!fatMountSimple ("fat", &__io_gcsdb))
		{
			WaitPrompt("Error Mounting SD fat!");
			return 0;
		}
	}
#endif
	return 1;
}
*/
void deinitFAT()
{
	//First unmount all the devs...
	fatUnmount (fatpath);
	fatpath[0]='/0';
	have_sd = 0;
	CUR_DEVICE = 0;
	//...and then shutdown em!
	if (DEVICES[DEV_GCSDA] == DEV_MOUNTED){
		__io_gcsda.shutdown();
		DEVICES[DEV_GCSDA]=DEV_AVAIL;
	}
	if (DEVICES[DEV_GCSDB] == DEV_MOUNTED){
		__io_gcsdb.shutdown();
		DEVICES[DEV_GCSDB]=DEV_AVAIL;
	}
#ifdef HW_DOL
	if (DEVICES[DEV_GCSDC] == DEV_MOUNTED){
		__io_gcsd2.shutdown();
		DEVICES[DEV_GCSDC]=DEV_AVAIL;
	}
	if (DEVICES[DEV_GCODE] == DEV_MOUNTED){
		__io_gcode.shutdown();
		DEVICES[DEV_GCODE]=DEV_AVAIL;
	}
#endif
#ifdef	HW_RVL
	if (DEVICES[DEV_WIISD] == DEV_MOUNTED){
		__io_wiisd.shutdown();
		DEVICES[DEV_WIISD]=DEV_AVAIL;
	}
	if (DEVICES[DEV_WIIUSB] == DEV_MOUNTED){
		__io_usbstorage.shutdown();
		DEVICES[DEV_WIIUSB]=DEV_AVAIL;
	}
#endif
}
/*
void deinitFAT2()
{
	//First unmount all the devs...
	fatUnmount ("fat");
	//...and then shutdown em!
#ifdef	HW_RVL
	__io_wiisd.shutdown();
	__io_usbstorage.shutdown();
#else
	if (SD2SP2)
	{
		__io_gcsd2.shutdown();
	}
	else
	{
		if(MEM_CARD)
			__io_gcsda.shutdown();
		if(!MEM_CARD)
			__io_gcsdb.shutdown();
	}
#endif
}
*/
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
	char buffer[256], text[64];
	
	displaypath = 0;

	clearRightPane();
	DrawText(386,130,"B a c k u p   M o d e");
	DrawText(386,134,"_____________________");
	writeStatusBar("Reading memory card... ", "");
	/*** Get the directory listing from the memory card ***/
	memitems = CardGetDirectory (MEM_CARD);

	setfontsize (14);
	writeStatusBar("Pick a file using UP or DOWN ", "Press A to backup savegame") ;
#ifdef HW_RVL
	DrawText(40, 50, "Press R/1 to backup ALL savegames");
#else
	DrawText(40, 50, "Press R to backup ALL savegames");
#endif

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
		else if(doall)
		{
			doall = WaitPromptChoice("Are you sure you want to backup all files?", "No", "Yes");
			if (doall)
			{
				//Backup All files
				for ( selected = 0; selected < memitems; selected++ ) {
					/*** Backup files ***/
					sprintf(buffer, "[%d/%d] Reading from MC slot %s", selected+1, memitems, (MEM_CARD) ? "B" : "A");
					ShowAction(buffer);
					bytestodo = CardReadFile(MEM_CARD, selected);
					if (bytestodo)
					{
						sprintf(buffer, "[%d/%d] Saving to FAT device", selected+1, memitems);
						ShowAction(buffer);
						if (!SDSaveMCImage())
						{
							strncpy(text, (char*)filelist[selected], 32);
							text[32]='\0';
							sprintf(buffer, "Error during backup (%s). Continue?", text);
							doall = WaitPromptChoice(buffer, "Yes", "No");
							if (doall)
							{
								WaitPrompt ("Backup action cancelled due to error!");
								return;
							}
						}

					}
					else
					{
						WaitPrompt ("Error reading MC file");
						return;
					}
				}

				WaitPrompt("Full card backup done!");
				return;
			}

		}
		else
		{
			/*** Backup the file ***/
			ShowAction ("Reading File From MC SLOT B");
			bytestodo = CardReadFile (MEM_CARD, selected);
			if (bytestodo)
			{
				ShowAction ("Saving to FAT device");
				if (SDSaveMCImage())
				{
					WaitPrompt ("Backup complete");
					return;
				}
				else
				{
					WaitPrompt ("Backup failed");
					return;
				}
			}
			else
			{
				WaitPrompt ("Error reading MC file");
				return;
			}

		}
	}
    return;
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
	
	displaypath = 0;

	clearRightPane();
	DrawText(386,130," B a c k u p   A l l ");
	DrawText(386,134,"_____________________");

	setfontsize (14);
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
				sprintf(buffer, "[%d/%d] Saving to FAT device", selected+1, memitems);
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
	char buffer[256], text[64];
	int inProgress = 1;
	
	displaypath = 1;

	clearRightPane();
	DrawText(380,130,"R e s t o r e  M o d e");
	DrawText(380,134,"______________________");
	writeStatusBar("Reading files... ", "");
	
	//Dragonbane: Curr Folder to default
	
	sprintf((char*)currFolder, "MCBACKUP");

	files = SDGetFileList (1);

	setfontsize (14);
#ifdef HW_RVL
	DrawText(40, 50, "Press R/1 to restore ALL savegames");
#else
	DrawText(40, 50, "Press R to restore ALL savegames");
#endif

	if (!files)
	{
		WaitPrompt ("No saved games in FAT device to restore !");
	}
    else
	{
		while(inProgress == 1)
		{
			writeStatusBar("Pick a file using UP or DOWN", "Press A to restore to Memory Card ") ;
			
			//It will wait here until user selected a file
			selected = ShowSelector (1);

			if (cancel)
			{
				if (strcmp((char*)currFolder, "MCBACKUP") == 0)
				{
					WaitPrompt ("Restore action cancelled !");
					return;
				}
				else
				{
					//Go back one folder			
					char* pos = strrchr( (char*)currFolder, '/' );

					currFolder[(pos-(char*)currFolder)] = 0; 

					files = SDGetFileList (1);
					
					cancel = 0;
					offsetchanged = true;
					usleep(300000);
					continue;
				}
			}
			else if (doall)
			{
				doall = WaitPromptChoice("Are you sure you want to restore -ALL- files from this folder?", "Yes", "No");
				if (!doall)
				{
					//Restore All files (from current folder)	
					for ( selected = folderCount; selected < files; selected++ ) {
						/*** Restore files ***/
						sprintf(buffer, "[%d/%d] Reading from FAT device", selected+1, files);
						ShowAction(buffer);
						if (SDLoadMCImage ((char*)filelist[selected]))
						{
							sprintf(buffer, "[%d/%d] Saving to MC slot %s", selected+1, files, (MEM_CARD) ? "B" : "A");
							ShowAction(buffer);
							if (!CardWriteFile (MEM_CARD))
							{
								strncpy(text, (char*)filelist[selected], 32);
								text[32]='\0';
								sprintf(buffer, "Error during restore (%s). Continue?", text);
								doall = WaitPromptChoice(buffer, "Yes", "No");
								if (doall)
								{
									WaitPrompt ("Restore action cancelled due to error!");
									return;
								}
							}
						}
						else
						{
							WaitPrompt ("Error reading image");
							return;
						}
					}

					WaitPrompt("Full card restore done!");
					return;
				}
				else
				{
					return;
				}
			}
			else
			{
				//Check if selection is folder
				char folder[1024];
				sprintf (folder, "%s:/%s/%s",fatpath, currFolder, (char*)filelist[selected]);
		
				if(isdir_sd(folder) == 1)
				{
					//Enter folder
					sprintf((char*)currFolder, "%s/%s", currFolder, (char*)filelist[selected]);

					files = SDGetFileList (1);
					if (!files)
					{
						WaitPrompt("Folder is empty!");
						
						//Go back again			
						char* pos = strrchr( (char*)currFolder, '/' );

						currFolder[(pos-(char*)currFolder)] = 0; 

						files = SDGetFileList (1);
					}
					
					offsetchanged = true;
					usleep(300000);
					continue;
				}
				else //Selection is a file
				{
					ShowAction ("Reading from FAT device");
					if (SDLoadMCImage ((char*)filelist[selected]))
					{
						ShowAction ("Updating Memory Card");
						if (CardWriteFile (MEM_CARD))
						{
							WaitPrompt ("Restore Complete");
							return;
						}
						else
						{
							WaitPrompt ("Error during restore");
							return;
						}
					}
					else
					{
						WaitPrompt ("Error reading image");
						return;
					}
				}
			}
		}
	}
    return;
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
	
	displaypath = 0;
	
	clearRightPane();

	DrawText(394,224,"___________________");	
	DrawText(394,248,"R A W   B a c k u p");
	DrawText(454,268,"M o d e");
	DrawText(394,272,"___________________");
	
	setfontsize (14);	
	writeStatusBar("Reading memory card... ", "");

	if (BackupRawImage(MEM_CARD, &writen) == 1)
	{
		sprintf(msg, "Backup complete! Wrote %d bytes to FAT device",writen);
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
	int inProgress = 1;

	displaypath = 1;

	clearRightPane();
	DrawText(380,130,"R A W   R e s t o r e");
	DrawText(450,150,"M o d e");
	DrawText(380,154,"_____________________");

	writeStatusBar("Reading files... ", "");
	
	//Curr Folder to default
	sprintf((char*)currFolder, "MCBACKUP");
	
	files = SDGetFileList (0);

	setfontsize (14);
	writeStatusBar("Pick a file using UP or DOWN", "Press A to restore to Memory Card ");

	if (!files)
	{
		WaitPrompt ("No raw backups in FAT device to restore !");
	}else
	{
		while(inProgress == 1)
		{
			setfontsize (14);
			writeStatusBar("Pick a file using UP or DOWN", "Press A to restore to Memory Card ") ;
			
			//It will wait here until user selected a file
			selected = ShowSelector (0);

			if (cancel)
			{
				if (strcmp((char*)currFolder, "MCBACKUP") == 0)
				{
					WaitPrompt ("Restore action cancelled !");
					return;
				}
				else
				{
					//Go back one folder			
					char* pos = strrchr( (char*)currFolder, '/' );

					currFolder[(pos-(char*)currFolder)] = 0; 

					files = SDGetFileList (0);
					
					cancel = 0;
					offsetchanged = true;
					usleep(300000);
					continue;
				}
			}
			else
			{
				//Check if selection is folder
				char folder[1024];
				sprintf (folder, "%s:/%s/%s",fatpath, currFolder, (char*)filelist[selected]);
		
				if(isdir_sd(folder) == 1)
				{
					//Enter folder
					sprintf((char*)currFolder, "%s/%s", currFolder, (char*)filelist[selected]);

					files = SDGetFileList (0);
					if (!files)
					{
						WaitPrompt("Folder is empty!");
						
						//Go back again			
						char* pos = strrchr( (char*)currFolder, '/' );

						currFolder[(pos-(char*)currFolder)] = 0; 

						files = SDGetFileList (0);
					}
					
					offsetchanged = true;
					usleep(300000);
					continue;
				}
				else //Selection is a file
				{
#ifdef FLASHIDCHECK
					//Now imageserial and sramex.flash_id[MEM_CARD] variables should hold the proper information
					for (i=0;i<12;i++){
						if (imageserial[i] != sramex->flash_id[MEM_CARD][i]){
							WaitPrompt ("Card and image flash ID don't match !");
							return;
						}
					}
#endif
					ShowAction ("Reading from FAT device...");
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
	}
	return;
}

u8 skip_selector = 1;
int device_select()
{
	u32 p, ph;
#ifdef HW_RVL
	u32 wp, wph;
#endif
	
	int x = 170;
	int y = 280;
	int x_text = x+15;
	int y_text = y+20;
	int selected = 0;
	u8 draw = 1;
	setfontsize(14);
	setfontcolour(COL_FONT);

	int i = 0;
	u8 selected2device[4] = {0, 0, 0, 0};
	//If there are no fat devices skip
	if (!DEVICES[DEV_NUM]){
		WaitPrompt (NOFAT_MSG);
		return 0;
	}else if (DEVICES[DEV_NUM] == 1 && skip_selector ){ //Only one device was detected, so mount it, but only on first boot
		skip_selector = 0;
		for (i=1;i<=DEV_TOTAL;i++)
		{
			if (DEVICES[i])
				return i;
		}
	}else{
	skip_selector = 0;
	//Selector screen
		while(1)
		{
			//Draw device selector
			if (draw){
				writeStatusBar("Press A to select","");
				draw = 0;
				DrawBoxFilled(x, y, x+290, y+90, COL_BG1);
				y_text= y+20;
				x_text = x+15;
				DrawText(x_text, y_text, "Please select your device:");
				y_text+=20;
				x_text += 24;
		#ifdef HW_RVL
				if(DEVICES[DEV_WIISD]){
					DrawText(x_text, y_text, "Wii Front SD slot");
					y_text+=15;
					for (i=0;i<4;i++)
					{
						if (!selected2device[i])
						{
							selected2device[i]=DEV_WIISD;
							break;
						}
					}
				}
				if(DEVICES[DEV_WIIUSB]){
					DrawText(x_text, y_text, "USB storage device");
					y_text+=15;
					for (i=0;i<4;i++)
					{
						if (!selected2device[i])
						{
							selected2device[i]=DEV_WIIUSB;
							break;
						}
					}
				}
				if(DEVICES[DEV_GCSDA]){
					DrawText(x_text, y_text, "Slot A SD Gecko");
					y_text+=15;
					for (i=0;i<4;i++)
					{
						if (!selected2device[i])
						{
							selected2device[i]=DEV_GCSDA;
							break;
						}
					}
				}
				if(DEVICES[DEV_GCSDB]){
					DrawText(x_text, y_text, "Slot B SD Gecko");
					y_text=15;
					for (i=0;i<4;i++)
					{
						if (!selected2device[i])
						{
							selected2device[i]=DEV_GCSDB;
							break;
						}
					}
				}
		#else
				if(DEVICES[DEV_GCSDC]){
					DrawText(x_text, y_text, "SD2SP2");
					y_text+=15;
					for (i=0;i<4;i++)
					{
						if (!selected2device[i])
						{
							selected2device[i]=DEV_GCSDC;
							break;
						}
					}
				}
				if(DEVICES[DEV_GCSDA]){
					DrawText(x_text, y_text, "Slot A SD Gecko");
					y_text+=15;
					for (i=0;i<4;i++)
					{
						if (!selected2device[i])
						{
							selected2device[i]=DEV_GCSDA;
							break;
						}
					}
				}
				if(DEVICES[DEV_GCSDB]){
					DrawText(x_text, y_text, "Slot B SD Gecko");
					y_text+=15;
					for (i=0;i<4;i++)
					{
						if (!selected2device[i])
						{
							selected2device[i]=DEV_GCSDB;
							break;
						}
					}
				}
				if(DEVICES[DEV_GCODE]){
					DrawText(x_text, y_text, "GC Loader");
					y_text+=15;
					for (i=0;i<4;i++)
					{
						if (!selected2device[i])
						{
							selected2device[i]=DEV_GCODE;
							break;
						}
					}
				}
		#endif
				
				//Draw cursor
				y_text= y+40;
				x_text = x+20;
				switch (selected)
				{
					case 0:
						DrawText(x_text, y_text, ">>");
						break;
					case 1:
						DrawText(x_text, y_text+15, ">>");
						break;
					case 2:
						DrawText(x_text, y_text+15+15, ">>");
						break;
					case 3:
						DrawText(x_text, y_text+15+15+15, ">>");
						break;
					default:
						break;
				}
			}//end if (draw)
			
			p = PAD_ButtonsDown (0);
	#ifdef HW_RVL
			wp = WPAD_ButtonsDown (0);
	#endif
			
			if (p & PAD_BUTTON_A
		#ifdef HW_RVL
				|| wp & WPAD_BUTTON_A
		#endif
			)
			{
				//Do something
				return selected2device[selected];
			}
			if (p & PAD_BUTTON_DOWN
		#ifdef HW_RVL
				|| wp & WPAD_BUTTON_DOWN
		#endif
			)
			{
				selected +=1;
				if (selected >= DEVICES[DEV_NUM]) selected = 0;
				draw = 1;
			}
			if (p & PAD_BUTTON_UP
		#ifdef HW_RVL
				|| wp & WPAD_BUTTON_UP
		#endif
			)
			{
				selected -=1;
				if (selected < 0) selected = DEVICES[DEV_NUM]-1;
				draw = 1;
			}
			//char msg[256];
			//sprintf(msg, "Selected device: %d Total %d DN%d SDA%d SDB%d SDC%d GL%d WSD%d WU%d", selected, DEVICES[DEV_NUM], DEVICES[1],DEVICES[2],DEVICES[3],DEVICES[4],DEVICES[5],DEVICES[6] );
			/*
#define DEV_NUM 	0
#define DEV_GCSDA 	1
#define DEV_GCSDB 	2
#define DEV_GCSDC 	3
#define DEV_GCODE 	4
#define DEV_WIISD 	5
#define DEV_WIIUSB	6
			*/
			//writeStatusBar(msg, "");
			ShowScreen();
		}//end while(1)
	}
	return 0;
}

/****************************************************************************
* Main
****************************************************************************/
int main (int argc, char *argv[])
{

	have_sd = false;
	CUR_DEVICE = 0;

#ifdef HW_DOL
	int *psoid = (int *) 0x80001800;
	void (*PSOReload) () = (void (*)()) 0x80001800;
#endif

	Initialise ();	/*** Start video ***/
	FT_Init ();		/*** Start FreeType ***/

#ifdef HW_RVL
	initialise_power();
	//have_sd = initFAT(WaitPromptChoice ("Use internal SD or FAT32 USB device?", "USB", "SD"));
#endif

	detect_devices();
	//Check for command line 
	if (argc > 1)
	{
		if (!strcasecmp(argv[1], "ask"))
		{
			//Run device selection screen
			skip_selector = 0;
			selector_flag = 1;
			ClearScreen();
			ShowScreen();
			
			CUR_DEVICE = device_select();
		}
		else if (!strcasecmp(argv[1], "sdgecko"))
		{
			if (DEVICES[DEV_GCSDA] == DEV_AVAIL && !(DEVICES[DEV_GCSDB] == DEV_AVAIL))
			{
				CUR_DEVICE = DEV_GCSDA;
			}
			else if (DEVICES[DEV_GCSDB] == DEV_AVAIL && !(DEVICES[DEV_GCSDA] == DEV_AVAIL))
			{
				CUR_DEVICE = DEV_GCSDB;
			}
			else //Run selector screen if there are both an SDGecko in slot A and Slot B...you won't be able to do anything with that setup though
			{
				//Run device selection screen
				selector_flag = 1;
				ClearScreen();
				ShowScreen();
				
				CUR_DEVICE = device_select();
			}
		}
		else if (!strcasecmp(argv[1], "sdgeckoA"))
		{
			if (DEVICES[DEV_GCSDA] == DEV_AVAIL)
			{
				CUR_DEVICE = DEV_GCSDA;
			}
		}
		else if (!strcasecmp(argv[1], "sdgeckoB"))
		{
			if (DEVICES[DEV_GCSDB] == DEV_AVAIL)
			{
				CUR_DEVICE = DEV_GCSDB;
			}
		}
#ifdef HW_DOL
		else if (!strcasecmp(argv[1], "gcloader"))
		{
			if (DEVICES[DEV_GCODE] == DEV_AVAIL)
			{
				CUR_DEVICE = DEV_GCODE;
			}
		}
		else if (!strcasecmp(argv[1], "sd2sp2"))
		{
			if (DEVICES[DEV_GCSDC] == DEV_AVAIL)
			{
				CUR_DEVICE = DEV_GCSDC;
			}
		}
#endif
#ifdef HW_RVL
		else if (!strcasecmp(argv[1], "wiisd"))
		{
			if (DEVICES[DEV_WIISD] == DEV_AVAIL)
			{
				CUR_DEVICE = DEV_WIISD;
			}
		}
		else if (!strcasecmp(argv[1], "wiiusb"))
		{
			if (DEVICES[DEV_WIIUSB] == DEV_AVAIL)
			{
				CUR_DEVICE = DEV_WIIUSB;
			}
		}
#endif
		//If command line failed to set a CUR_DEVICE, run selector screen
		if (CUR_DEVICE == 0)
		{
			selector_flag = 1;
			ClearScreen();
			ShowScreen();
		
			CUR_DEVICE = device_select();
		}
		
	}
	else
	{
		//Run device selection screen (will be skipped at first run if there's only one available device)
		selector_flag = 1;
		ClearScreen();
		ShowScreen();
		
		CUR_DEVICE = device_select();
	}
	
	if (CUR_DEVICE) have_sd = initFAT(CUR_DEVICE);
	
	//Set correct memory card slot when SD gecko is selected device
	if (CUR_DEVICE == DEV_GCSDB) MEM_CARD = 0;
	else if (CUR_DEVICE == DEV_GCSDA) MEM_CARD = 1;

/*
	u32 p = PAD_ButtonsHeld(0);

	//First try for a SD2SP2 device
	SD2SP2 = 1;
	u8 ask_device = 0;
	if (argc > 1)
	{
		if (!strcmp(argv[1], "ask"))
		{
			ask_device = 1;
		}
		else if (!strcmp(argv[1], "sdgecko"))
		{
			SD2SP2 = 0;
			ask_device = 2;
		}
	}
	//If Z trigger or A button is held in startup, ask for device to use, regardless of bootup
	if ( p & PAD_BUTTON_A || p & PAD_TRIGGER_Z)
	{
		SD2SP2 = 1;
		ask_device = 1;
	}
	
	if (SD2SP2)
	{
		__io_gcsd2.startup();
		if (__io_gcsd2.isInserted() && ask_device != 2)
		{
			if (ask_device == 1 )
			{
				writeStatusBar("Use SD2SP2 or SD Gecko?", "B = SD Gecko   :   A = SD2SP2");
				WaitRelease();
				SD2SP2 = WaitPromptChoice ("Use SD2SP2 or SD Gecko?", "SD Gecko", "SD2SP2");
			}
			if(SD2SP2)
			{
				if (!fatMountSimple ("fat", &__io_gcsd2))
				{
					WaitPrompt("SD2SP2 detected, but Error mounting fat! Trying SD Gecko");
					SD2SP2 = 0;
					
				}else
				{
					writeStatusBar("SD2SP2 mounted!", ""); //Also refreshes screen and makes version string appear
					//WaitPrompt("SD2SP2 mounted!");
					have_sd = 1;
				}
			}
		}
		else
		{
			//WaitPrompt("SD2SP2 not detected!");
			SD2SP2 = 0;
		}
	}

	if (!SD2SP2) //Ask where SD gecko is located if no SD2SP2 is detected
	{
		//Returns 1 (memory card in slot B, sd gecko in slot A) if A button was pressed and 0 if B button was pressed
		MEM_CARD = WaitPromptChoice ("Please select the slot where SD Gecko is inserted", "SLOT B", "SLOT A");
		have_sd = initFAT(MEM_CARD);
	}
#endif
*/
	selector_flag = 0;
	skip_selector = 0;
	for (;;)
	{
		/*** Select Mode ***/
		ClearScreen();
		setfontsize (FONT_SIZE);
		freecardbuf();
		cancel = 0;/******a global value to track action aborted by user pressing button B********/
		doall = 0;
		mode = SelectMode ();

		//Allow memory card selection for some devices
		if ((mode != 500 ) && (mode != 100) && (mode != 600) && (mode != 1000 ) && (CUR_DEVICE==DEV_WIISD || CUR_DEVICE==DEV_WIIUSB || CUR_DEVICE==DEV_GCSDC || CUR_DEVICE==DEV_GCODE)	){
			if (WaitPromptChoice ("Please select a memory card slot", "Slot B", "Slot A") == 1)
			{
				MEM_CARD = CARD_SLOTA;
			}else
			{
				MEM_CARD = CARD_SLOTB;
			}
		}

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
			else WaitPrompt(NOFAT_MSG);
			break;
		case 400 : //User wants to restore
			if (have_sd) SD_RestoreMode();
			else WaitPrompt(NOFAT_MSG);
			break;
		case 500 ://exit
			ShowAction ("Exiting...");
#ifdef HW_RVL
			deinitFAT();
			//if there's a loader stub load it, if not return to wii menu.
			if (!!*(u32*)0x80001800){ShowAction ("Exiting...Loader"); exit(1);}
			else {ShowAction ("Exiting...SysMenu");SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);}
#else
			if (psoid[0] == PSOSDLOADID){
				deinitFAT();
				ShowAction ("Exiting...Loader");
				PSOReload ();
			}
			if (have_sd){
				char exitdol[64];
				sprintf(exitdol, "%s:/autoexec.dol",fatpath);
				FILE *fp = fopen(exitdol, "rb");
				if (fp) {
					ShowAction ("Exiting...autoexec.dol");
					fseek(fp, 0, SEEK_END);
					int size = ftell(fp);
					fseek(fp, 0, SEEK_SET);
					if ((size > 0) && (size < (AR_GetSize() - (64*1024)))) {
						u8 *dol = (u8*) memalign(32, size);
						if (dol) {
							fread(dol, 1, size, fp);
							fclose(fp);
							deinitFAT();
							DOLtoARAM(dol, 0, NULL);
						}
						WaitPrompt ("Something went wrong with autoexec.dol. Press A to reboot.");
						//We shouldn't reach this point
						if (dol != NULL) free(dol);
					}
				fclose(fp);//We shouldn't reach here either
				}
				char msg[64];
				sprintf(msg, "Couldn't open %s. Press A to reboot.", exitdol);
				WaitPrompt (msg);
			}
			deinitFAT();
			ShowAction ("Exiting...Reboot");
			SYS_ResetSystem(SYS_HOTRESET, 0, 0);
#endif
			break; //PSO_Reload
		case 600 : //User wants to backup full card
			/*
			if (have_sd) SD_BackupModeAllFiles();
			else WaitPrompt("Reboot aplication with an SD card");
			*/
			break;
		case 700 : //Raw backup mode
			if (have_sd)
			{
				SD_RawBackupMode();
			}else
			{
				WaitPrompt(NOFAT_MSG);
			}
			break;
		case 800 : //Raw restore mode
			//These two lines are a work around for the second call of CARD_Probe to detect a newly inserted memory card
			CARD_Probe(MEM_CARD);
			VIDEO_WaitVSync ();
			if (CARD_Probe(MEM_CARD) > 0)
			{
				if (have_sd) SD_RawRestoreMode();
				else WaitPrompt(NOFAT_MSG);

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
				clearRightPane();
				DrawText(390,224,"____________________");
				DrawText(390,248,"F o r m a t  C a r d");
				DrawText(460,268,"M o d e");
				DrawText(390,272,"____________________");				
				MC_FormatMode(MEM_CARD);

			}else if (MEM_CARD)
			{
				WaitPrompt("Please insert a memory card in slot B");
			}else
			{
				WaitPrompt("Please insert a memory card in slot A");
			}
			break;
		case 1000: //Device select mode
			deinitFAT();
			WaitPrompt("Device unmounted. Insert/Remove any devices now.");
			selector_flag = 1;
			ShowScreen();
			ClearScreen();
			detect_devices();
			CUR_DEVICE = device_select();
			if (CUR_DEVICE) have_sd = initFAT(CUR_DEVICE);
			//Set correct memory card slot when SD gecko is selected device
			if (CUR_DEVICE == DEV_GCSDB) MEM_CARD = 0;
			else if (CUR_DEVICE == DEV_GCSDA) MEM_CARD = 1;
			selector_flag = 0;
			break;
		}

		offsetchanged = true;
	}
	while (1);
	//Should never reach this point...
#ifdef HW_RVL
	//if there's a loader stub load it, if not return to wii menu.
	if (!!*(u32*)0x80001800) exit(1);
	else SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
#else
	SYS_ResetSystem(SYS_HOTRESET, 0, 0);
#endif
	return 0;
}
