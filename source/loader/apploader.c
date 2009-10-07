#include <stdio.h>
#include <ogcsys.h>
#include <string.h>

#include "load_dol.h"
#include "apploader.h"
#include "wdvd.h"
#include "patchcode.h"
#include "disc.h"
#include "videopatch.h"

/*KENOBI! - FISHEARS*/
extern const unsigned char kenobiwii[];
extern const int kenobiwii_size;
/*KENOBI! - FISHEARS*/

typedef struct _SPatchCfg
{
	bool cheat;
	u8 vidMode;
	bool vipatch;
	bool countryString;
	u8 patchVidModes;
} SPatchCfg;

/* Apploader function pointers */
typedef int   (*app_main)(void **dst, int *size, int *offset);
typedef void  (*app_init)(void (*report)(const char *fmt, ...));
typedef void *(*app_final)();
typedef void  (*app_entry)(void (**init)(void (*report)(const char *fmt, ...)), int (**main)(), void *(**final)());

/* Apploader pointers */
static u8 *appldr = (u8 *)0x81200000;


/* Constants */
#define APPLDR_OFFSET	0x2440

/* Variables */
static u32 buffer[0x20] ATTRIBUTE_ALIGN(32);

static void dolPatches(void *dst, int len, void *params);
static void maindolpatches(void *dst, int len, bool cheat, u8 vidMode, bool vipatch, bool countryString, bool err002fix, u8 patchVidModes);
static bool Remove_001_Protection(void *Address, int Size);
static void Anti_002_fix(void *Address, int Size);

static void __noprint(const char *fmt, ...)
{
}

static bool compare_videomodes(GXRModeObj* mode1, GXRModeObj* mode2)
{
	return memcmp(mode1, mode2, sizeof *mode1) == 0;	// padding seems to always be 0
}


static void patch_videomode(GXRModeObj* mode1, GXRModeObj* mode2)
{
	memcpy(mode1, mode2, sizeof *mode1);
}

static GXRModeObj* PAL2NTSC[]={
	&TVMpal480IntDf,		&TVNtsc480IntDf,
	&TVPal264Ds,			&TVNtsc240Ds,
	&TVPal264DsAa,			&TVNtsc240DsAa,
	&TVPal264Int,			&TVNtsc240Int,
	&TVPal264IntAa,			&TVNtsc240IntAa,
	&TVPal524IntAa,			&TVNtsc480IntAa,
	&TVPal528Int,			&TVNtsc480IntAa,
	&TVPal528IntDf,			&TVNtsc480IntDf,
	&TVPal574IntDfScale,	&TVNtsc480IntDf,
	&TVEurgb60Hz240Ds,		&TVNtsc240Ds,
	&TVEurgb60Hz240DsAa,	&TVNtsc240DsAa,
	&TVEurgb60Hz240Int,		&TVNtsc240Int,
	&TVEurgb60Hz240IntAa,	&TVNtsc240IntAa,
	&TVEurgb60Hz480Int,		&TVNtsc480IntAa,
	&TVEurgb60Hz480IntDf,	&TVNtsc480IntDf,
	&TVEurgb60Hz480IntAa,	&TVNtsc480IntAa,
	&TVEurgb60Hz480Prog,	&TVNtsc480Prog,
	&TVEurgb60Hz480ProgSoft,&TVNtsc480Prog,
	&TVEurgb60Hz480ProgAa,  &TVNtsc480Prog,
	0,0
};

static GXRModeObj* NTSC2PAL[]={
	&TVNtsc240Ds,			&TVPal264Ds,
	&TVNtsc240DsAa,			&TVPal264DsAa,
	&TVNtsc240Int,			&TVPal264Int,
	&TVNtsc240IntAa,		&TVPal264IntAa,
	&TVNtsc480IntDf,		&TVPal528IntDf,
	&TVNtsc480IntAa,		&TVPal524IntAa,
	&TVNtsc480Prog,			&TVPal528IntDf,
	0,0
};

static GXRModeObj* NTSC2PAL60[]={
	&TVNtsc240Ds,			&TVEurgb60Hz240Ds,
	&TVNtsc240DsAa,			&TVEurgb60Hz240DsAa,
	&TVNtsc240Int,			&TVEurgb60Hz240Int,
	&TVNtsc240IntAa,		&TVEurgb60Hz240IntAa,
	&TVNtsc480IntDf,		&TVEurgb60Hz480IntDf,
	&TVNtsc480IntAa,		&TVEurgb60Hz480IntAa,
	&TVNtsc480Prog,			&TVEurgb60Hz480Prog,
	0,0
};

static bool Search_and_patch_Video_Modes(void *Address, u32 Size, GXRModeObj* Table[])
{
	u8 *Addr = (u8 *)Address;
	bool found = 0;
	u32 i;

	while(Size >= sizeof(GXRModeObj))
	{
		for(i = 0; Table[i]; i+=2)
		{
			if(compare_videomodes(Table[i], (GXRModeObj*)Addr))
			{
				found = 1;
				patch_videomode((GXRModeObj*)Addr, Table[i+1]);
				Addr += (sizeof(GXRModeObj)-4);
				Size -= (sizeof(GXRModeObj)-4);
				break;
			}
		}
		Addr += 4;
		Size -= 4;
	}
	return found;
}

