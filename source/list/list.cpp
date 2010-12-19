#include "list.hpp"

CList * CList::instance = NULL;

CList::~CList()
{
	//DestroyInstance();
}

CList * CList::Instance()
{
	if (instance == NULL)
		instance = new CList();

	return instance;
}

void CList::DestroyInstance()
{
    if(instance) delete instance;
    instance = NULL;
}

void CList::GetPaths(safe_vector<string> &pathlist, string containing, string directory)
{
	if (strncasecmp(DeviceHandler::Instance()->PathToFSName(directory.c_str()), "WBFS", 4) != 0)
	{
		struct stat filestat;
		struct stat cache;
		
		string buffer = directory;
		size_t find = buffer.find(":/");
		buffer.replace(find, 2, "_");
		while(true)
		{
			find = buffer.find("/");
			if(find == string::npos) break;
			buffer[find] = '_';
		}
		
		m_database = sfmt("%s/%s.db"/* "%s/%s_i.db" */, m_cacheDir.c_str(), buffer.c_str()/*,  disk guid (4) */);

		if(stat(directory.c_str(), &filestat) == -1) return;
		update = (stat(m_database.c_str(), &cache) == -1 || filestat.st_mtime > cache.st_mtime);

		if(update)
		{
			safe_vector<string> compares = stringToVector(containing, '|');
			safe_vector<string> temp_pathlist;

			char entry[1024] = {0};

			/* Open primary directory */
			DIR_ITER *dir_itr = diropen(directory.c_str());
			if (!dir_itr) return;

			/* Read primary entries */
			while(dirnext(dir_itr, entry, &filestat) == 0)
			{
				if(!S_ISDIR(filestat.st_mode))
				{
					for(safe_vector<string>::iterator compare = compares.begin(); compare != compares.end(); compare++)
						if(((string)entry).rfind(*compare) != string::npos)
						{
							//if(entry[strlen(entry) - 1] == '3' || !isdigit(entry[strlen(entry) - 1]))
								pathlist.push_back(sfmt("%s/%s", directory.c_str(), entry));
							break;
						}
				}
				else temp_pathlist.push_back(sfmt("%s/%s", directory.c_str(), entry));
			}
			dirclose(dir_itr);

			for(safe_vector<string>::iterator templist = temp_pathlist.begin(); templist != temp_pathlist.end(); templist++)
			{
				dir_itr = diropen((*templist).c_str());
				if (!dir_itr) continue;

				/* Read secondary entries */
				while(dirnext(dir_itr, entry, &filestat) == 0)
				{
					if(S_ISDIR(filestat.st_mode)) continue;
					if (strlen(entry) < 8) continue;

					for(safe_vector<string>::iterator compare = compares.begin(); compare != compares.end(); compare++)
						if(((string)entry).rfind(*compare) != string::npos)
						{
							//if(entry[strlen(entry) - 1] == '3' || !isdigit(entry[strlen(entry) - 1]))
								pathlist.push_back(sfmt("%s/%s", (*templist).c_str(), entry));
							break;
						}
				}
				dirclose(dir_itr);
			}
		}
	}
	else
	{
		int partition = DeviceHandler::Instance()->PathToDriveType(directory.c_str());
		wbfs_t* handle = DeviceHandler::Instance()->GetWbfsHandle(partition);
		if (!handle) return;

		u32 count = wbfs_count_discs(handle);
		for(u32 i = 0; i < count; i++)
			pathlist.push_back(directory);
	}
}

