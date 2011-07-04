
#include "mem2.hpp"
#include "mem2alloc.hpp"
#include "objalloc.hpp"

#include <malloc.h>
#include <string.h>

#define MEM2_PRIORITY_SIZE	0x40
#define NUM_OBJALLOC		20
#define OBJALLOC_SIZE		(0x40000 >> 2)		// 256 KB

// Forbid the use of MEM2 through malloc
u32 MALLOC_MEM2 = 0;

static CMEM2Alloc g_mem2gp;
static CMEM2Alloc g_mem2covers;
static CMEM2Alloc g_mem1locovers;
static CMEM2Alloc g_mem1hicovers;
static CObjAlloc *g_mem2obj[8][NUM_OBJALLOC];	// maybe this should be a stack, using a linked list (as it works inside the CObjAlloc)
static int g_curmem2obj[8];

extern int __cdat_end;
extern int __init_start;

void MEM2_init(unsigned int mem2Size, unsigned int coverSize)
{
	g_mem2gp.init(mem2Size);
	g_mem2covers.init(coverSize);
	g_mem1locovers.init(&__cdat_end + 0x100, &__init_start - 0x100);
	//g_mem1hicovers.init((void *)0x81200000, (void *)0x816FFFF0);
	g_mem1hicovers.init((void *)0x81200000, (void *)0x815FFFF0);
	memset(g_mem2obj, 0, sizeof g_mem2obj);
}

void MEM2_cleanup(void)
{
	for (int n = 0; n < 8; ++n)
		for (int i = 0; i < NUM_OBJALLOC && g_mem2obj[n][i] != 0; ++i)
			g_mem2obj[n][i]->cleanup();
	g_mem1hicovers.cleanup();
	g_mem1locovers.cleanup();
	g_mem2covers.cleanup();
	g_mem2gp.cleanup();
	memset(g_mem2obj, 0, sizeof g_mem2obj);
}

extern "C" void *MEM2_allocSmall(unsigned int s)
{
	unsigned int n = 0;
	for (unsigned int i = (s - 1) >> 2; i > 0 && n < 8; i >>= 1)
		++n;
	if (n < 8)
	{
		for (int i = g_curmem2obj[n]; i >= 0 && g_mem2obj[n][i] != 0; --i)
		{
			void *p = g_mem2obj[n][i]->allocate(s);
			if (p != 0)
				return p;
		}
		if (g_curmem2obj[n] < NUM_OBJALLOC - 1)
		{
			g_mem2obj[n][g_curmem2obj[n] + 1] = CObjAlloc::create(MEM2_alloc, 1 << (n + 2), OBJALLOC_SIZE >> n);
			if (g_mem2obj[n][g_curmem2obj[n] + 1] != 0)
			{
				++g_curmem2obj[n];
				return g_mem2obj[n][g_curmem2obj[n]]->allocate(s);
			}
		}
	}
	return g_mem2gp.allocate(s);
}

extern "C" void *MEM2_alloc(unsigned int s)
{
	return g_mem2gp.allocate(s);
}

extern "C" void *COVER_allocMem1(unsigned int s)
{
	void *p = g_mem1locovers.allocate(s);
	if (p != 0)
		return p;
	return g_mem1hicovers.allocate(s);
}

extern "C" void *COVER_allocMem2(unsigned int s)
{
	return g_mem2covers.allocate(s);
}

extern "C" void *COVER_alloc(unsigned int s)
{
	void *p = g_mem2covers.allocate(s);
	if (p != 0)
		return p;
	p = g_mem1locovers.allocate(s);
	if (p != 0)
		return p;
	return g_mem1hicovers.allocate(s);
}

extern "C" void *FAT_alloc(unsigned int s)
{
	return MEM2_alloc(s);
}

extern "C" void MEM2_free(void *p)
{
	for (int n = 0; n < 8; ++n)
		for (int i = g_curmem2obj[n]; i >= 0 && g_mem2obj[n][i] != 0; --i)
			if (g_mem2obj[n][i]->owns(p))
			{
				g_mem2obj[n][i]->release(p);
				return;
			}
	g_mem2gp.release(p);
}

extern "C" void COVER_free(void *p)
{
	if (((u32)p & 0x10000000) != 0)
		g_mem2covers.release(p);
	else if (p < &__init_start)
		g_mem1locovers.release(p);
	else
		g_mem1hicovers.release(p);
}

extern "C" void FAT_free(void *p)
{
	MEM2_free(p);
}

