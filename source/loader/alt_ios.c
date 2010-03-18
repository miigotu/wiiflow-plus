// mload from uloader by Hermes

#include "mload.h"
#include "ehcmodule_2.h"
#include "dip_plugin_2.h"
#include "ehcmodule_3.h"
#include "dip_plugin_3.h"
#include "ehcmodule_frag.h"
#include "dip_plugin_4.h"
#include "fs.h"
#include "wdvd.h"
#include "disc.h"
#include "usbstorage.h"
#include "mem2.hpp"
#include "alt_ios.h"
#include "mload_modules.h"

#include <malloc.h>
#include <wiiuse/wpad.h>

#define FMT_EHCMODULE_PATH	"sd:/wiiflow/ehcmodule%i.elf"

extern int __Arena2Lo;

int mainIOS = MAIN_IOS;
int mainIOSminRev = MAIN_IOS_MIN_REV;
int mainIOSRev = 0;

static int load_ehc_module_ex(void)
{
	switch (IOS_GetRevision())
	{
		case 2:
			ehcmodule = ehcmodule_2;
			size_ehcmodule = size_ehcmodule_2;
			dip_plugin = dip_plugin_2;
			size_dip_plugin = size_dip_plugin_2;
			break;
		case 3:
			ehcmodule = ehcmodule_3;
			size_ehcmodule = size_ehcmodule_3;
			dip_plugin = dip_plugin_3;
			size_dip_plugin = size_dip_plugin_3;
			break;
		case 4:
		default:
			ehcmodule = ehcmodule_frag;
			size_ehcmodule = size_ehcmodule_frag;
			dip_plugin = dip_plugin_4;
			size_dip_plugin = size_dip_plugin_4;
			break;
	}
	load_ehc_module();
	return 0;
}

bool loadIOS(int n, bool init)
{
	bool iosOK;

	if (init)
	{
		WPAD_Flush(0);
		WPAD_Disconnect(0);
		WPAD_Shutdown();
		Fat_Unmount();
		WDVD_Close();
		USBStorage_Deinit();
//		if (IOS_GetVersion() == 222 || IOS_GetVersion() == 223)
//			mload_close();
		usleep(500000);
	}
	void *backup = COVER_allocMem1(0x200000);	// 0x126CA0 bytes were needed last time i checked. But take more just in case.
	if (backup != 0)
	{
		memcpy(backup, &__Arena2Lo, 0x200000);
		DCFlushRange(backup, 0x200000);
	}
	iosOK = IOS_ReloadIOS(n) >= 0;
	if (n != 249) sleep(1); // Narolez: sleep after IOS reload lets power down/up the harddisk when cIOS 249 is used!
	if (backup != 0)
	{
		memcpy(&__Arena2Lo, backup, 0x200000);
		DCFlushRange(&__Arena2Lo, 0x200000);
		COVER_free(backup);
	}
	if (iosOK && (n == 222 || n == 223))
		load_ehc_module_ex();
	if (init)
	{
		Fat_Mount();
		Disc_Init();
		WPAD_Init();
		WPAD_SetDataFormat(0, WPAD_FMT_BTNS_ACC_IR);
	}
	return iosOK;
}