s32 Apploader_Run(entry_point *entry, bool cheat, u8 vidMode, bool vipatch, bool countryString, bool error002Fix, const u8 *altdol, u32 altdolLen, u8 patchVidModes)
{
	void *dst = NULL;
	int len = 0;
	int offset = 0;
	app_entry appldr_entry;
	app_init  appldr_init;
	app_main  appldr_main;
	app_final appldr_final;

	u32 appldr_len;
	s32 ret;

	SYS_SetArena1Hi((void *)0x816FFFF0);
	/* Read apploader header */
	ret = WDVD_Read(buffer, 0x20, APPLDR_OFFSET);
	if (ret < 0)
		return ret;

	/* Calculate apploader length */
	appldr_len = buffer[5] + buffer[6];

	/* Read apploader code */
	// Either you limit memory usage or you don't touch the heap after that, because this is writing at 0x1200000
	ret = WDVD_Read(appldr, appldr_len, APPLDR_OFFSET + 0x20);
	if (ret < 0)
		return ret;
	DCFlushRange(appldr, appldr_len);

	/* Set apploader entry function */
	appldr_entry = (app_entry)buffer[4];

	/* Call apploader entry */
	appldr_entry(&appldr_init, &appldr_main, &appldr_final);

	/* Initialize apploader */
	appldr_init(__noprint);

	if (cheat)
	{
		/*HOOKS STUFF - FISHEARS*/
		memcpy((void *)0x80001800, kenobiwii, kenobiwii_size);
		DCFlushRange((void *)0x80001800, kenobiwii_size);
		hooktype = 1;
		memcpy((void *)0x80001800, (char *)0x80000000, 6);	// For WiiRD
		/*HOOKS STUFF - FISHEARS*/
	}

	while (appldr_main(&dst, &len, &offset))
	{
		/* Read data from DVD */
		WDVD_Read(dst, len, (u64)(offset << 2));
		maindolpatches(dst, len, cheat, vidMode, vipatch, countryString, error002Fix, patchVidModes);
	}
	WDVD_Close();
	/* Alternative dol */
	if (altdol != 0)
	{
		SPatchCfg patchCfg;
		patchCfg.cheat = cheat;
		patchCfg.vidMode = vidMode;
		patchCfg.vipatch = vipatch;
		patchCfg.countryString = countryString;
		patchCfg.patchVidModes = patchVidModes;
		void *altEntry = (void *)load_dol(altdol, altdolLen, dolPatches, &patchCfg);
		if (altEntry == 0)
			return -1;
		*entry = altEntry;
	}
	else
		/* Set entry point from apploader */
		*entry = appldr_final();
	/* ERROR 002 fix (WiiPower) */
	if (error002Fix)
		*(u32 *)0x80003140 = *(u32 *)0x80003188;
//		*(u32 *)0x80003188 = *(u32 *)0x80003140;

	return 0;
}

static void dolPatches(void *dst, int len, void *params)
{
	const SPatchCfg *p = (const SPatchCfg *)params;

	maindolpatches(dst, len, p->cheat, p->vidMode, p->vipatch, p->countryString, false, p->patchVidModes);
	Remove_001_Protection(dst, len);
}

static void PatchCountryStrings(void *Address, int Size)
{
	u8 SearchPattern[4] = { 0x00, 0x00, 0x00, 0x00 };
	u8 PatchData[4] = { 0x00, 0x00, 0x00, 0x00 };
	u8 *Addr = (u8*)Address;
	int wiiregion = CONF_GetRegion();

	switch (wiiregion)
	{
		case CONF_REGION_JP:
			SearchPattern[0] = 0x00;
			SearchPattern[1] = 'J';
			SearchPattern[2] = 'P';
			break;
		case CONF_REGION_EU:
			SearchPattern[0] = 0x02;
			SearchPattern[1] = 'E';
			SearchPattern[2] = 'U';
			break;
		case CONF_REGION_KR:
			SearchPattern[0] = 0x04;
			SearchPattern[1] = 'K';
			SearchPattern[2] = 'R';
			break;
		case CONF_REGION_CN:
			SearchPattern[0] = 0x05;
			SearchPattern[1] = 'C';
			SearchPattern[2] = 'N';
			break;
		case CONF_REGION_US:
		default:
			SearchPattern[0] = 0x01;
			SearchPattern[1] = 'U';
			SearchPattern[2] = 'S';
	}
	switch (((const u8 *)0x80000000)[3])
	{
		case 'J':
			PatchData[1] = 'J';
			PatchData[2] = 'P';
			break;
		case 'D':
		case 'F':
		case 'P':
		case 'X':
		case 'Y':
			PatchData[1] = 'E';
			PatchData[2] = 'U';
			break;

		case 'E':
		default:
			PatchData[1] = 'U';
			PatchData[2] = 'S';
	}
	while (Size >= 4)
		if (Addr[0] == SearchPattern[0] && Addr[1] == SearchPattern[1] && Addr[2] == SearchPattern[2] && Addr[3] == SearchPattern[3])
		{
			//*Addr = PatchData[0];
			Addr += 1;
			*Addr = PatchData[1];
			Addr += 1;
			*Addr = PatchData[2];
			Addr += 1;
			//*Addr = PatchData[3];
			Addr += 1;
			Size -= 4;
		}
		else
		{
			Addr += 4;
			Size -= 4;
		}
}

