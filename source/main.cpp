#include "video.hpp"
#include "menu/menu.hpp"
#include "loader/disc.h"

#include "loader/alt_ios.h"
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "text.hpp"
#include <ogc/system.h>
#include <unistd.h>
#include <wiilight.h>
#include "DeviceHandler.hpp"
#include "homebrew.h"
#include "gecko.h"
#include "wifi_gecko.h"

extern "C" { extern void __exception_setreload(int t);}

extern const u8 wait_hdd_png[];

CMenu *mainMenu;
extern "C" void ShowError(const wstringEx &error){mainMenu->error(error); }
extern "C" void HideWaitMessage() {mainMenu->_hideWaitMessage(); }

void parse_ios_arg(int arg, int *ios, int *min_rev)
{
	*ios = arg;
	gprintf("Passed IOS: %i\n", *ios);
	*min_rev = D2X_MIN_REV;
	gprintf("Passed IOS Minimum Rev: %i\n", *min_rev);
}

int old_main(int argc, char **argv)
{
	geckoinit = InitGecko();
	__exception_setreload(5);

	SYS_SetArena1Hi(APPLOADER_START);	//  Make sure libogc doesnt use appldr area
	CVideo vid;

	char *gameid = NULL;

	for (int i = 0; i < argc; i++)
	{
		if (argv[i] != NULL && strcasestr(argv[i], "ios=") != NULL && strlen(argv[i]) > 4)
		{
			while(argv[i][0] && !isdigit(argv[i][0])) argv[i]++;
			if (atoi(argv[i]) < 254 && atoi(argv[i]) > 0)
				parse_ios_arg(atoi(argv[i]), &mainIOS, &mainIOSminRev);
		}
		else if (argv[i] != NULL && strcasestr(argv[i], "port=") != NULL)
		{
			while(argv[i][0] && !isdigit(argv[i][0])) argv[i]++;
			bool port = atoi(argv[i]);
			if (port <= 1 && port >= 0)
				use_port1 = port;
		}
		else if (strlen(argv[i]) == 6)
		{
			gameid = argv[i];
			for (int i=0; i < 5; i++)
				if (!isalnum(gameid[i]))
					gameid = NULL;
		}
	}
	gprintf("Loading cIOS: %d, Port: %d\n", mainIOS, use_port1);

	// Load Custom IOS
	bool iosOK = loadIOS(mainIOS, false);
	mainIOSRev = IOS_GetRevision();
	iosOK = iosOK && mainIOSRev >= mainIOSminRev;

	SYS_SetArena1Hi(APPLOADER_START);	// Make sure libogc doesnt use appldr area after ios reload.
	MEM2_init(52);

	// Init video
	vid.init();
	WIILIGHT_Init();

	vid.waitMessage(0.2f);

	// Init
	Sys_Init();
	Sys_ExitTo(EXIT_TO_HBC);

	int ret = 0;

	do 
	{
		Open_Inputs();

		bool HddMounted = false;
		DeviceHandler::Instance()->MountAll();
		for(int i = USB1; i <= USB8; i++)
			if(DeviceHandler::Instance()->IsInserted(i))
				HddMounted = true;
  		if(!HddMounted)
		{
			STexture texWaitHDD;
			texWaitHDD.fromPNG(wait_hdd_png, GX_TF_RGB565, ALLOC_MALLOC);
			vid.hideWaitMessage();
			vid.waitMessage(texWaitHDD);

			while(!HddMounted) //Wait indefinitely until HDD is there or exit requested.
			{
				WPAD_ScanPads(); PAD_ScanPads();

				u32 wbtnsPressed = 0, gbtnsPressed = 0,
					wbtnsHeld = 0, gbtnsHeld = 0;

				for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
				{
					wbtnsPressed |= WPAD_ButtonsDown(chan);
					gbtnsPressed |= PAD_ButtonsDown(chan);

					wbtnsHeld |= WPAD_ButtonsHeld(chan);
					gbtnsHeld |= PAD_ButtonsHeld(chan);
				 }

				if (Sys_Exiting() || (wbtnsPressed & WBTN_HOME) || (gbtnsPressed & GBTN_HOME))
				{
					DeviceHandler::Instance()->UnMountAll();
					Sys_ExitTo(1);
					Sys_Exit(0);
				}

				VIDEO_WaitVSync();
				VIDEO_WaitVSync();

				for(int i = USB1; i <= USB8; i++)
					if(DeviceHandler::Instance()->IsInserted(i))
					{
						gprintf("%s is Available\n", DeviceName[i]);
					}

				for(int i = USB1; i <= USB8; i++)
					if(DeviceHandler::Instance()->IsInserted(i))
						HddMounted = true;

				if(!HddMounted) DeviceHandler::Instance()->MountAll();

			}
			vid.hideWaitMessage();
			vid.waitMessage(0.2f);
			SMART_FREE(texWaitHDD.data);
		}
		for(int i = 1; i < MAXDEVICES; i++)
		{
			if(DeviceHandler::Instance()->IsInserted(i))
			{
				gprintf("%s is Available.\n", DeviceName[i]);
			}
		}

		ISFS_Initialize();
		bool dipOK = Disc_Init() >= 0;

		CMenu menu(vid);
		menu.init();
		mainMenu = &menu;
		if (!iosOK)
		{
			menu.error(sfmt("IOS %i rev%i or later is required", mainIOS, mainIOSminRev));
			break;
		}
		else if (!dipOK)
		{
			menu.error(L"Could not initialize the DIP module!");
			break;
		}
		else
		{
			if (gameid != NULL && strlen(gameid) == 6)
				menu._directlaunch(gameid);
			else
				ret = menu.main();
		}
		vid.cleanup();
		if (bootHB)
		{
			IOS_ReloadIOS(58);
			BootHomebrew();
		}

	} while (ret == 1);
	
	WifiGecko_Close();

	return ret;
};

int main(int argc, char **argv)
{
	Sys_Exit(old_main(argc, argv));
	return 0;
}

