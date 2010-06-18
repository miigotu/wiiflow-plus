#ifndef _DISC_H_
#define _DISC_H_

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
	u8 unused1[6]; // Was 14, but removed 8 for chantitle above

	/* Magic word */
	u32 magic;

	/* Padding */
	u8 unused2[4];

	/* Game title */
	char title[64];

	/* Encryption/Hashing */
	u8 encryption;
	u8 h3_verify;

	/* Padding */
	u8 unused3[30];
} ATTRIBUTE_PACKED;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Prototypes */
s32  Disc_Init(void);
s32  Disc_Open(void);
s32  Disc_Wait(void);
s32  Disc_SetUSB(const u8 *);
s32  Disc_ReadHeader(void *);
s32  Disc_IsWii(void);
s32  Disc_BootPartition(u64, u8, const u8 *, u32, bool, bool, bool, const u8 *, u32, u8, u32, u8);
s32  Disc_WiiBoot(u8, const u8 *, u32, bool, bool, bool, const u8 *, u32, u8, u32, u8);
s32 Disc_OpenPartition(u8 *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

