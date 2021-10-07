/****************************************************************************
* GCI Memory Card stuff
*
* This is an attempt at compatibility.
****************************************************************************/
#ifndef __GCIHDR_
#define __GCIHDR_

#include <gccore.h>
#include <ogc/libversion.h>
#if (_V_MAJOR_ <= 2) && (_V_MINOR_ <= 2)
typedef struct _card_direntry {
	u8 gamecode[4];
	u8 company[2];
	u8 pad_00;
	u8 banner_fmt;
	u8 filename[CARD_FILENAMELEN];
	u32 last_modified;
	u32 icon_addr;
	u16 icon_fmt;
	u16 icon_speed;
	u8 permission;
	u8 copy_times;
	u16 block;
	u16 length;
	u16 pad_01;
	u32 comment_addr;
} card_direntry;
#else
#include <ogc/card.h>
#endif
// If STATUSOGC defined uses CARD_GetStatus and CARD_SetStatus
// Default (undefined) uses __card_getstatusex and __card_setstatusex
// Diference: when restoring a savegame with the ex functions original time, copy counter, etc are preserved
// Also, libogc creates card entries with time since 1970 and gamecube uses time since 2000... thus libogc adds 30 years!

//#define STATUSOGC

#define SDCARD_GetBannerFmt(banner_fmt)		((banner_fmt) & 0x03)
#define SDCARD_GetIconFmt(icon_fmt,n)		(((icon_fmt)>>(2*(n))) & 0x03)
#define SDCARD_GetIconSpeed(icon_speed,n)	(((icon_speed)>>(2*(n))) & 0x03)
#define SDCARD_GetIconSpeedBounce(icon_speed,n,i)	((n) < (i) ? (((icon_speed)>>(2*(n))) & 0x03) : (((icon_speed)>>(2*((i*2)-2-n)) & 0x03)) )
#define SDCARD_GetIconAnim(banner_fmt)		((banner_fmt) & 0x04)

#define MCDATAOFFSET 64

#endif
