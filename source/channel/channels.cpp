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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogc/conf.h>
#include <ogc/isfs.h>
#include <ogc/wiilaunch.h>
#include <ogc/es.h>

#include "certs.h"
#include "channels.h"
#include "MD5.h"
#include "wstringEx.hpp"
#include "gecko.h"
#include "fs.h"
#include "config.hpp"
#include "text.hpp"

#define IMET_OFFSET			0x40
#define IMET_SIGNATURE		0x494d4554
#define DOWNLOADED_CHANNELS	0x00010001
#define SYSTEM_CHANNELS		0x00010002
#define RF_NEWS_CHANNEL		0x48414741
#define RF_FORECAST_CHANNEL	0x48414641

extern "C" void ShowError(const wstringEx &error);
#define error(x) //ShowError(x)

typedef struct
{
	u8  zeroes1[0x40];
	u32 sig;	// "IMET"
	u32 unk1;
	u32 unk2;
	u32 filesizes[3];
	u32 unk3;
	u16 name_japanese[IMET_MAX_NAME_LEN];
	u16 name_english[IMET_MAX_NAME_LEN];
	u16 name_german[IMET_MAX_NAME_LEN];
	u16 name_french[IMET_MAX_NAME_LEN];
	u16 name_spanish[IMET_MAX_NAME_LEN];
	u16 name_italian[IMET_MAX_NAME_LEN];
	u16 name_dutch[IMET_MAX_NAME_LEN];
	u16 name_simp_chinese[IMET_MAX_NAME_LEN];
	u16 name_trad_chinese[IMET_MAX_NAME_LEN];
	u16 name_korean[IMET_MAX_NAME_LEN];
	u8  zeroes2[0x24c];
	u8  md5[0x10];
} IMET;

Channels::Channels()
{
	/*u32 ret = ES_Identify((u32*)Certificates, sizeof(Certificates), (u32*)Tmd, sizeof(Tmd), (u32*)Ticket, sizeof(Ticket), 0);
	isIdentified = ret == 0;*/
}

