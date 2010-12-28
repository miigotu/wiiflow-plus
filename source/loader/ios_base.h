#ifndef _IOS_BASE_H_
#define _IOS_BASE_H_

#define info_number 35

#include <ogcsys.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern u32 hashes[info_number][5];
extern char bases[info_number][3];
extern char revs[info_number][4];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_IOS_BASE_H_