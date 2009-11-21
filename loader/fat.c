#include <ogcsys.h>
#include <fat.h>

/* Disc interfaces */
extern const DISC_INTERFACE __io_wiisd;
extern DISC_INTERFACE __io_usbstorage;
extern const DISC_INTERFACE __io_sdhc;

static bool g_sdOK = false;
static bool g_usbOK = false;

#define CACHE                8
#define SECTORS             64
#define SDHC_SECTOR_SIZE 0x200

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
		g_sdOK = false;
	}
	if (g_usbOK)
	{
		fatUnmount("usb:");
		g_usbOK = false;
	}
}

bool Fat_MountSDOnly(void)
{
	if (g_sdOK)
	{
		fatUnmount("sd:");
		g_sdOK = false;
	}
	if (!g_sdOK)
		g_sdOK = fatMount("sd", &__io_wiisd, 0, CACHE, SECTORS);
	if (!g_sdOK)
		g_sdOK = fatMount("sd", &__io_sdhc, 0, CACHE, SDHC_SECTOR_SIZE);
	
	return g_sdOK;
}

bool Fat_Mount(void)
{
	Fat_Unmount();
	if (!g_sdOK)
		g_sdOK = fatMount("sd", &__io_wiisd, 0, CACHE, SECTORS);
	if (!g_sdOK)
		g_sdOK = fatMount("sd", &__io_sdhc, 0, CACHE, SDHC_SECTOR_SIZE);
	if (!g_usbOK)
		g_usbOK = fatMount("usb", &__io_usbstorage, 0, CACHE, SECTORS);
		
	return g_sdOK || g_usbOK;
}
