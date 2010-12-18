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
		safe_vector<string> compares = stringToVector(containing, '|');
		safe_vector<string> temp_pathlist;

		struct stat filestat;
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
						if(entry[strlen(entry) - 1] == '3' || !isdigit(entry[strlen(entry) - 1]))
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
						if(entry[strlen(entry) - 1] == '3' || !isdigit(entry[strlen(entry) - 1]))
							pathlist.push_back(sfmt("%s/%s", (*templist).c_str(), entry));
						break;
					}
			}
			dirclose(dir_itr);
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
	u32 count = 0;
	u8 id[6] = {0};

	for(safe_vector<string>::iterator itr = pathlist.begin(); itr != pathlist.end(); itr++)
	{
		bzero(&tmp, sizeof(dir_discHdr));
		bzero(id, 6);

		strncpy(tmp.path, (*itr).c_str(), sizeof(tmp.path));

		bool wbfs = (*itr).find(".wbfs") != string::npos;
		bool iso  = (*itr).find(".iso")  != string::npos;
		bool dol  = (*itr).find(".dol")  != string::npos;

		if ((wbfs || iso))
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
				char *idstart = strcasestr(strrchr(tmp.path,'/'), (char *)tmp.hdr.id);
				for (u32 i = 0; i < 6; ++i)
					idstart[i] = toupper(idstart[i]);

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
				char *idstart = strcasestr(strrchr(tmp.path,'/'), (char *)tmp.hdr.id);
				for (u32 i = 0; i < 6; ++i)
					idstart[i] = toupper(idstart[i]);

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
				{
					char *idstart = strcasestr(strrchr(tmp.path,'/'), (char *)tmp.hdr.id);
					for (u32 i = 0; i < 6; ++i)
						idstart[i] = toupper(idstart[i]);
						
					headerlist.push_back(tmp);
				}

				WBFS_Ext_ClosePart(part);
				continue;
			}
		}

		else if(dol)
		{
			char *buffer = (char *)(*itr).c_str();
			buffer[(*itr).find_last_of('/')] = '\0';
			(*itr).assign(buffer);
			buffer = (char *)(*itr).c_str();
			(*itr).assign(&buffer[(*itr).find_last_of('/') + 1]);

			(*itr)[0] = toupper((*itr)[0]);
			for (u32 i = 1; i < (*itr).size(); ++i)
				if((*itr)[i] == ' ')
				{
					(*itr)[i + 1] = toupper((*itr)[i + 1]);
					i++;
				}
				else (*itr)[i] = tolower((*itr)[i]);
			strcpy(tmp.hdr.title, (*itr).c_str());

			for (u32 i = 0; i < 7; ++i)
			{
				(*itr)[i] = toupper((*itr)[i]);
				if ((*itr)[i] < 'A' && (*itr)[i] > 'Z')
					(*itr)[i] = 'X';
			}
			memcpy(tmp.hdr.id, (*itr).c_str(), 6);

			headerlist.push_back(tmp);
			continue;
		}
		else if(strncasecmp(DeviceHandler::Instance()->PathToFSName((*itr).c_str()), "WBFS", 4) == 0)
		{
			int partition = DeviceHandler::Instance()->PathToDriveType((*itr).c_str());
			wbfs_t* handle = DeviceHandler::Instance()->GetWbfsHandle(partition);
			if (!handle) return;

			s32 ret = wbfs_get_disc_info(handle, count, (u8 *)&tmp.hdr, sizeof(struct discHdr), NULL);
			if(ret != 0) continue;
			
			if (tmp.hdr.magic == 0x5D1C9EA3	&& memcmp(tmp.hdr.id, "__CFG_", sizeof tmp.hdr.id) != 0)
				headerlist.push_back(tmp);
			
			continue;
		}
		count++;
	}
}

void CList::Check_For_ID(u8 *id, string path, string one, string two)
{
	size_t idstart = path.find_last_of(one);
	size_t idend = path.find_last_of(two);
	if (idend != string::npos && idstart != string::npos && idend - idstart == 7)
		for(u8 pos = 0; pos < 6; pos++)
			id[pos] = toupper(path.at(idstart + 1 + pos));
	
}