
#include <wiiuse/wpad.h>
#include "video.hpp"
#include "menu/menu.hpp"
#include "loader/disc.h"
#include "loader/fat.h"
#include "loader/alt_ios.h"
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "text.hpp"
#include <ogc/system.h>

extern const u8 wait_png[];
extern const u8 wait_hdd_png[];

extern int mainIOS;
extern int mainIOSminRev;

int old_main(int argc, char **argv)
{
	SYS_SetArena1Hi((void *)0x81200000);	// See loader/apploader.c
	CVideo vid;
	bool iosOK = false;
	bool dipOK = false;
	bool wbfsOK = false;
	int ret = 0;
	bool hbc;
	
	// Narolez: check if ios argument is passed in argv[1]
	if (argc > 1 && argv[1] != NULL && strcasestr(argv[1], "ios=") != 0)
	{
		if(strcasestr(argv[1], "ios=249") != 0)
		{
			mainIOS = 249;
			mainIOSminRev = IOS_249_MIN_REV;
		}
		else if(strcasestr(argv[1], "ios=222-mload") != 0)
		{
			mainIOS = 222;
			mainIOSminRev = IOS_222_MIN_REV;
		}
		else if(strcasestr(argv[1], "ios=223-mload") != 0)
		{
			mainIOS = 223;
			mainIOSminRev = IOS_223_MIN_REV;
		}
	}
	
	// Load (passed) Custom IOS
	iosOK = loadIOS(mainIOS, false) && IOS_GetRevision() >= mainIOSminRev;

	// Launched through the HBC?
    hbc = *((u32 *) 0x80001804) == 0x53545542 && *((u32 *) 0x80001808) == 0x48415858;
	
	// MEM2 usage :
	// 36 MB for general purpose allocations
	// 12 MB for covers (and temporary buffers)
	// adds 15 MB from MEM1 to obtain 27 MB for covers (about 150 HQ covers on screen)
	MEM2_init(36, 12);	// Max ~48
	// Init video
	vid.init();
	// Init
	STexture texWait;
	STexture texWaitHDD;
	texWait.fromPNG(wait_png, GX_TF_RGB565, ALLOC_MALLOC);
	vid.waitMessage(texWait);
	texWaitHDD.fromPNG(wait_hdd_png, GX_TF_RGB565, ALLOC_MALLOC);
	Sys_Init();
	Sys_ExitToWiiMenu(true);
	// 
	if (iosOK)
	{
		wbfsOK = WBFS_Init(WBFS_DEVICE_USB, 1) >= 0;
		if (!wbfsOK)
		{
			// Wait for HDD
			vid.waitMessage(texWaitHDD);
			for (int i = 0; i < 40; ++i)
			{
				iosOK = loadIOS(mainIOS, false);
				if (!iosOK)
					break;
				wbfsOK = WBFS_Init(WBFS_DEVICE_USB, 1) >= 0;
				if (wbfsOK)
					break;
				if (Sys_Exiting())
					Sys_Exit(0);
			}
		}
	}
	dipOK = Disc_Init() >= 0;
	vid.waitMessage(texWait);
	texWait.data.release();
	texWaitHDD.data.release();
	WPAD_Init();
	WPAD_SetDataFormat(0, WPAD_FMT_BTNS_ACC_IR);
	MEM2_takeBigOnes(true);
	do
	{
		Fat_Mount();
		CMenu menu(vid);
		menu.init(hbc);
		// 
		if (!iosOK)
			menu.error(sfmt("IOS %i rev%i or later is required", mainIOS, mainIOSminRev));
		else if (!dipOK)
			menu.error(L"Could not initialize DIP module!");
		else if (!wbfsOK)
			menu.error(L"WBFS_Init failed");
		else
		{
			ret = menu.main();
		}
		vid.cleanup();
		Fat_Unmount();
	} while (ret == 1);
	return ret;
};

int main(int argc, char **argv)
{
	Sys_Exit(old_main(argc, argv));
	return 0;
}
