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

#define D2X_MIN_VERSION 	21000
#define D2X_MAX_VERSION 	21100

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
	void Sys_LoadMenu(void);
	bool Sys_Exiting(void);
	void Sys_Test(void);
	void Sys_Exit(int);
	void Sys_ExitTo(int);
	bool Sys_SupportsExternalModule(bool part_select);

	s32  Sys_GetCerts(signed_blob **, u32 *);
    s32 GetTMD(u64 TicketID, signed_blob **Output,  u32 *Length);
	
	void Open_Inputs(void);
	void Close_Inputs(void);
    char* str_seek_end(char **str, int *size);
	int get_ios_type();
	int is_ios_type(int type);

	void get_all_ios_info_str(int i, char *str, int size);
	char* get_ios_info_from_tmd();
	u32 get_ios_info(signed_blob *TMD, u32 size);
	char* get_iosx_info_from_tmd(int ios_slot, u32 *version);
	bool shadow_mload();

typedef struct _iosinfo_t {
        u32 magicword; //0x1ee7c105
        u32 magicversion; // 1
        u32 version; // Example: 5
        u32 baseios; // Example: 56
        char name[0x10]; // Example: d2x
        char versionstring[0x10]; // Example: beta2
} __attribute__((packed)) iosinfo_t;

bool get_iosinfo(int ios, signed_blob *TMD, iosinfo_t *iosinfo);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
