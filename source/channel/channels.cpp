/***************************************************************************
 * Copyright (C) 2010
 * by dude
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * Channel Launcher Class
 *
 * for WiiXplorer 2010
 ***************************************************************************/

#include <malloc.h>
#include "mem2.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogc/conf.h>
#include <ogc/wiilaunch.h>
#include <ogc/es.h>

#include "channels.h"
#include "banner.h"
#include "wstringEx.hpp"
#include "gecko.h"
#include "utils.h"
#include "fs.h"
#include "config.hpp"
#include "text.hpp"

#include "channel_launcher.h"

#define DOWNLOADED_CHANNELS	0x00010001
#define SYSTEM_CHANNELS		0x00010002
#define RF_NEWS_CHANNEL		0x48414741
#define RF_FORECAST_CHANNEL	0x48414641

extern "C" void ShowError(const wstringEx &error);
#define error(x) //ShowError(x)

Channels::Channels()
{
}

void Channels::Init(u32 channelType, string lang, bool reload)
{
	if (reload) init = !reload;
	if (!init || channelType != this->channelType ||
		lang != this->langCode)
	{
		this->channelType = channelType;
		this->langCode = lang;
	
		this->channels.clear();
		Search(channelType, lang);
		init = true;
	}
}

Channels::~Channels()
{
}

u32 * Channels::Load(u64 title, char *id)
{
	char app[ISFS_MAXPATH];
	u32 Size;
	u16 bootcontent;

	if(!GetAppNameFromTmd(title, app, true, &bootcontent))
		return NULL;

	return GetDol(title, id, &Size, bootcontent, false);	
}

bool Channels::Launch(u32 *data, u64 chantitle, u8 vidMode, bool vipatch, bool countryString, u8 patchVidMode)
{
	return BootChannel(data, chantitle, vidMode, vipatch, countryString, patchVidMode);
}

u64* Channels::GetChannelList(u32* count)
{
	u32 countall;
	u32 ret = ES_GetNumTitles(&countall);

	if (ret || !countall) return NULL;

	u64* titles = (u64*)MEM2_alloc(ALIGN32(countall * sizeof(u64)));
	if (!titles) return NULL;

	u64* channels = (u64*)MEM2_alloc(countall * sizeof(u64));
	if (!channels)
	{
		SAFE_FREE(titles);
		return NULL;
	}

	ret = ES_GetTitles(titles, countall);

	*count = 0;
	for (u32 i = 0; i < countall; i++)
	{
		u32 type = TITLE_UPPER(titles[i]);

		if (type == DOWNLOADED_CHANNELS || type == SYSTEM_CHANNELS)
		{
 			if (TITLE_LOWER(titles[i]) == RF_NEWS_CHANNEL ||	// skip region free news and forecast channel
				TITLE_LOWER(titles[i]) == RF_FORECAST_CHANNEL)
				continue;

			channels[(*count)++] = titles[i];
		}
	}
	SAFE_FREE(titles);

	return (u64*)MEM2_realloc(channels, *count * sizeof(u64));
}

bool Channels::GetAppNameFromTmd(u64 title, char* app, bool dol, u16* bootcontent)
{
	char tmd[ISFS_MAXPATH];

	u32 high = TITLE_UPPER(title);
	u32 low  = TITLE_LOWER(title);

	bool ret = false;

	sprintf(tmd, "/title/%08x/%08x/content/title.tmd", high, low);

	u32 size;
	u32 *data = (u32 *) ISFS_GetFile((u8 *) &tmd, &size, -1);
	if (data != NULL)
	{
		if (size > 0x208)
		{
			if (dol)
			{
				struct _tmd * tmd_file = (struct _tmd *) SIGNATURE_PAYLOAD(data);
				*bootcontent = tmd_file->contents[tmd_file->boot_index].cid;
				sprintf(app, "/title/%08x/%08x/content/%08x.app", high, low, (u32)*bootcontent);
				//gprintf("Dol-App Path: %s\n", app);
				ret = true;
			}
			else
			{
				u16 i;
				struct _tmd * tmd_file = (struct _tmd *) SIGNATURE_PAYLOAD(data);
				for(i = 0; i < tmd_file->num_contents; ++i)
					if(tmd_file->contents[i].index == 0)
						break;

				sprintf(app, "/title/%08x/%08x/content/%08x.app", high, low, tmd_file->contents[i].cid);
				//gprintf("Banner-App Path: %s\n", app);
				ret = true;
			}
		}
		SAFE_FREE(data);
	}

	return ret;
}

