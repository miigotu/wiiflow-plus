#include "list.hpp"
#include "gecko.h"
#include "GameTDB.hpp"
#include "config.hpp"
#include "defines.h"
#include "channels.h"

void CList::GetPaths(vector<string> &pathlist, string containing, string directory, bool wbfs_fs)
{
	if(!wbfs_fs)
	{
		/* Open primary directory */
		DIR *dir_itr = opendir(directory.c_str());
		if(!dir_itr) return;

		vector<string> compares = stringToVector(containing, "|");
		vector<string> subdirs;

		struct dirent *ent;

		/* Read primary entries */
		while((ent = readdir(dir_itr)) != NULL)
		{
			if(ent->d_name[0] == '.' || strlen(ent->d_name) < 6) continue;

			if(ent->d_type == DT_REG)
			{
				for(vector<string>::iterator compare = compares.begin(); compare != compares.end(); compare++)
					if(strcasestr(ent->d_name, (*compare).c_str()) != NULL)
					{
						//gprintf("Pushing %s to the list.\n", sfmt("%s/%s", directory.c_str(), ent->d_name).c_str());
						pathlist.push_back(sfmt("%s/%s", directory.c_str(), ent->d_name));
						break;
					}
			}
			else subdirs.push_back(sfmt("%s/%s", directory.c_str(), ent->d_name));
		}
		closedir(dir_itr);

		if(subdirs.size() > 0)
		{
			for(vector<string>::iterator subdir = subdirs.begin(); subdir != subdirs.end(); subdir++)
			{
				dir_itr = opendir((*subdir).c_str());
				if(!dir_itr) continue;

				/* Read secondary entries */
				while((ent = readdir(dir_itr)) != NULL)
				{
					if(ent->d_type != DT_REG) continue;
					if(strlen(ent->d_name) < 8) continue;

					for(vector<string>::iterator compare = compares.begin(); compare != compares.end(); compare++)
						if(strcasestr(ent->d_name, (*compare).c_str()) != NULL)
						{
							//gprintf("Pushing %s to the list.\n", sfmt("%s/%s", (*subdir).c_str(), ent->d_name).c_str());
							pathlist.push_back(sfmt("%s/%s", (*subdir).c_str(), ent->d_name));
							break;
						}
				}
				closedir(dir_itr);
			}
		}
	}
	else
	{
		if(strcasestr(containing.c_str(), ".dol") != 0) return;

		int partition = DeviceHandler::Instance()->PathToDriveType(directory.c_str());
		wbfs_t* handle = DeviceHandler::Instance()->GetWbfsHandle(partition);
		if(!handle) return;

		u32 count = wbfs_count_discs(handle);
		for(u32 i = 0; i < count; i++)
			pathlist.push_back(directory);
	}
}

#define NUM_CONVENTIONS 5
static const string test_strings[NUM_CONVENTIONS][2] = 
{
	{"[", "]"},	/* [GAMEID] Title, [GAMEID]_Title, Title [GAMEID], Title_[GAMEID] */
	{"/", "."},	/* GAMEID.wbfs, GAMEID.iso */
	{"/", "_"},	/* GAMEID_Title */
	{"_", "."},	/* Title_GAMEID */ // <-- Unsafe?
	{" ", "."}	/* Title GAMEID */ //<-- Unsafe?
};

static bool Check_For_ID(char *id, string path)
{
	for(u32 i = 0; i < NUM_CONVENTIONS; i++)
	{
		size_t idstart = path.find_last_of(test_strings[i][0]);
		size_t idend = path.find_last_of(test_strings[i][1]);
		if(idend != string::npos && idstart != string::npos && idend > idstart && idend - idstart == 7)
		{
			for(u8 pos = 0; pos < 6; pos++)
			{
				id[pos] = toupper(path[idstart + 1 + pos]);
				if(!isalnum(id[pos])) return false;
			}

			return true;
		}
	}
	return false;
}

