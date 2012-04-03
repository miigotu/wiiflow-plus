#ifndef _DISC_H_
#define _DISC_H_

#ifndef APPLOADER_START		/* Also defined in mem2.hpp */
#define APPLOADER_START (void *)0x81200000
#endif
#ifndef APPLOADER_END		/* Also defined in mem2.hpp */
#define APPLOADER_END (void *)0x81700000
#endif

#define	Sys_Magic	((vu32*)0x80000020)
#define	Version		((vu32*)0x80000024)
#define	Arena_L		((vu32*)0x80000030)
#define	BI2			((vu32*)0x800000F4)
#define	Bus_Speed	((vu32*)0x800000F8)
#define	CPU_Speed	((vu32*)0x800000Fc)

/* Disc header structure */
struct discHdr
{
	/* Game ID */
	char id[6];

	/* Game version */
	u8 discNumber;
	u8 version;

	/* Audio streaming */
	u8 streaming;
	u8 bufsize;

	/* Padding */
	u8 padding[14];

	/* Wii Magic word */
	u32 magic;

	/* GC Magic word */
	u32 gc_magic;
	/* Game title - Offset 0x20 */
	char title[64];
	/* Encryption/Hashing - Offset 0x60 */
	//u8 encryption;
	//u8 h3_verify;

	/* Padding */
	//u8 unused3[30];
	/* Offset 0x80 */
	//u8 padding3[384]; // Wiibrew says this has a length of 380, which makes no sense.
} ATTRIBUTE_PACKED;
//Total size of a wii disc header is 512 bytes if all members are uncommented.
//We only use it for id and creating coverflow titles, so we dont need to add the padding to the vector.
//I will make the gamelist use a smaller container than this struct eventually since we dont use anyhting
//from it except those two offsets.
//I moved custom fields out of the main structure because the disc read sets those feilds to values that could be invalid.
struct dir_discHdr
{
	char id[7]; // GAMEID + \0
	char path[160]; // usb1:/{char[64] title} [GAMEID]/{char[64] title} [GAMEID].wbfs 
	wchar_t wtitle[65];

	u64 chantitle; // Used for channels

	/* Sorting */
//	u16 index;
//	u8 esrb;
//	u8 controllers;
	u8 players;
	u8 wifi;

	u32 casecolor;
} ATTRIBUTE_PACKED;

struct gc_discHdr
{
	/* Game ID */
	u8 id[6];

	/* Game version */
	u16 version;

	/* Audio streaming */
	u8 streaming;
	u8 bufsize;

	/* Padding */
	u8 unused1[18];

	/* Magic word */
	u32 magic;

	/* Game title */
	char title[64];
	
	/* Padding */
	u8 unused2[64];
} ATTRIBUTE_PACKED;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	/* Prototypes */
	s32	Disc_Init(void);
	s32	Disc_Open(void);
	s32	Disc_Wait(void);
	s32	Disc_SetUSB(char *);
	s32	Disc_ReadHeader(void *);
	s32 Disc_ReadGCHeader(void *);
	s32 Disc_Type(bool);
	s32	Disc_IsWii(void);
	s32	Disc_IsGC(void);
	s32	Disc_BootPartition(u64, u8, bool, bool, u8, int);
	s32	Disc_WiiBoot(u8, bool, bool, u8, int);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

