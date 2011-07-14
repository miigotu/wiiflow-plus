#include "DeviceHandler.hpp"
#include "wdvd.h"
#include "disc.h"
#include "usbstorage.h"
#include "mem2.hpp"
#include "alt_ios.h"
#include "sys.h"
#include "wbfs.h"

#include <malloc.h>

#include "gecko.h"

extern "C" {extern u8 currentPartition;}

int mainIOSRev = 0;

bool loadIOS(int ios, bool launch_game)
{
	bool iosOK;
	int partition = currentPartition;

	Close_Inputs();

	DeviceHandler::Instance()->UnMountAll();

	WDVD_Close();
	USBStorage_Deinit();

	gprintf("Reloading into IOS %i...", ios);
	iosOK = IOS_ReloadIOS(ios) == 0;
	gprintf("%s, Current IOS: %i\n", iosOK ? "OK" : "FAILED!", IOS_GetVersion());

 	if (launch_game)
	{
		DeviceHandler::Instance()->MountAll();
		DeviceHandler::Instance()->Open_WBFS(partition);
		Disc_Init();
	}
	else Open_Inputs();

	return iosOK;
}