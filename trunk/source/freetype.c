/****************************************************************************
* libFreeType
*
* Needed to show the user what's the hell's going on !!
* These functions are generic, and do not contain any Memory Card specific
* routines.
****************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "freetype.h"
#include "gci.h"
#include "mcard.h"
#include "sdsupp.h"
#include "bitmap.h"

#ifdef HW_RVL
#include <wiiuse/wpad.h>
#endif

#define MCDATAOFFSET 64
#define FONT_SIZE 16 //pixels

/*** Globals ***/
FT_Library ftlibrary;
FT_Face face;
FT_GlyphSlot slot;
FT_UInt glyph_index;
extern int cancel;
extern int mode;

static int fonthi, fontlo;

/*** The actual TrueType Font ***/
extern char fontface[];		/*** From fontface.s ***/
extern int fontsize;			/*** From fontface.s ***/

/*** From video subsystem ***/
extern int screenheight;
extern u32 *xfb[2];
extern int whichfb;
extern GXRModeObj *vmode;

/*** File listings ***/
extern card_dir CardList[];
extern u8 filelist[1024][1024];
extern u32 maxfile;
extern GCI gci;
#define MAXFILEBUFFER (1024 * 2048)
extern u8 FileBuffer[MAXFILEBUFFER] ATTRIBUTE_ALIGN (32);
extern u8 CommentBuffer[64] ATTRIBUTE_ALIGN (32);

#define PAGESIZE 18


#ifdef HW_RVL
// POWER BUTTON
bool power=false;

void PowerOff()
{
	//LWP_MutexDestroy(m_audioMutex);
	SYS_ResetSystem(SYS_POWEROFF, 0, 0);
	exit(0);
}

static void set_power_var()
{
	power = true;
}
static void set_wiimote_power_var(s32 chan)
{
	power = true;
}
void initialise_power()
{
	SYS_SetPowerCallback(set_power_var);
	WPAD_SetPowerButtonCallback(set_wiimote_power_var);
}
// END POWER BUTTON
#endif

/****************************************************************************
* FT_Init
*
* Initialise freetype library
****************************************************************************/
int FT_Init ()
{

	int err;

	err = FT_Init_FreeType (&ftlibrary);
	if (err)
		return 1;

	err =
	    FT_New_Memory_Face (ftlibrary, (FT_Byte *) fontface, fontsize, 0, &face);
	if (err)
		return 1;

	setfontsize (FONT_SIZE);
	setfontcolour (0xff, 0xff, 0xff);

	slot = face->glyph;

	return 0;

}

/****************************************************************************
* setfontsize
****************************************************************************/
void setfontsize (int pixelsize)
{
	int err;

	err = FT_Set_Pixel_Sizes (face, 0, pixelsize);

	if (err)
		printf ("Error setting pixel sizes!");
}

static void DrawCharacter (FT_Bitmap * bmp, FT_Int x, FT_Int y)
{
	FT_Int i, j, p, q;
	FT_Int x_max = x + bmp->width;
	FT_Int y_max = y + bmp->rows;
	int spos;
	unsigned int pixel;
	int c;

	for (i = x, p = 0; i < x_max; i++, p++)
	{
		for (j = y, q = 0; j < y_max; j++, q++)
		{
			if (i < 0 || j < 0 || i >= 640 || j >= screenheight)
				continue;

			/*** Convert pixel position to GC int sizes ***/
			spos = (j * 320) + (i >> 1);

			pixel = xfb[whichfb][spos];
			c = bmp->buffer[q * bmp->width + p];

			/*** Cool Anti-Aliasing doesn't work too well at hires on GC ***/
			if (c > 128)
			{
				if (i & 1)
					pixel = (pixel & 0xffff0000) | fontlo;
				else
					pixel = ((pixel & 0xffff) | fonthi);

				xfb[whichfb][spos] = pixel;
			}
		}
	}
}

/**
 * DrawText
 *
 * Place the font bitmap on the screen
 */
void DrawText (int x, int y, char *text)
{
	int px, n;
	int i;
	int err;
	int value, count;

	n = strlen (text);
	if (n == 0)
		return;

	/*** x == -1, auto centre ***/
	if (x == -1)
	{
		value = 0;
		px = 0;
	}
	else
	{
		value = 1;
		px = x;
	}

	for (count = value; count < 2; count++)
	{
		/*** Draw the string ***/
		for (i = 0; i < n; i++)
		{
			err = FT_Load_Char (face, text[i], FT_LOAD_RENDER);

			if (err)
			{
				printf ("Error %c %d\n", text[i], err);
				continue;				/*** Skip unprintable characters ***/
			}

			if (count)
				DrawCharacter (&slot->bitmap, px + slot->bitmap_left,
				               y - slot->bitmap_top);

			px += slot->advance.x >> 6;
		}

		px = (640 - px) >> 1;

	}
}