void CList::GetHeaders(vector<string> pathlist, deque<dir_discHdr> &headerlist, string settingsDir, string curLanguage)
{
	if(pathlist.size() < 1) return;

	gprintf("Getting headers for paths in pathlist (%d)\n", pathlist.size());

	dir_discHdr game;
	discHdr hdr;
	u32 count = 0;
	string GTitle;

	Config custom_titles;
	if(settingsDir.size() > 0)
	{
		string custom_titles_path = sfmt("%s/" CTITLES_FILENAME, settingsDir.c_str());
		custom_titles.load(custom_titles_path.c_str());
	}

	GameTDB gameTDB;
	if(settingsDir.size() > 0)
	{
		gameTDB.OpenFile(sfmt("%s/wiitdb.xml", settingsDir.c_str()).c_str());
		if(curLanguage.size() == 0) curLanguage = "EN";
		gameTDB.SetLanguageCode(curLanguage.c_str());
	}

	for(vector<string>::iterator itr = pathlist.begin(); itr != pathlist.end(); itr++)
	{
		bzero(&game, sizeof(dir_discHdr));
		bzero(&hdr, sizeof(discHdr));
		strncpy(game.path, itr->c_str(), sizeof(game.path));
		game.casecolor = 1;
		
		size_t dotPos = itr->rfind(".");
		if(dotPos >= itr->size()) continue;

		string extension = lowerCase(string(*itr, dotPos));
		bool wbfs = extension == ".wbfs";
		if(wbfs || extension == ".iso")
		{
			if(!Check_For_ID(game.id, *itr))
			{
				gprintf("Skipping file: '%s'\n", itr->c_str());
				continue;
			}

			// Get info from custom titles
			GTitle = custom_titles.getString("TITLES", game.id);
			int ccolor = custom_titles.getColor("COVERS", game.id, game.casecolor).intVal();

			if(GTitle.size() > 0 || (gameTDB.IsLoaded() && gameTDB.GetTitle(game.id, GTitle)))
			{
				mbstowcs(game.wtitle, GTitle.c_str(), sizeof(game.wtitle));
				Asciify(game.wtitle);
				game.casecolor = ccolor != 1 ? ccolor : gameTDB.GetCaseColor(game.id);

				game.wifi = gameTDB.GetWifiPlayers(game.id);
				game.players = gameTDB.GetPlayers(game.id);
				//game.controllers = gameTDB.GetAccessories(game.id);

				headerlist.push_back(game);
				continue;
			}

			FILE *fp = fopen(itr->c_str(), "rb");
			if(fp)
			{
				fseek(fp, wbfs ? 512 : 0, SEEK_SET);
				fread(&hdr, sizeof(discHdr), 1, fp);
				SAFE_CLOSE(fp);
			}

			if(hdr.magic == 0x5D1C9EA3 && memcmp(game.id, hdr.id, sizeof hdr.id) == 0)
			{
				mbstowcs(game.wtitle, hdr.title, sizeof(game.wtitle));
				Asciify(game.wtitle);
				game.casecolor = ccolor;
				headerlist.push_back(game);
				continue;
			}
		}
		else if(extension == ".dol" || extension == ".elf")
		{
			string filename = lowerCase(string(*itr, itr->find_last_of('/') + 1));
			if(filename != "boot.dol" && filename != "boot.elf") continue;

			itr->resize(itr->find_last_of('/'));
			string name = string(*itr, itr->find_last_of('/') + 1);

			strncpy(game.id, upperCase(name).c_str(), 6);
			for (u32 i = 0; i < 6; ++i)
				if(!isalnum(game.id[i]) || game.id[i] == ' ' || game.id[i] == '\0')
					game.id[i] = '_';

			string line;
			bool xml_good = false;
			struct stat dummy;
			string xmlfile = itr->append("/meta.xml");
			if(stat(xmlfile.c_str(), &dummy) != -1)
			{
				string templine;
				size_t pos1, pos2;
				ifstream xml (xmlfile.c_str(), ifstream::in);
				while(xml.good())
				{
					getline(xml, templine);
					pos1 = templine.find("name>");
					pos2 = templine.find("</name");
					if(pos1 != string::npos || pos2 != string::npos)
						line += templine;
						
					if((pos1 = line.find("name>") != string::npos) && (pos2 = line.find("</name")) != string::npos)
					{
						line = string(line, line.find(">") + 1);
						line.resize(line.find("<"));
						xml_good = true;
						break;
					}
				}
			}
			if(xml_good)
				name = line;
			else
			{
				name[0] = toupper(name[0]);
				for (u32 i = 1; i < name.size(); ++i)
				{
					if(name[i] == '_' || name[i] == '-')
						name[i] = ' ';

					name[i] = name[i - 1] == ' ' ? toupper(name[i]): tolower(name[i]);
				}
			}

			// Get info from custom titles
			GTitle = custom_titles.getString("TITLES", game.id);
			int ccolor = custom_titles.getColor("COVERS", game.id, game.casecolor).intVal();
			if(GTitle.size() > 0 || (gameTDB.IsLoaded() && gameTDB.GetTitle(game.id, GTitle)))
			{
				mbstowcs(game.wtitle, GTitle.c_str(), sizeof(game.wtitle));
				game.casecolor = ccolor != 1 ? ccolor : gameTDB.GetCaseColor(game.id);
			}
			else
			{
				mbstowcs(game.wtitle, name.c_str(), sizeof(game.wtitle));
				game.casecolor = ccolor;
			}
			Asciify(game.wtitle);
			headerlist.push_back(game);
		}
		else if(strncasecmp(DeviceHandler::Instance()->PathToFSName(itr->c_str()), "WBFS", 4) == 0)
		{
			u8 partition = DeviceHandler::Instance()->PathToDriveType(itr->c_str());
			wbfs_t* handle = DeviceHandler::Instance()->GetWbfsHandle(partition);
			if(!handle) return;

			s32 ret = wbfs_get_disc_info(handle, count, (u8 *)&hdr, sizeof(struct discHdr), NULL);
			count++;

			if(ret != 0) continue;

			if(hdr.magic == 0x5D1C9EA3 && isalnum(hdr.id[0]))
			{
				memcpy(game.id, hdr.id, 6);
				GTitle = custom_titles.getString("TITLES", game.id);
				int ccolor = custom_titles.getColor("COVERS", game.id, 1).intVal();
				if(GTitle.size() > 0 || (gameTDB.IsLoaded() && gameTDB.GetTitle(game.id, GTitle)))
				{
					mbstowcs(game.wtitle, GTitle.c_str(), sizeof(game.wtitle));
					game.casecolor = ccolor != 1 ? ccolor : gameTDB.GetCaseColor(game.id);

					game.wifi = gameTDB.GetWifiPlayers(game.id);
					game.players = gameTDB.GetPlayers(game.id);
					//game.hdr.controllers = gameTDB.GetAccessories(game.id);
				}
				else
				{
					mbstowcs(game.wtitle, hdr.title, sizeof(game.wtitle));
					game.casecolor = ccolor;
				}
				Asciify(game.wtitle);
				headerlist.push_back(game);
			}
		}
	}

	if(gameTDB.IsLoaded())
		gameTDB.CloseFile();
}

