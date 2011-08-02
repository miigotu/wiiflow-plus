#include <stdio.h>
#include <ogcsys.h>
#include <stdlib.h>
#include <wiiuse/wpad.h>
#include <malloc.h>
#include <string.h>
#include "sys.h"
#include "gecko.h"
#include "loader/playlog.h"
#include <ogc/machine/processor.h>
#include "sha1.h"
#include "fs.h"
#include "mem2.hpp"

#define TITLE_ID(x,y)		(((u64)(x) << 32) | (y))
#define TITLE_HIGH(x)		((u32)((x) >> 32))
#define TITLE_LOW(x)		((u32)(x))

/* Variables */
static bool reset = false;
static bool shutdown = false;
bool exiting = false;

static bool return_to_hbc = false;
static bool return_to_menu = false;
static bool return_to_priiloader = false;
static bool return_to_disable = false;
static bool return_to_bootmii = false;

void Open_Inputs(void)
{
	if(WPAD_GetStatus() != WPAD_STATE_ENABLED && WPAD_GetStatus() != WPAD_STATE_ENABLING)
	{
		WPAD_Init();
		PAD_Init();
		WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
	}
}

void Close_Inputs(void)
{
	while(WPAD_GetStatus() == WPAD_STATE_ENABLING); //Possible freeze if i keep this here?
	if(WPAD_GetStatus() == WPAD_STATE_ENABLED)
	{
		WPAD_Flush(WPAD_CHAN_ALL);
		WPAD_Shutdown();
	}
}

bool Sys_Exiting(void)
{
	return reset || shutdown || exiting;
}

void Sys_Test(void)
{
	if(reset || shutdown) Close_Inputs();

	if (reset) SYS_ResetSystem(SYS_RESTART, 0, 0);
	else if (shutdown) SYS_ResetSystem(SYS_POWEROFF, 0, 0);
}

void Sys_ExitTo(int option)
{
	return_to_hbc = option == EXIT_TO_HBC;
	return_to_menu = option == EXIT_TO_MENU;
	return_to_priiloader = option == EXIT_TO_PRIILOADER;
	return_to_disable = option == EXIT_TO_DISABLE;
	return_to_bootmii = option == EXIT_TO_BOOTMII;

	//magic word to force wii menu in priiloader.
	if(return_to_menu)
		write32(0x8132fffb, 0x50756e65);
	else if(return_to_priiloader)
		write32(0x8132fffb,0x4461636f);
}