/**
 * getcolour
 *
 * Simply converts RGB to Y1CbY2Cr format
 *
 * I got this from a pastebin, so thanks to whoever originally wrote it!
 */

static unsigned int getcolour (u8 r1, u8 g1, u8 b1)
{
	int y1, cb1, cr1, y2, cb2, cr2, cb, cr;
	u8 r2, g2, b2;

	r2 = r1;
	g2 = g1;
	b2 = b1;

	y1 = (299 * r1 + 587 * g1 + 114 * b1) / 1000;
	cb1 = (-16874 * r1 - 33126 * g1 + 50000 * b1 + 12800000) / 100000;
	cr1 = (50000 * r1 - 41869 * g1 - 8131 * b1 + 12800000) / 100000;

	y2 = (299 * r2 + 587 * g2 + 114 * b2) / 1000;
	cb2 = (-16874 * r2 - 33126 * g2 + 50000 * b2 + 12800000) / 100000;
	cr2 = (50000 * r2 - 41869 * g2 - 8131 * b2 + 12800000) / 100000;

	cb = (cb1 + cb2) >> 1;
	cr = (cr1 + cr2) >> 1;

	return ((y1 << 24) | (cb << 16) | (y2 << 8) | cr);
}

/**
 * setfontcolour
 *
 * Uses RGB triple values.
 */
void setfontcolour (u8 r, u8 g, u8 b)
{
	u32 fontcolour;

	fontcolour = getcolour (r, g, b);
	fonthi = fontcolour & 0xffff0000;
	fontlo = fontcolour & 0xffff;
}

/****************************************************************************
* ClearScreen
****************************************************************************/
/*void ClearScreen (){
  whichfb ^= 1;
  VIDEO_ClearFrameBuffer (vmode, xfb[whichfb], COLOR_BLACK);
}
*/
/****************************************************************************
* ShowScreen
****************************************************************************/
void ShowScreen ()
{
	VIDEO_SetNextFramebuffer (xfb[whichfb]);
	VIDEO_Flush ();
	VIDEO_WaitVSync ();
}

/**
 * Show an action in progress
 */
void ShowAction (char *msg)
{
	//int ypos = screenheight >> 1;
	//ypos += 16;

	//ClearScreen ();
	//DrawText (-1, ypos, msg);
	writeStatusBar(msg,"");
	ShowScreen ();
}

