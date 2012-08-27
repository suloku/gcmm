×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                         GCMM 1.0                              ·oø×O0|
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
* Deletes savegames from slot B memory card
* Wiimote and GameCube controller support
* Power button support
* Front SD (wii) and SDGecko (gc) support
* A nice UI
* Open Source!

×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                         UPDATE HISTORY                        ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'

Accepted PlabloACZ and Picachu025 modifications, with the following changes:

*tweaked mount function
*Filenames are no longer prefixed with a number for current sesion. Instead, when backing up a savegame if the same file exists on SD it will be prefixed with a growing number (if 00-savegame.gci exists, then it will try 01-savegame.gci, 02-savegame.gci... and so on)
*Infinite loop won't happen when backupping a savegame

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
|0O×øo·                                 TO DO                         ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'
* Add icon and banner display for the savegames
* Add raw image read and write support
* Add hotswapping

×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                          LIMITATIONS                          ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'

    Protected Savegames
--------------------------
Protected gamesaves will only be able to be restored on the card it was backuped from. Even though, some protected savegames will still not work as some save protection rely in the situation of the gamesave on the memory card.

Protected savegames rely on the serial ID that is given to the memory card when it is formated. That's why they won't work in other cards and why they won't work on the same card if that has been formatted.

Restoring a raw image to a diferent card has still to be tested to see if that will permit using a protected gamesave on another memory card.

    Other savegame formats
----------------------------
There are computer programs that can transform other savegame formats into GCI.

GCP format is a raw savegame image of the memory card. At the moment creating a raw image is possible thanks to Masken, but there are still problems when restoring.


×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                         SETUP & INSTALLATION                  ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'

Unzip the archive. You will find the following folders inside:

apps			Contains Homebrew Channel ready files
				(see Homebrew Channel instructions below)
				
gamecube		Contains GameCube DOL file (not required for Wii)
				

    Wii
----------
On the Wii, the savegames will be read from and written to the front SD slot.
Memory card should be in slot B.


  Gamecube
------------
On the GameCube you will need a Gecko SD adapter in the slot A and place the memory card on slot B.

------------------------------
Loading / Running the app:
------------------------------

Wii - Via Homebrew Channel:
--------------------
The most popular method of running homebrew on the Wii is through the Homebrew
Channel. If you already have the channel installed, just copy over the apps folder
included in the archive into the root of your SD card.

If you haven't installed the Homebrew Channel yet, read about how to here:
http://hbc.hackmii.com/

Gamecube:
---------
You can load gcmm via sdload and an SD card in slot A, or by streaming 
it to your Gamecube, or by booting a bootable DVD with gcmm on it. 
This document doesn't cover how to do any of that. A good source for information 
on these topics is the tehskeen forums: http://www.tehskeen.com/forums/

×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                          CONTROLS                             ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'

They are shown at the screen.

×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                           COMPILING                           ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'
Currently gcmm uses:

*DevkitPPC r24: http://sourceforge.net/projects/devkitpro/files/devkitPPC
*libOGC r1.8.8: http://sourceforge.net/projects/devkitpro/files/libogc
*libFreeType 2.4.2 port: http://sourceforge.net/projects/devkitpro/files/portlibs/ppc

×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                           CREDITS                             ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'

*SoftDev for his contributions to the GC/WII scene
*Costis for helping with some doubts, he's allways there
*Masken for his code on raw data reading/writing
*Justb & dsbomb for originally creating gcmm
*CowTRobo & Samsom for very useful old sources
*Tantric for pointing out that official memory cards won't work on wii mode, which encouraged me to continue gcmm as all my previous efforts where in vane due to using an official card for the testing.

×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                                                               ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'