void Sys_Exit(int ret)
{
	if(return_to_disable) return;

	/* Shutdown Inputs */
	Close_Inputs();

	/* Clear Playlog */
	Playlog_Delete();

	if (return_to_menu || return_to_priiloader) Sys_LoadMenu();
	else if(return_to_bootmii) IOS_ReloadIOS(254);
	if(WII_LaunchTitle(HBC_108)<0)
		if(WII_LaunchTitle(HBC_HAXX)<0)
			if(WII_LaunchTitle(HBC_JODI)<0)
				SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

void __Sys_ResetCallback(void)
{
	reset = true;
}

void __Sys_PowerCallback(void)
{
	shutdown = true;
}


void Sys_Init(void)
{
	/* Set RESET/POWER button callback */
	SYS_SetResetCallback(__Sys_ResetCallback);
	SYS_SetPowerCallback(__Sys_PowerCallback);
}

void Sys_LoadMenu(void)
{
	/* Return to the Wii system menu */
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

s32 GetTMD(u64 TicketID, signed_blob **Output, u32 *Length)
{
	signed_blob* TMD = NULL;

	u32 TMD_Length;
	s32 ret;

	/* Retrieve TMD length */
	ret = ES_GetStoredTMDSize(TicketID, &TMD_Length);
	if (ret < 0)
		return ret;

	/* Allocate memory */
	TMD = (signed_blob*)MEM2_alloc((TMD_Length+31)&(~31));
	if (!TMD) return IPC_ENOMEM;

	/* Retrieve TMD */
	ret = ES_GetStoredTMD(TicketID, TMD, TMD_Length);
	if (ret < 0)
	{
		SAFE_FREE(TMD);
		return ret;
	}

	/* Set values */
	*Output = TMD;
	*Length = TMD_Length;

	return 0;
}



s32 checkIOS(u32 IOS)
{
	signed_blob *TMD = NULL;
	tmd *t = NULL;
	u32 TMD_size = 0;
	u64 title_id = 0;
	s32 ret = 0;

	// Get tmd to determine the version of the IOS
	title_id = (((u64)(1) << 32) | (IOS));
	ret = GetTMD(title_id, &TMD, &TMD_size);

	if (ret == 0) {
		t = (tmd*)SIGNATURE_PAYLOAD(TMD);
		if (t->title_version == 65280) {
			ret = -1;
		}
	} else {
		ret = -2;
	}
	SAFE_FREE(TMD);
	return ret;
}

// ios base detection from NeoGamma R9 (by WiiPower)


struct ios_hash_info
{
	u32 slot;
	u32 hash[5];
	char *info;
};

static struct ios_hash_info ios_info[] =
{
	// d2x v1
	{ 249, {0x00ed2993, 0x0bae0cb2, 0xc7e430a2, 0x2e6eaf18, 0x156a9a70}, "37 r21-d2x-v1" },
	{ 249, {0x00d74607, 0x2d3fe23e, 0x47ecb019, 0x0f5d4380, 0x37ea6b50}, "38 r21-d2x-v1" },
	{ 249, {0x003d11ce, 0x4eb3b8bb, 0xe2c02fda, 0x5f879e74, 0x44a257de}, "56 r21-d2x-v1" },
	{ 249, {0x00ba4b4f, 0x27803366, 0x8d9121fa, 0x954eb5d5, 0x92242691}, "57 r21-d2x-v1" },
	// d2x v2
	{ 249, {0x00475dce, 0x81a744dd, 0xf24157e4, 0x870fa3d8, 0xfc39fa8a}, "37 r21-d2x-v2" },
	{ 249, {0x00711af6, 0x017c48d4, 0xea0267d3, 0x1666600b, 0x38a8fe16}, "38 r21-d2x-v2" },
	{ 249, {0x00815782, 0x8604fe34, 0x474653b5, 0xbdbc5651, 0xf43b427a}, "56 r21-d2x-v2" },
	{ 249, {0x00d8e857, 0x8c96eb52, 0x4d006568, 0x95cf5415, 0xdb7712e8}, "57 r21-d2x-v2" },
	// d2x v3
	{ 249, {0x0054e91c, 0xe022e307, 0x26d72e03, 0x53b6e157, 0x42adbe49}, "37 r21-d2x-v3" },
	{ 249, {0x000bd035, 0xe649cc22, 0x8bf647c5, 0xe0710e6a, 0xd79a5355}, "38 r21-d2x-v3" },
	{ 249, {0x00b8ca9c, 0x9b4053a3, 0x8de94a72, 0x1192fce5, 0x098e7404}, "56 r21-d2x-v3" },
	{ 249, {0x00e8e05f, 0x2aa4cd1e, 0x8c8f5529, 0x498f259b, 0xfa41258e}, "57 r21-d2x-v3" },
	{ 249, {0x0028dbf1, 0x3827be46, 0x28c82eb2, 0x122325c3, 0xc72dbd46}, "58 r21-d2x-v3" },
	// d2x v3 (r21003)
	{ 249, {0x006ec958, 0xbc59364d, 0x5b2f58a0, 0xf8feeac4, 0x89a0b697}, "37 r21-d2x-v3" },
	{ 249, {0x00fe6ad4, 0xbbf9a5e2, 0xeb2b0110, 0xc1fddbdf, 0xfb634350}, "38 r21-d2x-v3" },
	{ 249, {0x00ecc3a8, 0xec2d3b64, 0x506314e3, 0x740274ef, 0x6505cc75}, "56 r21-d2x-v3" },
	{ 249, {0x008e68fd, 0xa1221185, 0xc09a1a26, 0xfeb09ead, 0xf375c2f2}, "57 r21-d2x-v3" },
	{ 249, {0x006237ad, 0xda4cb0e1, 0xa97e4b41, 0xf1bb24a2, 0xd663b7f7}, "58 r21-d2x-v3" },

	// d2x v4 (r21004) beta 2
	{ 249, {0x00733fa4, 0x1d3e6245, 0xb0311e24, 0x675868b1, 0x353d882c}, "37 r21-d2x-v4-b2" },
	{ 249, {0x007b9fca, 0x0a6f40c5, 0xccd37b25, 0x1c49064b, 0x1041ddb3}, "38 r21-d2x-v4-b2" },
	{ 249, {0x000d243c, 0x07a183df, 0x0592ce22, 0x2bb81b46, 0x64cce4a7}, "56 r21-d2x-v4-b2" },
	{ 249, {0x00c7b39a, 0xed42a4a0, 0xcc125669, 0xf23c1f6e, 0x2244cb9b}, "57 r21-d2x-v4-b2" },
	{ 249, {0x00120cb0, 0x4cb9b4b1, 0xbe02e0e6, 0x30bfcb95, 0xfbfcaba5}, "58 r21-d2x-v4-b2" },

	// d2x v4 (r21004) release / hb installer
	{ 249, {0xba124a8e, 0x73f5b2cb, 0x5e2439be, 0x76629335, 0xa3f36418}, "37 r21-d2x-v4" },
	{ 249, {0x2e5b15a4, 0x6e638735, 0x6d3d7403, 0xa78cdcc0, 0xe62a106b}, "38 r21-d2x-v4" },
	{ 249, {0xf13c731c, 0x3d77c021, 0x48c3119f, 0x7679939f, 0xbde8857f}, "53 r21-d2x-v4" },
	{ 249, {0x292464d3, 0xf94267d3, 0x849fd15c, 0x03200bde, 0xe8e0d6e8}, "55 r21-d2x-v4" },
	{ 249, {0xfefb9f74, 0x9961dd76, 0xe5416e0f, 0x7df6a95b, 0x923d2561}, "56 r21-d2x-v4" },
	{ 249, {0xb7272b2f, 0x72bdab83, 0x31d0639f, 0xfabc719d, 0xad818d91}, "57 r21-d2x-v4" },
	{ 249, {0x91a7d59f, 0x4ae0671a, 0x9bdf2593, 0xf7535426, 0x85af4073}, "58 r21-d2x-v4" },

	// d2x v4 (r21004) release / modmii
	{ 249, {0x00de8cad, 0x17183381, 0x889a1299, 0x834a85eb, 0x45b59e05}, "37 r21-d2x-v4" },
	{ 249, {0x0007e951, 0x173de10f, 0x0324b33f, 0xaa1f93c7, 0x28461fbe}, "38 r21-d2x-v4" },
	{ 249, {0x00f92f4f, 0x8f989389, 0xcd9e2732, 0x7752e50b, 0xa47fde40}, "53 r21-d2x-v4" },
	{ 249, {0x0056c457, 0x99c9a90f, 0xf0d9d994, 0x0724362a, 0xbe8ac29f}, "55 r21-d2x-v4" },
	{ 249, {0x00a1872f, 0x412f94cf, 0xd90c818b, 0xde15681e, 0x63c41b52}, "56 r21-d2x-v4" },
	{ 249, {0x00b06c85, 0xab7a94c2, 0x674785fc, 0x8f133335, 0xc9b84d49}, "57 r21-d2x-v4" },
	{ 249, {0x000530f4, 0x0c472b29, 0xb8f22f5a, 0x752b0613, 0x109bace1}, "58 r21-d2x-v4" },
};

const int ios_info_number = sizeof(ios_info) / sizeof(struct ios_hash_info);

s32 brute_tmd(tmd *p_tmd)
{
	u16 fill;
	for(fill = 0; fill < 65535; fill++)
	{
		p_tmd->fill3 = fill;
		sha1 hash;
		SHA1((u8 *)p_tmd, TMD_SIZE(p_tmd), hash);;
		if (hash[0] == 0) return 0;
	}
	return -1;
}

s32 brute_modmii(signed_blob *TMD, int size, unsigned char *hash) 
{
	u16 fill;
	tmd *p_tmd = (tmd*)SIGNATURE_PAYLOAD(TMD);
	for(fill = 0; fill < 65535; fill++) 
	{
		p_tmd->fill3 = fill;
		SHA1((u8 *)TMD, size, hash);;
		if (hash[0] == 0) return 0;
	}
	return -1;
}

char* get_iosx_info_from_tmd(int ios_slot, u32 *version)
{
	signed_blob *TMD = NULL;
	tmd *t = NULL;
	u32 TMD_size = 0;
	u64 title_id = TITLE_ID(1, ios_slot);
	u32 i;
	int retry_count = 0;
	sha1 hash;
	int ret;
	char *info = NULL;
	static char info_str[32];
	int is_modmii = 0;

	ret = GetTMD(title_id, &TMD, &TMD_size);
	if (ret != 0) goto out;

	t = (tmd*)SIGNATURE_PAYLOAD(TMD);

	if (version) *version = t->title_version;

	// safety check
	if (t->title_id != title_id) goto out;

	iosinfo_t iosinfo;
	if (get_iosinfo(ios_slot, TMD, &iosinfo))
	{
		info = info_str;
		sprintf(info, "%u %s v%u%s", iosinfo.baseios, iosinfo.name,
					iosinfo.version, iosinfo.versionstring);                                
		goto out;
	}

	// modmii check
	// brute match?
	signed_blob *TMD_copy = malloc(TMD_size);
	memcpy(TMD_copy, TMD, TMD_size);
	tmd *tt = (tmd*)SIGNATURE_PAYLOAD(TMD_copy);
	brute_tmd(tt);
	int match = memcmp(TMD, TMD_copy, TMD_size) == 0;
	//if (!match) dbg_printf("\nbrute match: %d %u %u\n", match, t->fill3, tt->fill3);
	SAFE_FREE(TMD_copy);
	SHA1((u8 *)TMD, TMD_size, hash);
	if (!match && hash[0] == 0)
		is_modmii = 1;

retry:
	if (retry_count)
	{
		if (retry_count > 100) goto out;
		brute_tmd(t);
	}
	retry_count++;

	SHA1((u8 *)TMD, TMD_size, hash);

	int mm;
	for (mm = 0; mm < 2; mm++)
	{
		for (i = 0; i < ios_info_number; i++)
		{
			if (ios_info[i].slot != TITLE_LOW(t->title_id)) continue;
			if (memcmp((void *)hash, &ios_info[i].hash, sizeof(sha1)) == 0)
			{
				info = ios_info[i].info;
				break;
			}
		}
		if (info == NULL && is_modmii)
		{
			// use alternative brute hash
			// to search for modmii hashes in table
			brute_modmii(TMD, TMD_size, hash);
		}
		else break;
	}

	if (info == NULL)
	{
		if (is_modmii == 1)
		{
			is_modmii++;
			// if modmii, first do the brute_tmd
			goto retry;
		}
		// patch title id, so hash matches
		if (ios_slot >= 245 && ios_slot <= 250)
		{
			if (t->title_id != TITLE_ID(1, 249))
			{
				t->title_id = TITLE_ID(1, 249);
				goto retry;
			}
			if (ios_slot == 250)
			{
				// ios 250 has a fixed version of 65535 patch to a lower one
				if (t->title_version > 13)
				{
					if (t->title_version > 21)
						t->title_version = 21;
					else 
						t->title_version--;

					goto retry;
				}
			}
		}
		if (t->title_id == TITLE_ID(1, 224))
		{
			t->title_id = TITLE_ID(1, 223);
			goto retry;
		}
		if (t->title_id == TITLE_ID(1, 223))
		{
			t->title_id = TITLE_ID(1, 222);
			goto retry;
		}
	}

out:
	SAFE_FREE(TMD);
	if (info && is_modmii)
	{
		if (!strstr(info, "modmii"))
		{
			strcpy(info_str, info);
			strcat(info_str, " modmii");
			info = info_str;
		}
	}
    return info;
}

char* get_ios_info_from_tmd()
{
    return get_iosx_info_from_tmd(IOS_GetVersion(), NULL);
}

bool get_iosinfo(int ios, signed_blob *TMD, iosinfo_t *iosinfo)
{
        char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(0x20);
        u8 *buffer = NULL;
        u32 filesize;
        bool retval = false;

        ISFS_Initialize();
        sprintf(filepath, "/title/%08x/%08x/content/%08x.app", 0x00000001, ios, *(u8 *)((u32)TMD+0x1E7));
						
		buffer = ISFS_GetFile((u8 *) filepath, &filesize, 0);

        if (buffer)
		{
			memcpy(iosinfo, buffer, sizeof(*iosinfo));
			if (iosinfo->magicword == 0x1ee7c105 && iosinfo->magicversion == 1)
					retval = true;

			SAFE_FREE(buffer);
        }
        return retval;
}