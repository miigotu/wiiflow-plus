#include "font.hpp"

using namespace std;

bool SFont::fromBuffer(const SmartBuf &buffer, u32 bufferSize, u32 size, u32 lspacing, u32 weight, u32 index)
{
	if (!buffer || !font) return false;

	SMART_FREE(data);
	data = smartMem2Alloc(bufferSize);
	if(!data) return false;

	memcpy(data.get(), buffer.get(), bufferSize);

	lineSpacing = min(max(6u, lspacing), 1000u);
	font->loadFont(data.get(), bufferSize, min(max(6u, size), 1000u), min(weight, 32u), index, false);

	return true;
}

bool SFont::fromFile(const char *filename, u32 size, u32 lspacing, u32 weight, u32 index)
{
	if (!font || !filename) return false;

	FILE *file = fopen(filename, "rb");
	if (file == NULL) return false;

	fseek(file, 0, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);
	if (fileSize == 0) return false;

	SMART_FREE(data);
	data = smartMem2Alloc(fileSize);
	if (!data || fread(data.get(), 1, fileSize, file) != fileSize)
	{
		SAFE_CLOSE(file);
		return false;
	}

	lineSpacing = min(max(6u, lspacing), 1000u);
	font->loadFont(data.get(), fileSize, min(max(6u, size), 1000u), min(weight, 32u), index, false);
	return true;
}