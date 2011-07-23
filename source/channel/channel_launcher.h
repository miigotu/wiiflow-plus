#ifndef _CHAN_LAUNCHER
#define _CHAN_LAUNCHER

#define TITLE_UPPER(x)		((u32)((x) >> 32))
#define TITLE_LOWER(x)		((u32)(x) & 0xFFFFFFFF)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gctypes.h>
#include <gccore.h>

s32 BootChannel(u32 *data, u64 chantitle, u8 vidMode, bool cheat, u32 cheatSize, bool vipatch, bool countryString, u8 patchVidMode);

u32 LoadChannel(u32 *buffer);
void PatchChannel(bool cheat, u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes);

u32 * GetDol(u64 title, char *id, u32 *contentSize, u16 bootcontent, bool skip_bootcontent);
bool FindDol(u64 title, char *DolPath, u16 bootcontent);

bool Identify(u64 titleid, u32 *ios);
bool Identify_GenerateTik(signed_blob **outbuf, u32 *outlen);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* _CHAN_LAUNCHER */
