
#ifndef __CONFIG_HPP
#define __CONFIG_HPP

#include <map>
#include <string>

#include "video.hpp"
#include "smartptr.hpp"
#include "wstring.hpp"

class CColor;
class Vector3D;

class Config
{
public:
	Config(void);
	void clear(void) { m_domains.clear(); }
	bool load(const char *filename = 0);
	void save(void);
	bool loaded(void) const { return m_loaded; }
	bool has(const std::string &domain, const std::string &key) const;
	// Set
	void setWString(const std::string &domain, const std::string &key, const wstring &val);
	void setString(const std::string &domain, const std::string &key, const std::string &val);
	void setBool(const std::string &domain, const std::string &key, bool val);
	void setOptBool(const std::string &domain, const std::string &key, int val);
	void setInt(const std::string &domain, const std::string &key, int val);
	void setFloat(const std::string &domain, const std::string &key, float val);
	void setVector3D(const std::string &domain, const std::string &key, const Vector3D &val);
	void setColor(const std::string &domain, const std::string &key, const CColor &val);
	// Get
	wstring getWString(const std::string &domain, const std::string &key, const wstring &defVal = wstring());
	std::string getString(const std::string &domain, const std::string &key, const std::string &defVal = std::string());
	bool getBool(const std::string &domain, const std::string &key, bool defVal = false);
	int getOptBool(const std::string &domain, const std::string &key, int defVal = 2);
	bool testOptBool(const std::string &domain, const std::string &key, bool defVal);
	int getInt(const std::string &domain, const std::string &key, int defVal = 0);
	float getFloat(const std::string &domain, const std::string &key, float defVal = 0.f);
	Vector3D getVector3D(const std::string &domain, const std::string &key, const Vector3D &defVal = Vector3D());
	CColor getColor(const std::string &domain, const std::string &key, const CColor &defVal = CColor());
	// 
	const std::string &firstDomain(void);
	const std::string &nextDomain(void);
	const std::string &nextDomain(const std::string &start) const;
	const std::string &prevDomain(const std::string &start) const;
	bool hasDomain(const std::string &domain) const;
	void copyDomain(const std::string &dst, const std::string &src);
private:
	typedef std::map<std::string, std::string> KeyMap;
	typedef std::map<std::string, KeyMap> DomainMap;
private:
	bool m_loaded;
	DomainMap m_domains;
	std::string m_filename;
	DomainMap::iterator m_iter;
	static const std::string emptyString;
private:
	Config(const Config &);
	Config &operator=(const Config &);
};

#endif // !defined(__CONFIG_HPP)