extern "C" void *MEM2_realloc(void *p, unsigned int s)
{
	for (int n = 0; n < 8; ++n)
		for (int i = g_curmem2obj[n]; i >= 0 && g_mem2obj[n][i] != 0; --i)
			if (g_mem2obj[n][i]->owns(p))
			{
				u32 curSize = g_mem2obj[n][i]->usableSize(p);
				if (s <= curSize)
					return p;
				void *n = MEM2_allocSmall(s);
				if (n == 0)
					return 0;
				if (p != 0)
				{
					memcpy(n, p, curSize < s ? curSize : s);
					MEM2_free(p);
				}
				return n;
			}
	return g_mem2gp.reallocate(p, s);
}

extern "C" unsigned int MEM2_usableSize(void *p)
{
	for (int n = 0; n < 8; ++n)
		for (int i = g_curmem2obj[n]; i >= 0 && g_mem2obj[n][i] != 0; --i)
			if (g_mem2obj[n][i]->owns(p))
				return g_mem2obj[n][i]->usableSize(p);
	return CMEM2Alloc::usableSize(p);
}

void COVER_clear(void)
{
	g_mem2covers.clear();
	g_mem1locovers.clear();
	g_mem1hicovers.clear();
}

// Memory extension for malloc

// Give priority to MEM2 for big allocations
// Used for saving some space in malloc, which is required for 2 reasons :
// - decent speed on small and frequent allocations
// - newlib uses its malloc internally (for *printf for example) so it should always have some memory left
bool g_bigGoesToMem2 = false;

void MEM2_takeBigOnes(bool b)
{
	g_bigGoesToMem2 = b;
}

extern "C"
{

extern __typeof(malloc) __real_malloc;
extern __typeof(calloc) __real_calloc;
extern __typeof(realloc) __real_realloc;
extern __typeof(memalign) __real_memalign;
extern __typeof(free) __real_free;
extern __typeof(malloc_usable_size) __real_malloc_usable_size;

void *__wrap_malloc(size_t size)
{
	void *p;
	if (g_bigGoesToMem2 && size > MEM2_PRIORITY_SIZE)
	{
		p = MEM2_alloc(size);
		if (p != 0)
			return p;
		return __real_malloc(size);
	}
	p = __real_malloc(size);
	if (p != 0)
		return p;
	return MEM2_allocSmall(size);	
}

void *__wrap_calloc(size_t n, size_t size)
{
	void *p;
	if (g_bigGoesToMem2 && size > MEM2_PRIORITY_SIZE)
	{
		p = MEM2_alloc(n * size);
		if (p != 0)
		{
			memset(p, 0, n * size);
			return p;
		}
		return __real_calloc(n, size);
	}
	p = __real_calloc(n, size);
	if (p != 0)
		return p;
	p = MEM2_allocSmall(n * size);
	if (p != 0)
		memset(p, 0, n * size);
	return p;
}

void *__wrap_memalign(size_t a, size_t size)
{
	void *p;
	if (g_bigGoesToMem2 && size > MEM2_PRIORITY_SIZE)
	{
		if (a <= 32 && 32 % a == 0)
		{
			p = MEM2_alloc(size);
			if (p != 0)
				return p;
		}
		return __real_memalign(a, size);
	}
	p = __real_memalign(a, size);
	if (p != 0 || a > 32 || 32 % a != 0)
		return p;
	return MEM2_alloc(size);
}

void __wrap_free(void *p)
{
	if (((u32)p & 0x10000000) != 0)
		MEM2_free(p);
	else
		__real_free(p);
}

void *__wrap_realloc(void *p, size_t size)
{
	void *n;
	// ptr from mem2
	if (((u32)p & 0x10000000) != 0 || (p == 0 && g_bigGoesToMem2 && size > MEM2_PRIORITY_SIZE))
	{
		n = MEM2_realloc(p, size);
		if (n != 0)
			return n;
		n = __real_malloc(size);
		if (n == 0)
			return 0;
		if (p != 0)
		{
			memcpy(n, p, MEM2_usableSize(p) < size ? MEM2_usableSize(p) : size);
			MEM2_free(p);
		}
		return n;
	}
	// ptr from malloc
	n = __real_realloc(p, size);
	if (n != 0)
		return n;
	n = MEM2_allocSmall(size);
	if (n == 0)
		return 0;
	if (p != 0)
	{
		memcpy(n, p, __real_malloc_usable_size(p) < size ? __real_malloc_usable_size(p) : size);
		__real_free(p);
	}
	return n;
}

size_t __wrap_malloc_usable_size(void *p)
{
	if (((u32)p & 0x10000000) != 0)
		return MEM2_usableSize(p);
	return __real_malloc_usable_size(p);
}

}
