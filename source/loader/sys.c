#include <stdio.h>
#include <ogcsys.h>
#include <stdlib.h>
#include <wiiuse/wpad.h>
#include <malloc.h>
#include <string.h>
#include "sys.h"
#include "gecko.h"
#include "loader/playlog.h"
#include "sha1.h"
#include "fs.h"

#define TITLE_ID(x,y)		(((u64)(x) << 32) | (y))
#define TITLE_HIGH(x)		((u32)((x) >> 32))
#define TITLE_LOW(x)		((u32)(x))

/* Constants */
#define CERTS_LEN	0x280

/* Variables */
static const char certs_fs[] ATTRIBUTE_ALIGN(32) = "/sys/cert.sys";
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
	{
		Write32(0x8132fffb, 0x50756e65);
	}
	else if(return_to_priiloader)
	{
		Write32(0x8132fffb,0x4461636f);
	}
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

s32 Sys_GetCerts(signed_blob **certs, u32 *len)
{
	static signed_blob certificates[CERTS_LEN] ATTRIBUTE_ALIGN(32);

	s32 fd, ret;

	/* Open certificates file */
	fd = IOS_Open(certs_fs, 1);
	if (fd < 0)
		return fd;

	/* Read certificates */
	ret = IOS_Read(fd, certificates, sizeof(certificates));

	/* Close file */
	IOS_Close(fd);

	/* Set values */
	if (ret > 0)
	{
		*certs = certificates;
		*len = sizeof(certificates);
	}

	return ret;
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
	TMD = (signed_blob*)memalign(32, (TMD_Length+31)&(~31));
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
        { 249, {0x20e60607, 0x4e02c484, 0x2bbc5758, 0xee2b40fc, 0x35a68b0a}, "?? rev13a" },
        { 249, {0x620c57c7, 0xd155b67f, 0xa451e2ba, 0xfb5534d7, 0xaa457878}, "?? rev13b" },
        // rev 17
        { 249, {0x369047f1, 0x4e37a34f, 0x3cca2936, 0xa60d527b, 0xac141d4a}, "38 rev17" },
        { 249, {0x58ba3f8b, 0x7f8dccd3, 0x8b6c9109, 0x9a736e80, 0xc6c5ea67}, "38 rev17b" },
        // rev 18
        { 249, {0x3c968e54, 0x9e915458, 0x9ecc3bda, 0x16d0a0d4, 0x8cac7917}, "37 rev18" },
        { 249, {0xe811bca8, 0xe1df1e93, 0x779c40e6, 0x2006e807, 0xd4403b97}, "38 rev18" },
        { 249, {0x697676f0, 0x7a133b19, 0x881f512f, 0x2017b349, 0x6243c037}, "57 rev18" },
        { 249, {0x34ec540b, 0xd1fb5a5e, 0x4ae7f069, 0xd0a39b9a, 0xb1a1445f}, "60 rev18" },
        { 249, {0xd98a4dd9, 0xff426ddb, 0x1afebc55, 0x30f75489, 0x40b27ade}, "70 rev18" },
        // rev 19
        { 249, {0x0a49cd80, 0x6f8f87ff, 0xac9a10aa, 0xefec9c1d, 0x676965b9}, "37 rev19" },
        { 249, {0x09179764, 0xeecf7f2e, 0x7631e504, 0x13b4b7aa, 0xca5fc1ab}, "38 rev19" },
        { 249, {0x6010d5cf, 0x396415b7, 0x3c3915e9, 0x83ded6e3, 0x8f418d54}, "57 rev19" },
        { 249, {0x589d6c4f, 0x6bcbd80a, 0xe768f258, 0xc53a322c, 0xd143f8cd}, "60 rev19" },
        { 249, {0x8969e0bf, 0x7f9b2391, 0x31ecfd88, 0x1c6f76eb, 0xf9418fe6}, "70 rev19" },
        // rev 20       
        { 249, {0x30aeadfe, 0x8b6ea668, 0x446578c7, 0x91f0832e, 0xb33c08ac}, "36 rev20" },
        { 249, {0xba0461a2, 0xaa26eed0, 0x482c1a7a, 0x59a97d94, 0xa607773e}, "37 rev20" },
        { 249, {0xb694a33e, 0xf5040583, 0x0d540460, 0x2a450f3c, 0x69a68148}, "38 rev20" },
        { 249, {0xf6058710, 0xfe78a2d8, 0x44e6397f, 0x14a61501, 0x66c352cf}, "53 rev20" },
        { 249, {0xfa07fb10, 0x52ffb607, 0xcf1fc572, 0xf94ce42e, 0xa2f5b523}, "55 rev20" },
        { 249, {0xe30acf09, 0xbcc32544, 0x490aec18, 0xc276cee6, 0x5e5f6bab}, "56 rev20" },
        { 249, {0x595ef1a3, 0x57d0cd99, 0x21b6bf6b, 0x432f6342, 0x605ae60d}, "57 rev20" },
        { 249, {0x687a2698, 0x3efe5a08, 0xc01f6ae3, 0x3d8a1637, 0xadab6d48}, "60 rev20" },
        { 249, {0xea6610e4, 0xa6beae66, 0x887be72d, 0x5da3415b, 0xa470523c}, "61 rev20" },
        { 249, {0x64e1af0e, 0xf7167fd7, 0x0c696306, 0xa2035b2d, 0x6047c736}, "70 rev20" },
        { 249, {0x0df93ca9, 0x833cf61f, 0xb3b79277, 0xf4c93cd2, 0xcd8eae17}, "80 rev20" },
        // rev 21
        { 249, {0x074dfb39, 0x90a5da61, 0x67488616, 0x68ccb747, 0x3a5b59b3}, "36 rev21" },
        { 249, {0x6956a016, 0x59542728, 0x8d2efade, 0xad8ed01e, 0xe7f9a780}, "37 rev21" },
        { 249, {0xdc8b23e6, 0x9d95fefe, 0xac10668a, 0x6891a729, 0x2bdfbca0}, "38 rev21" },
        { 249, {0xaa2cdd40, 0xd628bc2e, 0x96335184, 0x1b51404c, 0x6592b992}, "53 rev21" },
        { 249, {0x4a3d6d15, 0x014f5216, 0x84d65ffe, 0x6daa0114, 0x973231cf}, "55 rev21" },
        { 249, {0xca883eb0, 0x3fe8e45c, 0x97cc140c, 0x2e2d7533, 0x5b369ba5}, "56 rev21" },
        { 249, {0x469831dc, 0x918acc3e, 0x81b58a9a, 0x4493dc2c, 0xaa5e57a0}, "57 rev21" },
        { 249, {0xe5af138b, 0x029201c7, 0x0c1241e7, 0x9d6a5d43, 0x37a1456a}, "58 rev21" },
        { 249, {0x0fdee208, 0xf1d031d3, 0x6fedb797, 0xede8d534, 0xd3b77838}, "60 rev21" },
        { 249, {0xaf588570, 0x13955a32, 0x001296aa, 0x5f30e37f, 0x0be91316}, "61 rev21" },
        { 249, {0x50deaba2, 0x9328755c, 0x7c2deac8, 0x385ecb49, 0x65ea3b2b}, "70 rev21" },
        { 249, {0x811b6a0b, 0xe26b9419, 0x7ffd4930, 0xdccd6ed3, 0x6ea2cdd2}, "80 rev21" },
        // hermes
        { 222, {0xafbfc2c1, 0x28c59142, 0x953b1c6c, 0x809a984f, 0x427c9270}, "38 v4" },
        { 222, {0xb72b71cd, 0xf42b2730, 0x3b9a4f2c, 0x41128ef9, 0x26f6dbcc}, "38+37 v4" },
        { 222, {0xd6f1472f, 0x68740b4c, 0xbdf0078d, 0xb8ebb00a, 0x8c9afe2b}, "38+60 v4" },
        // v5.0
        { 222, {0xfa8ad097, 0x6c18542a, 0x5691bdec, 0xd0c47a6a, 0xbb857b08}, "38 v5.0" },
        { 223, {0x07b9c8f2, 0xa0dbad4d, 0xa6ee0761, 0x7c371591, 0x4e4c63ec}, "37 v5.0" },
        { 223, {0x0d0a12e0, 0x16065574, 0x844e39b4, 0x2d2dbdf1, 0x5b497869}, "57 v5.0" },
        { 223, {0x0aa83bf0, 0x8fbd610f, 0x87bae3c1, 0x06f43826, 0x39524429}, "60 v5.0" },
        // v5.1
        { 222, {0xf865dfa5, 0xa909e4fb, 0x345f48ea, 0x804f5a64, 0x3704dd5a}, "38 v5.1" },
        { 223, {0x889511aa, 0xdde5c849, 0xe30e6d20, 0x9687c95a, 0xb935342b}, "37 v5.1" },
        { 223, {0x0584100d, 0xc3364080, 0xc3b8ffd0, 0x8c351aee, 0xf4632159}, "57 v5.1" },
        { 223, {0x1cb7981c, 0xd791a2bf, 0x736395d6, 0x0224e181, 0x38324674}, "60 v5.1" },
        // modmii special
        { 249, {0x00dc1209, 0x944c39db, 0x59eec2ab, 0x0212b86c, 0x7076cd3b}, "56 r21+r19 modmii" },
        { 249, {0x00298dc2, 0x58fdae1a, 0x233b0958, 0x17269047, 0x8188633d}, "57 r21+r19 modmii" },
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

        /*
        // modmii 249
        { 249, {0x005b6439, 0xf4a2e0b7, 0xfce05f75, 0xdb1a66ce, 0x7a0811c1}, "38 r17 modmii" }, 
        { 249, {0x007da65a, 0x1b57b279, 0x06a5443f, 0xc61fd6cb, 0x4ea9866a}, "37 r19 modmii" },
        { 249, {0x00cb38fc, 0x3abc550f, 0xc128f0aa, 0xa1dc96b8, 0x3d3ed542}, "38 r19 modmii" },
        { 249, {0x00cc7ee1, 0xc1af6682, 0x8a5a2b6f, 0xc417eb3d, 0x607377ec}, "57 r19 modmii" },
        { 249, {0x00e96e3f, 0xe861fd7a, 0x092d0fcb, 0xa65af414, 0xd375d6bb}, "38 r20 modmii" }, 
        { 249, {0x009e5dde, 0x2589d21d, 0x4db9dfaa, 0x765c4279, 0xc4a5ba75}, "56 r20 modmii" },
        { 249, {0x003c1bd8, 0x7830d7dc, 0x79e74949, 0x9609bb13, 0x4b5c5072}, "57 r20 modmii" },
        { 249, {0x000f5724, 0xe6002c66, 0x6718313c, 0x8c4ec895, 0x478480ce}, "37 r21 modmii" },
        { 249, {0x00c61cad, 0x30328d5d, 0xe69eb487, 0x27f77d5e, 0xc3c47d0d}, "38 r21 modmii" },
        { 249, {0x00994d20, 0xbe74e78b, 0x00d4f00c, 0xfc9da208, 0x262c5f90}, "56 r21 modmii" },
        { 249, {0x009fb539, 0x5a56f778, 0x329fbfd7, 0xc3a8ff58, 0x6fdb010b}, "57 r21 modmii" },
        { 249, {0x002a7dfe, 0xdc36d6d9, 0x9af35191, 0x54862ecb, 0xd8087cb3}, "58 r21 modmii" },
        // modmii 250
        { 250, {0x00186ce5, 0xe6ced602, 0x552e621d, 0xaf882fb8, 0xa479e47b}, "37 r19 modmii" },
        { 250, {0x00e2ea30, 0x56c4568d, 0x4f2c165d, 0xc00471bd, 0x6939c9f1}, "38 r19 modmii" },
        { 250, {0x005849db, 0xa1fc4519, 0x530a963b, 0x31810e9e, 0xea1f207a}, "57 r19 modmii" },
        { 250, {0x00641c3c, 0xa7346fa0, 0x0fa518a6, 0xeeac8097, 0x60eb2e87}, "38 r20 modmii" },
        { 250, {0x009c97f0, 0xda7da42e, 0xd3320862, 0xb33d22db, 0xdc80b9e2}, "56 r20 modmii" },
        { 250, {0x006d76ce, 0x0a294191, 0x62c9705d, 0x355ec87b, 0xff152dd5}, "57 r20 modmii" },
        { 250, {0x006937f9, 0x8fdccb08, 0x4f9396b3, 0xc91b8761, 0x8ff1f1bb}, "37 r21 modmii" },
        { 250, {0x004ba2f2, 0x9e4269c6, 0xe5d91fd0, 0x2e6410db, 0x772b1986}, "38 r21 modmii" },
        { 250, {0x00bca8d8, 0x82105397, 0xb140ddb2, 0x8774d57a, 0x66418504}, "56 r21 modmii" },
        { 250, {0x000ac379, 0x719a8850, 0x469445e7, 0xcf51108e, 0xb832d628}, "57 r21 modmii" },
        { 250, {0x008567af, 0xe41cdb0b, 0xce85dc29, 0x10970d12, 0xe0b608f3}, "58 r21 modmii" },
        // modmii hermes
        { 222, {0x00ec3a7b, 0xc9869c77, 0x013cf962, 0x4eef5726, 0xda9d1488}, "38 v4 modmii" },
        { 223, {0x0073251f, 0x88b53db8, 0x390234af, 0x26910ff6, 0x041f3d3f}, "38+37 v4 modmii" },
        { 222, {0x00d801b3, 0xe280e6e2, 0x1c99b236, 0x470ed5a9, 0xf3544f86}, "38 v5.0 modmii" },
        { 223, {0x007d38f5, 0xb6b841b4, 0xf57579db, 0xa47526fe, 0xc3b3d12a}, "37 v5.0 modmii" },
        { 223, {0x00f08071, 0x2672d68b, 0xc63bed5a, 0x06ae3b3c, 0xcff913d7}, "57 v5.0 modmii" },
        */
};