/****************************************************************************
* SelectMode
****************************************************************************/
int SelectMode ()
{
	/*
	int ypos;

	ypos = (screenheight >> 1);

	ClearScreen ();
	DrawText (-1, ypos, "Press A for SMB BACKUP mode");
	DrawText (-1, ypos + 20, "Press B for SMB RESTORE mode");
	DrawText (-1, ypos + 40, "Press Y for SD CARD BACKUP mode");
	DrawText (-1, ypos + 60, "Press X for SD CARD RESTORE mode");
	DrawText (-1, ypos + 80, "Press Z for SD/PSO Reload");
	ShowScreen ();*/
	ClearScreen ();
	//setfontcolour(84,174,211);
// setfontcolour(28,28,28);

// DrawText(495,305,"choose your mode");
	writeStatusBar("Choose your mode","");
	ShowScreen();
	/*** Clear any pending buttons - button 'debounce' ***/
	while (PAD_ButtonsHeld (0)
#ifdef HW_RVL
	        | WPAD_ButtonsHeld(0)
#endif
	      )
		VIDEO_WaitVSync ();

	for (;;)
	{

		if (PAD_ButtonsHeld (0) & PAD_BUTTON_A)
		{
			while ((PAD_ButtonsDown (0) & PAD_BUTTON_A))
			{
				VIDEO_WaitVSync ();
			}
			return 100;
		}
		if (PAD_ButtonsHeld (0) & PAD_TRIGGER_Z)
		{
			while ((PAD_ButtonsDown (0) & PAD_BUTTON_B))
			{
				VIDEO_WaitVSync ();
			}
			return 200;
		}
		if (PAD_ButtonsHeld (0) & PAD_BUTTON_Y)
		{
			while ((PAD_ButtonsDown (0) & PAD_BUTTON_Y))
			{
				VIDEO_WaitVSync ();
			}
			return 300;
		}
		if (PAD_ButtonsHeld (0) & PAD_BUTTON_X)
		{
			while ((PAD_ButtonsDown (0) & PAD_BUTTON_X))
			{
				VIDEO_WaitVSync ();
			}
			return 400;
		}
		if (PAD_ButtonsHeld (0) & PAD_BUTTON_START)
		{
			while ((PAD_ButtonsDown (0) & PAD_BUTTON_START))
			{
				VIDEO_WaitVSync ();
			}
			return 500;
		}
		if (PAD_ButtonsHeld (0) & PAD_TRIGGER_R)
		{
			while ((PAD_ButtonsDown (0) & PAD_TRIGGER_R))
			{
				VIDEO_WaitVSync ();
			}
			return 600;
		}

#ifdef HW_RVL
		if (WPAD_ButtonsHeld (0) & WPAD_BUTTON_A)
		{
			while ((WPAD_ButtonsDown (0) & WPAD_BUTTON_A))
			{
				VIDEO_WaitVSync ();
			}
			return 100;
		}
		if (WPAD_ButtonsHeld (0) & WPAD_BUTTON_2)
		{
			while ((WPAD_ButtonsDown (0) & WPAD_BUTTON_MINUS))
			{
				VIDEO_WaitVSync ();
			}
			return 200;
		}
		if (WPAD_ButtonsHeld (0) & WPAD_BUTTON_MINUS)
		{
			while ((WPAD_ButtonsDown (0) & WPAD_BUTTON_MINUS))
			{
				VIDEO_WaitVSync ();
			}
			return 300;
		}
		if (WPAD_ButtonsHeld (0) & WPAD_BUTTON_PLUS)
		{
			while ((WPAD_ButtonsDown (0) & WPAD_BUTTON_PLUS))
			{
				VIDEO_WaitVSync ();
			}
			return 400;
		}
		if (WPAD_ButtonsHeld (0) & WPAD_BUTTON_HOME)
		{
			while ((WPAD_ButtonsDown (0) & WPAD_BUTTON_HOME))
			{
				VIDEO_WaitVSync ();
			}
			return 500;
		}
		if (WPAD_ButtonsHeld (0) & WPAD_BUTTON_1)
		{
			while ((WPAD_ButtonsDown (0) & WPAD_BUTTON_1))
			{
				VIDEO_WaitVSync ();
			}
			return 600;
		}
		if (power)
		{
			PowerOff();
		}
#endif

		VIDEO_WaitVSync ();
	}
}

/**
 * Wait for user to press A
 */
void WaitButtonA ()
{
#ifdef HW_DOL
	while (PAD_ButtonsDown (0) & PAD_BUTTON_A );
	while (!(PAD_ButtonsDown (0) & PAD_BUTTON_A));
#else
	while (1)
	{
		if (PAD_ButtonsDown (0) & PAD_BUTTON_A )
			break;
		if (WPAD_ButtonsDown (0) & WPAD_BUTTON_A )
			break;
		if (power)
		{
			PowerOff();
		}
	}
	while (1)
	{
		if (!(PAD_ButtonsDown (0) & PAD_BUTTON_A))
			break;
		if (!(WPAD_ButtonsDown (0) & WPAD_BUTTON_A))
			break;
		if (power)
		{
			PowerOff();
		}
	}
#endif
}

/****************************************************************************
 * Wait for user to press A or B. Returns 0 = B; 1 = A
 ****************************************************************************/

int WaitButtonAB ()
{
#ifdef HW_RVL
	u32 gc_btns, wm_btns;

	while ( (PAD_ButtonsDown (0) & (PAD_BUTTON_A | PAD_BUTTON_B))
	        || (WPAD_ButtonsDown(0) & (WPAD_BUTTON_A | WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_A | WPAD_CLASSIC_BUTTON_B))
	      ) VIDEO_WaitVSync();

	while (1)
	{
		gc_btns = PAD_ButtonsDown (0);
		wm_btns = WPAD_ButtonsDown (0);
		if ( (gc_btns & PAD_BUTTON_A) || (wm_btns & (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A)) )
			return 1;
		else if ( (gc_btns & PAD_BUTTON_B) || (wm_btns & (WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B)) )
			return 0;
	}
#else
	u32 gc_btns;

	while ( (PAD_ButtonsDown (0) & (PAD_BUTTON_A | PAD_BUTTON_B)) ) VIDEO_WaitVSync();

	while (1)
	{
		gc_btns = PAD_ButtonsDown (0);
		if ( gc_btns & PAD_BUTTON_A )
			return 1;
		else if ( gc_btns & PAD_BUTTON_B )
			return 0;
	}
#endif
}



