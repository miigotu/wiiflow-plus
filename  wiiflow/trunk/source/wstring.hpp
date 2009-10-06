
#ifndef __WSTRING_HPP
#define __WSTRING_HPP

#include <string>

class wstring : public std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >
{
public:
	wstring(void) { }
	wstring(const wchar_t *s);
	wstring(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > &ws);
	wstring(const std::string &s);
	wstring &operator=(const std::string &s);
	void fromUTF8(const char *s);
	std::string toUTF8(void) const;
};


#endif // !defined(__WSTRING_HPP)
