#include <stdio.h>
#include <ogcsys.h>

#include "gecko.h"

#define MAX_BLOCKSIZE           0x10000

u64 le64(u64 x)
{
	return 
	((x & 0xFF00000000000000ULL) >> 56) |
	((x & 0x00FF000000000000ULL) >> 40) |
	((x & 0x0000FF0000000000ULL) >> 24) |
	((x & 0x000000FF00000000ULL) >> 8 ) |
	((x & 0x00000000FF000000ULL) << 8 ) |
	((x & 0x0000000000FF0000ULL) << 24) |
	((x & 0x000000000000FF00ULL) << 40) |
	((x & 0x00000000000000FFULL) << 56);
}

u32 le32(u32 x)
{
	return
	((x & 0x000000FF) << 24) |
	((x & 0x0000FF00) << 8) |
	((x & 0x00FF0000) >> 8) |
	((x & 0xFF000000) >> 24);
}

u16 le16(u16 x)
{
	return
	((x & 0x00FF) << 8) |
	((x & 0xFF00) >> 8);
}