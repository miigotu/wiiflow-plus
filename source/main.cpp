
#include "video.hpp"
#include "menu/menu.hpp"
#include "loader/disc.h"
#include "loader/fs.h"
#include "loader/alt_ios.h"
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "text.hpp"
#include <ogc/system.h>
#include "gecko.h"

extern "C"
{
    extern void __exception_setreload(int t);
}

extern const u8 wait_png[];
extern const u8 wait_hdd_png[];

extern bool geckoinit;
extern int mainIOS;
extern int mainIOSminRev;
extern int mainIOSRev;

CMenu *mainMenu;
extern "C" void ShowError(const wstringEx &error){mainMenu->error(error); }

void parse_ios_arg(char *arg, int *ios, int *min_rev)
{
	if(strcasestr(arg, "ios=249") != 0)
	{
		*ios = 249;
		*min_rev = IOS_249_MIN_REV;
	}
	else if(strcasestr(arg, "ios=250") != 0)
	{
		*ios = 250;
		*min_rev = IOS_250_MIN_REV;
	}
	else if(strcasestr(arg, "ios=222-mload") != 0)
	{
		*ios = 222;
		*min_rev = IOS_222_MIN_REV;
	}
	else if(strcasestr(arg, "ios=223-mload") != 0)
	{
		*ios = 223;
		*min_rev = IOS_223_MIN_REV;
	}
	else if(strcasestr(arg, "ios=224-mload") != 0)
	{
		*ios = 224;
		*min_rev = IOS_224_MIN_REV;
	}
}

int old_main(int argc, char **argv)
{
	geckoinit = InitGecko();
	__exception_setreload(5);

	SYS_SetArena1Hi((void *)0x81200000);	// See loader/apploader.c
	CVideo vid;
	bool iosOK = false;
	bool dipOK = false;
	bool wbfsOK = false;
	int ret = 0;
	bool hbc;
	
	char *gameid = NULL;
	
	// Narolez: check if ios argument is passed in argv[0] (by NForwarder) or argv[1] (by wiiload or whatever)
	for (int i = 0; i < argc; i++)
	{
		if (argv[i] != NULL && strcasestr(argv[i], "ios=") != 0)
		{
			parse_ios_arg(argv[i], &mainIOS, &mainIOSminRev);
		}
		else if (strlen(argv[i]) == 6)
		{
			gameid = argv[i];
		}
	}
	
	// Mount_Devices(); // Wake up certain drives

	gprintf("Loading cIOS: %d\n", mainIOS);

	// Load (passed) Custom IOS
	iosOK = loadIOS(mainIOS, false);
	mainIOSRev = IOS_GetRevision();
	iosOK = iosOK && mainIOSRev >= mainIOSminRev;

	// MEM2 usage :
	// 36 MB for general purpose allocations
	// 12 MB for covers (and temporary buffers)
	// adds 15 MB from MEM1 to obtain 27 MB for covers (about 150 HQ covers on screen)
	MEM2_init(36, 12);	// Max ~48

	// Launched through the HBC?
    hbc = *((u32 *) 0x80001804) == 0x53545542 && *((u32 *) 0x80001808) == 0x48415858;

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

	if (iosOK)
	{
		Mount_Devices(); // this will power up the drive if it is not ready
		
		gprintf("SD Available: %d\n", FS_SDAvailable());
		gprintf("USB Available: %d\n", FS_USBAvailable());
				
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
	PAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
	MEM2_takeBigOnes(true);
	do
	{
		Mount_Devices();
		CMenu menu(vid);
		menu.init(hbc);
		mainMenu = &menu;
		//
		if (!iosOK)
			menu.error(sfmt("IOS %i rev%i or later is required", mainIOS, mainIOSminRev));
		else if (!dipOK)
			menu.error(L"Could not initialize DIP module!");
		else if (!wbfsOK)
			menu.error(L"WBFS_Init failed");
		else
		{
			if (gameid != NULL && strlen(gameid) == 6)
			{
				menu._directlaunch(gameid);
			}
			else
			{
				ret = menu.main();
			}
		}
		vid.cleanup();
		Unmount_All_Devices();
	} while (ret == 1);
	return ret;
};

int main(int argc, char **argv)
{
	Sys_Exit(old_main(argc, argv));
	return 0;
}

