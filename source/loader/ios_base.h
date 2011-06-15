#ifndef _IOS_BASE_H_
#define _IOS_BASE_H_

#define info_number 113

#include <ogcsys.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern u32 hashes[info_number][5];
extern char bases[info_number][6];
extern char revs[info_number][16];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_IOS_BASE_H_