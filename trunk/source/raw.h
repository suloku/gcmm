typedef struct {
							//Offset        Size    Description
	// Serial in libogc
	u8 serial[12];			//0x0000		12		?
	u64 formatTime;			//0x000c		8		time of format (OSTime value)
	u32 SramBias;			//0x0014		4		sram bias at time of format
	u32 SramLang;			//0x0018		4		sram language
	u8 Unk2[4];				//0x001c		4		? almost always 0
	// end Serial in libogc
	u8 deviceID[2];			//0x0020		2		0 if formated in slot A 1 if formated in slot B
	u8 SizeMb[2];			//0x0022		2		size of memcard in Mbits
	u16 Encoding;			//0x0024		2		encoding (ASCII or japanese)
	u8 Unused1[468];		//0x0026		468		unused (0xff)
	u16 UpdateCounter;		//0x01fa		2		update Counter (?, probably unused)
	u16 Checksum;			//0x01fc		2		Additive Checksum
	u16 Checksum_Inv;		//0x01fe		2		Inverse Checksum
	u8 Unused2[7680];		//0x0200		0x1e00	unused (0xff)
 } __attribute__((__packed__)) Header;

void getserial(u8 *serial);
s8 BackupRawImage(s32 slot, s32 *bytes_writen);
s8 RestoreRawImage(s32 slot, char *sdfilename, s32 *bytes_writen);
