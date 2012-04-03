#include "cache.hpp"

CCache::CCache(deque<dir_discHdr> &list, string path , CMode mode) /* Load/Save All */
{
	filename = path;
	//gprintf("Openning DB: %s\n", filename.c_str());

	cache = fopen(filename.c_str(), io[mode]);
	if(!cache) return;

	switch(mode)
	{
		case LOAD:
			LoadAll(list);
			break;
		case SAVE:
			SaveAll(list);
			break;
		default:
			return;
	}
}

CCache::~CCache()
{
	if(cache) fclose(cache);
	cache = NULL;
}

void CCache::SaveAll(deque<dir_discHdr> list)
{
	if(!cache) return;
	for(deque<dir_discHdr>::iterator iter = list.begin(); iter != list.end(); iter++)
		fwrite(&(*iter), 1, sizeof(dir_discHdr), cache);
}

void CCache::LoadAll(deque<dir_discHdr> &list)
{
	if(!cache) return;

	dir_discHdr tmp;
	fseek(cache, 0, SEEK_END);
	u64 fileSize = ftell(cache);
	fseek(cache, 0, SEEK_SET);

	u32 count = (u32)(fileSize / sizeof(dir_discHdr));

	for(u32 i = 0; i < count; i++)
	{
		LoadOne(tmp, i);
		list.push_back(tmp);
	}
}

void CCache::LoadOne(dir_discHdr &tmp, u32 index)
{
	if(!cache) return;

	fseek(cache, index * sizeof(dir_discHdr), SEEK_SET);
	fread(&tmp, 1, sizeof(dir_discHdr), cache);
}