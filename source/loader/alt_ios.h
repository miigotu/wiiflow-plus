#ifndef _ALT_IOS_H_
#define _ALT_IOS_H_

#include <ogcsys.h>

#ifdef __cplusplus
extern "C" {
#endif

#define D2X_MIN_REV 20

bool loadIOS(int ios, bool launch_game);

extern int mainIOS;
extern int mainIOSRev;
extern int mainIOSminRev;
extern bool use_port1;

#ifdef __cplusplus
}
#endif

#endif
