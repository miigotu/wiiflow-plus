
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

// read-only
extern const DISC_INTERFACE __io_sdhc_ro;
extern DISC_INTERFACE __io_usbstorage_ro;

//void _FAT_mem_init();
extern sec_t _FAT_startSector;

extern s32 wbfsDev;
extern s32 InitPartitionList();
extern PartList plist;

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

int   fs_usb_mount = 0;
sec_t fs_usb_sec = 0;

int   fs_wbfs_mount = 0;
sec_t fs_wbfs_sec = 0;

int   fs_ntfs_mount = 0;
sec_t fs_ntfs_sec = 0;

bool Fat_SDAvailable(void)
{
	return g_fat_sdOK;
}

bool Fat_USBAvailable(void)
{
	return g_fat_usbOK;
}

bool Ntfs_SDAvailable(void)
{
	return g_ntfs_sdOK;
}

bool Ntfs_USBAvailable(void)
{
	return g_ntfs_usbOK;
}

void Unmount_All_Devices(void)
{
	Fat_Unmount();
	NTFS_Unmount();
	WBFS_Unmount();
}

bool Mount_Devices(void)
{
	u8 i;
	u32 fat_sector = 0, ntfs_sector = 0;
	bool fat_found = false;
	bool ntfs_found = false;
	s32 ret = InitPartitionList();
	if (ret || plist.num == 0) return false;
	for (i=0; i<plist.num; i++) 
	{
		if (i > plist.fat_n) break;
		if (plist.pentry[i].type == 0x0b || plist.pentry[i].type == 0x0c)
		{
			fat_sector = plist.pentry[i].sector;
			fat_found = true;
			break;
		}
	}
	for (i=0; i<plist.num; i++) 
	{
		gprintf("plist.pentry[%i].type = %d\n", i, plist.pentry[i].type);
		if (i > plist.ntfs_n) break;
		if (plist.pentry[i].type == 0x07)
		{
			gprintf("plist.pentry[%i].sector = %i\n", i, plist.pentry[i].sector);
			ntfs_sector = plist.pentry[i].sector;
			ntfs_found = true;
			break;
		}
	}
 	if (!ntfs_found && !fat_found)
	{
		for (i=0; i<plist.num; i++) 
		{
			if (i > plist.fat_n) break;
			if (plist.pentry[i].type == 0x04 || plist.pentry[i].type == 0x06 || plist.pentry[i].type == 0x0e)
			{
				fat_sector = plist.pentry[i].sector;
				fat_found = true;
				break;
			}
		}
	}
	if (fat_found)
	{
		Fat_Mount(fat_sector);
		return true;
	}
	if (ntfs_found)
	{
		Fat_MountSDOnly();
		NTFS_Mount(ntfs_sector);
		return true;
	}
	return false;
}

void Fat_Unmount(void)
{
	if (g_fat_sdOK)
	{
		fatUnmount("sd:");
		__io_wiisd.shutdown();
		__io_sdhc.shutdown();	
		g_fat_sdOK = false;
		
		fs_sd_mount = 0;
		fs_sd_sec = 0;
	}
	if (g_fat_usbOK)
	{
		fatUnmount("usb:");
		g_fat_usbOK = false;

		fs_usb_mount = 0;
		fs_usb_sec = 0;
	}
	if (!(g_fat_usbOK || g_ntfs_usbOK || g_wbfsOK))
	{
		__io_usbstorage.shutdown();
	}
	if (!(g_fat_sdOK || g_ntfs_sdOK))
	{
		__io_wiisd.shutdown();
		__io_sdhc.shutdown();
	}
}

bool Fat_Mount(u32 sector)
{
	Fat_Unmount();
	if (!g_fat_sdOK) 
	{
		__io_wiisd.startup();
		g_fat_sdOK = fatMount("sd", &__io_wiisd, 0, CACHE, SECTORS);
				
		if (g_fat_sdOK)
		{
			fs_sd_mount = 1;
			fs_sd_sec = _FAT_startSector;
		}
	}
	if (!g_fat_sdOK) 
	{
		__io_sdhc.startup();
		g_fat_sdOK = fatMount("sd", &__io_sdhc, 0, CACHE, SDHC_SECTOR_SIZE);
		
		if (g_fat_sdOK)
		{
			fs_sd_mount = 1;
			fs_sd_sec = _FAT_startSector;
		}
	}
	if (!g_fat_usbOK)
	{
		__io_usbstorage.startup();
		g_fat_usbOK = fatMount("usb", &__io_usbstorage, sector, CACHE, SECTORS);

		if (g_fat_usbOK)
		{
			fs_usb_mount = 1;
			fs_usb_sec = sector;
		}
	}
		
	return g_fat_sdOK || g_fat_usbOK;
}

bool Fat_MountSDOnly(void)
{
	if (g_fat_sdOK)
	{
		fatUnmount("sd:");
		__io_wiisd.shutdown();
		__io_sdhc.shutdown();
		g_fat_sdOK = false;
	}
	if (!g_fat_sdOK)
	{
		__io_wiisd.startup();
		g_fat_sdOK = fatMount("sd", &__io_wiisd, 0, CACHE, SECTORS);
	}
	if (!g_fat_sdOK)
	{
		__io_sdhc.startup();
		g_fat_sdOK = fatMount("sd", &__io_sdhc, 0, CACHE, SDHC_SECTOR_SIZE);
	}
	if (g_fat_sdOK)
	{
		fs_sd_mount = 1;
		fs_sd_sec = _FAT_startSector;
	}
	return g_fat_sdOK;
}

