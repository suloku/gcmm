/****************************************************************************
* mcard support prototypes
****************************************************************************/
#ifndef _MCARDSUP_
#define _MCARDSUP_

void GCIMakeHeader();
void ExtractGCIHeader();
int MountCard(int cslot);
int CardGetDirectory (int slot);
void CardListFiles ();
int CardReadFileHeader (int slot, int id);
int CardReadFile (int slot, int id);
int CardWriteFile (int slot);
void MC_DeleteMode(int slot);
void MC_FormatMode(s32 slot);
void WaitCardError(char *src, int error);
static int OFFSET = 0;
#endif
