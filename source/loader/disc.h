#ifndef _DISC_H_
#define _DISC_H_

#ifndef APPLOADER_START		/* Also defined in mem2.hpp */
#define APPLOADER_START (void *)0x81200000
#endif
#ifndef APPLOADER_END		/* Also defined in mem2.hpp */
#define APPLOADER_END (void *)0x816FFFF0
#endif

/* Disc header structure */
struct discHdr
{
	/* Game ID */
	u8 id[6];

	/* Game version */
	u16 version;

	/* Audio streaming */
	u8 streaming;
	u8 bufsize;

	/* Padding */
	u64 chantitle; // Used for channels

	/* Sorting */
	u16 index;
	u8 esrb;
	u8 controllers;
	u8 players;
	u8 wifi;

	/* Wii Magic word */
	u32 magic;

	/* GC Magic word */
	u32 gc_magic;

	/* Game title */
	char title[64];

	/* Encryption/Hashing */
	u8 encryption;
	u8 h3_verify;

	/* Padding */
	int casecolor;
	u8 unused3[26];
} ATTRIBUTE_PACKED;

struct dir_discHdr
{
	struct discHdr hdr;
	char path[256];
	wchar_t title[64];
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

	/* Padding */
	u8 unused2[4];

	/* Game title */
	char title[124];
} ATTRIBUTE_PACKED;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	/* Prototypes */
	s32	Disc_Init(void);
	s32	Disc_Open(void);
	s32	Disc_Wait(void);
	s32	Disc_SetUSB(const u8 *);
	s32	Disc_ReadHeader(void *);
	s32 Disc_ReadGCHeader(void *);
	s32 Disc_Type(bool);
	s32	Disc_IsWii(void);
	s32	Disc_IsGC(void);
	s32	Disc_BootPartition(u64, u8, bool, bool, u8, u8);
	s32	Disc_WiiBoot(u8, bool, bool, u8, u8);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

