/****************************************************************************
* GCI Memory Card stuff
*
* This is an attempt at compatibility.
****************************************************************************/
#ifndef __GCIHDR_
#define __GCIHDR_

#include <gccore.h>

// If STATUSOGC defined uses CARD_GetStatus and CARD_SetStatus
// Default (undefined) uses __card_getstatusex and __card_setstatusex
// Diference: when restoring a savegame with the ex functions original time, copy counter, etc are preserved
// Also, libogc creates card entries with time since 1970 and gamecube uses time since 2000... thus libogc adds 30 years!

//#define STATUSOGC

#define SDCARD_GetBannerFmt(banner_fmt)		((banner_fmt) & 0x03)
#define SDCARD_GetIconFmt(icon_fmt,n)		(((icon_fmt)>>(2*(n))) & 0x03)
#define SDCARD_GetIconSpeed(icon_speed,n)	(((icon_speed)>>(2*(n))) & 0x03)
#define SDCARD_GetIconAnim(banner_fmt)		((banner_fmt) & 0x04)

typedef struct {
	u8 gamecode[4];
	u8 company[2];
	u8 reserved01;	/*** Always 0xff ***/
	u8 banner_fmt;
	u8 filename[CARD_FILENAMELEN];
	u32 time;
	u32 icon_addr;  /*** Offset to banner/icon data ***/
	u16 icon_fmt;
	u16 icon_speed;
	u8 unknown1;	/*** Permission key ***/
	u8 unknown2;	/*** Copy Counter ***/
	u16 index;		/*** Start block of savegame in memory card (Ignore - and throw away) ***/
	u16 filesize8;	/*** File size / 8192 ***/
	u16 reserved02;	/*** Always 0xffff ***/
	u32 comment_addr;
} __attribute__((__packed__)) GCI;

#define MCDATAOFFSET 64

#endif
