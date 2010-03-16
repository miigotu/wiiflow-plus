#ifndef __WIP_H__
#define __WIP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void wipreset();
void wipregisteroffset(u32 dst, u32 len);
void wipparsebuffer(u8 *buffer, u32 length);
void load_wip_patches(u8 *wippath, u8 *discid);
void do_wip_patches();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__WIP_H__
