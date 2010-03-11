#include <stdio.h>
#include <ogcsys.h>
#include <stdlib.h>

#include "sys.h"
#include "gecko.h"

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
}

void Sys_Exit(int ret)
{
	if (return_to_menu)
		Sys_LoadMenu();
	else
		exit(ret);
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
	/* Restart console */
	STM_RebootSystem();
}

void Sys_Shutdown(void)
{
	/* Poweroff console */
	if(CONF_GetShutdownMode() == CONF_SHUTDOWN_IDLE) {
		s32 ret;

		/* Set LED mode */
		ret = CONF_GetIdleLedMode();
		if(ret >= 0 && ret <= 2)
			STM_SetLedMode(ret);

		/* Shutdown to idle */
		STM_ShutdownToIdle();
	} else {
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
	if (ret > 0) {
		*certs = certificates;
		*len   = sizeof(certificates);
	}

	return ret;
}

bool Sys_SupportsExternalModule(void)
{
	u32 version = IOS_GetVersion();
	u32 revision = IOS_GetRevision();
	
	bool retval = revision == 4 && (version == 222 || version == 223 || version == 224);
	gprintf("IOS Version: %d, Revision %d, returning %d\n", version, revision, retval);
	return retval;
}
