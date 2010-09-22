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

#define ALIGN(x) (((x) + 3) & ~3)
#define ALIGN32(x) (((x) + 31) & ~31)

/* Prototypes */
bool FS_SDAvailable(void);
bool FS_USBAvailable(void);
bool FS_USB_isNTFS(void);

bool Wakeup_USB(void);

bool Mount_Devices(void);
void Unmount_All_Devices(bool);

bool FS_Mount_USB(u32, bool);
void FS_Unmount_USB(void);

bool FS_Mount_SD(void);
void FS_Unmount_SD(void);

bool WBFS_Mount(u32, bool);
void WBFS_Unmount(void);

u8 *ISFS_GetFile(u8 *path, u32 *size, s32 length);

extern int 	 global_mount; // 1   -> sd_ok SD was mounted
						   // 2   -> ud_ok  USB was mounted
						   // 128 -> Use NAND Emulation. Used for load_fat_module()

extern int   fs_sd_mount;
extern sec_t fs_sd_sec;
extern int   fs_fat_mount;
extern sec_t fs_fat_sec;
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