/**
 * Show a prompt
 */
void WaitPrompt (char *msg)
{
	//int ypos = (screenheight) >> 1;
	//ClearScreen ();
	//DrawText (-1, ypos, msg);
	//ypos += 20;
	//DrawText (-1, ypos, "Press A to continue");
	writeStatusBar(msg,"Press A to continue");
	ShowScreen ();
	WaitButtonA ();
	//clear the text
	writeStatusBar("","");

}

/****************************************************************************
 * Show a prompt with choice of two options. Returns 1 if A button was pressed
   and 0 if B button was pressed.
 ****************************************************************************/
int WaitPromptChoice ( char *msg, char *bmsg, char *amsg)
{
	char txt[80];
	int ret;
	sprintf (txt, "B = %s   :   A = %s", bmsg, amsg);
	writeStatusBar(msg, txt);
	ret = WaitButtonAB ();
	//Clear the text
	writeStatusBar("","");

	return ret;
}

void DrawLineFast (int startx, int endx, int y, u8 r, u8 g, u8 b)
{
	int width;
	u32 offset;
	u32 colour;
	int i;

	offset = (y * 320) + (startx >> 1);
	colour = getcolour (r, g, b);
	width = (endx - startx) >> 1;

	for (i = 0; i < width; i++)
		xfb[whichfb][offset++] = colour;
}

void showSaveInfo(int sel)
{
	int y = 160, x = 380, j;
	char gamecode[5], company[3], txt[1024];

	//clear right pane, but just the save info
	int bgcolor = getcolour(84,174,211);
	DrawBoxFilled(375, 145, 605, 380, bgcolor);

	// read file, display some more info
	// TODO: only read the necessary header + comment, display banner and icon files
	if (mode == RESTORE_MODE)
	{
		j = SDLoadMCImageHeader((char*)filelist[sel]);
		// struct gci now contains header info
		memcpy(company, gci.company, 2);
		memcpy(gamecode, gci.gamecode, 4);
		company[2] = gamecode[4] = 0;

	}
	else if( (mode == BACKUP_MODE) | (mode == DELETE_MODE) )
	{
		memcpy(company, CardList[sel].company, 2);
		memcpy(gamecode, CardList[sel].gamecode, 4);
		company[2] = gamecode[4] = 0;
		j = CardReadFileHeader(CARD_SLOTB, sel);
		// struct gci now contains header info

	}


	sprintf(txt, "#%d %s/%s", sel, gamecode, company);
	DrawText(x, y, txt);
	y += 20;

	// Compute date/time
	u32 t = gci.time;  // seconds since jan 1, 2000
	int month, day, year, hour, min, sec;
	month = day = year = hour = min = sec = 0;
	sprintf(txt, "D: %08X", t);
	DrawText(x, y, txt);
	y += 20;

	int monthdays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	char monthstr[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
	                         "Aug", "Sep", "Oct", "Nov", "Dec"
	                       };
	sec = t % 60;
	t /= 60;
	min = t % 60;
	t /= 60;
	hour = t % 24;
	t /= 24;
	day = t % 365;
	year = t / 365;
	// figure in leap years
	day -= year / 4;
	if ( (year % 4 == 0) && (day > 31+29) )
	{
		// TODO: check if day<0, go back one month
		day--;
	}
	// day now is number of days into the year
	for(j=0; j<12; j++)
	{
		if (day > monthdays[j])
		{
			month++;
			day -= monthdays[j];
		}
		else
		{
			j = 12;
		}
	}

	sprintf(txt, "Date: %s %02d, %04d", monthstr[month], day+1, year+2000);
	DrawText(x, y, txt);
	y += 20;
	sprintf(txt, "Time: %02d:%02d:%02d", hour, min, sec);
	DrawText(x, y, txt);
	y += 20;
	sprintf(txt, "Blocks: %d", gci.filesize8);
	DrawText(x, y, txt);
	y += 20;
	//M-> Cant' move the file //C->can't copy the file //P->public file //Most frecuent: xxP
	sprintf(txt, "Perm: %s%s%s",
	        (gci.unknown1 & 16) ? "M " : "x",
	        (gci.unknown1 & 8) ? "C " : "x",
	        (gci.unknown1 & 4) ? "P" : "x");
	DrawText(x, y, txt);
	y += 20;

	//Bad way to display the file comment, should reposition to just show the full 32 char lines instead of spliting.
	//For testing how many chars fit the screen (they might not be all upper case in savegames so...)
	/*DrawText(x, y, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF");
	y += 20;
	*/
	//This relies in FONT_SIZE being 16 pixels
	char comment1[26];
	char comment2[6];

	strncpy(comment1, (char*)CommentBuffer, 26);
	DrawText(x, y, comment1);
	y += 20;
	memset(comment1, 0, sizeof(comment1));

	strncpy(comment2, (char*)CommentBuffer+26, 6);
	DrawText(x, y, comment2);
	y += 20;
	memset(comment2, 0, sizeof(comment2));

	strncpy(comment1, (char*)CommentBuffer+32, 26);
	DrawText(x, y, comment1);
	y += 20;
	memset(comment1, 0, sizeof(comment1));

	strncpy(comment2, (char*)CommentBuffer+58, 6);
	DrawText(x, y, comment2);
	y += 20;
	memset(comment2, 0, sizeof(comment2));

	/* // Comment at MCDATAOFFSET+(comment addr)
	 //char *comment = (char*) (FileBuffer+MCDATAOFFSET+gci.comment_addr);
	 j = 0;
	 int x2, z = 0;
	 int offset = 0;
	 for (x2=t=0; (t<strlen(comment)) && (t<64); t++) {
	     z++;
	     //x2 += font_size[(int)comment[t]];
		x2 += 16;
	     if (x2 > 225) { // max width of left pane
	         strncpy(txt, comment+offset, z-1);
	         txt[z-1] = 0;
	         //char c = txt[t];
	         //txt[t] = 0;
	         DrawText(x, y, txt);
	         y += 20;
	         offset = t-1;
	         //txt[t] = c;
	         //strcpy(txt, comment+t);
	         z = 0;
	         //x2 = font_size[(int)comment[t]];
			x2 = 16;
	     }
	 }

	 if (z) {
	     strcpy(txt, comment+offset);
	     //txt[z] = 0;
	     DrawText(x, y, txt);
	     y += 20;
	 }*/
}


