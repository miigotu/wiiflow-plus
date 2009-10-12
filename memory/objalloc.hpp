// In case frequent small allocations reach MEM2

#ifndef __OBJALLOC_HPP
#define __OBJALLOC_HPP

#include <ogc/mutex.h>

class CObjAlloc
{
public:
	void *allocate(u32 s);
	void release(void *p);
	bool owns(void *p) const { return  (u32)((u8 *)p - (u8 *)&m_data) < m_maxNumObj * (m_objSize + sizeof *m_firstFree); }
	u32 usableSize(void *) const { return m_objSize; }
	void cleanup(void);
	u32 getNumObj(void) const { return m_curNumObj; }
	u32 getCapacity(void) const { return m_maxNumObj; }
	u32 getCount(void) const { return m_count; }
	static CObjAlloc *create(void *(*a)(u32), u32 objSize, u32 numObj);
private:
	struct SHdr
	{
		SHdr *next;
	} __attribute__((packed));
	mutex_t m_mutex;
	u32 m_objSize;
	SHdr *m_firstFree;
	u32 m_maxNumObj;
	u32 m_curNumObj;
	u32 m_count;
	SHdr m_data;
private:
	CObjAlloc(void);
	CObjAlloc(const CObjAlloc &);
};

#endif // !defined(__OBJALLOC_HPP)
