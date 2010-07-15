#include <stdio.h>
#include <ogcsys.h>
#include <stdlib.h>
#include <wiiuse/wpad.h>
#include "sys.h"
#include "gecko.h"
#include "loader/playlog.h"

/* Constants */
#define CERTS_LEN	0x280

/* Variables */
static const char certs_fs[] ATTRIBUTE_ALIGN(32) = "/sys/cert.sys";
static bool reset = false;
static bool shutdown = false;
static bool return_to_menu = false;

bool Sys_Exiting(void)
{
	return reset || shutdown;
}

void Sys_Test(void)
{
	if (reset)
		Sys_Reboot();
	else if (shutdown)
		Sys_Shutdown();
}

void Sys_ExitToWiiMenu(bool b)
{
	return_to_menu = b;
	//magic word to force wii menu in priiloader.
	if (b)
	{
		DCFlushRange((void*)0x8132fffb,4);
		*(vu32*)0x8132fffb = 0x50756e65;
	}
	else
		DCFlushRange((void*)0x8132fffb,4);
	
}

void Sys_Exit(int ret)
{
	WPAD_Flush(0);
	WPAD_Disconnect(0);
	WPAD_Flush(1);
	WPAD_Disconnect(1);
	WPAD_Flush(2);
	WPAD_Disconnect(2);
	WPAD_Flush(3);
	WPAD_Disconnect(3);
    WPAD_Shutdown();
	if (return_to_menu)
	{
		Playlog_Delete();
		Sys_LoadMenu();
	}
	else
	{
		Playlog_Delete();
		exit(ret);
	}
}

void __Sys_ResetCallback(void)
{
	reset = true;
	/* Reboot console */
//	Sys_Reboot();
}

void __Sys_PowerCallback(void)
{
	shutdown = true;
	/* Poweroff console */
//	Sys_Shutdown();
}


void Sys_Init(void)
{
	/* Initialize video subsytem */
//	VIDEO_Init();

	/* Set RESET/POWER button callback */
	SYS_SetResetCallback(__Sys_ResetCallback);
	SYS_SetPowerCallback(__Sys_PowerCallback);
}

void Sys_Reboot(void)
{
	WPAD_Flush(0);
	WPAD_Disconnect(0);
	WPAD_Flush(1);
	WPAD_Disconnect(1);
	WPAD_Flush(2);
	WPAD_Disconnect(2);
	WPAD_Flush(3);
	WPAD_Disconnect(3);
    WPAD_Shutdown();

	/* Restart console */
	STM_RebootSystem();
}

void Sys_Shutdown(void)
{
	WPAD_Flush(0);
	WPAD_Disconnect(0);
	WPAD_Flush(1);
	WPAD_Disconnect(1);
	WPAD_Flush(2);
	WPAD_Disconnect(2);
	WPAD_Flush(3);
	WPAD_Disconnect(3);
    WPAD_Shutdown();

	/* Poweroff console */
	if(CONF_GetShutdownMode() == CONF_SHUTDOWN_IDLE)
	{
		s32 ret;

		/* Set LED mode */
		ret = CONF_GetIdleLedMode();
		if(ret >= 0 && ret <= 2)
			STM_SetLedMode(ret);

		/* Shutdown to idle */
		STM_ShutdownToIdle();
	}
	else
	{
		/* Shutdown to standby */
		STM_ShutdownToStandby();
	}
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
		*len   = sizeof(certificates);
	}

	return ret;
}

bool Sys_SupportsExternalModule(void)
{
//	u32 version = IOS_GetVersion();
	u32 revision = IOS_GetRevision();
	
	bool retval = (is_ios_type(IOS_TYPE_WANIN) && revision >= 18) || (is_ios_type(IOS_TYPE_HERMES) && revision >= 4);
	gprintf("IOS Version: %d, Revision %d, returning %d\n", IOS_GetVersion(), revision, retval);
	return retval;
}

int get_ios_type()
{
	switch (IOS_GetVersion()) {
		case 249:
		case 250:
			return IOS_TYPE_WANIN;
		case 222:
		case 223:
			if (IOS_GetRevision() == 1)
				return IOS_TYPE_KWIIRK;
		case 224:
			return IOS_TYPE_HERMES;
	}
	return IOS_TYPE_UNK;
}

int is_ios_type(int type)
{
	return (get_ios_type() == type);
}

