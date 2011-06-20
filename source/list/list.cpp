#include "list.hpp"
#include "gecko.h"

template <typename T>
void CList<T>::CountGames(string directory, bool wbfs_fs, u32 *cnt)
{
	if (!wbfs_fs) 
	{
		u32 count = 0;
		DIR_ITER *dir_itr = diropen(directory.c_str());
		if (!dir_itr) return;
	
		char entry[1024] = {0};
		struct stat filestat;
	
		while(dirnext(dir_itr, entry, &filestat) == 0)
		{
			if (entry[0] == '.') continue;
			if (strlen(entry) < 6) continue;
			count++;
		}
		
		*cnt = count - 1;
	}
}

template <typename T>
void CList<T>::GetPaths(safe_vector<string> &pathlist, string containing, string directory, bool wbfs_fs)
{
	if (!wbfs_fs)
	{
		/* Open primary directory */
		DIR_ITER *dir_itr = diropen(directory.c_str());
		if (!dir_itr) return;

		safe_vector<string> compares = stringToVector(containing, '|');
		safe_vector<string> temp_pathlist;

		char entry[1024] = {0};
		struct stat filestat;

		/* Read primary entries */
		while(dirnext(dir_itr, entry, &filestat) == 0)
		{
			if (entry[0] == '.') continue;
			if (strlen(entry) < 6) continue;			

			if(!S_ISDIR(filestat.st_mode))
			{
				for(safe_vector<string>::iterator compare = compares.begin(); compare != compares.end(); compare++)
					if (strcasestr(entry, (*compare).c_str()) != NULL)
					{
						pathlist.push_back(sfmt("%s/%s", directory.c_str(), entry));
						break;
					}
			}
			else
			{
				temp_pathlist.push_back(sfmt("%s/%s", directory.c_str(), entry));
			}
		}
		dirclose(dir_itr);

		if(temp_pathlist.size() > 0)
		{
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
						if (strcasestr(entry, (*compare).c_str()) != NULL)
						{
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
		if(strcasestr(containing.c_str(), ".dol") != 0) return;

		int partition = DeviceHandler::Instance()->PathToDriveType(directory.c_str());
		wbfs_t* handle = DeviceHandler::Instance()->GetWbfsHandle(partition);
		if (!handle) return;

		u32 count = wbfs_count_discs(handle);
		for(u32 i = 0; i < count; i++)
			pathlist.push_back(directory);
	}
}

template <>
void CList<string>::GetHeaders(safe_vector<string> pathlist, safe_vector<string> &headerlist, findtitle_callback_t, void *)
{
	gprintf("Getting headers for CList<string>\n");
	headerlist = pathlist;
}

template <>
void CList<dir_discHdr>::GetHeaders(safe_vector<string> pathlist, safe_vector<dir_discHdr> &headerlist, findtitle_callback_t callback, void *callback_data)
{
	dir_discHdr tmp;
	u32 count = 0;

	gprintf("Getting headers for paths in pathlist (%d)\n", pathlist.size());

	if(pathlist.size() < 1) return;
	headerlist.reserve(pathlist.size() + headerlist.size());

	for(safe_vector<string>::iterator itr = pathlist.begin(); itr != pathlist.end(); itr++)
	{
		bzero(&tmp, sizeof(dir_discHdr));
		strncpy(tmp.path, (*itr).c_str(), sizeof(tmp.path));
		tmp.hdr.index = headerlist.size();

		bool wbfs = (*itr).rfind(".wbfs") != string::npos;
		if ((wbfs || (*itr).rfind(".iso")  != string::npos))
		{
			Check_For_ID(tmp.hdr.id, (*itr).c_str(), "[", "]"); 	 			/* [GAMEID] Title, [GAMEID]_Title, Title [GAMEID], Title_[GAMEID] */
			if(tmp.hdr.id[0] == 0)
			{
				Check_For_ID(tmp.hdr.id, (*itr).c_str(), "/", "."); 			/* GAMEID.wbfs, GAMEID.iso */
				if(tmp.hdr.id[0] == 0)
				{
					Check_For_ID(tmp.hdr.id, (*itr).c_str(), "/", "_"); 		/* GAMEID_Title */
					if(tmp.hdr.id[0] == 0)
					{
						Check_For_ID(tmp.hdr.id, (*itr).c_str(), "_", "."); 	/* Title_GAMEID */ // <-- Unsafe?
						if(tmp.hdr.id[0] == 0)
							Check_For_ID(tmp.hdr.id, (*itr).c_str(), " ", "."); /* Title GAMEID */ //<-- Unsafe?
					}
				}
			}

			if (tmp.hdr.id[0] == 0)
			{
				gprintf("Skipping file: '%s'\n", (*itr).c_str());
				continue;
			}
			
//			gprintf("Get information for '%s'\n", tmp.hdr.id);
			if (memcmp(tmp.hdr.id, "__CFG_", sizeof tmp.hdr.id) == 0)
			{
				// Skip uLoader configuration
				continue;
			}

			// Use a callback to menu.cpp (or something like that) to find the title in titles.ini
			// If found, use that!
			/*if (callback != NULL && callback(callback_data, tmp.hdr.id, tmp.hdr.title, 64))
			{
				gprintf("Callback executed, game found\n");
				tmp.hdr.magic = 0x5D1C9EA3;
				headerlist.push_back(tmp);
				continue;
			}*/

			FILE *fp = fopen((*itr).c_str(), "rb");
			if (fp)
			{
				fseek(fp, wbfs ? 512 : 0, SEEK_SET);
				fread(&tmp.hdr, sizeof(discHdr), 1, fp);
				SAFE_CLOSE(fp);
			}

			if (tmp.hdr.magic == 0x5D1C9EA3)
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
				else (*itr)[i] = tolower((*itr)[i]);

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

			s32 ret = wbfs_get_disc_info(handle, count, (u8 *)&tmp.hdr, sizeof(struct discHdr), NULL);
			count++;
			if(ret != 0) continue;
			
			if (tmp.hdr.magic == 0x5D1C9EA3	&& memcmp(tmp.hdr.id, "__CFG_", sizeof tmp.hdr.id) != 0)
				headerlist.push_back(tmp);
			
			continue;
		}
	}
	pathlist.clear();
}

template <typename T>
void CList<T>::Check_For_ID(u8 *id, string path, string one, string two)
{
	size_t idstart = path.find_last_of(one);
	size_t idend = path.find_last_of(two);
	if (idend != string::npos && idstart != string::npos && idend - idstart == 7)
		for(u8 pos = 0; pos < 6; pos++)
			id[pos] = toupper(path[idstart + 1 + pos]);
}

template class CList<dir_discHdr>;
template class CList<string>;