#define MCSAVES "MCBACKUP"

int SDSaveMCImage();
int SDLoadMCImage(char *sdfilename);
int SDLoadMCImageHeader(char *sdfilename);
int SDLoadCardImageHeader(char *sdfilename);
int isdir_sd(char *path);
int SDGetFileList(int mode);
bool file_exists(const char * filename);
