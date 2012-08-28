/*** 2D Video Globals ***/
extern GXRModeObj *vmode; /*** Graphics Mode Object ***/
extern u32 *xfb[2]; /*** Framebuffers ***/
extern int whichfb; /*** Frame buffer toggle ***/
extern u8 bitmapfile[];
u32 ShowBMP (u8 *bmpfile);
void ShowBanner ();
void ShowIcon   ();
void ClearScreen();
