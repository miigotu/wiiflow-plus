#include "channel_launcher.h"

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogc/isfs.h>
#include <stdio.h>
#include <wiiuse/wpad.h>

#include "disc.h"
#include "patchcode.h"
#include "videopatch.h"
#include "fst.h"
#include "lz77.h"
#include "utils.h"
#include "fs.h"
#include "gecko.h"
#include "mem2.hpp"

GXRModeObj * __Disc_SelectVMode(u8 videoselected, u64 chantitle);
void PatchCountryStrings(void *Address, int Size);
void __Disc_SetLowMem(void);
void __Disc_SetVMode(void);
void __Disc_SetTime(void);
void _unstub_start();

extern void __exception_closeall();

bool bootcontent_used = false;

typedef void (*entrypoint) (void);

typedef struct _dolheader{
	u32 text_pos[7];
	u32 data_pos[11];
	u32 text_start[7];
	u32 data_start[11];
	u32 text_size[7];
	u32 data_size[11];
	u32 bss_start;
	u32 bss_size;
	u32 entry_point;
} dolheader;

u32 entryPoint;

s32 BootChannel(u32 *data, u64 chantitle, u8 vidMode, bool vipatch, bool countryString, u8 patchVidMode)
{
	entryPoint = LoadChannel(data);
	SAFE_FREE(data);

	/* Select an appropriate video mode */
	GXRModeObj * vmode = __Disc_SelectVMode(vidMode, chantitle);

	u32 ios;
	Identify(chantitle, &ios);

	//ISFS_Deinitialize();

	/*if (entryPoint != 0x3400)
		__Disc_SetLowMem();*/

	if (entryPoint != 0x3400)
	{
		//gprintf("Setting bus speed\n");
		*(u32*)0x800000F8 = 0x0E7BE2C0;
		//gprintf("Setting cpu speed\n");
		*(u32*)0x800000FC = 0x2B73A840;

		DCFlushRange((void*)0x800000F8, 0xFF);
	}
	
	// Remove 002 error
	*(u16 *)0x80003140 = ios;
	*(u16 *)0x80003142 = 0xffff;
	*(u16 *)0x80003188 = ios;
	*(u16 *)0x8000318A = 0xffff;
	
	DCFlushRange((void*)0x80003140, 4);
	DCFlushRange((void*)0x80003188, 4);

	if (hooktype != 0)
		ocarina_do_code();

	PatchChannel(vidMode, vmode, vipatch, countryString, patchVidMode);

	entrypoint appJump = (entrypoint)entryPoint;

	/* Set time */
	__Disc_SetTime();

	/* Set an appropriate video mode */
	__Disc_SetVMode();

	/* Shutdown IOS subsystems */
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);

	gprintf("Jumping to entrypoint %08x\n", entryPoint);
	
	if (entryPoint != 0x3400)
	{
		if (hooktype != 0)
		{
			__asm__(
				"lis %r3, entryPoint@h\n"
				"ori %r3, %r3, entryPoint@l\n"
				"lwz %r3, 0(%r3)\n"
				"mtlr %r3\n"
				"lis %r3, 0x8000\n"
				"ori %r3, %r3, 0x18A8\n"
				"mtctr %r3\n"
				"bctr\n"
			);
		}
		else  appJump();	
	}
 	else if (hooktype != 0)
	{
		__asm__(
			"lis %r3, returnpoint@h\n"
			"ori %r3, %r3, returnpoint@l\n"
			"mtlr %r3\n"
			"lis %r3, 0x8000\n"
			"ori %r3, %r3, 0x18A8\n"
			"mtctr %r3\n"
			"bctr\n"
			"returnpoint:\n"
			"bl DCDisable\n"
			"bl ICDisable\n"
			"li %r3, 0\n"
			"mtsrr1 %r3\n"
			"lis %r4, entryPoint@h\n"
			"ori %r4,%r4,entryPoint@l\n"
			"lwz %r4, 0(%r4)\n"
			"mtsrr0 %r4\n"
			"rfi\n"
		);
	}
	else _unstub_start();

	return 0;
}

