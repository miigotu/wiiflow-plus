
#include <ogcsys.h>
#include <locale.h>
#include <ogc/isfs.h>
#include "libfat/fat.h"
#include "libntfs/ntfs.h"
#include "libwbfs/libwbfs.h"
#include "usbstorage.h"
#include "sdhc.h"
#include "fs.h"
#include "partition.h"
#include "gecko.h"

/* Disc interfaces */
extern const DISC_INTERFACE __io_wiisd;
extern DISC_INTERFACE __io_usbstorage;
extern const DISC_INTERFACE __io_sdhc;

//void _FAT_mem_init();
extern sec_t _FAT_startSector;

extern s32 wbfsDev;
extern s32 InitPartitionList();
extern PartList plist;

void ntfsInit();

bool g_fat_sdOK = false;
bool g_fat_usbOK = false;
bool g_ntfs_sdOK = false;
bool g_ntfs_usbOK = false;
bool g_wbfsOK = false;

#define CACHE   32
#define SECTORS 64
#define SECTORS_SD 32

int   fs_sd_mount = 0;
sec_t fs_sd_sec = 0; // u32

int   fs_fat_mount = 0;
sec_t fs_fat_sec = 0;

int   fs_wbfs_mount = 0;
sec_t fs_wbfs_sec = 0;

int   fs_ntfs_mount = 0;
sec_t fs_ntfs_sec = 0;

bool FS_SDAvailable(void)
{
	return g_fat_sdOK || g_ntfs_sdOK;
}

bool FS_USBAvailable(void)
{
	return g_fat_usbOK || g_ntfs_usbOK;
}

bool FS_USB_isNTFS(void)
{
	return g_ntfs_usbOK;
}

void Unmount_All_Devices(void)
{
	FS_Unmount_USB();
	FS_Unmount_SD();
	WBFS_Unmount();
	ISFS_Deinitialize();
}

bool Mount_Devices(void)
{
	ISFS_Initialize();

	FS_Unmount_SD();
	FS_Unmount_USB();

	ntfsInit();
	setlocale(LC_CTYPE, "C-UTF-8");
	setlocale(LC_MESSAGES, "C-UTF-8");
	int i;
	u32 sector = 0;
	bool ntfs_found = false, fat_found = false;

	FS_Mount_SD();

	s32 ret = InitPartitionList();
	
	if (ret || plist.num == 0) return false;

	for (i=0; i<plist.num; i++)
	{
 		switch(plist.pinfo[i].fs_type)
		{
			case FS_TYPE_WBFS:
				continue;
			case FS_TYPE_NTFS:
				ntfs_found = true;
				sector = plist.pentry[i].sector;
				break;
			case FS_TYPE_FAT32:
			case FS_TYPE_FAT16:
				fat_found = true;
				sector = plist.pentry[i].sector;
				break;
			default: 
				continue;
		}
		if (fat_found || ntfs_found)
			break;
	}
	return FS_Mount_USB(sector, ntfs_found);
}

void FS_Unmount_SD(void)
{
	if (g_fat_sdOK)
	{
		fatUnmount("sd:");
		g_fat_sdOK = false;
	}
	if (g_ntfs_sdOK)
	{
		ntfsUnmount("sd:", true);
		g_ntfs_sdOK = false;
	}
	__io_wiisd.shutdown();
	__io_sdhc.shutdown();	

	fs_sd_mount = 0;
	fs_sd_sec = 0;
}

void FS_Unmount_USB(void)
{
	if (g_fat_usbOK)
	{
		fatUnmount("usb:");
		g_fat_usbOK = false;

		fs_fat_mount = 0;
		fs_fat_sec = 0;
	}
	if (g_ntfs_usbOK)
	{
		ntfsUnmount("usb:", true);
		g_ntfs_usbOK = false;

		fs_ntfs_mount = 0;
		fs_ntfs_sec = 0;
	}
	if (!(g_fat_usbOK || g_ntfs_usbOK || g_wbfsOK))
		__io_usbstorage.shutdown();
}

