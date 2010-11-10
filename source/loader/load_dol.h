
#ifndef _LOAD_DOL_H_
#define _LOAD_DOL_H_

#ifdef __cplusplus
extern "C" {
#endif

u32 load_dol(const void *dol_data, int dol_len, void (*f)(void *, int, void *), void *userData);

#ifdef __cplusplus
}
#endif

#endif
