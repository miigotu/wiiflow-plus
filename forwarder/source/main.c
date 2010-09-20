 /****************************************************************************
 * Copyright 2009 The Lemon Man and thanks to luccax, Wiipower, Aurelio and crediar
 * Copyright 2010 Dimok
 *
 * Original forwarder source by
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ogc/machine/processor.h>
#include <wiiuse/wpad.h>

#include "video.h"
#include "background_image.h"
#include "dolloader.h"
#include "filelist.h"
#include "devicemounter.h"

void __exception_setreload(int t);

int main(int argc, char **argv)
{
	int i;
	u32 cookie;
	FILE *exeFile = NULL;
	void * exeBuffer = (void *)EXECUTABLE_MEM_ADDR;
	u32 exeSize = 0;
	u32 exeEntryPointAddress = 0;
	entrypoint exeEntryPoint = NULL;
	__exception_setreload(0);

	//Initialize Wiimote
	WPAD_Init();

	/* check devices */
	if ((SDCard_Init() == -1) && (USBDevice_Init() == -1))
	{
		SDCard_deInit();
		USBDevice_deInit();
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	}

	/* int videomod */
	InitVideo();

	/* get imagedata */
	u8 * imgdata = GetImageData();
	fadein(imgdata);

	bool hbc = false;
	bool reboot = false;
	
    char filepath[50];
	char argIOS[15];
	
	u32 countdown = 5000000;

	while(countdown > 0)
	{
		WPAD_ScanPads();
		u32 buttons = WPAD_ButtonsDown(0);
		u32 buttonsheld = WPAD_ButtonsHeld(0);
		
		if((buttons & WPAD_BUTTON_B) || (buttonsheld & WPAD_BUTTON_B))
		{
			if((buttons & WPAD_BUTTON_LEFT) || (buttonsheld & WPAD_BUTTON_LEFT))
			{
				sprintf(argIOS, "ios=222-mload");
				goto out;
			}
			else if((buttons & WPAD_BUTTON_UP) || (buttonsheld & WPAD_BUTTON_UP))
			{
				sprintf(argIOS, "ios=223-mload");
				goto out;
			}
			else if((buttons & WPAD_BUTTON_RIGHT) || (buttonsheld & WPAD_BUTTON_RIGHT))
			{
				sprintf(argIOS, "ios=224-mload");
				goto out;
			}
			else if((buttons & WPAD_BUTTON_DOWN) || (buttonsheld & WPAD_BUTTON_DOWN))
			{
				sprintf(argIOS, "ios=250");
				goto out;
			}
			else if((buttons & WPAD_BUTTON_PLUS) || (buttonsheld & WPAD_BUTTON_PLUS))
			{
				hbc = true;
				goto out;
			}
			else if((buttons & WPAD_BUTTON_HOME) || (buttonsheld & WPAD_BUTTON_HOME))
			{
				DCFlushRange((void*)0x8132FFFB,4);//Clear any magic words set.
				*(vu32*)0x8132FFFB = 0x50756E65; //magic word set to Pune
				reboot = true;
				goto out;
			}
			else if((buttons & WPAD_BUTTON_MINUS) || (buttonsheld & WPAD_BUTTON_MINUS))
			{
				DCFlushRange((void*)0x8132FFFB,4);//Clear any magic words set.
				*(vu32*)0x8132FFFB = 0x4461636F; //magic word set to Daco
				reboot = true;
				goto out;
			}
		}
		if (!(buttons || buttonsheld)) countdown--;
	}
out:
	//Shutdown wiimotes.
	for(i=0; i<4; i++)
	{
		WPAD_Flush(i);
		WPAD_Disconnect(i);
	}
	WPAD_Shutdown();

	if (!(hbc || reboot))
	{
		int dev;

		for(dev = SD; dev < MAXDEVICES; ++dev)
		{
			if(exeFile == NULL)
			{
				sprintf(filepath, "%s:/apps/wiiflow/boot.dol", DeviceName[dev]);
				exeFile = fopen(filepath ,"rb");
			}
			if(exeFile != NULL)
				break;
		}

		// if nothing found exiting
		if(exeFile == NULL)
		{
			fadeout(imgdata);
			fclose(exeFile);
			SDCard_deInit();
			USBDevice_deInit();
			StopGX();
			free(imgdata);
			SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
		}

		fseek(exeFile, 0, SEEK_END);
		exeSize = ftell(exeFile);
		rewind(exeFile);

		if(fread(exeBuffer, 1, exeSize, exeFile) != exeSize)
		{
			fadeout(imgdata);
			fclose(exeFile);
			SDCard_deInit();
			USBDevice_deInit();
			StopGX();
			free(imgdata);
			SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
		}
		fclose(exeFile);

		/* load entry point */
		struct __argv args;
		bzero(&args, sizeof(args));
		args.argvMagic = ARGV_MAGIC;
		args.length = strlen(filepath) + strlen(argIOS) + 2;
		args.commandLine = (char*)malloc(args.length);
		if (!args.commandLine) SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
		strcpy(args.commandLine, filepath);
		strcpy(&args.commandLine[strlen(filepath) + 1], argIOS);
		args.commandLine[strlen(filepath)] = '\0';
		args.commandLine[args.length - 1] = '\0';
		args.argc = 2;
		args.argv = &args.commandLine;
		args.endARGV = args.argv + 1;

		u8 * appboot_buff = (u8 *) malloc(app_booter_dol_size);
		if(!appboot_buff)
		{
			fadeout(imgdata);
			SDCard_deInit();
			USBDevice_deInit();
			StopGX();
			free(imgdata);
			SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
		}

		memcpy(appboot_buff, app_booter_dol, app_booter_dol_size);

		exeEntryPointAddress = load_dol_image(appboot_buff, &args);

		if(appboot_buff)
			free(appboot_buff);
	}
	fadeout(imgdata);
	SDCard_deInit();
	USBDevice_deInit();
	StopGX();
	free(imgdata);

	if(!(hbc || reboot))
	{
		if(exeEntryPointAddress == 0)
			SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);

		exeEntryPoint = (entrypoint) exeEntryPointAddress;
	}

	/* cleaning up and load dol */
	if(hbc) 
	{
		if(WII_LaunchTitle(0x00010001af1bf516ULL)<0)//HBC 1.0.8
			if(WII_LaunchTitle(0x0001000148415858ULL)<0)//HAXX
				if(WII_LaunchTitle(0x000100014a4f4449ULL)<0)//JODI
					SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	}	
	else if(reboot)
		SYS_ResetSystem(SYS_RETURNTOMENU,0,0);
	else
		SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);

	_CPU_ISR_Disable(cookie);
	__exception_closeall();
	exeEntryPoint();
	_CPU_ISR_Restore (cookie);
	return 0;
}
