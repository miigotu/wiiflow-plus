/****************************************************************************
 * WiiMC
 * Tantric 2009-2010
 * Modified for wiiflow by Miigotu
 ***************************************************************************/

#include <stdio.h>
#include <gccore.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <ogc/machine/processor.h>
#include <sys/iosupport.h>
#include <wiiuse/wpad.h>

#include "gecko.h"
#include "pngu.h"
#include "fileop.h"

extern bool geckoinit;
int mload_init();
int mload_close();
bool load_ehci_module();
void USB2Enable(bool e);
extern void __exception_closeall();
typedef void (*entrypoint) (void);
u32 load_dol_image (void *dolstart, struct __argv *argv);

extern const u8		background_png[];
extern const u32	background_png_size;
extern const u8		background_wide_png[];
extern const u32	background_wide_png_size;

void InitVideo();
void StopGX();
void Menu_Render();
void Menu_DrawImg(f32 xpos, f32 ypos, u16 width, u16 height, u8 data[], f32 degrees, f32 scaleX, f32 scaleY, u8 alphaF );

static ssize_t __out_write(struct _reent *r, int fd, const char *ptr, size_t len)
{
	return -1;
}

const devoptab_t phony_out = 
{ "stdout",0,NULL,NULL,__out_write,
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, NULL,0,NULL,NULL,NULL,NULL,NULL };

#define HW_REG_BASE   0xcd800000
#define HW_ARMIRQMASK (HW_REG_BASE + 0x03c)
#define HW_ARMIRQFLAG (HW_REG_BASE + 0x038)

int have_hw_access()
{
	if((*(volatile unsigned int*)HW_ARMIRQMASK)&&(*(volatile unsigned int*)HW_ARMIRQFLAG))
		return 1;
  return 0;
}

int main(int argc, char **argv)
{
	geckoinit = InitGecko();
	bool hbc = false;
	bool reboot = false;
	entrypoint exeEntryPoint =  NULL;
	
	void *buffer = (void *)0x92000000;
	
	devoptab_list[STD_OUT] = &phony_out; // to keep libntfs happy
	devoptab_list[STD_ERR] = &phony_out; // to keep libntfs happy

	// only reload IOS if AHBPROT is not enabled
	u32 version = IOS_GetVersion();
	if(version != 58 && have_hw_access() != 1)
	{
		IOS_ReloadIOS(202);
		if(mload_init() >= 0 && load_ehci_module())
		USB2Enable(true);
	}

	InitVideo();

	u8 *bg;
	int bgWidth, bgHeight;
	int a,i,j;

	if (CONF_GetAspectRatio() == CONF_ASPECT_16_9)
		bg = DecodePNG(background_wide_png, &bgWidth, &bgHeight);
	else
		bg = DecodePNG(background_png, &bgWidth, &bgHeight);

	for(a = 0; a <= 255; a+=15)
	{
		Menu_DrawImg(0, 0, bgWidth, bgHeight, bg, 0, 1, 1, a);
		Menu_Render();
	}	
	//Initialize Wiimote
	WPAD_Init();
	char filepath[34];
	char argIOS[15];
	u32 countdown = 5000000;
	sprintf(filepath, "apps/wiiflow/boot.dol");

	while(true)
	{
		WPAD_ScanPads();
		u32 buttons = WPAD_ButtonsDown(0);
		u32 buttonsheld = WPAD_ButtonsHeld(0);
		if (buttons & WPAD_BUTTON_B) || (buttonsheld & WPAD_BUTTON_B))
		{
	 		if((buttons & WPAD_BUTTON_LEFT) || (buttonsheld & WPAD_BUTTON_LEFT))
			{
				sprintf(argIOS, "ios=222-mload");
				break;
			}
			else if((buttons & WPAD_BUTTON_UP) || (buttonsheld & WPAD_BUTTON_UP))
			{
				sprintf(argIOS, "ios=223-mload");
				break;
			}
			else if((buttons & WPAD_BUTTON_RIGHT) || (buttonsheld & WPAD_BUTTON_RIGHT))
			{
				sprintf(argIOS, "ios=224-mload");
				break;
			}
			else if((buttons & WPAD_BUTTON_DOWN) || (buttonsheld & WPAD_BUTTON_DOWN))
			{
				sprintf(argIOS, "ios=250");
				break;
			}
			else if((buttons & WPAD_BUTTON_A) || (buttonsheld & WPAD_BUTTON_A))
			{
				sprintf(argIOS, "ios=249");
				break;
			}
			else  if((buttons & WPAD_BUTTON_PLUS) || (buttonsheld & WPAD_BUTTON_PLUS))
			{
				hbc = true;
				gprintf("Booting HBC\n");
				break;
			}
			else if((buttons & WPAD_BUTTON_HOME) || (buttonsheld & WPAD_BUTTON_HOME))
			{
				DCFlushRange((void*)0x8132FFFB,4);//Clear any magic words set.
				*(vu32*)0x8132FFFB = 0x50756E65; //magic word set to Pune
				gprintf("magic word changed to %x\n",*(vu32*)0x8132FFFB);
				reboot = true;
				gprintf("Booting System Menu\n");
				break;
			}
			else if((buttons & WPAD_BUTTON_MINUS) || (buttonsheld & WPAD_BUTTON_MINUS))
			{
				DCFlushRange((void*)0x8132FFFB,4);//Clear any magic words set.
				*(vu32*)0x8132FFFB = 0x4461636F; //magic word set to Daco
				gprintf("magic word changed to %x\n",*(vu32*)0x8132FFFB);
				reboot = true;
				gprintf("Booting Priiloader menu\n");
				break;
			}
		}
		else if (countdown == 0)
			break;

		countdown--;
	}
	if (hbc || reboot) goto found;

	// mount devices so we can look for file
	MountAllDevices();
	FILE *fp = NULL;
	for(i=0; i < 2; i++)
	{
		for(j=0; j < MAX_DEVICES; j++)
		{
			if(part[i][j].type == 0)
				continue;
			char fullpath[40];
			sprintf(fullpath, "%s:/%s", part[i][j].mount, filepath);
			gprintf("%s\n", fullpath);
			fp = fopen(fullpath, "rb");
			if(fp)
				goto found;
		}
	}
	if(!fp)
	{
		StopGX();
		gprintf("Dol not found!\n");
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	}