static void ShowFiles (int offset, int selection)
{
	int i, j;
	char text[80];
	int ypos;
	int w;

	clearLeftPane();

	//setfontsize (16);
	setfontsize (14);
	//ypos = 70;
	ypos = (screenheight - (PAGESIZE * 20)) >> 1;
	//ypos += 20;
	ypos += 25;
	j = 0;
	for (i = offset; i < (offset + PAGESIZE) && (i < maxfile); i++)
	{
		strcpy (text, (char*)filelist[i]);
		if (j == (selection - offset))
		{
			/*** Highlighted text entry ***/
			for (w = 0; w < 20; w++)
			{
				// DrawLineFast (30, 610, (j * 20) + (ypos - 16) + w, 0x00, 0x00, 0xC0);
				//DrawLineFast (35, 330, (j * 20) + (ypos - 16) + w, 0xff, 0xff, 0xff);
				DrawLineFast (35, 330, (j * 20) + (ypos - 14) + w, 0xff, 0xff, 0xff);

			}
			//setfontcolour (0xff, 0xff, 0xff);
			//setfontcolour (84, 174, 211);
			setfontcolour (28, 28, 28);
			// DrawText (-1, (j * 20) + ypos, text);
			DrawText (35, (j * 20) + ypos, text);

			showSaveInfo(selection);

			setfontcolour (0xff, 0xff, 0xff);

			//setfontcolour (0xc0, 0xc0, 0xc0);
		}
		else
		{
			/*** Normal entry ***/
			//****************
			for (w = 0; w < 20; w++)
			{
				// DrawLineFast (30, 610, (j * 20) + (ypos - 16) + w, 0x00, 0x00, 0xC0);

				//DrawLineFast (35, 330, (j * 20) + (ypos - 16) + w, 84, 174,211);
				DrawLineFast (35, 330, (j * 20) + (ypos - 14) + w, 84, 174,211);
				//***********************************************************
			}
			DrawText (35, (j * 20) + ypos, text);
		}
		j++;
	}
	ShowScreen ();
}