static void __Patch_CoverRegister(void *buffer, u32 len)
{
	static const u8 oldcode[] = { 0x54, 0x60, 0xF7, 0xFF, 0x40, 0x82, 0x00, 0x0C, 0x54, 0x60, 0x07, 0xFF, 0x41, 0x82, 0x00, 0x0C };
	static const u8 newcode[] = { 0x54, 0x60, 0xF7, 0xFF, 0x40, 0x82, 0x00, 0x0C, 0x54, 0x60, 0x07, 0xFF, 0x48, 0x00, 0x00, 0x0C };
	int n;

   /* Patch cover register */
	for (n = 0; n < len - sizeof oldcode; n += 4)
		if (memcmp(buffer + n, (void *)oldcode, sizeof oldcode) == 0) 
			memcpy(buffer + n, (void *)newcode, sizeof newcode);
}

static void maindolpatches(void *dst, int len, bool cheat, u8 vidMode, bool vipatch, bool countryString, bool err002fix, u8 patchVidModes)
{
	GXRModeObj **table = 0;

	__Patch_CoverRegister(dst, len);
	if (vidMode == 4) // patch
	{
		switch (CONF_GetVideo())
		{
			case CONF_VIDEO_PAL:
				table = CONF_GetEuRGB60() > 0 ? NTSC2PAL60 : NTSC2PAL;
				break;
			case CONF_VIDEO_MPAL:
				table = NTSC2PAL;
				break;
			default:
				table = PAL2NTSC;
				break;
		}
		Search_and_patch_Video_Modes(dst, len, table);
	}
	if (cheat)
		dogamehooks(dst, len);
	if (vipatch)
		vidolpatcher(dst, len);
	if (configbytes[0] != 0xCD)
		langpatcher(dst, len);
	// 
	if (err002fix && ((IOS_GetVersion() == 249 && IOS_GetRevision() < 13) || IOS_GetVersion() == 250))
		Anti_002_fix(dst, len);
	// Country Patch by WiiPower
	if (countryString)
		PatchCountryStrings(dst, len);
	// Patch video modes
	if (patchVidModes > 0)
	{
		GXRModeObj *vmode = 0;
		bool progressive = (CONF_GetProgressiveScan() > 0) && VIDEO_HaveComponentCable();
		switch (vidMode)
		{
			case 1:
				vmode = &TVPal528IntDf;
				break;
			case 2:
				vmode = progressive ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
				break;
			case 3:
				vmode = progressive ? &TVNtsc480Prog : &TVNtsc480IntDf;
				break;
		}
		if (vmode != 0)
			applyVideoPatch(dst, len, vmode, patchVidModes - 1);
	}
	DCFlushRange(dst, len);
}

static bool Remove_001_Protection(void *Address, int Size)
{
	static const u8 SearchPattern[] = { 0x40, 0x82, 0x00, 0x0C, 0x38, 0x60, 0x00, 0x01, 0x48, 0x00, 0x02, 0x44, 0x38, 0x61, 0x00, 0x18 };
	static const u8 PatchData[] = 	{ 0x40, 0x82, 0x00, 0x04, 0x38, 0x60, 0x00, 0x01, 0x48, 0x00, 0x02, 0x44, 0x38, 0x61, 0x00, 0x18 };
	u8 *Addr_end = Address + Size;
	u8 *Addr;

	for (Addr = Address; Addr <= Addr_end - sizeof SearchPattern; Addr += 4)
		if (memcmp(Addr, SearchPattern, sizeof SearchPattern) == 0) 
		{
			memcpy(Addr, PatchData, sizeof PatchData);
			return true;
		}
	return false;
}

static void Anti_002_fix(void *Address, int Size)
{
	static const u8 SearchPattern[] = { 0x2C, 0x00, 0x00, 0x00, 0x48, 0x00, 0x02, 0x14, 0x3C, 0x60, 0x80, 0x00 };
	static const u8 PatchData[] = 	{ 0x2C, 0x00, 0x00, 0x00, 0x40, 0x82, 0x02, 0x14, 0x3C, 0x60, 0x80, 0x00 };
	void *Addr = Address;
	void *Addr_end = Address + Size;

	while (Addr <= Addr_end - sizeof SearchPattern)
	{
		if (memcmp(Addr, SearchPattern, sizeof SearchPattern) == 0) 
			memcpy(Addr, PatchData, sizeof PatchData);
		Addr += 4;
	}
}
