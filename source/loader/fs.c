#include <ogcsys.h>
#include <locale.h>
#include <ogc/isfs.h>
#include "libfat/fat.h"
#include "libntfs/ntfs.h"
#include "libwbfs/libwbfs.h"
#include "usbstorage.h"
#include "sdhc.h"
#include "fs.h"
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

bool g_sdOK = false;
bool g_usbOK = false;
bool g_wbfsOK = false;
bool g_ntfsOK = false;

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
	return g_sdOK;
}

bool Fat_USBAvailable(void)
{
	return g_usbOK;
}

void Fat_Unmount(void)
{
	if (g_sdOK)
	{
		fatUnmount("sd:");
		__io_wiisd.shutdown();
		__io_sdhc.shutdown();	
		g_sdOK = false;
		
		fs_sd_mount = 0;
		fs_sd_sec = 0;
	}
	if (g_usbOK)
	{
		fatUnmount("usb:");
		g_usbOK = false;

		fs_usb_mount = 0;
		fs_usb_sec = 0;
	}
	if (g_wbfsOK)
	{
		fatUnmount("wbfs:/");
		g_wbfsOK = false;

		fs_wbfs_mount = 0;
		fs_wbfs_sec = 0;
	}	
	if (g_ntfsOK)
	{
		ntfsUnmount("ntfs:/", true);
		g_ntfsOK = false;
		
		fs_ntfs_mount = 0;
		fs_ntfs_sec = 0;		
	}
	
	if (!(g_usbOK || g_ntfsOK || g_wbfsOK))
	{
		__io_usbstorage.shutdown();
	}
}

bool Fat_MountSDOnly(void)
{
	if (g_sdOK)
	{
		fatUnmount("sd:");
		__io_wiisd.shutdown();
		__io_sdhc.shutdown();
		g_sdOK = false;
	}
	if (!g_sdOK)
	{
		__io_wiisd.startup();
		g_sdOK = fatMount("sd", &__io_wiisd, 0, CACHE, SECTORS);
	}
	if (!g_sdOK)
	{
		__io_sdhc.startup();
		g_sdOK = fatMount("sd", &__io_sdhc, 0, CACHE, SDHC_SECTOR_SIZE);
	}
	if (g_sdOK)
	{
		fs_sd_mount = 1;
		fs_sd_sec = _FAT_startSector;
	}
	return g_sdOK;
}

bool Fat_Mount(void)
{
	Fat_Unmount();
	if (!g_sdOK) 
	{
		__io_wiisd.startup();
		g_sdOK = fatMount("sd", &__io_wiisd, 0, CACHE, SECTORS);
				
		if (g_sdOK)
		{
			fs_sd_mount = 1;
			fs_sd_sec = _FAT_startSector;
		}
	}
	if (!g_sdOK) 
	{
		__io_sdhc.startup();
		g_sdOK = fatMount("sd", &__io_sdhc, 0, CACHE, SDHC_SECTOR_SIZE);
		
		if (g_sdOK)
		{
			fs_sd_mount = 1;
			fs_sd_sec = _FAT_startSector;
		}
	}
	if (!g_usbOK)
	{
		__io_usbstorage.startup();
		g_usbOK = fatMount("usb", &__io_usbstorage, 0, CACHE, SECTORS);

		if (g_usbOK)
		{
			fs_usb_mount = 1;
			fs_usb_sec = _FAT_startSector;
		}
	}
		
	return g_sdOK || g_usbOK;
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
		//closing all open Files write back the cache and then shutdown em!
		fatUnmount("wbfs:/");
		g_wbfsOK = false;

		fs_wbfs_mount = 0;
		fs_wbfs_sec = 0;
	}
}

void ntfsInit();

bool NTFS_Mount(u32 sector) 
{
	NTFS_Unmount();
	
	if (!g_ntfsOK)
	{
//		_FAT_mem_init();
		ntfsInit(); // Call ntfs init here, to prevent locale resets
		
		// ntfsInit resets locale settings
		// which breaks unicode in console
		// so we change it back to C-UTF-8
		setlocale(LC_CTYPE, "C-UTF-8");
		setlocale(LC_MESSAGES, "C-UTF-8");

		if (wbfsDev == WBFS_DEVICE_USB) {
			/* Initialize WBFS interface */
			__io_usbstorage.startup();

			/* Mount device */
			
			g_ntfsOK = ntfsMount("ntfs", &__io_usbstorage, sector, CACHE, SECTORS, NTFS_SHOW_HIDDEN_FILES | NTFS_RECOVER);
		} else if (wbfsDev == WBFS_DEVICE_SDHC) {
			g_ntfsOK = ntfsMount("ntfs", &__io_sdhc, 0, CACHE, SECTORS, NTFS_SHOW_HIDDEN_FILES | NTFS_RECOVER);
			
/* 			g_ntfsOK = ntfsMount("ntfs", &__io_usbstorage_ro, sector, CACHE, SECTORS, NTFS_DEFAULT);
		} else if (wbfsDev == WBFS_DEVICE_SDHC) {
			g_ntfsOK = ntfsMount("ntfs", &__io_sdhc_ro, 0, CACHE, SECTORS, NTFS_DEFAULT); */
			if (!g_ntfsOK) {
				g_ntfsOK = ntfsMount("ntfs", &__io_wiisd, 0, CACHE, SECTORS_SD, NTFS_DEFAULT);
			}
		}

		if (g_ntfsOK)
		{
			fs_ntfs_mount = 1;
			fs_ntfs_sec = sector;
		}
	}
	return g_ntfsOK;	
}

void NTFS_Unmount()
{
	if (g_ntfsOK)
	{
		/* Unmount device */
		ntfsUnmount("ntfs:", true);
		g_ntfsOK = false;
		
		fs_ntfs_mount = 0;
		fs_ntfs_sec = 0;
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
