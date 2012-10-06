/*** 2D Video Globals ***/
extern GXRModeObj *vmode; /*** Graphics Mode Object ***/
extern u32 *xfb[2]; /*** Framebuffers ***/
extern int whichfb; /*** Frame buffer toggle ***/
extern u8 bitmapfile[];
u32 CvtRGB(u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2);
u32 ShowBMP (u8 *bmpfile);
void ShowBanner ();
void ShowIcon   ();
void ClearScreen();
