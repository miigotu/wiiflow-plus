#ifndef _SYS_H_
#define _SYS_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Prototypes */
void Sys_Init(void);
void Sys_Reboot(void);
void Sys_Shutdown(void);
void Sys_LoadMenu(void);
bool Sys_Exiting(void);
void Sys_Test(void);
void Sys_Exit(int);
void Sys_ExitToWiiMenu(bool);
bool Sys_SupportsExternalModule(void);

s32  Sys_GetCerts(signed_blob **, u32 *);

#define IOS_TYPE_UNK    0
#define IOS_TYPE_WANIN  1
#define IOS_TYPE_HERMES 2
#define IOS_TYPE_KWIIRK 3

int get_ios_type();
int is_ios_type(int type);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
