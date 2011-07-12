#include "smartptr.hpp"

SmartBuf smartMalloc(unsigned int size)
{
	return SmartBuf((unsigned char *)malloc(size), SmartBuf::SRCALL_MALLOC);
}

SmartBuf smartMemAlign32(unsigned int size)
{
	return SmartBuf((unsigned char *)memalign(32, size), SmartBuf::SRCALL_MALLOC);
}

SmartBuf smartMem2Alloc(unsigned int size)
{
	return SmartBuf((unsigned char *)MEM2_alloc(size), SmartBuf::SRCALL_MEM2);
}

SmartBuf smartAnyAlloc(unsigned int size)
{
	SmartBuf p(smartMem2Alloc(size));
	return !!p ? p : smartMalloc(size);
}