void CList::GetHeaders(safe_vector<string> pathlist, safe_vector<dir_discHdr> &headerlist)
{
	dir_discHdr tmp;

	if(update)
	{
		for(safe_vector<string>::iterator itr = pathlist.begin(); itr != pathlist.end(); itr++)
		{
			bzero(&tmp, sizeof(dir_discHdr));
			strncpy(tmp.path, (*itr).c_str(), sizeof(tmp.path));

			bool wbfs = (*itr).rfind(".wbfs") != string::npos;
			if ((wbfs || (*itr).rfind(".iso")  != string::npos))
			{
				Check_For_ID(tmp.hdr.id, (*itr).c_str(), "[", "]"); 	 	 /* [GAMEID] Title, [GAMEID]_Title, Title [GAMEID], Title_[GAMEID] */
				if(tmp.hdr.id[0] == 0)
				{
					Check_For_ID(tmp.hdr.id, (*itr).c_str(), "/", "."); 	 /* GAMEID.wbfs, GAMEID.iso */
					if(tmp.hdr.id[0] == 0)
						Check_For_ID(tmp.hdr.id, (*itr).c_str(), "/", "_"); /* GAMEID_Title */
				}
				if(tmp.hdr.id[0] != 0)
				{
					headerlist.push_back(tmp);
					continue;
				}

				FILE *fp = fopen((*itr).c_str(), "rb");
				if (fp)
				{
					fseek(fp, wbfs ? 512 : 0, SEEK_SET);
					fread(&tmp.hdr, sizeof(discHdr), 1, fp);
					SAFE_CLOSE(fp);
				}

				if (tmp.hdr.magic == 0x5D1C9EA3	&& memcmp(tmp.hdr.id, "__CFG_", sizeof tmp.hdr.id) != 0)
				{
					headerlist.push_back(tmp);
					continue;
				}
				
				if(wbfs)
				{
					wbfs_t *part = WBFS_Ext_OpenPart((char *)(*itr).c_str());
					if (!part) continue;

					/* Get header */
					if(wbfs_get_disc_info(part, 0, (u8*)&tmp.hdr,	sizeof(discHdr), NULL) == 0
					&& memcmp(tmp.hdr.id, "__CFG_", sizeof tmp.hdr.id) != 0)
						headerlist.push_back(tmp);

					WBFS_Ext_ClosePart(part);
					continue;
				}
			}

			else if((*itr).rfind(".dol")  != string::npos)
			{
				(*itr)[(*itr).find_last_of('/')] = 0;
				(*itr).assign(&(*itr)[(*itr).find_last_of('/') + 1]);
				
				(*itr)[0] = toupper((*itr)[0]);
				for (u32 i = 1; i < (*itr).size(); ++i)
					if((*itr)[i] == ' ')
					{
						(*itr)[i + 1] = toupper((*itr)[i + 1]);
						i++;
					}
				strcpy(tmp.hdr.title, (*itr).c_str());

				for (u32 i = 0; i < 6; ++i)
					(*itr)[i] = toupper((*itr)[i]);
				memcpy(tmp.hdr.id, (*itr).c_str(), 6);

				headerlist.push_back(tmp);
				continue;
			}
			else if(strncasecmp(DeviceHandler::Instance()->PathToFSName((*itr).c_str()), "WBFS", 4) == 0)
			{
				u8 partition = DeviceHandler::Instance()->PathToDriveType((*itr).c_str());
				wbfs_t* handle = DeviceHandler::Instance()->GetWbfsHandle(partition);
				if (!handle) return;

				static u32 count = 0;
				s32 ret = wbfs_get_disc_info(handle, count, (u8 *)&tmp.hdr, sizeof(struct discHdr), NULL);
				count++;
				if(ret != 0) continue;
				
				if (tmp.hdr.magic == 0x5D1C9EA3	&& memcmp(tmp.hdr.id, "__CFG_", sizeof tmp.hdr.id) != 0)
					headerlist.push_back(tmp);
				
				continue;
			}
		}
	}
	else
	{
		//gprintf("Update gamelist database? NO! Reading DB:\n%s\n", m_database.c_str());
		FILE *cache = fopen(m_database.c_str(), "rb");
		if (!cache) return;

		fseek(cache, 0, SEEK_END);
		u64 fileSize = ftell(cache);
		fseek(cache, 0, SEEK_SET);

		u32 count = fileSize / sizeof(dir_discHdr);
		for(u32 i = 0; count >= 1 && i < count; i++)
		{
			fread((void *)&tmp, 1, sizeof(dir_discHdr), cache);
			headerlist.push_back(tmp);
		}			
		SAFE_CLOSE(cache);
	}

	if (update)
	{
		//gprintf("Update gamelist database? YES! Saving DB:\n%s\n", m_database.c_str());
		FILE *dump = fopen(m_database.c_str(), "wb");
		if (dump) fwrite((void *)&headerlist[0], 1, headerlist.size() * sizeof(dir_discHdr), dump);
		SAFE_CLOSE(dump);
	}
}

void CList::Check_For_ID(u8 *id, string path, string one, string two)
{
	size_t idstart = path.find_last_of(one);
	size_t idend = path.find_last_of(two);
	if (idend != string::npos && idstart != string::npos && idend - idstart == 7)
		for(u8 pos = 0; pos < 6; pos++)
			id[pos] = toupper(path[idstart + 1 + pos]);
}