#ifndef _FAT_H_
#define _FAT_H_

#define APPDATA_DIR		"wiiflow"
#define APPDATA_DIR2	"apps/wiiflow"
#define CFG_FILENAME	"wiiflow.ini"
#define LANG_FILENAME	"languages.ini"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Prototypes */
bool Fat_Mount(void);
bool Fat_MountSDOnly(void);
void Fat_Unmount(void);
bool Fat_SDAvailable(void);
bool Fat_USBAvailable(void);
bool WBFS_Mount(u32);
void WBFS_Unmount(void);
bool NTFS_Mount(u32);
void NTFS_Unmount(void);

u8 *ISFS_GetFile(u8 *path, u32 *size, s32 length);

extern int   fs_sd_mount;
extern sec_t fs_sd_sec;
extern int   fs_usb_mount;
extern sec_t fs_usb_sec;
extern int   fs_wbfs_mount;
extern sec_t fs_wbfs_sec;
extern int   fs_ntfs_mount;
extern sec_t fs_ntfs_sec;

extern bool g_sdOK;
extern bool g_usbOK;
extern bool g_wbfsOK;
extern bool g_ntfsOK;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
