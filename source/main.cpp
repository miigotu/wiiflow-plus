#include "video.hpp"
#include "menu/menu.hpp"
#include "loader/disc.h"
#include "loader/fs.h"
#include "loader/alt_ios.h"
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "text.hpp"
#include <ogc/system.h>
#include <wiilight.h>
#include "gecko.h"

extern "C"
{
    extern void __exception_setreload(int t);
}

extern const u8 wait_hdd_png[];

extern bool geckoinit;
extern int mainIOS;
extern int mainIOSRev;
extern int mainIOSminRev;

CMenu *mainMenu;
extern "C" void ShowError(const wstringEx &error){mainMenu->error(error); }
extern "C" void HideWaitMessage() {mainMenu->_hideWaitMessage(); } //Crashes if used before mainMenu = &menu @ line 163

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

bool use_port1 = false; //Set port you want to use here for now.

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

	// Load Custom IOS
	iosOK = loadIOS(mainIOS, false, false);
	mainIOSRev = IOS_GetRevision();
	iosOK = iosOK && mainIOSRev >= mainIOSminRev;

	// MEM2 usage :
	// 36 MB for general purpose allocations
	// 12 MB for covers (and temporary buffers)
	// adds 15 MB from MEM1 to obtain 27 MB for covers (about 150 HQ covers on screen)
	MEM2_init(36, 12);	// Max ~48

	// Init video
	vid.init();
	WIILIGHT_Init();

	vid.waitMessage(0.2f);

	// Init
	Sys_Init();
	Sys_ExitTo(0);

	if (iosOK)
	{
		Mount_Devices();
		wbfsOK = WBFS_Init(WBFS_DEVICE_USB, 1) >= 0;

/*  		if (!wbfsOK && is_ios_type(IOS_TYPE_HERMES) && !FS_Mount_USB()) //Try swapping here first to avoid HDD Wait screen.
		{
			use_port1 = !use_port1;
			loadIOS(mainIOS, false, true); //Reload the EHC module.
			if (FS_Mount_USB())
				wbfsOK = WBFS_Init(WBFS_DEVICE_USB, 1) >= 0;
		}
*/		if (!wbfsOK)
		{
			//s16 switch_port = -2;

			// Show HDD Wait Screen
			STexture texWaitHDD;
			texWaitHDD.fromPNG(wait_hdd_png, GX_TF_RGB565, ALLOC_MALLOC);
			vid.hideWaitMessage();
			vid.waitMessage(texWaitHDD);

			while(!wbfsOK)
			{
				wbfsOK = WBFS_Init(WBFS_DEVICE_USB, 1) >= 0;
				if(!wbfsOK)
				{
					while(!FS_Mount_USB()) //Wait indefinitely until HDD is there or exit requested.
					{
						//switch_port++;
						WPAD_ScanPads(); PAD_ScanPads();
						u32 wbtnsPressed = 0, gbtnsPressed = 0;
						for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
						{
							wbtnsPressed |= WPAD_ButtonsDown(chan);
							gbtnsPressed |= PAD_ButtonsDown(chan);
						 }

						if (Sys_Exiting() || (wbtnsPressed & WBTN_HOME) != 0 || (gbtnsPressed & GBTN_HOME) != 0)
							Sys_Exit(0);

/* 						if (is_ios_type(IOS_TYPE_HERMES) && (switch_port >= 250 || switch_port == -1))
						{
							use_port1 = !use_port1;
							loadIOS(mainIOS, false, true);
						}
						if (switch_port >= 250)
							switch_port = 0; */
						VIDEO_WaitVSync();
					}
				}
			}
			vid.hideWaitMessage();
			vid.waitMessage(0.2f);
			texWaitHDD.data.release();
		}
	}

	dipOK = Disc_Init() >= 0;
	MEM2_takeBigOnes(true);
	do
	{
		gprintf("SD Available: %d\n", FS_SDAvailable());
		gprintf("USB Available: %d\n", FS_USBAvailable());
		Mount_Devices(); //Why the hell does it need to mount devices again when the above gprintf's are both true?  Without this we have a dump

		CMenu menu(vid);
		menu.init();
		mainMenu = &menu;
		if (!iosOK)
			menu.error(sfmt("IOS %i rev%i or later is required", mainIOS, mainIOSminRev));
		else if (!dipOK)
			menu.error(L"Could not initialize the DIP module!");
		else if (!wbfsOK)
			menu.error(L"WBFS_Init failed");
		else
		{
			if (gameid != NULL && strlen(gameid) == 6)
				menu._directlaunch(gameid);
			else
				ret = menu.main();
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