const int ios_info_number = sizeof(ios_info) / sizeof(struct ios_hash_info);

s32 brute_tmd(tmd *p_tmd)
{
	u16 fill;
	for(fill=0; fill<65535; fill++)
	{
		p_tmd->fill3 = fill;
		sha1 hash;
		SHA1((u8 *)p_tmd, TMD_SIZE(p_tmd), hash);;
		if (hash[0]==0)
		{
			return 0;
		}
	}
	return -1;
}

s32 brute_modmii(signed_blob *TMD, int size, unsigned char *hash) 
{
	u16 fill;
	tmd *p_tmd = (tmd*)SIGNATURE_PAYLOAD(TMD);
	for(fill=0; fill<65535; fill++) 
	{
		p_tmd->fill3 = fill;
		SHA1((u8 *)TMD, size, hash);;
		if (hash[0]==0) 
		{
			return 0;
		}
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

	//static char default_info[32];
	//sprintf(default_info, "IOS%u (Rev %u)", IOS_GetVersion(), IOS_GetRevision());
	//info = (char *)default_info;

	ret = GetTMD(title_id, &TMD, &TMD_size);
	if (ret != 0) goto out;

	t = (tmd*)SIGNATURE_PAYLOAD(TMD);

	//dbg_printf("\ntmd id: %llx %x-%x t: %x v: %d",
	//t->title_id, TITLE_HIGH(t->title_id), TITLE_LOW(t->title_id),
	//t->title_type, t->title_version);
	if (version) *version = t->title_version;

	// safety check
	if (t->title_id != title_id) goto out;

	iosinfo_t iosinfo;
	if (get_iosinfo(ios_slot, TMD, &iosinfo)) {
		//sprintf(info, "%s%uv%u%s (%u)", iosinfo->name, iosinfo->baseios, iosinfo->version, iosinfo->versionstring, CIOS_VERSION);                             
		// Example: "d2x56v5beta2 (249)"
		// "56 r21-d2x-v4"
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
	if (!match && hash[0] == 0) {
		is_modmii = 1;
	}

retry:
	if (retry_count) {
		if (retry_count > 100) goto out;
		brute_tmd(t);
	}
	retry_count++;

	SHA1((u8 *)TMD, TMD_size, hash);

	int mm;
	for (mm = 0; mm < 2; mm++) {
		for (i = 0; i < ios_info_number; i++)
		{
			if (ios_info[i].slot != TITLE_LOW(t->title_id)) continue;
			if (memcmp((void *)hash, &ios_info[i].hash, sizeof(sha1)) == 0)
			{
				info = ios_info[i].info;
				break;
			}
		}
		if (info == NULL && is_modmii) {
			// use alternative brute hash
			// to search for modmii hashes in table
			brute_modmii(TMD, TMD_size, hash);
		} else {
			break;
		}
	}

	if (info == NULL) {
		if (is_modmii == 1) {
			is_modmii++;
			// if modmii, first do the brute_tmd
			goto retry;
		}
		// patch title id, so hash matches
		if (ios_slot >= 245 && ios_slot <= 250) {
			if (t->title_id != TITLE_ID(1, 249)) {
				t->title_id = TITLE_ID(1, 249);
				goto retry;
			}
			if (ios_slot == 250) {
				// ios 250 has a fixed version of 65535 patch to a lower one
				if (t->title_version > 13) {
					if (t->title_version > 21) {
						t->title_version = 21;
					} else {
						t->title_version--;
					}
					goto retry;
				}
			}
		}
		if (t->title_id == TITLE_ID(1, 224)) {
			t->title_id = TITLE_ID(1, 223);
			goto retry;
		}
		if (t->title_id == TITLE_ID(1, 223)) {
			t->title_id = TITLE_ID(1, 222);
			goto retry;
		}
	}

out:
	SAFE_FREE(TMD);
	if (info && is_modmii) {
		if (!strstr(info, "modmii")) {
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
        sprintf(filepath, "/title/%08x/%08x/content/%08x.app",
                        0x00000001, ios, *(u8 *)((u32)TMD+0x1E7));
						
		buffer = ISFS_GetFile((u8 *) filepath, &filesize, 0);

        // ISFS_Deinitialize();
        // If executed, it causes Castlevania to freeze as soon as the game "sees" a wii mote
        // It is executed now before an IOS Reload is executed
        if (buffer) {
                memcpy(iosinfo, buffer, sizeof(*iosinfo));
                if (iosinfo->magicword == 0x1ee7c105 && iosinfo->magicversion == 1) {
                        retval = true;
                }
			SAFE_FREE(buffer);
        }
        return retval;
}

void get_all_ios_info_str(int i, char *str, int size)
{
        //int i;
        char *info;
        int ret;
 
                //if (i > 224 && i < 245) continue;
                snprintf(str, size, "IOS%d ", i);
                str_seek_end(&str, &size);
                ret = checkIOS(i);
                if (ret == -2) {
                        snprintf(str, size, ": not installed\n");
                } else if (ret == -1) {
                        snprintf(str, size, ": stub\n");
                } else {
                        u32 ver;
                        info = get_iosx_info_from_tmd(i, &ver);
                        snprintf(str, size, "Base: %s (r%u)\n", info ? info : "??", ver);
                }
                str_seek_end(&str, &size);

}

char* str_seek_end(char **str, int *size)
{
        int len = strlen(*str);
        *size -= len;
        *str += len;
        return *str;
}
