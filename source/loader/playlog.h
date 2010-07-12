#ifndef _PLAYLOG_H_
#define _PLAYLOG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Prototypes */
u64 getWiiTime(void);
int Playlog_Update(const char ID[6], const char title[42]);
int Playlog_Delete(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

