#include "utils.h"
#include "fat.h"
#include "ntfs.h"

#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <ogcsys.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#ifndef FS_UTILS
#define FS_UTILS

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

s32 FS_DeleteDir(const char *dirpath);
int FS_Read_File(const char *filepath, void **buffer, int *length);
int FS_Write_File(const char *filepath, void *buffer, int length);
int FS_Copy_File(const char *filepath_ori, const char *filepath_dest);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif