#ifndef __FONT_HPP
#define __FONT_HPP

#include "FreeTypeGX.h"
#include "smartptr.hpp"

struct SFont
{
		SmartPtr<FreeTypeGX> font;
		u32 lineSpacing;
		bool fromBuffer(const SmartBuf &buffer, u32 bufferSize, u32 size = 0, u32 lspacing = 0, u32 weight = 0, u32 index = 0);
		bool fromFile(const char *filename, u32 size = 0, u32 lspacing = 0, u32 weight = 0, u32 index = 0);
		SFont(void) : font(SmartPtr<FreeTypeGX>(new FreeTypeGX)), lineSpacing(0) { }
	protected:
		SmartBuf data;
};

#endif /* __FONT_HPP */