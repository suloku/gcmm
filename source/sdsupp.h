#define MCSAVES "MCBACKUP"

int SDSaveMCImage ();
int SDLoadMCImage(char *sdfilename);
int SDLoadMCImageHeader(char *sdfilename);
int SDGetFileList ();