void CList::GetChannels(deque<dir_discHdr> &headerlist, string settingsDir, u32 channelType, string curLanguage)
{
	Channels m_channels;
	m_channels.Init(channelType, curLanguage, true);

	Config custom_titles;
	if(settingsDir.size() > 0)
	{
		string custom_titles_path = sfmt("%s/" CTITLES_FILENAME, settingsDir.c_str());
		custom_titles.load(custom_titles_path.c_str());
	}

	GameTDB gameTDB;
	if(settingsDir.size() > 0)
	{
		gameTDB.OpenFile(sfmt("%s/wiitdb.xml", settingsDir.c_str()).c_str());
		if(curLanguage.size() == 0) curLanguage = "EN";
		gameTDB.SetLanguageCode(curLanguage.c_str());
	}

	u32 count = m_channels.Count();

	for (u32 i = 0; i < count; ++i)
	{
		Channel *chan = m_channels.GetChannel(i);

		if(chan->id == NULL) continue; // Skip invalid channels

		dir_discHdr game;
		bzero(&game, sizeof(dir_discHdr));
		game.casecolor = 1;

		memcpy(game.id, chan->id, 4);

		string GTitle = custom_titles.getString("TITLES", game.id);
		int ccolor = custom_titles.getColor("COVERS", game.id, game.casecolor).intVal();

		if(GTitle.size() > 0 || (gameTDB.IsLoaded() && gameTDB.GetTitle(game.id, GTitle)))
			mbstowcs(game.wtitle, GTitle.c_str(), sizeof(game.wtitle));
		else
			memcpy(game.wtitle, chan->name, sizeof(game.wtitle));

		Asciify(game.wtitle);

		game.casecolor = ccolor != 1 ? ccolor : gameTDB.GetCaseColor(game.id);

		game.wifi = gameTDB.GetWifiPlayers(game.id);
		game.players = gameTDB.GetPlayers(game.id);
		//game.hdr.controllers = gameTDB.GetAccessories(game.id);

		game.chantitle = chan->title;

		headerlist.push_back(game);
	}

	if(gameTDB.IsLoaded())
		gameTDB.CloseFile();
}