/*void ShowFiles(int offset, int selected, int maxfiles) {
    int y = 110, tmp;

    //WaitPrompt("ShowFiles");
    clearLeftPane();
    for (tmp = offset; (tmp<offset+PAGESIZE) && (tmp < maxfiles); tmp++) {
        char str[1024], txt[1024];
        int x, t;
        strcpy(str, CardList[tmp].filename);
        for (x=t=0; t<strlen(str); t++) {
            x += font_size[(int)str[t]];
            if (x > 290) { // max width of left pane
                str[t] = 0;
                t = strlen(str);
            }
        }
        strcpy(txt, str);
        //txt[t-1] = 0;
        if (tmp == selected) {
            // Highlight this file
            fntDrawBoxFilled(35, y, 330, y+23, hlcolor);
			showSaveInfo(selected);
        }
        write_font(35, y, txt);  // write filename on left
        y += 20;
    }
}
*/


/****************************************************************************
* ShowSelector
*
* Let's the user select a file to backup or restore.
* Returns index of file chosen.
****************************************************************************/
int ShowSelector ()
{
	u32 p;
#ifdef HW_RVL
	u32 wp;
#endif
	int redraw = 1;
	int quit = 0;
	int offset, selection;

	offset = selection = 0;

	while (quit == 0)
	{
		if (redraw)
		{
			ShowFiles (offset, selection);
			redraw = 0;
		}
		p = PAD_ButtonsDown (0);
#ifdef HW_RVL
		wp = WPAD_ButtonsDown (0);
#endif
		if ((p & PAD_BUTTON_B )
#ifdef HW_RVL
		        | (wp & WPAD_BUTTON_B)
#endif
		   )
		{
			cancel = 1;
			return -1;
		}
#ifdef HW_RVL
		if (power)
		{
			PowerOff();
		}
#endif

		if ((p & PAD_BUTTON_A)
#ifdef HW_RVL
		        | (wp & WPAD_BUTTON_A)
#endif
		   )
		{
			return selection;
		}
		if ((p & PAD_BUTTON_DOWN)
#ifdef HW_RVL
		        | (wp & WPAD_BUTTON_DOWN)
#endif
		   )
		{
			selection++;
			if (selection == maxfile)
			{
				selection = offset = 0;
			}
			if ((selection - offset) >= PAGESIZE)
			{
				offset += PAGESIZE;
			}
			redraw = 1;
		}
		if ((p & PAD_BUTTON_UP)
#ifdef HW_RVL
		        | (wp & WPAD_BUTTON_UP)
#endif
		   )
		{
			selection--;
			if (selection < 0)
			{
				selection = maxfile - 1;
				offset = selection - PAGESIZE + 1;
			}
			if (selection < offset)
			{
				offset -= PAGESIZE;
			}
			if (offset < 0)
			{
				offset = 0;
			}
			redraw = 1;
		}
	}
	return selection;
}

void writeStatusBar( char *line1, char *line2)
{
	int bgcolor = getcolour(0xff,0xff,0xff);
	DrawBoxFilled(10, 404, 510, 455, bgcolor);
	//setfontcolour(84,174,211);
	setfontcolour(28,28,28);
	DrawText(40, 425, line1);
	DrawText(40, 450, line2);
}

void clearLeftPane()
{
	int bgcolor = getcolour(84,174,211);
	DrawBoxFilled(34, 110, 333, 380, bgcolor);
}

void clearRightPane()
{
	int bgcolor = getcolour(84,174,211);
	DrawBoxFilled(376, 112, 605, 380, bgcolor);
}

void DrawHLine (int x1, int x2, int y, int color)
{
	int i;
	y = 320 * y;
	x1 >>= 1;
	x2 >>= 1;
	for (i = x1; i <= x2; i++)
	{
		u32 *tmpfb = xfb[whichfb];
		tmpfb[y+i] = color;
	}
}

void DrawVLine (int x, int y1, int y2, int color)
{
	int i;
	x >>= 1;
	for (i = y1; i <= y2; i++)
	{
		u32 *tmpfb = xfb[whichfb];
		tmpfb[x + (640 * i) / 2] = color;
	}
}

void DrawBox (int x1, int y1, int x2, int y2, int color)
{
	DrawHLine (x1, x2, y1, color);
	DrawHLine (x1, x2, y2, color);
	DrawVLine (x1, y1, y2, color);
	DrawVLine (x2, y1, y2, color);
}

void DrawBoxFilled (int x1, int y1, int x2, int y2, int color)
{
	int h;
	for (h = y1; h <= y2; h++)
		DrawHLine (x1, x2, h, color);
}










