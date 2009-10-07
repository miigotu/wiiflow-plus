#ifndef _USBSTORAGE_H_
#define _USBSTORAGE_H_ 

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Prototypes */
s32  USBStorage_GetCapacity(u32 *);
s32  USBStorage_Init(void);
void USBStorage_Deinit(void);
s32  USBStorage_ReadSectors(u32, u32, void *);
s32  USBStorage_WriteSectors(u32, u32, void *);
s32  USBStorage_Watchdog(u32 on_off);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
