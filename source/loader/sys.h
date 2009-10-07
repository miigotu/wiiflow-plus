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

s32  Sys_GetCerts(signed_blob **, u32 *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
