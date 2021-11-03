#define MCSAVES "MCBACKUP"

#define DEV_NUM 	0
#define DEV_GCSDA 	1
#define DEV_GCSDB 	2
#define DEV_GCSDC 	3
#define DEV_GCODE 	4
#define DEV_WIISD 	5
#define DEV_WIIUSB	6

#define DEV_TOTAL 6

#define DEV_ND		0
#define DEV_AVAIL	1
#define DEV_MOUNTED 2

int SDSaveMCImage();
int SDLoadMCImage(char *sdfilename);
int SDLoadMCImageHeader(char *sdfilename);
int SDLoadCardImageHeader(char *sdfilename);
int isdir_sd(char *path);
int SDGetFileList(int mode);
bool file_exists(const char * filename);
