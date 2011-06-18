#ifndef CACHEDLIST
#define CACHEDLIST

#include "list.hpp"
#include "cache.hpp"
#include "safe_vector.hpp"
#include "gecko.h"
#include "config.hpp"

using namespace std;

template <typename T = dir_discHdr>
class CachedList : public safe_vector<T>
{
  public:
    void Init(string cachedir){m_cacheDir = cachedir; m_loaded = false; m_update = false; m_database = "";}

    void Load(string path, string containing);
    void Unload(){if(m_loaded) {this->clear(); m_loaded = false; m_database = "";}};
    void Save() {if(m_loaded && m_update) CCache<T>(*this, m_database, &gcnt, rcnt, SAVE);}				/* Save All */

    //void get(T &tmp, u32 index) {if(m_loaded) CCache(tmp, m_database, index, LOAD);}		/* Load One */
    void Set(T tmp, u32 index) {if(m_loaded) CCache<T>(tmp, m_database, index, SAVE);}		/* Save One */

    void Add(T tmp) {if(m_loaded) CCache<T>(*this, m_database, tmp, ADD);}					/* Add One */
    void Remove(u32 index) {if(m_loaded) CCache<T>(*this, m_database, index, REMOVE);}		/* Remove One */

  private:
    string make_db_name(string path);

    bool m_loaded;
    bool m_update;
    bool m_wbfsFS;
    CList<T> list;
    string m_database;
    string m_cacheDir;
	Config m_cfg;
	
	u32 rcnt;
	u32 gcnt;
};

#endif