#ifndef _SYS_H_
#define _SYS_H_

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define IOS_TYPE_UNK    	0
#define IOS_TYPE_WANIN  	1
#define IOS_TYPE_HERMES 	2
#define IOS_TYPE_KWIIRK 	3
#define IOS_TYPE_D2X		4

#define HBC_108				0x00010001af1bf516ULL
#define HBC_JODI			0x0001000148415858ULL
#define HBC_HAXX			0x000100014a4f4449ULL

#define EXIT_TO_MENU 		0
#define EXIT_TO_HBC 		1
#define EXIT_TO_PRIILOADER 	2
#define EXIT_TO_DISABLE 	3
#define EXIT_TO_BOOTMII 	4
#define EXIT_TO_WIIFLOW		5

	/* Prototypes */
	void Sys_Init(void);
	void Sys_Reboot(void);
	void Sys_Shutdown(void);
	void Sys_LoadMenu(void);
	bool Sys_Exiting(void);
	void Sys_Test(void);
	void Sys_Exit(int);
	void Sys_ExitTo(int);
	bool Sys_SupportsExternalModule(bool part_select);

	s32  Sys_GetCerts(signed_blob **, u32 *);

	void Open_Inputs(void);
	void Close_Inputs(void);

	int get_ios_type();
	int is_ios_type(int type);

	u32 get_ios_info_from_tmd();
	u32 get_ios_info(signed_blob *TMD, u32 size);
	bool shadow_mload();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
