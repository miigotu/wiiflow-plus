#include "utils.h"
#include "fat.h"
#include "ntfs.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef FFS_UTILS
#define FFS_UTILS

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

static s32 scan_for_shared2(const char *dirpath);
void scan_for_shared(bool is_usb)

int FFS_Install_Wad(char *filename, bool is_usb);
void FFS_Install(bool is_usb);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* FFS_UTILS */