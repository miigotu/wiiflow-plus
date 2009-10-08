// mload from uloader by Hermes

#include "mload.h"
#include "ehcmodule_2.h"
#include "dip_plugin_2.h"
#include "ehcmodule_3.h"
#include "dip_plugin_3.h"
#include "ehcmodule_4.h"
#include "dip_plugin_4.h"
#include "fat.h"
#include "wdvd.h"
#include "disc.h"
#include "usbstorage.h"
#include "../mem2.hpp"
#include "alt_ios.h"
#include <malloc.h>
#include <wiiuse/wpad.h>

#define FMT_EHCMODULE_PATH	"sd:/wiiflow/ehcmodule%i.elf"

extern int __Arena2Lo;

int mainIOS = MAIN_IOS;
int mainIOSminRev = MAIN_IOS_MIN_REV;

static u32 ios_36[16] ATTRIBUTE_ALIGN(32) =
{
	0, // DI_EmulateCmd
	0,
	0x2022DDAC, // dvd_read_controlling_data
	0x20201010+1, // handle_di_cmd_reentry (thumb)
	0x20200b9c+1, // ios_shared_alloc_aligned (thumb)
	0x20200b70+1, // ios_shared_free (thumb)
	0x20205dc0+1, // ios_memcpy (thumb)
	0x20200048+1, // ios_fatal_di_error (thumb)
	0x20202b4c+1, // ios_doReadHashEncryptedState (thumb)
	0x20203934+1, // ios_printf (thumb)
};

static u32 ios_38[16] ATTRIBUTE_ALIGN(32) =
{
	0, // DI_EmulateCmd
	0,
	0x2022cdac, // dvd_read_controlling_data
	0x20200d38+1, // handle_di_cmd_reentry (thumb)
	0x202008c4+1, // ios_shared_alloc_aligned (thumb)
	0x20200898+1, // ios_shared_free (thumb)
	0x20205b80+1, // ios_memcpy (thumb)
	0x20200048+1, // ios_fatal_di_error (thumb)
	0x20202874+1, // ios_doReadHashEncryptedState (thumb)
	0x2020365c+1, // ios_printf (thumb)
};

static u32 patch_data[8] ATTRIBUTE_ALIGN(32);

static int load_ehc_module(void)
{
	static void *external_ehcmodule = 0;
	static u32 size_external_ehcmodule = 0;
	int is_ios = 0;
	int my_thread_id = 0;
	data_elf my_data_elf;
	void *ehcmodule = 0;
	u32 size_ehcmodule;
	void *dip_plugin = 0;
	u32 size_dip_plugin;
	char modulePath[sizeof FMT_EHCMODULE_PATH + 4];

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
			ehcmodule = ehcmodule_4;
			size_ehcmodule = size_ehcmodule_4;
			dip_plugin = dip_plugin_4;
			size_dip_plugin = size_dip_plugin_4;
			break;
	}
	if (external_ehcmodule == 0)
	{
		FILE *fp = 0;
		Fat_MountSDOnly();
		snprintf(modulePath, sizeof modulePath, FMT_EHCMODULE_PATH, IOS_GetRevision());
		if (Fat_SDAvailable())
			fp = fopen(modulePath, "rb");
		if (fp != 0)
		{
			fseek(fp, 0, SEEK_END);
			size_external_ehcmodule = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			external_ehcmodule = memalign(32, size_external_ehcmodule);
			if (external_ehcmodule != 0)
				if (fread(external_ehcmodule, 1, size_external_ehcmodule, fp) != size_external_ehcmodule)
				{
					free(external_ehcmodule);
					external_ehcmodule = 0;
				}
			fclose(fp);
		}
	}
	if (external_ehcmodule == 0)
	{
		if (mload_init() < 0)
			return -1;
		mload_elf((void *)ehcmodule, &my_data_elf);
		my_thread_id = mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio);
		if (my_thread_id < 0)
			return -1;
		//if(mload_module(ehcmodule, size_ehcmodule)<0) return -1;
	}
	else
	{
		//if(mload_module(external_ehcmodule, size_external_ehcmodule)<0) return -1;
		if (mload_init() < 0)
			return -1;
		mload_elf((void *)external_ehcmodule, &my_data_elf);
		my_thread_id = mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio);
		if (my_thread_id < 0)
			return -1;
	}
	usleep(350 * 1000);
	// Test for IOS
	mload_seek(0x20207c84, SEEK_SET);
	mload_read(patch_data, 4);
	if (patch_data[0] == 0x6e657665)
		is_ios = 38;
	else
		is_ios = 36;
	if (is_ios == 36)
	{
		// IOS 36
		memcpy(ios_36, dip_plugin, 4);		// copy the entry_point
		memcpy(dip_plugin, ios_36, 4 * 10);	// copy the adresses from the array
		mload_seek(0x1377E000, SEEK_SET);	// copy dip_plugin in the starlet
		mload_write(dip_plugin, size_dip_plugin);
		// enables DIP plugin
		mload_seek(0x20209040, SEEK_SET);
		mload_write(ios_36, 4);
	}
	if (is_ios == 38)
	{
		// IOS 38
		memcpy(ios_38, dip_plugin, 4);		// copy the entry_point
		memcpy(dip_plugin, ios_38, 4 * 10);	// copy the adresses from the array
		mload_seek(0x1377E000, SEEK_SET);	// copy dip_plugin in the starlet
		mload_write(dip_plugin, size_dip_plugin);
		// enables DIP plugin
		mload_seek(0x20208030, SEEK_SET);
		mload_write(ios_38, 4);
	}
	mload_close();
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
		load_ehc_module();
	if (init)
	{
		Fat_Mount();
		Disc_Init();
		WPAD_Init();
		WPAD_SetDataFormat(0, WPAD_FMT_BTNS_ACC_IR);
	}
	return iosOK;
}

void disableIOSReload(void)
{
	const u8 *dip_plugin = 0;
	if (mload_init() < 0)
		return;
	switch (IOS_GetRevision())
	{
		case 2:
			return;
		case 3:
			dip_plugin = dip_plugin_3;
			break;
		case 4:
		default:
			dip_plugin = dip_plugin_4;
			break;
	}
	patch_data[0] = *((u32 *)(dip_plugin + 16 * 4));
	mload_set_ES_ioctlv_vector((void *)patch_data[0]);
	mload_close();
}