void*	dolchunkoffset[18];
u32		dolchunksize[18];
u32		dolchunkcount;

u32 LoadChannel(u32 *buffer)
{
	dolchunkcount = 0;
	
	dolheader *dolfile = (dolheader *)buffer;
	
	//gprintf("Entrypoint: %08x\n", dolfile->entry_point);
	//gprintf("BSS: %08x, size = %08x(%u)\n", dolfile->bss_start, dolfile->bss_size, dolfile->bss_size);

	memset((void *)dolfile->bss_start, 0, dolfile->bss_size);
	DCFlushRange((void *)dolfile->bss_start, dolfile->bss_size);
	
    //gprintf("BSS cleared\n");
	
	int i;
 	for (i = 0; i < 7; i++)
	{
		if ((!dolfile->text_size[i]) || (dolfile->text_start[i] < 0x100)) continue;
		if(dolfile->text_pos[i] < sizeof(dolheader))
			continue;

		dolchunkoffset[dolchunkcount] = (void *)dolfile->text_start[i];
		dolchunksize[dolchunkcount] = dolfile->text_size[i];

		gprintf("Moving text section %u from offset %08x to %08x-%08x...\n", i, dolfile->text_pos[i], dolchunkoffset[dolchunkcount], dolchunkoffset[dolchunkcount]+dolchunksize[dolchunkcount]);

		memmove (dolchunkoffset[dolchunkcount], (void *)buffer + dolfile->text_pos[i], dolchunksize[dolchunkcount]);
		DCFlushRange (dolchunkoffset[dolchunkcount], dolchunksize[dolchunkcount]);
		ICInvalidateRange (dolchunkoffset[dolchunkcount],dolchunksize[dolchunkcount]);
		
		dolchunkcount++;
	}

	for(i = 0; i < 11; i++)
	{
		if ((!dolfile->data_size[i]) || (dolfile->data_start[i] < 0x100)) continue;
		if(dolfile->data_pos[i] < sizeof(dolheader))
			continue;

		dolchunkoffset[dolchunkcount] = (void *)dolfile->data_start[i];
		dolchunksize[dolchunkcount] = dolfile->data_size[i];			

		gprintf("Moving data section %u from offset %08x to %08x-%08x...\n", i, dolfile->data_pos[i], dolchunkoffset[dolchunkcount], dolchunkoffset[dolchunkcount]+dolchunksize[dolchunkcount]);

		memmove (dolchunkoffset[dolchunkcount], (void *)buffer + dolfile->data_pos[i], dolchunksize[dolchunkcount]);
		DCFlushRange (dolchunkoffset[dolchunkcount], dolchunksize[dolchunkcount]);
		ICInvalidateRange (dolchunkoffset[dolchunkcount],dolchunksize[dolchunkcount]);

		dolchunkcount++;
	}
	return dolfile->entry_point;
}

void PatchChannel(u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes)
{
	int i;
	bool hookpatched = false;

	for (i=0;i < dolchunkcount;i++)
	{		
		if (!bootcontent_used)
		{
			patchVideoModes(dolchunkoffset[i], dolchunksize[i], vidMode, vmode, patchVidModes);
			if (vipatch) vidolpatcher(dolchunkoffset[i], dolchunksize[i]);
			if (configbytes[0] != 0xCD) langpatcher(dolchunkoffset[i], dolchunksize[i]);
			if (countryString) PatchCountryStrings(dolchunkoffset[i], dolchunksize[i]); // Country Patch by WiiPower
		}

		// Before this can be done, the codehandler needs to be in memory, and the code to patch needs to be in the right pace
		if (hooktype != 0)
			if (dogamehooks(dolchunkoffset[i], dolchunksize[i], true, bootcontent_used))
				hookpatched = true;
	}
	if (hooktype != 0 && !hookpatched)
	{
		gprintf("Error: Could not patch the hook\n");
		gprintf("Ocarina and debugger won't work\n");
	}
}

