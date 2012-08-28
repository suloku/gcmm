/****************************************************************************
* GCI Memory Card stuff
*
* This is an attempt at compatibility.
****************************************************************************/
#ifndef __GCIHDR_
#define __GCIHDR_

#include <gccore.h>

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
