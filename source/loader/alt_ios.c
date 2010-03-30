// mload from uloader by Hermes

#include "mload.h"
#include "ehcmodule_2.h"
#include "dip_plugin_2.h"
#include "ehcmodule_3.h"
#include "dip_plugin_3.h"
#include "ehcmodule_frag.h"
#include "dip_plugin_4.h"
#include "dip_plugin_249.h"
#include "fs.h"
#include "wdvd.h"
#include "disc.h"
#include "usbstorage.h"
#include "mem2.hpp"
#include "alt_ios.h"
#include "mload_modules.h"
#include "sys.h"

#include <malloc.h>
#include <wiiuse/wpad.h>

#include "alt_ios_gen.h"

#include "gecko.h"

#define FMT_EHCMODULE_PATH	"sd:/wiiflow/ehcmodule%i.elf"

extern int __Arena2Lo;

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

void load_dip_249()
{
	gprintf("load_dip_249\n");
	int ret;
	if (is_ios_type(IOS_TYPE_WANIN) && IOS_GetRevision() >= 18)
	{
		gprintf("mload_init\n");
		if(mload_init()<0) {
			return;
		}
		gprintf("mload_module\n");
		ret = mload_module(dip_plugin_249, size_dip_plugin_249);
		gprintf("mload_close\n");
		mload_close();
	}
	gprintf("mload done\n");
}

void try_hello()
{
	int ret;
	gprintf("mload init\n");
	if(mload_init()<0) {
		sleep(3);
		return;
	}
	u32 base;
	int size;
   	mload_get_load_base(&base, &size);
	gprintf("base: %08x %x\n", base, size);
	mload_close();
	gprintf("disc init:\n");
	ret = Disc_Init();
	gprintf("= %d\n", ret);
	u32 x = 6;
	s32 WDVD_hello(u32 *status);
	ret = WDVD_hello(&x);
	gprintf("hello: %d %x %d\n", ret, x, x);
	ret = WDVD_hello(&x);
	gprintf("hello: %d %x %d\n", ret, x, x);
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
	if (iosOK)
	{
		if (is_ios_type(IOS_TYPE_HERMES))
			load_ehc_module_ex();
		else if (is_ios_type(IOS_TYPE_WANIN))
		{
			load_dip_249();
//			try_hello();
		}
	}
	if (init)
	{
		Fat_Mount();
		Disc_Init();
		WPAD_Init();
		WPAD_SetDataFormat(0, WPAD_FMT_BTNS_ACC_IR);
	}
	return iosOK;
}