bool Identify_GenerateTik(signed_blob **outbuf, u32 *outlen)
{
	/* Allocate memory */
	signed_blob *buffer = (signed_blob *)MEM2_alloc(STD_SIGNED_TIK_SIZE);
	if (!buffer) return false;

	/* Clear buffer */
	memset(buffer, 0, STD_SIGNED_TIK_SIZE);

	/* Generate signature */
	sig_rsa2048 *signature=(sig_rsa2048 *)buffer;
	signature->type = ES_SIG_RSA2048;

	/* Generate ticket */
	tik *tik_data  = (tik *)SIGNATURE_PAYLOAD(buffer);

	strcpy(tik_data->issuer, "Root-CA00000001-XS00000003");
	memset(tik_data->cidx_mask, 0xFF, 32);

	/* Set values */
	*outbuf = buffer;
	*outlen = STD_SIGNED_TIK_SIZE;

	return true;
}

bool Identify(u64 titleid, u32 *ios)
{
	char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	
	sprintf(filepath, "/title/%08x/%08x/content/title.tmd", TITLE_UPPER(titleid), TITLE_LOWER(titleid));
	u32 tmdSize;
	u8 *tmdBuffer = ISFS_GetFile((u8 *) &filepath, &tmdSize, -1);
	if (tmdBuffer == NULL || tmdSize == 0)
	{
		gprintf("Reading TMD...Failed!\n");
		return false;
	}

	*ios = (u32)(tmdBuffer[0x18b]);

	u32 tikSize;
	signed_blob *tikBuffer = NULL;

	if(!Identify_GenerateTik(&tikBuffer,&tikSize))
	{
		gprintf("Generating fake ticket...Failed!");
		return false;
	}

	sprintf(filepath, "/sys/cert.sys");
	u32 certSize;
	u8 *certBuffer = ISFS_GetFile((u8 *) &filepath, &certSize, -1);
	if (certBuffer == NULL || certSize == 0)
	{
		gprintf("Reading certs...Failed!\n");
		SAFE_FREE(tmdBuffer);
		SAFE_FREE(tikBuffer);
		return false;
	}
	
	s32 ret = ES_Identify((signed_blob*)certBuffer, certSize, (signed_blob*)tmdBuffer, tmdSize, tikBuffer, tikSize, NULL);
	if (ret < 0)
	{
		switch(ret)
		{
			case ES_EINVAL:
				gprintf("Error! ES_Identify (ret = %d;) Data invalid!\n", ret);
				break;
			case ES_EALIGN:
				gprintf("Error! ES_Identify (ret = %d;) Data not aligned!\n", ret);
				break;
			case ES_ENOTINIT:
				gprintf("Error! ES_Identify (ret = %d;) ES not initialized!\n", ret);
				break;
			case ES_ENOMEM:
				gprintf("Error! ES_Identify (ret = %d;) No memory!\n", ret);
				break;
			default:
				gprintf("Error! ES_Identify (ret = %d)\n", ret);
				break;
		}
	}
	
	SAFE_FREE(tmdBuffer);
	SAFE_FREE(tikBuffer);
	SAFE_FREE(certBuffer);

	return ret < 0 ? false : true;
}

#define MAXAPPLOADERGAMES 7
static const char games[MAXAPPLOADERGAMES][3]=
{
	"WAL",	//ArtSyle: light trax
	"WDH",	//ArtSyle: Rotohex
	"WOB",	//ArtSyle: ORBIENT
	"WPR",	//ArtSyle: CUBELLO
	"WA8",	//ArtSyle: Penta Tentacles
	"WB7",	//Midnight Pool
	"WSP"	//Pokemon Rumble
};

