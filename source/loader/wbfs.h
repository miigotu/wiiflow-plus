#ifndef _WBFS_H_
#define _WBFS_H_

#include "libwbfs/libwbfs.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Macros */
#define WBFS_MIN_DEVICE		1
#define WBFS_MAX_DEVICE		2

#define PART_FS_WBFS 0
#define PART_FS_FAT  1
#define PART_FS_NTFS 2

extern s32 wbfsDev;
extern int wbfs_part_fs;
extern u32 wbfs_part_idx;
extern u32 wbfs_part_lba;
extern char wbfs_fs_drive[16];

/* Prototypes */
s32 WBFS_Init(u32, u32);
s32 WBFS_Format(u32, u32);
s32 WBFS_GetCount(u32 *);
s32 WBFS_GetHeaders(void *, u32, u32);
s32 WBFS_CheckGame(u8 *, char *);
s32 WBFS_AddGame(progress_callback_t spinner, void *spinner_data);
s32 WBFS_RemoveGame(u8 *, char *);
s32 WBFS_GameSize(u8 *, char *, f32 *);
s32 WBFS_DVD_Size(u64 *comp_size, u64 *real_size);
s32 WBFS_DiskSpace(f32 *, f32 *);

s32 WBFS_OpenPart(u32 part_fs, u32 part_idx, u32 part_lba, u32 part_size, char *partition);
wbfs_disc_t* WBFS_OpenDisc(u8 *discid, char *path);
void WBFS_CloseDisc(wbfs_disc_t *disc);
bool WBFS_Close();
bool WBFS_Mounted();

#ifdef __cplusplus
}
#endif

#endif
