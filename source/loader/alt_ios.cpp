// mload from uloader by Hermes

#include "mload.h"
#include "ehcmodule_5.h"
#include "dip_plugin_249.h"
#include "odip_frag.h"
#include "DeviceHandler.hpp"
#include "wdvd.h"
#include "disc.h"
#include "usbstorage.h"
#include "mem2.hpp"
#include "alt_ios.h"
#include "ios_base.h"
#include "mload_modules.h"
#include "sys.h"
#include "wbfs.h"

#include <malloc.h>

#include "gecko.h"

extern "C" {extern u8 currentPartition;}

int mainIOSRev = 0;

static int load_ehc_module_ex(void)
{
	ehcmodule = ehcmodule_5;
	size_ehcmodule = size_ehcmodule_5;
	dip_plugin = odip_frag;
	size_dip_plugin = size_odip_frag;

	u8 *ehc_cfg = search_for_ehcmodule_cfg((u8 *)ehcmodule, size_ehcmodule);
	if (ehc_cfg)
	{
		ehc_cfg += 12;
		ehc_cfg[0] = use_port1;
		gprintf("EHC Port info = %i\n", ehc_cfg[0]);
		DCFlushRange((void *) (((u32)ehc_cfg[0]) & ~31), 32);
	}
	if(use_port1)	// release port 0 and use port 1
	{
		u32 dat=0;
		u32 addr;

		// get EHCI base registers
		mload_getw((void *) 0x0D040000, &addr);

		addr&=0xff;
		addr+=0x0D040000;
		
		mload_getw((void *) (addr+0x44), &dat);
		if((dat & 0x2001)==1) 
			mload_setw((void *) (addr+0x44), 0x2000);
			
		mload_getw((void *) (addr+0x48), &dat);
		
		if((dat & 0x2000)==0x2000)
			mload_setw((void *) (addr+0x48), 0x1001);

	}
	load_ehc_module();

	return 0;
}

void load_dip_249()
{
	int ret;
	if (is_ios_type(IOS_TYPE_WANIN) && IOS_GetRevision() >= 17)
	{
		gprintf("Starting mload\n");
		if(mload_init()<0)
			return;

//		mload_set_gecko_debug();
		gprintf("Loading 249 dip...");
		ret = mload_module((void *) dip_plugin_249, size_dip_plugin_249);
		gprintf("%d\n", ret);
		mload_close();
	}
}

bool loadIOS(int ios, bool launch_game)
{
	bool iosOK;
	int partition = currentPartition;

	Close_Inputs();

	DeviceHandler::Instance()->UnMountAll();

	WDVD_Close();
	USBStorage_Deinit();

	mload_close();


	gprintf("Reloading into IOS %i...", ios);
	iosOK = IOS_ReloadIOS(ios) == 0;
	gprintf("%s, Current IOS: %i\n", iosOK ? "OK" : "FAILED!", IOS_GetVersion());

	if (iosOK && is_ios_type(IOS_TYPE_HERMES))
		load_ehc_module_ex();
	else if (iosOK && is_ios_type(IOS_TYPE_WANIN))
		load_dip_249();

 	if (launch_game)
	{
		DeviceHandler::Instance()->MountAll();
		DeviceHandler::Instance()->Open_WBFS(partition);
		Disc_Init();
	}
	else Open_Inputs();

	return iosOK;
}

u32 get_ios_base()
{
	u32 revision = IOS_GetRevision();
	if (is_ios_type(IOS_TYPE_WANIN) && revision >= 17)
	{
		u32 index = get_ios_info_from_tmd();

		if (index != 0xFF) return atoi(bases[index]);
		else return wanin_mload_get_IOS_base();
	} 
	else if (is_ios_type(IOS_TYPE_HERMES) && revision >= 4)
		return mload_get_IOS_base();

	return 0;
}