void ntfsInit();

bool NTFS_Mount(u32 sector) 
{
	NTFS_Unmount();
	if (!g_ntfs_usbOK) 
	{
		__io_usbstorage.startup();
		ntfsInit();
		setlocale(LC_CTYPE, "C-UTF-8");
		setlocale(LC_MESSAGES, "C-UTF-8");
		g_ntfs_usbOK = ntfsMount("ntfs", &__io_usbstorage, sector, CACHE, SECTORS, NTFS_SHOW_HIDDEN_FILES | NTFS_RECOVER);
	} 
	else if (!g_fat_sdOK && !g_ntfs_sdOK && wbfsDev == WBFS_DEVICE_SDHC)
	{
		__io_sdhc.startup();
		ntfsInit();
		setlocale(LC_CTYPE, "C-UTF-8");
		setlocale(LC_MESSAGES, "C-UTF-8");
		ntfsInit();
		g_ntfs_sdOK = ntfsMount("ntfs", &__io_sdhc, 0, CACHE, SECTORS, NTFS_SHOW_HIDDEN_FILES | NTFS_RECOVER);
		if (!g_ntfs_sdOK)
		{
			__io_wiisd.startup();
			g_ntfs_sdOK = ntfsMount("ntfs", &__io_wiisd, 0, CACHE, SECTORS_SD, NTFS_DEFAULT);
		}
	}
	if (g_ntfs_usbOK)
	{
		fs_ntfs_mount = 1;
		fs_ntfs_sec = sector;
		return g_ntfs_usbOK;
	}
	else if (g_ntfs_sdOK)
	{
		fs_ntfs_mount = 1;
		fs_ntfs_sec = 0;
		return g_ntfs_sdOK;
	}

	return false;	
}

void NTFS_Unmount()
{
	if (g_ntfs_usbOK)
	{
		/* Unmount device */
		ntfsUnmount("ntfs:", true);
		g_ntfs_usbOK = false;
		g_ntfs_sdOK = false;
		
		fs_ntfs_mount = 0;
		fs_ntfs_sec = 0;
	}
	if (!(g_fat_usbOK || g_ntfs_usbOK || g_wbfsOK))
		__io_usbstorage.shutdown();
	if (!(g_fat_sdOK || g_ntfs_sdOK))
	{
		__io_wiisd.shutdown();
		__io_sdhc.shutdown();
	}
}

bool WBFS_Mount(u32 sector) 
{
    WBFS_Unmount();

	if (!g_wbfsOK)
	{
		__io_usbstorage.startup();
		g_wbfsOK = fatMount("wbfs", &__io_usbstorage, sector, CACHE, SECTORS);

		if (g_wbfsOK)
		{
			fs_wbfs_mount = 1;
			fs_wbfs_sec = _FAT_startSector;
			if (sector && fs_wbfs_sec != sector) {
				// This is an error situation...actually, but is ignored in Config loader also
				// this is the situation that the mounted partition is not the requested one
			}
		}
	}
	
	return g_wbfsOK;
}

void WBFS_Unmount() 
{
	if (g_wbfsOK)
	{
		fatUnmount("wbfs:/");
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

static void *fat_pool = NULL;
static size_t fat_size;
#define FAT_SLOTS (CACHE * 3)
#define FAT_SLOT_SIZE (512 * SECTORS)
#define FAT_SLOT_SIZE_MIN (512 * SECTORS_SD)
static int fat_alloc[FAT_SLOTS];

void _FAT_mem_init()
{
        if (fat_pool) return;
        fat_size = FAT_SLOTS * FAT_SLOT_SIZE;
        fat_pool = memalign(32, fat_size);
}

void* _FAT_mem_allocate(size_t size)
{
        return malloc(size);
}

void* _FAT_mem_align(size_t size)
{
        if (size < FAT_SLOT_SIZE_MIN || size > FAT_SLOT_SIZE) goto fallback;
        if (fat_pool == NULL) goto fallback;
        int i;
        for (i=0; i<FAT_SLOTS; i++) {
                if (fat_alloc[i] == 0) {
                        void *ptr = fat_pool + i * FAT_SLOT_SIZE;
                        fat_alloc[i] = 1;
                        return ptr;
                }      
        }
        fallback:
        return memalign (32, size);            
}

void _FAT_mem_free(void *mem)
{
        if (fat_pool == NULL || mem < fat_pool || mem >= fat_pool + fat_size) {
                free(mem);
                return;
        }
        int i;
        for (i=0; i<FAT_SLOTS; i++) {
                if (fat_alloc[i]) {
                        void *ptr = fat_pool + i * FAT_SLOT_SIZE;
                        if (mem == ptr) {
                                fat_alloc[i] = 0;
                                return;
                        }
                }      
        }
        // FATAL
}