void Channels::Init(u32 channelType, string lang)
{
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
/*
bool Channels::CanIdentify()
{
	return isIdentified;
}
*/
u64* Channels::GetChannelList(u32* count)
{
	u32 countall;
	u32 ret = ES_GetNumTitles(&countall);

	if (ret || !countall)
		return NULL;

	u64* titles = (u64*)memalign(32, countall * sizeof(u64));
	if (!titles)
		return NULL;

	u64* channels = (u64*)malloc(countall * sizeof(u64));
	if (!channels)
	{
		free(titles);
		return NULL;
	}

	ret = ES_GetTitles(titles, countall);

	*count = 0;
	for (u32 i = 0; i < countall; i++)
	{
		u32 type = titles[i] >> 32;

		if (type == DOWNLOADED_CHANNELS || type == SYSTEM_CHANNELS)
		{
			if ((titles[i] & 0xFFFFFFFF) == RF_NEWS_CHANNEL ||	// skip region free news and forecast channel
				(titles[i] & 0xFFFFFFFF) == RF_FORECAST_CHANNEL)
				continue;

			channels[(*count)++] = titles[i];
		}
	}

	free(titles);

	return (u64*)realloc(channels, *count * sizeof(u64));
}

bool Channels::GetAppNameFromTmd(u64 title, char* app)
{
	char tmdpath[ISFS_MAXPATH];

	u32 high = (u32)(title >> 32);
	u32 low  = (u32)(title & 0xFFFFFFFF);

	bool ret = false;

	sprintf(tmdpath, "/title/%08x/%08x/content/title.tmd", high, low);

	u32 size;
	SmartBuf data = SmartBuf(ISFS_GetFile((u8 *) &tmdpath, &size, -1));
	if (size > 0)
	{
		u32 appName;
		memcpy(&appName, &data.get()[0x1E4], 4);
		sprintf(app, "/title/%08x/%08x/content/%08x.app\n", high, low, appName);
		ret = true;
	}

	return ret;
}

void Channels::GetBanner(SmartBuf &bnr, u32 &size, u64 title, s32 length)
{
	ISFS_Initialize();
	GetBnr(bnr, size, title, length);
	ISFS_Deinitialize();
}

bool Channels::GetBnr(SmartBuf &bnr, u32 &size, u64 title, s32 length)
{
	char app[ISFS_MAXPATH];

	if (!GetAppNameFromTmd(title, app)) {
		return false;
	}
	
	bnr = SmartBuf(ISFS_GetFile((u8 *) &app, &size, length), SmartBuf::SRCALL_MALLOC);
	return size != 0;
}

bool Channels::GetChannelNameFromApp(u64 title, wchar_t* name, int language)
{
	bool ret = false;

	if (language > CONF_LANG_KOREAN)
		language = CONF_LANG_ENGLISH;

	SmartBuf bnr;
	u32 size;
	if (GetBnr(bnr, size, title, sizeof(IMET) + IMET_OFFSET))
	{
		IMET *imet = (IMET *) (bnr.get() + IMET_OFFSET);
		if (imet->sig == IMET_SIGNATURE)
		{
			unsigned char md5[16];
			unsigned char imetmd5[16];

			memcpy(imetmd5, imet->md5, 16);
			memset(imet->md5, 0, 16);
			
			MD5(md5, (unsigned char*)(imet), sizeof(IMET));
			if (memcmp(imetmd5, md5, 16) == 0)
			{
				//now we can be pretty sure that we have a valid imet :)
				if (imet->name_japanese[language*IMET_MAX_NAME_LEN] == 0)
				{
					// channel name is not available in system language
					if (imet->name_english[0] != 0)
					{
						language = CONF_LANG_ENGLISH;
					}
					else
					{
						// channel name is also not available on english, get ascii name
						language = -1;
						for (int i = 0; i < 4; i++)
						{
							name[i] = ((title&0xFFFFFFFF) >> (24-i*8)) & 0xFF;
						}
						name[4] = 0;
					}
				}

				if (language >= 0)
				{
					// retrieve channel name in system language or on english
					for (int i = 0; i < IMET_MAX_NAME_LEN; i++)
					{
						name[i] = imet->name_japanese[i+(language*IMET_MAX_NAME_LEN)];
					}						
				}
				ret = true;
			}
			else
				gprintf("Invalid md5\n");
		}
		else
			gprintf("Invalid sig\n");
	}

	return ret;
}
/*
void Channels::Launch(int index)
{
	WII_LaunchTitle(channels.at(index).title);
}
*/
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
	error(wfmt(L"You have %i channels installed\n(Press A)", count).c_str());	
	if (!list)
		return;

	ES_Identify((u32*)Certificates, sizeof(Certificates), (u32*)Tmd, sizeof(Tmd), (u32*)Ticket, sizeof(Ticket), 0);
	ISFS_Initialize();

	int language = lang.size() == 0 ? CONF_GetLanguage() : GetLanguage(lang.c_str());

	for (u32 i = 0; i < count; i++)
	{
		if (channelType == 0 || channelType == (list[i] >> 32))
		{
			Channel channel;
			if (GetChannelNameFromApp(list[i], channel.name, language))
			{
				channel.title = list[i];

				u32 title_h = (u32)channel.title;
				error(wfmt(L"Channel %i has ID '%c%c%c%c'\n(Press A)", i+1, title_h >> 24, title_h >> 16, title_h >> 8, title_h));
				sprintf(channel.id, "%c%c%c%c", title_h >> 24, title_h >> 16, title_h >> 8, title_h);
			
				channels.push_back(channel);
			}
		}
	}

	ISFS_Deinitialize();

	free(list);
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
	if (index < 0 || index > Count() - 1)
	{
		return (char *) "";
	}
	return channels.at(index).id;
}

u64 Channels::GetTitle(int index)
{
	if (index < 0 || index > Count() - 1)
	{
		return 0;
	}
	return channels.at(index).title;
}

Channel * Channels::GetChannel(int index)
{
	if (index < 0 || index > Count() - 1)
	{
		return NULL;
	}
	return &channels.at(index);
}
