#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <ogc/lwp_watchdog.h>
#include <wiiuse/wpad.h>
#include <ogc/machine/processor.h>

#include "apploader.h"
#include "disc.h"
#include "wdvd.h"
#include "sys.h"
#include "fat.h"
#include "videopatch.h"

#define ALIGNED(x) __attribute__((aligned(x)))

/* Constants */
#define PTABLE_OFFSET	0x40000
#define WII_MAGIC	0x5D1C9EA3

/* Disc pointers */
static u32 buffer[0x20] ALIGNED(32);
static u8  *diskid = (u8  *)0x80000000;
static char gameid[6 + 1];

GXRModeObj *vmode = NULL;
u32 vmode_reg = 0;

static u8	Tmd_Buffer[0x49e4 + 0x1C] ALIGNED(32);

#define        Sys_Magic		((u32*)0x80000020)
#define        Version			((u32*)0x80000024)
#define        Arena_L			((u32*)0x80000030)
#define        BI2				((u32*)0x800000f4)
#define        Bus_Speed		((u32*)0x800000f8)
#define        CPU_Speed		((u32*)0x800000fc)

void __Disc_SetLowMem(void)
{
	// Patch in info missing from apploader reads
	*Sys_Magic	= 0x0d15ea5e;
	*Version	= 1;
	*Arena_L	= 0x00000000;
	*Bus_Speed	= 0x0E7BE2C0;
	*CPU_Speed	= 0x2B73A840;

	/* Setup low memory */
	*(vu32 *)0x80000030 = 0x00000000;
	*(vu32 *)0x80000060 = 0x38A00040;
	*(vu32 *)0x800000E4 = 0x80431A80;
	*(vu32 *)0x800000EC = 0x81800000;
	*(vu32 *)0x800000F0 = 0x01800000;       // Simulated Memory Size
	*BI2 = 0x817E5480;
	*(vu32 *)0x800000F8 = 0x0E7BE2C0;

	// From NeoGamme R4 (WiiPower)
	*(vu32 *)0x800030F0 = 0x0000001C;
	*(vu32 *)0x8000318C = 0x00000000;
	*(vu32 *)0x80003190 = 0x00000000;
	*(vu32 *)0x800000FC = 0x2B73A840;

	/* Copy disc ID (online check) */
	memcpy((void *)0x80003180, (void *)0x80000000, 4);

	/* Flush cache */
	DCFlushRange((void *)0x80000000, 0x3F00);
}

void __Disc_SelectVMode(u8 videoselected)
{
    vmode = VIDEO_GetPreferredMode(0);

	/* Get video mode configuration */
	bool progressive = (CONF_GetProgressiveScan() > 0) && VIDEO_HaveComponentCable();

	/* Select video mode register */
	switch (CONF_GetVideo())
	{
		case CONF_VIDEO_PAL:
			vmode_reg = (CONF_GetEuRGB60() > 0) ? 5 : 1;
			break;

		case CONF_VIDEO_MPAL:
			vmode_reg = 2;
			break;

		case CONF_VIDEO_NTSC:
			vmode_reg = 0;
			break;
	}

    switch (videoselected)
	{
		case 0: // DEFAULT (DISC)
			/* Select video mode */
			switch (diskid[3])
			{
				// PAL
				case 'D':
				case 'F':
				case 'P':
				case 'X':
				case 'Y':
					if (CONF_GetVideo() != CONF_VIDEO_PAL)
					{
						vmode_reg = 1;
						vmode = progressive ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
					}
					break;
				// NTSC
				case 'E':
				case 'J':
				default:
					if (CONF_GetVideo() != CONF_VIDEO_NTSC)
					{
						vmode_reg = 0;
						vmode = progressive ? &TVNtsc480Prog : &TVNtsc480IntDf;
					}
					break;
			}
			break;
		case 1: // PAL50
			vmode =  &TVPal528IntDf;
			vmode_reg = vmode->viTVMode >> 2;
			break;
		case 2: // PAL60
			vmode = progressive ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
			vmode_reg = progressive ? TVEurgb60Hz480Prog.viTVMode >> 2 : vmode->viTVMode >> 2;
			break;
		case 3: // NTSC
			vmode = progressive ? &TVNtsc480Prog : &TVNtsc480IntDf;
			vmode_reg = vmode->viTVMode >> 2;
			break;
		case 4: // AUTO PATCH
		default:
			break;
	}
}

void __Disc_SetVMode(void)
{
	/* Set video mode register */
	*(vu32 *)0x800000CC = vmode_reg;

	/* Set video mode */
	if (vmode != 0)
	{
		VIDEO_Configure(vmode);

		/* Setup video */
		VIDEO_SetBlack(FALSE);
		VIDEO_Flush();
		VIDEO_WaitVSync();
		if (vmode->viTVMode & VI_NON_INTERLACE)
			VIDEO_WaitVSync();
	}
}

void __Disc_SetTime(void)
{
	/* Set proper time */
	settime(secs_to_ticks(time(NULL) - 946684800));
}

