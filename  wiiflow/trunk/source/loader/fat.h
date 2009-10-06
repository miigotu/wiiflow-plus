#ifndef _FAT_H_
#define _FAT_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Prototypes */
bool Fat_Mount(void);
bool Fat_MountSDOnly(void);
bool Fat_Unmount(void);
bool Fat_SDAvailable(void);
bool Fat_USBAvailable(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
