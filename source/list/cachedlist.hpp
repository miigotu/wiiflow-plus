#ifndef CACHEDLIST
#define CACHEDLIST

#include "list.hpp"
#include "cache.hpp"
#include <deque>
#include "gecko.h"

using namespace std;

enum {
	COVERFLOW_USB,
	COVERFLOW_CHANNEL,
	COVERFLOW_HOMEBREW,
	COVERFLOW_MAX
};

class CachedList : public deque<dir_discHdr>
{
  public:
	void Init(string cachedir, string settingsDir, string curLanguage)						/* Initialize Private Variables */
	{
		m_cacheDir = cachedir;
		m_settingsDir = settingsDir;
		m_curLanguage = m_lastLanguage = curLanguage;
		m_channelLang = m_lastchannelLang = curLanguage;
		m_loaded = false;
		m_database = "";
		m_update = false;
		for(u32 i = 0; i < COVERFLOW_MAX; i++)
			force_update[i] = false;
	}

	void Update(u32 view = COVERFLOW_MAX)					/* Force db update on next load */
	{
		if(view == COVERFLOW_MAX)
			for(u32 i = 0; i < COVERFLOW_MAX; i++)
				force_update[i] = true;
		else
			force_update[view] = true;
	}

    void Load(string path, string containing);
	void LoadChannels(string path, u32 channelType);

    void Unload(){if(m_loaded) {this->clear(); m_loaded = false; m_database = "";}};
    void Save() {if(m_loaded) CCache(*this, m_database, SAVE);}							/* Save All */

	void SetLanguage(string curLanguage) { m_curLanguage = m_channelLang = curLanguage; }
  private:

    bool m_loaded;
    bool m_update;
    bool m_wbfsFS;
	u8 force_update[COVERFLOW_MAX];
    CList list;
    string m_database;
    string m_cacheDir;
	string m_settingsDir;
	string m_curLanguage;
	string m_lastLanguage;
	string m_channelLang;
	string m_lastchannelLang;
};

#endif