found:
	for(i=0; i<4; i++)
	{
		WPAD_Flush(i);
		WPAD_Disconnect(i);
	}
	WPAD_Shutdown();
	if (!hbc && !reboot)
	{
		fseek (fp, 0, SEEK_END);
		int len = ftell(fp);
		fseek (fp, 0, SEEK_SET);
		fread(buffer, 1, len, fp);
		fclose (fp);
	}
	UnmountAllDevices();
	mload_close();

	if (!hbc && !reboot)
	{
		// load entry point
		struct __argv args;
		bzero(&args, sizeof(args));
		args.argvMagic = ARGV_MAGIC;
		args.length = strlen(filepath) + 1 + strlen(argIOS) + 2;
		args.commandLine = (char*)malloc(args.length);
		strcpy(args.commandLine, filepath);
		strcpy(&args.commandLine[strlen(filepath) + 1], argIOS);
		args.commandLine[strlen(filepath)] = '\0';
		args.commandLine[args.length - 1] = '\0';
		args.commandLine[args.length - 2] = '\0';
		args.argc = 2;
		args.argv = &args.commandLine;
		args.endARGV = args.argv + 2;

		u32 exeEntryPointAddress = load_dol_image(buffer, &args);

		exeEntryPoint = (entrypoint) exeEntryPointAddress;
	}

	// cleanup

	u32 level;

	if (hbc) 
	{
		gprintf("Boot HBC\n");
		if (WII_LaunchTitle(0x00010001af1bf516ULL)<0)
			SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	}	
	else if (reboot)
	{
		gprintf("SYS_RETURNTOMENU\n");
		gprintf("magic word is: %x\n",*(vu32*)0x8132FFFB);
		SYS_ResetSystem(SYS_RETURNTOMENU,0,0);
	}

	_CPU_ISR_Disable(level);
	__exception_closeall();

	if (!hbc && !reboot) exeEntryPoint();
	_CPU_ISR_Restore(level);

	for(a = 255; a >= 0; a-=15)
	{
		Menu_DrawImg(0, 0, bgWidth, bgHeight, bg, 0, 1, 1, a);
		Menu_Render();
	}
	StopGX();
	VIDEO_WaitVSync();
	return 0;
}