
#include "smartptr.hpp"

SmartBuf smartMalloc(unsigned int size)
{
	return SmartBuf((unsigned char *)malloc(size), SmartBuf::SRCALL_MALLOC);
}

SmartBuf smartMemAlign32(unsigned int size)
{
	return SmartBuf((unsigned char *)memalign(32, size), SmartBuf::SRCALL_MALLOC);
}

SmartBuf smartMemAlignPad32(unsigned int size)
{
	return SmartBuf((unsigned char *)memalign(32, (size + 31) & ~31), SmartBuf::SRCALL_MALLOC);
}

SmartBuf smartMem2Alloc(unsigned int size)
{
	return SmartBuf((unsigned char *)MEM2_alloc(size), SmartBuf::SRCALL_MEM2);
}

SmartBuf smartCoverAlloc(unsigned int size)
{
	return SmartBuf((unsigned char *)COVER_alloc(size), SmartBuf::SRCALL_COVER);
}

SmartBuf smartAnyAlloc(unsigned int size)
{
	SmartBuf p(smartMem2Alloc(size));
	if (!!p)
		return p;
	p = smartCoverAlloc(size);
	if (!!p)
		return p;
	return smartMemAlignPad32(size);
}
