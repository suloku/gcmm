ח����������� ������������������������������ �����������������������-����-���
|0O��o�                    GCMM 1.4f by suloku                        �o��O0|
|                      https://github.com/suloku/gcmm                       |
|                   (old site: http://code.google.com/p/gcmm)               |
|                          (Under GPL License)                              |
`������� ���������������� ��������������� �������������������� �������������'

GameCube/Wii Memory Manager is an aplication to backup Nintendo GameCube savegames.

Gcmm is a project started by dsbomb and justb, which is based on Askot's modification to add SD support to the mcbackup libogc example.

I (suloku) have updated the code to newest libraries to port it to the Wii system, and what I find more important: restoring savegames now works properly.

ח����������� ������������������������������ �����������������������-����-���
|0O��o�                            FEATURES                           �o��O0|
`������� ���������������� ��������������� �������������������� �������������'
* Backups and restores savegames into GCI format
* Restores savegames in GCS/SAV format
* Deletes savegames from memory card
* Backups memory card raw images in .raw format
* Restores memory card raw images from RAW/GCP/MCI format
* Format the memory card
* Wiimote and GameCube controller support
* Power button support
* Front SD and FAT32 USB device (wii) and SDGecko (gamecube) support
* Shows savegame information, alongside animated Icon and Banner!
* A (somewhat) nice UI
* Open Source!

ח����������� ������������������������������ �����������������������-����-���
|0O��o�                         UPDATE HISTORY                        �o��O0|
`������� ���������������� ��������������� �������������������� �������������'

[What's New 1.4f - april 05, 2017 - By suloku]
* dragonbane0 made a mod of version 1.4c with folder selection and alphabetical sorting. Zephiles pointed this out and the changes have been merged. Thanks you both!

[What's New 1.4e - february 27, 2016 - By suloku]
* Fixed bug in card.c that prevented correct backup/write of saves with same filename but different case. Bug affected Timesplitters 2, probably Timesplitters 3 too. Thanks to DakuTree for reporting and Antidote for fixing.

[What's New 1.4d - august 08, 2015 - By suloku]
* Fixed bug in card.c that prevented writing to the last block of the memory card, preventing restoring a savegame that would fill the memory card (thanks to undergroundmonorail)
* Fixed bug in card.c that prevented correct block freeing when deleting a file and only was fixed by using the memory card on official software (the wii/gamecube save manager or probably also games)
* Added libogc fix for a bug in card.c (the bug didn't affect GCMM, it was fixed in 1.4b, but I didn't change card.c)
* Changed button presses for single savegame delete to prevent unvoluntary savegame deleting

[What's New 1.4c - january 05, 2014 - By suloku]
* Disabled __sector_erase() check when raw restoring as some unofficial cards seem to have problems with it.

[What's New 1.4b - september 03, 2013 - By suloku]
* Card initialization was wrong all the way, wich lead to savegames of the same game of different region or those that shared similar filenames (Twilight Princess and WindWaker) to not work properly. Thanks to antidote.crk for noticing. (read considerations sections for more info)
* Added version display

[What's New 1.4a - october 18, 2012 - By suloku]
* Fixed SD Gecko when inserted/swapped at slot selection screen in GameCube mode.
* FLash ID display was missing a byte
* Changed some text that may confuse the user
* Made font sizes more coherent

[What's New 1.4 - october 08, 2012 - By suloku]
* Animated icon alongside several (minor) graphical improvements
* Ability to select SD Gecko slot in GameCube mode (just like Wii mode SD/USB prompt)
* Moved "Backup All" to Backup Mode (press R/1 when in Backup Mode)
* Added an analog "Restore All" in Restore Mode (overwriting is supported)
* Shows filename when prompted to overwrite (also in "Restore All")
* Savegame permisions are shown in a more explicit and user friendly way
* Shows memory card freeblocks
* Page number display in file selector
* Left and right now scroll 5 file entries at once
* Scrolling of file entries can be done holding the button (up/down/left/right)
* Added security checks to Raw Restore Mode
* Added some special characters to the font (needed for savegame comments)
* Minor code tweaks

Lots of thanks to bm123456 and themanuel for beta testing and support!

[What's New 1.3 - september 14, 2012 - By suloku]

* Shows card/image serial number in Raw Restore Mode
* F-zero and Phantasy Star Online Ep I&II and Phantasy Star Online III savegames
are patched uppon restoring so they will work on target card (by Ralf)

Lot's of thanks to Ralf at gscentral.org forums
http://board.gscentral.org/retro-hacking/53093.htm#post188949

[What's New 1.2d - september 08, 2012 - By suloku]

* Previous version couldn't raw backup if backup folder didn't exist in sd/usb
* Added (double)overwrite prompt when restoring a savefile to memory card (Nano/Excelsiior's idea)
* Updated graphics so raw mode commands are less cryptic (wii mode design based on JoostinOnline's for GCMM+)
* Use DejavuSans as font (much better readability) from GCMM+ by Nano/Excelsiior

[What's New 1.2c - september 06, 2012 - By suloku]

* Raw backups are now named with the number of blocks: insted of Backup_*timestamp*.raw now it is 0059b_*timestamp.raw, 2043b_*timestamp.raw...
* Minor code changes (just for safety)

[What's New 1.2b - september 06, 2012 - By suloku]

* Solved a potential bug, 1.2 and 1.2a seemed unaffected by it.

[What's New 1.2a - september 06, 2012 - By suloku]

* 1.2 wasn't correctly freeing memory and eventually raw backup and restore would hang the app (a 2043 block card would make it hang at the second attempt to raw backup the card)

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

ח����������� ������������������������������ �����������������������-����-���
|0O��o�                             TO DO                             �o��O0|
`������� ���������������� ��������������� �������������������� �������������'
* Add hotswapping (memory cards can be swapped, SD Gecko/SD/USB can't be swapped)

ח����������� ������������������������������ �����������������������-����-���
|0O��o�                 ABOUT SAVEGAMES AND RAW IMAGES                �o��O0|
`������� ���������������� ��������������� �������������������� �������������'

        Raw Images
--------------------------

A raw image is a 1:1 copy of the memory card. It can only be restored to the card it was made from.
Note that unofficial cards share the same Flash ID, which allows restoring raw images between unofficial cards (as long as they have the same size).

These limitations can be solved with Raw Tools: http://code.google.com/p/gcmm/wiki/Raw_Tools

For example:
You can grab a 59 block card raw image from an unofficial card and modify it so you can restore it to your 1019 blocks official card, mantaining the serial number of the card so your protected savegames still work.

note: as of GCMM 1.3 protected savegames can be restored to any card and they will work (thanks to Ralf!)


    Protected Savegames
--------------------------

This is just for information, as since GCMM 1.3 even serial protected savegames can be restored to any memory card.

There are two kinds of protected savegames:

* Permision protected savegames: this savegames can't be moved/copied from the Gamecube/Wii memory card manager. These saves can be backed up and restored without problems by GCMM

* Serial protected savegames: this savegames can't be moved/copied from the Gamecube/Wii memory card manager and will only work in the card they were backupped from. This protected savegames rely on the serial number that is given to the memory card when it is formated. That's why they won't work in other cards and why they won't work on the same card if that has been formatted.

Restoring a raw image to a diferent card won't work, it has to be the same card (it will work even if the card was formatted). Please note that all unofficial cards share the same Flash ID (which is different from the serial number), allowing raw image restoring between unofficial cards (as long as they are the same size).


    Other savegame formats
----------------------------
There are computer programs that can transform other savegame formats into GCI (currently GCMM supports SAV and GCS restoring).

RAW/GCP format is a raw image of the memory card with all its contents.

MCI format (createad by softdev's sdmc) is a raw image of the card, preceded by a 64 byte header.


ח����������� ������������������������������ �����������������������-����-���
|0O��o�                        CONSIDERATIONS                         �o��O0|
`������� ���������������� ��������������� �������������������� �������������'

Users:
* If you extract a device (USB, internal SD, USB gecko), it won't work againt. If you did so, reboot the GCMM. On the contrary
* Memory cards can be extracted/inserted at will at the main menu screen. It is not recommended to change the card in any other screen.
* Dolphin (wii/gc computer emulator) has a nice memory card manager, check it out!

About usb devices:
* Two of my usb devices won't work with GCMM, but those same devices make The Homebrew Channel crash. If you have problems try another usb device or an SD card.

Developers:

* LibOGC card functions works with time functions that use Epoch (seconds since jan 1, 1970) as reference, while GameCube works with seconds since jan 1, 2000). The difference is 946684800 seconds
* GCMM now uses libogc 1.8.11 git (2012-07-25) card.c, card.h and system.h with tueidj's patches for proper memory card unlocking. It would be wise to update those files in GCMM if changes are made to libogc concerning other functions. Note that even if libogc asumes the changes, as GCMM now uses some static functions from libogc, it needs card.c and card.h, but if libogc updates the sramex structure (system.h) and fixes the checksums in card.c, system.h will no longer be needed.
* Very good sources of documentation are libogc and dolphin's source code.
* Card_Init() shall be called only once. Any subsequent call will be pointless; to change the company and gamecode one should use Card_setgamecode() and Card_setcompany libogc functions.

ח����������� ������������������������������ �����������������������-����-���
|0O��o�                     SETUP & INSTALLATION                      �o��O0|
`������� ���������������� ��������������� �������������������� �������������'

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

ח����������� ������������������������������ �����������������������-����-���
|0O��o�                          CONTROLS                             �o��O0|
`������� ���������������� ��������������� �������������������� �������������'

They are shown at the screen.

Raw mode controls: hold L (gamecube pad) or B (wiimote) then press the corresponding button
* Raw Backup Mode:  GC_pad L+Y , Wiimote B+Minus
* Raw Restore Mode: GC_pad L+X , Wiimote B+Plus
* Format Card Mode: GC_pad L+Z , Wiimote B+2


ח����������� ������������������������������ �����������������������-����-���
|0O��o�                          COMPILING                            �o��O0|
`������� ���������������� ��������������� �������������������� �������������'
Currently gcmm uses:

* DevkitPPC r26: http://sourceforge.net/projects/devkitpro/files/devkitPPC/
* libOGC 1.8.11 git (2012-07-25): http://sourceforge.net/projects/devkitpro/files/libogc
	note: it compiles and runs fine with 1.8.11 release
* libfat 1.0.11: http://sourceforge.net/projects/devkitpro/files/libfat/
* libFreeType 2.4.2 port: http://sourceforge.net/projects/devkitpro/files/portlibs/ppc/

ח����������� ������������������������������ �����������������������-����-���
|0O��o�                           CREDITS                             �o��O0|
`������� ���������������� ��������������� �������������������� �������������'

* SoftDev for his contributions to the GC/WII scene
* Costis for helping with some doubts, he's allways there
* Masken for his code on raw data reading/writing
* Justb & dsbomb for originally creating gcmm
* CowTRobo & Samsom for very useful old sources
* Tantric for pointing out that official memory cards won't work on wii mode, which encouraged me to continue gcmm as all my previous efforts where in vane due to using an official card for the testing.
* tueidj, for his patches and very useful information and support. Official memory cards work due to his work.
* dronesplitter for banner and icon implementation
* PlabloACZ and Picachu025 for updating the source.
* Nano(Excelsiior), for inspiring me to finally working again on GCMM
* bm123456 and themanuel for beta testing and support

ח����������� ������������������������������ �����������������������-����-���
|0O��o�                                                               �o��O0|
`������� ���������������� ��������������� �������������������� �������������'