s32 __Disc_FindPartition(u64 *outbuf)
{
	u64 offset = 0, table_offset = 0;
	u32 cnt, nb_partitions;
	s32 ret;

	/* Read partition info */
	ret = WDVD_UnencryptedRead(buffer, 0x20, PTABLE_OFFSET);
	if (ret < 0)
		return ret;

	/* Get data */
	nb_partitions = buffer[0];
	table_offset  = buffer[1] << 2;
	
	if (nb_partitions > 8)
		return -1;

	/* Read partition table */
	ret = WDVD_UnencryptedRead(buffer, 0x20, table_offset);
	if (ret < 0)
		return ret;

	/* Find game partition */
	for (cnt = 0; cnt < nb_partitions; cnt++) {
		u32 type = buffer[cnt * 2 + 1];

		/* Game partition */
		if(!type)
			offset = buffer[cnt * 2] << 2;
	}

	/* No game partition found */
	if (!offset)
		return -1;

	/* Set output buffer */
	*outbuf = offset;

	WDVD_Seek(offset);

	return 0;
}


s32 Disc_Init(void)
{
	/* Init DVD subsystem */
	return WDVD_Init();
}

s32 Disc_Open(void)
{
	s32 ret;

	/* Reset drive */
	ret = WDVD_Reset();
	if (ret < 0)
		return ret;

	memset(diskid, 0, 32);

	/* Read disc ID */
	return WDVD_ReadDiskId(diskid);
}

s32 Disc_Wait(void)
{
	u32 cover = 0;
	s32 ret;

	/* Wait for disc */
	while (!(cover & 0x2)) {
		/* Get cover status */
		ret = WDVD_GetCoverStatus(&cover);
		if (ret < 0)
			return ret;
	}

	return 0;
}

s32 Disc_SetWBFS(u32 mode, u8 *id)
{
	/* Set WBFS mode */
	if (id != 0)
	{
		memcpy(gameid, id, sizeof gameid - 1);
		gameid[sizeof gameid - 1] = 0;
	}
	else
		memset(gameid, 0, sizeof gameid);
	return WDVD_SetWBFSMode(mode, id);
}

s32 Disc_ReadHeader(void *outbuf)
{
	/* Read disc header */
	return WDVD_UnencryptedRead(outbuf, sizeof(struct discHdr), 0);
}

s32 Disc_IsWii(void)
{
	struct discHdr *header = (struct discHdr *)buffer;

	s32 ret;

	/* Read disc header */
	ret = Disc_ReadHeader(header);
	if (ret < 0)
		return ret;

	/* Check magic word */
	if (header->magic != WII_MAGIC)
		return -1;

	return 0;
}


s32 Disc_BootPartition(u64 offset, u8 vidMode, const u8 *cheat, u32 cheatSize, bool vipatch, bool countryString, bool error002Fix, const u8 *altdol, u32 altdolLen, u8 patchVidMode)
{
	entry_point p_entry;

	s32 ret = WDVD_OpenPartition(offset, 0, 0, 0, Tmd_Buffer);
	if (ret < 0)
		return ret;

	/* Disconnect Wiimote */
    WPAD_Flush(0);
    WPAD_Disconnect(0);
    WPAD_Shutdown();

	/* Setup low memory */;
	__Disc_SetLowMem();

	/* Select an appropriate video mode */
	__Disc_SelectVMode(vidMode);

	/* Run apploader */
	ret = Apploader_Run(&p_entry, cheat != 0, vidMode, vmode, vipatch, countryString, error002Fix, altdol, altdolLen, patchVidMode);
	if (ret < 0)
		return ret;

	/* Set an appropriate video mode */
	__Disc_SetVMode();

	if (cheat != 0)
	{
		memcpy((void *)0x800027E8, cheat, cheatSize);
		*(vu8 *)0x80001807 = 0x01;
	}
	DCFlushRange((void*)0x80000000, 0xA00000);

	/* Set time */
	__Disc_SetTime();

	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	VIDEO_WaitVSync();

	usleep(100 * 1000);

	/* Shutdown IOS subsystems */
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);

	/* Jump to entry point */
	p_entry();

	return 0;
}

s32 Disc_OpenPartition(u32 mode, u8 *id)
{
	u64 offset;

	if (Disc_SetWBFS(mode, id) < 0)
		return -1;
	if (Disc_Open() < 0)
		return -2;
	if (__Disc_FindPartition(&offset) < 0)
		return -3;
	if (WDVD_OpenPartition(offset, 0, 0, 0, Tmd_Buffer) < 0)
		return -4;
	return 0;
}

s32 Disc_WiiBoot(u8 vidMode, const u8 *cheat, u32 cheatSize, bool vipatch, bool countryString, bool error002Fix, const u8 *altdol, u32 altdolLen, u8 patchVidModes)
{
	u64 offset;
	s32 ret;

	/* Find game partition offset */
	ret = __Disc_FindPartition(&offset);
	if (ret < 0)
		return ret;

	/* Boot partition */
	return Disc_BootPartition(offset, vidMode, cheat, cheatSize, vipatch, countryString, error002Fix, altdol, altdolLen, patchVidModes);
}
