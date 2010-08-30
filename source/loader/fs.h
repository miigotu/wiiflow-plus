#ifndef _FAT_H_
#define _FAT_H_

#define APPDATA_DIR		"wiiflow"
#define APPDATA_DIR2	"apps/wiiflow"
#define CFG_FILENAME	"wiiflow.ini"
#define LANG_FILENAME	"languages.ini"
#define CAT_FILENAME	"categories.ini"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Prototypes */
void Unmount_All_Devices(void);
bool Mount_Devices(void);

bool Fat_SDAvailable(void);
bool Fat_USBAvailable(void);
bool Ntfs_SDAvailable(void);
bool Ntfs_USBAvailable(void);

bool Fat_Mount(u32);
void Fat_Unmount(void);
bool Fat_MountSDOnly(void);

bool NTFS_Mount(u32);
void NTFS_Unmount(void);

bool WBFS_Mount(u32);
void WBFS_Unmount(void);


u8 *ISFS_GetFile(u8 *path, u32 *size, s32 length);

extern int   fs_sd_mount;
extern sec_t fs_sd_sec;
extern int   fs_usb_mount;
extern sec_t fs_usb_sec;
extern int   fs_wbfs_mount;
extern sec_t fs_wbfs_sec;
extern int   fs_ntfs_mount;
extern sec_t fs_ntfs_sec;

extern bool g_fat_sdOK;
extern bool g_fat_usbOK;

extern bool g_ntfs_sdOK;
extern bool g_ntfs_usbOK;

extern bool g_wbfsOK;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

