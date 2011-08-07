#include "cachedlist.hpp"
#include "channels.h"
#include <typeinfo>

template <class T>
void CachedList<T>::Load(string path, string containing)													/* Load All */
{
	//gprintf("\nLoading files containing %s in %s\n", containing.c_str(), path.c_str());
	m_loaded = false;
	m_database = sfmt("%s/%s.db", m_cacheDir.c_str(), (make_db_name(path)).c_str());
	
	//gprintf("Database file: %s\n", m_database.c_str());
	m_wbfsFS = strncasecmp(DeviceHandler::Instance()->PathToFSName(path.c_str()), "WBFS", 4) == 0;
	
	if(!m_wbfsFS)
	{
		if(force_update)
			remove(m_database.c_str());

		struct stat filestat, cache;
		gprintf("%s\n", path.c_str());
		if(stat(path.c_str(), &filestat) == -1) return;
		m_update = force_update || m_lastLanguage != m_curLanguage || stat(m_database.c_str(), &cache) == -1 || filestat.st_mtime > cache.st_mtime;
	}

	force_update = false;

	bool music = typeid(T) == typeid(std::string);

	if(m_update || m_wbfsFS || music)
	{
		//gprintf("Calling list to update filelist\n");
		safe_vector<string> pathlist;
		list.GetPaths(pathlist, containing, path, m_wbfsFS);
		list.GetHeaders(pathlist, *this, m_settingsDir, m_curLanguage);
		
		// Load titles and custom_titles files
		m_loaded = true;
		m_update = false;
		m_lastLanguage = m_curLanguage;

		if(!music && pathlist.size() > 0)
		{
			Save();
			pathlist.clear();
		}
	}
	else
		CCache<T>(*this, m_database, LOAD);

	m_loaded = true;
}

template<>
void CachedList<dir_discHdr>::LoadChannels(string path, u32 channelType)													/* Load All */
{
	m_loaded = false;
	m_update = true;

	bool emu = !path.empty();
	if(emu)
	{
		m_database = sfmt("%s/%s.db", m_cacheDir.c_str(), make_db_name(sfmt("%s/CHANNELS", path.c_str())).c_str());
		if(force_update)
			remove(m_database.c_str());

		//gprintf("%s\n", m_database.c_str());
		struct stat filestat, cache;

		string newpath = sfmt("%s%s", path.c_str(), "title");

		//gprintf("%s\n", newpath.c_str());
		if(stat(newpath.c_str(), &filestat) == -1) return;
		//gprintf("stat succeed\n");
		m_update = force_update || m_lastchannelLang != m_channelLang || stat(m_database.c_str(), &cache) == -1 || filestat.st_mtime > cache.st_mtime;
	}

	force_update = false;

	if(m_update)
	{
		Channels m_channels;
		m_channels.Init(channelType, m_channelLang, true);

		u32 count = m_channels.Count();
		u32 len = sizeof (dir_discHdr);

		SmartBuf buffer = smartMem2Alloc(len);
		if (!buffer) return;

		this->reserve(count);
		dir_discHdr *b = (dir_discHdr *)buffer.get();

		for (u32 i = 0; i < count; ++i)
		{
			Channel *chan = m_channels.GetChannel(i);
			
			if (chan->id == NULL) continue; // Skip invalid channels

			memset(b, 0, len);
			memcpy(b->hdr.id, chan->id, 4);
			wcstombs(b->hdr.title, chan->name, sizeof(b->hdr.title));
			b->hdr.chantitle = chan->title;
		
			this->push_back(*b);
		}
		
		// Load titles and custom_titles files
		m_loaded = true;
		m_update = false;
		m_lastchannelLang = m_channelLang;

		if(this->size() > 0 && emu) Save();
	}
	else
		CCache<dir_discHdr>(*this, m_database, LOAD);

	m_loaded = true;
}

template <class T>
string CachedList<T>::make_db_name(string path)
{
	string buffer = path;
	buffer.replace(buffer.find(":/"), 2, "_");
	size_t find = buffer.find("/");
	while(find != string::npos)
	{
		buffer[find] = '_';
		find = buffer.find("/");
	}
	return buffer;
}

template class CachedList<string>;
template class CachedList<dir_discHdr>;