bool FS_Mount_USB(u32 sector, bool ntfs)
{
	if (!g_fat_usbOK && !g_ntfs_usbOK && !ntfs)
	{
		__io_usbstorage.startup();
		g_fat_usbOK = fatMount("usb", &__io_usbstorage, sector, CACHE, SECTORS);
	}
	if (!g_ntfs_usbOK && !g_fat_usbOK && ntfs)
	{
		__io_usbstorage.startup();
		g_ntfs_usbOK = ntfsMount("usb", &__io_usbstorage, sector, CACHE, SECTORS, NTFS_SHOW_HIDDEN_FILES | NTFS_RECOVER);
	} 
	if (g_fat_usbOK && !ntfs)
	{
		fs_fat_mount = 1;
		fs_fat_sec = _FAT_startSector;
	}
	if (g_ntfs_usbOK && ntfs)
	{
		fs_ntfs_mount = 1;
		fs_ntfs_sec = sector;
	}
		
	return g_ntfs_usbOK || g_fat_usbOK;
}

bool FS_Mount_SD(void)
{
	if (!g_ntfs_sdOK && !g_fat_sdOK)
	{
		__io_wiisd.startup();
		g_fat_sdOK = fatMount("sd", &__io_wiisd, 0, CACHE, SDHC_SECTOR_SIZE);
	}
	if (!g_ntfs_sdOK && !g_fat_sdOK)
		g_ntfs_sdOK = ntfsMount("sd", &__io_wiisd, 0, CACHE, SECTORS, NTFS_SHOW_HIDDEN_FILES | NTFS_RECOVER);

	if (!g_ntfs_sdOK && !g_fat_sdOK)
	{
		__io_wiisd.shutdown();
		__io_sdhc.startup();
		g_fat_sdOK = fatMount("sd", &__io_sdhc, 0, CACHE, SECTORS);
	}
	if (!g_ntfs_sdOK && !g_fat_sdOK)
		g_ntfs_sdOK = ntfsMount("sd", &__io_sdhc, 0, CACHE, SECTORS_SD, NTFS_DEFAULT);

	if (!g_ntfs_sdOK && !g_fat_sdOK)
		__io_sdhc.shutdown();
	
	if (g_fat_sdOK)
	{
		fs_sd_mount = 1;
		fs_sd_sec = _FAT_startSector;
	}
	else if (g_ntfs_sdOK)
	{
		fs_sd_mount = 1;
		fs_sd_sec = 0;
	}
	return g_fat_sdOK || g_ntfs_sdOK;
}

bool WBFS_Mount(u32 sector, bool ntfs) 
{
    WBFS_Unmount();

	if (!g_wbfsOK)
	{
		__io_usbstorage.startup();
		if (!ntfs)
			g_wbfsOK = fatMount("wbfs", &__io_usbstorage, sector, CACHE, SECTORS);
		else
			g_wbfsOK = ntfsMount("wbfs", &__io_usbstorage, sector, CACHE, SECTORS, NTFS_SHOW_HIDDEN_FILES | NTFS_RECOVER);

		if (g_wbfsOK)
		{
			fs_wbfs_mount = 1;
			if (!ntfs)
				fs_wbfs_sec = _FAT_startSector;
			else
				fs_wbfs_sec = sector;
		}
	}
	
	return g_wbfsOK;
}

void WBFS_Unmount() 
{
	if (g_wbfsOK)
	{
		fatUnmount("wbfs:");
		ntfsUnmount("wbfs:", true);
		g_wbfsOK = false;

		fs_wbfs_mount = 0;
		fs_wbfs_sec = 0;
	}
	if (!(g_fat_usbOK || g_ntfs_usbOK || g_wbfsOK))
		__io_usbstorage.shutdown();
	if (!(g_fat_sdOK || g_ntfs_sdOK))
	{
		__io_wiisd.shutdown();
		__io_sdhc.shutdown();
	}
}

#define ALIGN(x) ((x % 32 != 0) ? (x / 32) * 32 + 32 : x)

u8 *ISFS_GetFile(u8 *path, u32 *size, s32 length)
{
	*size = 0;
	
	s32 fd = ISFS_Open((const char *) path, ISFS_OPEN_READ);
	u8 *buf = NULL;
	static fstats stats ATTRIBUTE_ALIGN(32);
	
	if (fd >= 0)
	{
		if (ISFS_GetFileStats(fd, &stats) >= 0)
		{
			if (length <= 0) length = stats.file_length;
			buf = (u8 *) memalign(32, ALIGN(length));
			if (buf != NULL)
			{
				*size = stats.file_length;
				if (ISFS_Read(fd, (void*)buf, length) != length)
				{
					*size = 0;
					free(buf);
					buf = NULL;
				}
			}
		}
		ISFS_Close(fd);
	}
	return buf;
}
