
#include "objalloc.hpp"
#include "string.h"
#include "lockMutex.hpp"

void *CObjAlloc::allocate(u32 s)
{
	if (m_firstFree == 0 || s > m_objSize)
		return 0;
	LockMutex lock(m_mutex);
	void *p = m_firstFree + 1;
	m_firstFree = m_firstFree->next;
	++m_curNumObj;
	++m_count;
	return p;
}

void CObjAlloc::release(void *p)
{
	CObjAlloc::SHdr *f = (CObjAlloc::SHdr *)p - 1;
	LockMutex lock(m_mutex);
	f->next = m_firstFree;
	m_firstFree = f;
	--m_curNumObj;
}

void CObjAlloc::cleanup(void)
{
	LWP_MutexDestroy(m_mutex);
	m_mutex = 0;
}

CObjAlloc *CObjAlloc::create(void *(*a)(u32), u32 objSize, u32 numObj)
{
	CObjAlloc *p = (CObjAlloc *)a(sizeof (CObjAlloc) - sizeof p->m_data + (objSize + sizeof (*p->m_firstFree)) * numObj);
	if (p == 0)
		return 0;
	LWP_MutexInit(&p->m_mutex, 0);
	p->m_objSize = objSize;
	p->m_maxNumObj = numObj;
	p->m_curNumObj = 0;
	p->m_firstFree = &p->m_data;
	p->m_count = 0;
	CObjAlloc::SHdr *h = p->m_firstFree;
	for (u32 i = 0; i < numObj - 1; ++i)
	{
		h->next = (CObjAlloc::SHdr *)((u8 *)(h + 1) + objSize);
		h = h->next;
	}
	h->next = 0;
	return p;
}
