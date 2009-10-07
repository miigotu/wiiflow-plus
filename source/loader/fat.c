#include <ogcsys.h>
#include <fat.h>

/* Disc interfaces */
extern const DISC_INTERFACE __io_wiisd;
extern const DISC_INTERFACE __io_usbstorage;

static bool g_sdOK = false;
static bool g_usbOK = false;

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
		g_sdOK = fatMountSimple("sd", &__io_wiisd);
	return g_sdOK;
}

bool Fat_Mount(void)
{
	Fat_Unmount();
	if (!g_sdOK)
		g_sdOK = fatMountSimple("sd", &__io_wiisd);
	if (!g_usbOK)
		g_usbOK = fatMount("usb", &__io_usbstorage, 0, 4, 32);
	return g_sdOK || g_usbOK;
}
