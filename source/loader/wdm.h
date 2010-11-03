#ifndef __WDM_H__
#define __WDM_H__

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
	u32 count;
	char name[64];
	char dolname[32];
	u32 parameter;
} wdm_entry_t;

extern wdm_entry_t *wdm_entries;
extern u32 wdm_count;

void free_wdm();
int load_wdm(const char *wdmpath, const char *discid);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__WDM_H__
