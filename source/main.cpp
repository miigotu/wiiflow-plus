
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
extern "C" void HideWaitMessage() {mainMenu->_hideWaitMessage(); }

void parse_ios_arg(int arg, int *ios, int *min_rev)
{
	*ios = arg;
	gprintf("Passed IOS: %i\n", *ios);
	switch (arg)
	{
		case 222: *min_rev = IOS_222_MIN_REV; break;
		case 223: *min_rev = IOS_223_MIN_REV; break;
		case 224: *min_rev = IOS_224_MIN_REV; break;
		case 249: *min_rev = IOS_249_MIN_REV; break;
		case 250: *min_rev = IOS_250_MIN_REV; break;
		default:  *min_rev = IOS_ODD_MIN_REV; break;
	}
	gprintf("Passed IOS Minimum Rev: %i\n", *min_rev);
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
	
	char *gameid = NULL;
	
	for (int i = 0; i < argc; i++)
	{
		if (argv[i] != NULL && strcasestr(argv[i], "ios=") != NULL)
		{
			//atoi returns 0 if the first byte is not numeric or whitespace, so move the pointer to the first digit.
			while(!isdigit(argv[i][0])) argv[i]++;
			//Dont allow ios reload attempts to slots the wii doesnt have
			if (atoi(argv[i]) < 254 && atoi(argv[i]) > 0)
				parse_ios_arg(atoi(argv[i]), &mainIOS, &mainIOSminRev);
		}
		else if (strlen(argv[i]) == 6)
		{
			gameid = argv[i];
			for (int i=0; i<5; i++)
				if (!isalnum(gameid[i]))
					gameid = NULL;
		}
	}
	
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

	// Init video
	vid.init();
	// Init
	STexture texWait;
	STexture texWaitHDD;
	texWait.fromPNG(wait_png, GX_TF_RGB565, ALLOC_MALLOC);
	vid.waitMessage(texWait);
	texWaitHDD.fromPNG(wait_hdd_png, GX_TF_RGB565, ALLOC_MALLOC);
	Sys_Init();
	Sys_ExitTo(0);
	
	WPAD_Init();
	PAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
	
	if (iosOK)
	{
		Mount_Devices();
				
		wbfsOK = WBFS_Init(WBFS_DEVICE_USB, 1) >= 0;
		if (!wbfsOK)
		{
			// Show HDD Wait Screen
			vid.waitMessage(texWaitHDD);

			while(!wbfsOK)
			{

				
				while(!((__io_usbstorage.startup() && __io_usbstorage.isInserted()))) //Wait indefinitely until HDD is there or exit requested.
				{
					__io_usbstorage.shutdown();

					WPAD_ScanPads(); PAD_ScanPads();
					u32 wbtnsPressed = 0, gbtnsPressed = 0;
					for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
					{
						wbtnsPressed |= WPAD_ButtonsDown(chan);
						gbtnsPressed |= PAD_ButtonsDown(chan);
					}

					if (Sys_Exiting() || (wbtnsPressed & WBTN_HOME) != 0 || (gbtnsPressed & BTN_HOME) != 0)
					Sys_Exit(0);
				}

				FS_Unmount_USB();
				Mount_Devices();

				wbfsOK = WBFS_Init(WBFS_DEVICE_USB, 1) >= 0;
				if (wbfsOK)
					break;
			}
		}
	}
	dipOK = Disc_Init() >= 0;
	vid.waitMessage(texWait);
	texWait.data.release();
	texWaitHDD.data.release();
	MEM2_takeBigOnes(true);
	do
	{
		Mount_Devices();
				
		gprintf("SD Available: %d\n", FS_SDAvailable());
		gprintf("USB Available: %d\n", FS_USBAvailable());

		CMenu menu(vid);
		menu.init();
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

