// by oggzee

#ifndef _WBFS_FAT_H
#define _WBFS_FAT_H

#include "libwbfs/libwbfs.h"

#ifdef __cplusplus
extern "C" {
#endif

wbfs_t* WBFS_FAT_OpenPart(char *fname);
void WBFS_FAT_ClosePart(wbfs_t* part);
s32  WBFS_FAT_GetCount(u32 *count);
s32  WBFS_FAT_GetHeaders(void *outbuf, u32 cnt, u32 len);
wbfs_disc_t* WBFS_FAT_OpenDisc(u8 *discid);
void WBFS_FAT_CloseDisc(wbfs_disc_t* disc);
s32  WBFS_FAT_DiskSpace(f32 *used, f32 *free);
s32  WBFS_FAT_RemoveGame(u8 *discid);
s32  WBFS_FAT_AddGame(progress_callback_t spinner, void *spinner_data);
s32  WBFS_FAT_DVD_Size(u64 *comp_size, u64 *real_size);
int  WBFS_FAT_find_fname(u8 *id, char *fname, int len);

bool WBFS_FAT_IsReadOnly(void);

#ifdef __cplusplus
}
#endif

#endif
