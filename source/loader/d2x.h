#ifndef _D2X_H_
#define _D2X_H_

#include <gctypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern bool disable_return_to_patch;

s32 is_ios_d2x();
void block_ios_reload();
u8 return_to_channel(u32 id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

