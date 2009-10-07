#ifndef _VIDEOPATCH_H_
#define _VIDEOPATCH_H_

#include <gccore.h>

// Level :
// 0 : If same number of lines and same mode type (interlaced, progressive)
// 1 : If same mode type
// 2 : Always
void applyVideoPatch(void *dst, u32 len, GXRModeObj *rmode, int level);

#endif // !defined(_VIDEOPATCH_H_)
