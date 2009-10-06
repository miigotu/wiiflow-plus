
#ifndef __TEXT_HPP
#define __TEXT_HPP

#include <vector>
#include <string>

#include "wstring.hpp"
#include "FreeTypeGX.h"
#include "video.hpp"

#include "smartptr.hpp"

struct SFont
{
	SmartBuf data;
	size_t dataSize;
	SmartPtr<FreeTypeGX> font;
	u32 lineSpacing;
public:
	bool fromBuffer(const u8 *buffer, u32 bufferSize, u32 size, u32 lspacing);
	bool fromFile(const char *filename, u32 size, u32 lspacing);
	bool newSize(u32 size, u32 lspacing);
	SFont(void) : dataSize(0), lineSpacing(0) { }
};

class CText
{
public:
	void setText(SFont font, const wstring &t);
	void setColor(const CColor &c);
	void setFrame(float width, u16 style, bool ignoreNewlines = false, bool instant = false);
	void tick(void);
	void draw(void);
private:
	struct SWord
	{
		wstring text;
		Vector3D pos;
		Vector3D targetPos;
	};
private:
	typedef std::vector<SWord> CLine;
	std::vector<CLine> m_lines;
	SFont m_font;
	CColor m_color;
};

// Nothing to do with CText. Q&D helpers for string formating.
const char *fmt(const char *format, ...);
std::string sfmt(const char *format, ...);
wstring wfmt(const wstring &format, ...);
bool checkFmt(const wstring &ref, const wstring &format);
wstring vectorToString(const std::vector<wstring> &vect, char sep);
std::vector<wstring> stringToVector(const wstring &text, char sep);
std::vector<std::string> stringToVector(const std::string &text, char sep);

#endif // !defined(__TEXT_HPP)
