#ifndef CACHEDLIST
#define CACHEDLIST

#include "list.hpp"
#include "cache.hpp"
#include "safe_vector.hpp"
#include "gecko.h"

using namespace std;

template <typename T = dir_discHdr>
class CachedList : public safe_vector<T>
{
  public:
	void Init(string cachedir, string settingsDir, string curLanguage)
	{
		return Init(cachedir, settingsDir, curLanguage, string());
	}
	void Init(string cachedir, string settingsDir, string curLanguage, string channelLang)						/* Initialize Private Variables */
	{
		m_cacheDir = cachedir;
		m_settingsDir = settingsDir;
		m_curLanguage = m_lastLanguage = curLanguage;
		m_channelLang = m_lastchannelLang = channelLang;
		m_loaded = false;
		m_database = "";
		force_update = false;
		
	}

	void Update() { force_update = true; }													/* Force db update on next load */
    void Load(string path, string containing);
	void LoadChannels(string path, u32 channelType);

    void Unload(){if(m_loaded) {this->clear(); m_loaded = false; m_database = "";}};
    void Save() {if(m_loaded) CCache<T>(*this, m_database, SAVE);}							/* Save All */

    void Get(T tmp, u32 index) {if(m_loaded) CCache<T>(tmp, m_database, index, LOAD);}		/* Load One */
    void Set(T tmp, u32 index) {if(m_loaded) CCache<T>(tmp, m_database, index, SAVE);}		/* Save One */

    void Add(T tmp) {if(m_loaded) CCache<T>(*this, m_database, tmp, ADD);}					/* Add One */
    void Remove(u32 index) {if(m_loaded) CCache<T>(*this, m_database, index, REMOVE);}		/* Remove One */
	
	void SetLanguage(string curLanguage) { m_curLanguage = curLanguage; }
  private:
    string make_db_name(string path);

    bool m_loaded;
    bool m_update;
    bool m_wbfsFS;
	bool force_update;
    CList<T> list;
    string m_database;
    string m_cacheDir;
	string m_settingsDir;
	string m_curLanguage;
	string m_lastLanguage;
	string m_channelLang;
	string m_lastchannelLang;
};

#endif