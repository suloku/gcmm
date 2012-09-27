/***
	All of the code in bannerload.c is thanks in large part to the Dolphin Wii/GcN Emulator
	Project. These functions serve to decode the RGB5A3 format that GCN Banner/Icon Files use
	and create an array of BGR values, used later by the ShowBanner function.
	ShowBanner is a simple modification of ShowBMP.
	
	Files from Dolphin Project that were used as the basis for this code:
	BannerLoaderGC.cpp, ColorUtil.cpp, and TextureDecoder.cpp
	(http://dolphin-emu.googlecode.com/svn/trunk/)
***/

//this is the background color in RGB for alpha blending
//values are BGR
//Blue color 5 10 140
//Purple color 55 15 95
#define BLUECOL ((140<<16)|(10<<8)|5)
#define PURPLECOL ((95<<16)|(15<<8)|55)
//This is the blueish background color
//#define BLENDCOL ((211<<16)|(174<<8)|84)

//16-bit banner data function
void bannerloadRGB(u16*);
//8-bit banner data function
void bannerloadCI(u8*, u16*);
//16-bit icon data function
void iconloadRGB(u16*);
//8-bit icon data function
void iconloadCI(u8*, u16*);
