×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                         GCMM 1.2                              ·oø×O0|
|                   http://code.google.com/p/gcmm                           |
|                          (Under GPL License)                              |
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'

GameCube/Wii Memory Manager is an aplication to backup Nintendo GameCube savegames.

Gcmm is a project started by dsbomb and justb, which is based on Askot's modification to add SD support to the mcbackup libogc example.

I (suloku) have updated the code to newest libraries to port it to the Wii system, and what I find more important: restoring savegames now works properly.

×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                            FEATURES                           ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'
* Backups and restores savegames into GCI format
* Restores savegames in GCS/SAV format
* Deletes savegames from memory card
* Backups memory card raw images in .raw format
* Restores memory card raw images from RAW/GCP/MCI format
* Format the memory card
* Wiimote and GameCube controller support
* Power button support
* Front SD and FAT32 USB device (wii) and SDGecko (gamecube) support
* Shows savegame information, alongside Icon and Banner!
* A (somewhat) nice UI
* Open Source!

×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                         UPDATE HISTORY                        ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'

[What's New 1.2 - september 06, 2012 - By suloku]

* Added raw backup mode (in .raw format, compatible with dolphin and devolution)
* Added RAW/GCP/MCI support for raw restore mode
* Added format mode
* Flash ID of inserted card and SD image are shown in Raw Restore Mode
* Protection against writing a raw image to the wrong card (trough Flash ID checking)
* Raw mode works with official and unofficial cards, as well as gci mode (thanks to tueidj for pointing me in the right path!)

[What's New 1.1 - august 29, 2012 - By suloku]

* Icon and banner by dronesplitter!
* Added USB-SD selection in wiimode (only at boot)
* Added card slot selection (wii mode only)
* Propper GCI backup and restore. Now GCMM uses __card_getstatusex and __card_setstatuex, which provide a more 1:1 backup/restore
* Correctly displaying savegame Date information
* Savegame information rearranged.

Accepted PlabloACZ and Picachu025 modifications, with the following changes:
* Tweaked mount function
* Filenames are no longer prefixed with a number for current sesion. Instead, savegames are suffixed with a number. When backing up a savegame if the same file exists on SD it will be prefixed with a growing number (if savegame_00.gci exists, then it will try savegame_01.gci, savegame_02.gci... and so on)
* Infinite loop can't happen when backupping a savegame as in r11 MOD 2.

[What's New r11 MOD 2 - September 11, 2011 - By Pikachu025]
* R (GC-Pad) / 1 (Wiimote) now launches a "backup all" mode, where all saves on the memory card are written to the SD card without any user prompts in the meantime.
* I came across a couple saves that had ridiculous filenames that refused to write to SD, so if the program comes across one of those, it'll now write them out as "illegal_name" instead of the actual filename.
* Filenames written to SD are now prefixed with a number, counting up from 1 for every file written during the current session. I added this since I had multiple files that resulted in the same filename.
* I also added a small check if the file was written correctly. If not, it'll retry. This probably results in an infinite loop when your SD card doesn't have enough free space, so ensure that I guess.
* I also (quite shoddily) edited the image listing all the options to add the new option, it's ugly but does its job. Feel free to fix, I don't have Photoshop or anything here.

[What's New r11 MOD - September 09, 2011 - By PabloACZ]
* SDGetFileList() function in sdsupp.c updated to reflect the changes in DevKitPPC/libogc from the last three years (diropen, dirnext and dirclose commands were replaced with opendir, readdir and closedir, respectively).
* Modified the MountCard() function in mcard.c to perform a probe on the GC Memory Card slot, to make sure it was mounted properly.
* Improved the compatibility with GCS/SAV files with the patch posted by jcwitzel in December 2009 (http://code.google.com/p/gcmm/issues/detail?id=1#c25).
* The Makefiles were modified to include the zlib in the libraries section. It seems that the latest libFreeType PPC port needs it to work.
* **Hopefully** Added compatibility with Official GameCube Memory Cards (see this: http://devkitpro.svn.sourceforge.net/viewvc/devkitpro?view=revision&revision=4049). According to a friend of mine, it works with a 256 blocks Memory Card.
* Compiled with DevKitPPC r24, libogc 1.8.8, libfat 1.0.10 and libFreetype 2.4.2.

[What's New 1.0 - December 31, 2008]
* Updated to libfat
* Added Wii support
* Fixed restore bug (yes, savegames will restore properly)
* New background
* Support for interlaced and widescreen in all regions
* Delete mode (dsbomb&justb)
* Shows the savegame information (mostly by dsbomb&justb)
* Many other fixes/modifications for the user

×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                             TO DO                             ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'
* Add hotswapping (memory cards can be swapped, SD Gecko/SD/USB can't be swapped)

×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                          LIMITATIONS                          ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'

    Protected Savegames
--------------------------
Protected gamesaves will only be able to be restored on the card it was backuped from. Even though, some protected savegames will still not work as some save protection rely in the situation of the gamesave on the memory card.

Protected savegames rely on the serial ID that is given to the memory card when it is formated. That's why they won't work in other cards and why they won't work on the same card if that has been formatted.

Restoring a raw image to a diferent card won't work. Please note that all unofficial cards share the same Flash ID (serial ID), allowing raw image restoring between unofficial cards (as long as they are the same size).

Known protected savegames:
* F-zero GX
* Phantasy Star Online (all of the games)

    Other savegame formats
----------------------------
There are computer programs that can transform other savegame formats into GCI (currently GCMM supports SAV and GCS restoring).

RAW/GCP format is a raw image of the memory card with all its contents.

MCI format (createad by softdev's sdmc) is a raw image of the card, preceded by a 64 byte header.


×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                        CONSIDERATIONS                         ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'

Users:
* If you extract a device (USB, internal SD, USB gecko), it won't work againt. If you did so, reboot the GCMM. On the contrary
* Memory cards can be extracted/inserted at will at the main menu screen. It is not recommended to change the card in any other screen.
* Dolphin (wii/gc computer emulator) has a nice memory card manager, check it out!

Developers:

* LibOGC card functions works with time functions that use Epoch (seconds since jan 1, 1970) as reference, while GameCube works with seconds since jan 1, 2000). The difference is 946684800 seconds
* GCMM now uses libogc 1.8.11 git (2012-07-25) card.c, card.h and system.h with tueidj's patches for proper memory card unlocking. It would be wise to update those files in GCMM if changes are made to libogc concerning other functions. Note that even if libogc asumes the changes, as GCMM now uses some static functions from libogc, it needs card.c and card.h, but if libogc updates the sramex structure (system.h) and fixes the checksums in card.c, system.h will no longer be needed.
* Very good sources of documentation are libogc and dolphin's source code.

×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                     SETUP & INSTALLATION                      ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'

Unzip the archive. You will find the following folders inside:

apps			Contains Homebrew Channel ready files
				(see Homebrew Channel instructions below)
				
gamecube		Contains GameCube DOL file (not required for Wii)
				

    Wii
----------
On the Wii, the savegames will be read from and written to the front SD slot or FAT32 USB device.
The user will be prompted at startup for which device to use.
If the selected device isn't available, GCMM will try to use the other device (i.e: user selects usb device but there's none connected, so GCMM will try to use the internal SD).
Memory card can be in either slot.


  Gamecube
------------
On the GameCube you will need a Gecko SD adapter in the slot A and place the memory card on slot B.

------------------------------
Loading / Running the app:
------------------------------

Wii - Via Homebrew Channel:
--------------------
The most popular method of running homebrew on the Wii is through the Homebrew Channel. If you already have the channel installed, just copy over the apps folder included in the archive into the root of your SD card.

If you haven't installed the Homebrew Channel yet, read about how to here:
http://hbc.hackmii.com/

Gamecube:
---------
You can load gcmm via sdload and an SD card in slot A, or by streaming 
it to your Gamecube, or by booting a bootable DVD with gcmm on it. 
This document doesn't cover how to do any of that. A good source for information on these topics is at http://www.gc-forever.com/wiki/index.php?title=Booting_Homebrew

×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                          CONTROLS                             ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'

They are shown at the screen.

Raw mode controls:
* Raw Backup Mode:  GC_pad L+Y , Wiimote B+Minus
* Raw Restore Mode: GC_pad L+X , Wiimote B+Plus
* Format Card Mode: GC_pad L+Z , Wiimote B+2


×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                          COMPILING                            ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'
Currently gcmm uses:

*DevkitPPC r26: http://sourceforge.net/projects/devkitpro/files/devkitPPC/
*libOGC 1.8.11 git (2012-07-25): http://sourceforge.net/projects/devkitpro/files/libogc
	note: it compiles and runs fine with 1.8.11 release
*libfat 1.0.11: http://sourceforge.net/projects/devkitpro/files/libfat/
*libFreeType 2.4.2 port: http://sourceforge.net/projects/devkitpro/files/portlibs/ppc/

×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                           CREDITS                             ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'

*SoftDev for his contributions to the GC/WII scene
*Costis for helping with some doubts, he's allways there
*Masken for his code on raw data reading/writing
*Justb & dsbomb for originally creating gcmm
*CowTRobo & Samsom for very useful old sources
*Tantric for pointing out that official memory cards won't work on wii mode, which encouraged me to continue gcmm as all my previous efforts where in vane due to using an official card for the testing.
*tueidj, for his patches and very useful information and support. Official memory cards work due to his work.
*dronesplitter for banner and icon implementation
*PlabloACZ and Picachu025 for updating the source.
*Nano, for inspiring me to finally working again on GCMM

×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                                                               ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'