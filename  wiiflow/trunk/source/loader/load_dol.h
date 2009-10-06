
#ifndef _LOAD_DOL_H_
#define _LOAD_DOL_H_

u32 load_dol(const void *dol_data, int dol_len, void (*f)(void *, int, void *), void *userData);

#endif