Banner * Channels::GetBanner(u64 title, bool imetOnly)
{
	char app[ISFS_MAXPATH];

	if (!GetAppNameFromTmd(title, app))
	{
		gprintf("No title found\n");
		return NULL;
	}
	return Banner::GetBanner(title, app, true, imetOnly);
}

bool Channels::GetChannelNameFromApp(u64 title, wchar_t* name, int language)
{
	bool ret = false;

	if (language > CONF_LANG_KOREAN)
		language = CONF_LANG_ENGLISH;

	Banner *banner = GetBanner(title, true);
	if (banner != NULL)
	{
		ret = banner->GetName(name, language);
		delete banner;
		banner = NULL;
	}
	
	return ret;
}

int Channels::GetLanguage(const char *lang)
{
	if (strncmp(lang, "JP", 2) == 0) return CONF_LANG_JAPANESE;
	else if (strncmp(lang, "EN", 2) == 0) return CONF_LANG_ENGLISH;
	else if (strncmp(lang, "DE", 2) == 0) return CONF_LANG_GERMAN;
	else if (strncmp(lang, "FR", 2) == 0) return CONF_LANG_FRENCH;
	else if (strncmp(lang, "ES", 2) == 0) return CONF_LANG_SPANISH;
	else if (strncmp(lang, "IT", 2) == 0) return CONF_LANG_ITALIAN;
	else if (strncmp(lang, "NL", 2) == 0) return CONF_LANG_DUTCH;
	else if (strncmp(lang, "ZHTW", 4) == 0) return CONF_LANG_TRAD_CHINESE;
	else if (strncmp(lang, "ZH", 2) == 0) return CONF_LANG_SIMP_CHINESE;
	else if (strncmp(lang, "KO", 2) == 0) return CONF_LANG_KOREAN;
	
	return CONF_LANG_ENGLISH; // Default to EN
}

void Channels::Search(u32 channelType, string lang)
{
	u32 count;
	u64* list = GetChannelList(&count);
	if (!list) return;

	int language = lang.size() == 0 ? CONF_GetLanguage() : GetLanguage(lang.c_str());

	for (u32 i = 0; i < count; i++)
	{
		if (channelType == 0 || channelType == TITLE_UPPER(list[i]))
		{
			Channel channel;
			if (GetChannelNameFromApp(list[i], channel.name, language))
			{
				channel.title = list[i];

				u32 title_h = (u32)channel.title;
				sprintf(channel.id, "%c%c%c%c", title_h >> 24, title_h >> 16, title_h >> 8, title_h);
			
				channels.push_back(channel);
			}
		}
	}

	SAFE_FREE(list);
}

wchar_t * Channels::GetName(int index)
{
	if (index < 0 || index > Count() - 1)
	{
		return (wchar_t *) "";
	}
	return channels.at(index).name;
}

int Channels::Count() 
{ 
	return channels.size(); 
}

char * Channels::GetId(int index)
{
	if (index < 0 || index > Count() - 1) return (char *) "";
	return channels.at(index).id;
}

u64 Channels::GetTitle(int index)
{
	if (index < 0 || index > Count() - 1) return 0;
	return channels.at(index).title;
}

Channel * Channels::GetChannel(int index)
{
	if (index < 0 || index > Count() - 1) return NULL;
	return &channels.at(index);
}
