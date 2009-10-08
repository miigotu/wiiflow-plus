#ifndef _WBFS_H_
#define _WBFS_H_

#include "libwbfs/libwbfs.h"

/* Device list */
enum {
	WBFS_DEVICE_USB = 1,	/* USB device */
	WBFS_DEVICE_SDHC	/* SDHC device */
};

/* Macros */
#define WBFS_MIN_DEVICE		1
#define WBFS_MAX_DEVICE		2

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Prototypes */
s32 WBFS_Init(u32, u32);
s32 WBFS_Open(void);
s32 WBFS_Format(u32, u32);
s32 WBFS_GetCount(u32 *);
s32 WBFS_GetHeaders(void *, u32, u32);
s32 WBFS_CheckGame(u8 *);
s32 WBFS_RemoveGame(u8 *);
s32 WBFS_GameSize(u8 *, f32 *);
s32 WBFS_DiskSpace(f32 *, f32 *);
wbfs_t *WBFS_GetHandle(void);
s32 __WBFS_ReadDVD(void *, u32, u32, void *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
