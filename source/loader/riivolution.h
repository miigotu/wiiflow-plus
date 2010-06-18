#ifndef _RIIVOLUTION_H
#define _RIIVOLUTION_H

#include "wbfs.h"

#ifdef __cplusplus
extern "C" {
#endif

u32 load_riivolution_files(u8 *riivolutionPath, u8 *id, u8 *name);
u32 do_riivolution_files(FST_ENTRY *fst);

#ifdef __cplusplus
}
#endif

#endif