u32 * GetDol(u64 title, char *id, u32 *contentSize, u16 bootcontent, bool skip_bootcontent)
{
	u32 i;
	for(i = 0; i < MAXAPPLOADERGAMES; i++)
		if(memcmp(id, &games[i], 3) == 0)
			skip_bootcontent = true;
	
	bootcontent_used = !skip_bootcontent;

	char filepath[ISFS_MAXPATH + 1];
	sprintf(filepath, "/title/%08x/%08x/content/%08x.app", TITLE_UPPER(title), TITLE_LOWER(title), bootcontent);
	
	if (skip_bootcontent) // skip_bootcontent to load without apploader
	{
		gprintf("Searching for main DOL...\n");
		bootcontent_used = !FindDol(title, filepath, bootcontent);
	}
	
	gprintf("Loading DOL: %s...", filepath);

	u32 *data = (u32 *) ISFS_GetFile((u8 *) &filepath, contentSize, -1);
	if (data != NULL)
	{	
		gprintf("Done!\n");
	
		if (isLZ77compressed((u8*)data))
		{
			u8 *decompressed;
			if (decompressLZ77content((u8 *)data, *contentSize, &decompressed, contentSize) < 0)
			{
				gprintf("Decompression failed\n");
				SAFE_FREE(data);
				return NULL;
			}
			SAFE_FREE(data);
			data = (u32 *)decompressed;
		}	
		return data;
	}
	gprintf("Failed!\n");
	return NULL;
}

bool FindDol(u64 title, char *DolPath, u16 bootcontent)
{
	u32 high = TITLE_UPPER(title);
	u32 low = TITLE_LOWER(title);

	u8 check[6] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
 
	char path[ISFS_MAXPATH + 1];
	sprintf(path, "/title/%08x/%08x/content", high, low);
	
	u32 Countall = 0;
	if(ISFS_ReadDir(path, NULL, &Countall) != ISFS_OK)
	{
		gprintf("Error: could not get dir entry count!\n");
		return false;
	}

	size_t bufferSize = ((12 + 1) * Countall) + 1;
	char *namesBuffer = (char *)MEM2_alloc(ALIGN32(bufferSize));
	char *holder = namesBuffer;
	if(namesBuffer == NULL)
	{
		gprintf("ERROR: could not allocate buffer for name list!\n");
		return false;
	}

	if(ISFS_ReadDir(path, namesBuffer, &Countall) != ISFS_OK)
	{
		gprintf("Reading content folder of the title failed!\n");
		SAFE_FREE(holder);
		return false;
	}
	DCFlushRange(namesBuffer, bufferSize);
	ICInvalidateRange(namesBuffer, bufferSize);

	while (strlen(namesBuffer) != 0)
	{
		if (strcasestr(namesBuffer, ".app") != NULL && strtoul(namesBuffer, NULL, 16) != bootcontent)
		{	
			sprintf(path, "/title/%08x/%08x/content/%s", high, low, namesBuffer);
			gprintf("Checking --> %s\n", path);

			u32 size = 0;
			u32 *data = (u32 *) ISFS_GetFile((u8 *) &path, &size, 6);
			if (data != NULL && size == 6)
			{
				if (isLZ77compressed((u8*)data))
				{
					gprintf("Found LZ77 %s compressed content --> %s\n", data[0] == 0x10 ? "0x10" : "0x11", namesBuffer);
					gprintf("This is most likely the main DOL, decompressing for checking\n");
					
					u32 *compressed = (u32 *) ISFS_GetFile((u8 *) &path, &size, -1);
					if (compressed != NULL)
					{
						u8 *decompressed;
						u32 decomp_size = 0;
						if (decompressLZ77content((u8 *)compressed, 32, &decompressed, &decomp_size) < 0)
						{
							gprintf("Decompressing file...Failed!\n");
							SAFE_FREE(compressed);
							continue;
						}
						memcpy(data, decompressed, 6);
						SAFE_FREE(decompressed);
						SAFE_FREE(compressed);
					}
				}
			
				if(memcmp(data, check, 6) == 0)
				{
					gprintf("Found DOL --> %s\n", namesBuffer);
					sprintf(DolPath, "%s", path);
					SAFE_FREE(holder);	
					SAFE_FREE(data);
					return true;
				} 
				SAFE_FREE(data);
			}
		}
		namesBuffer += strlen(namesBuffer) + 1;
	}
		
	SAFE_FREE(holder);	
	
	gprintf("No .dol found\n");
	return false;
}
