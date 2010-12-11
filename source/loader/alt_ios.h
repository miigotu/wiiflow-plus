#ifndef _ALT_IOS_H_
#define _ALT_IOS_H_

#include <ogcsys.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IOS_249_MIN_REV 17
#define IOS_250_MIN_REV 17
#define IOS_ODD_MIN_REV 20
#define IOS_222_MIN_REV 4
#define IOS_223_MIN_REV 4
#define IOS_224_MIN_REV 5

bool loadIOS(int ios, bool launch_game);
u32 get_ios_base();

extern int mainIOS;
extern int mainIOSRev;
extern int mainIOSminRev;
extern bool use_port1;

#ifdef __cplusplus
}
#endif

#endif
