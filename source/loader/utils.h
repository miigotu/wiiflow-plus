#ifndef _UTILS_H_
#define _UTILS_H_

#include <gctypes.h>
/* Constants */
#define KB_SIZE		1024.0
#define MB_SIZE		1048576.0
#define GB_SIZE		1073741824.0

/* Macros */
#define round_up(x,n)	(-(-(x) & -(n)))

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Prototypes */
u32 swap32(u32);

#define SAFE_FREE(P) if(P){free(P);P=NULL;}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
