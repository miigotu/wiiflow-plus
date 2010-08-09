/*
Load game information from XML - Lustar
*/
#include <zlib.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "../loader/utils.h"
#include "../loader/wbfs_fat.h"
#include "xml.h"

#include "unzip/unzip.h"
#include "unzip/miniunz.h"

/* config */
char xmlCfgLang[3];
char * xmlData;
struct gameXMLinfo gameinfo;
struct gameXMLinfo gameinfo_reset;
int xmlgameCnt;

bool xml_loaded = false;
int array_size = 0;

bool db_debug = 0;

static char langlist[11][22] =
{{"Console Default"},
{"Japanese"},
{"English"},
{"German"},
{"French"},
{"Spanish"},
{"Italian"},
{"Dutch"},
{"S. Chinese"},
{"T. Chinese"},
{"Korean"}};

static char langcodes[11][3] =
{{""},
{"JA"},
{"EN"},
{"DE"},
{"FR"},
{"ES"},
{"IT"},
{"NL"},
{"ZH"},
{"ZH"},
{"KO"}};

char * getLang(int lang) {
	if (lang < 1 || lang > 10) return "EN";
	return langcodes[lang+1];
}

bool str_replace(char *str, char *olds, char *news, int size)
{
	char *p;
	int len;
	p = strstr(str, olds);
	if (!p) return false;
	// new len
	len = strlen(str) - strlen(olds) + strlen(news);
	// check size
	if (len >= size) return false;
	// move remainder to fit (and nul)
	memmove(p+strlen(news), p+strlen(olds), strlen(p)-strlen(olds)+1);
	// copy new in place
	memcpy(p, news, strlen(news));
	// terminate
	str[len] = 0;
	return true;
}

bool str_replace_all(char *str, char *olds, char *news, int size) {
	int cnt = 0;
	bool ret = str_replace(str, olds, news, size);
	while (ret) {
		ret = str_replace(str, olds, news, size);
		cnt++;
	}
	return (cnt > 0);
}

#define STRAPPEND(DST,SRC) strappend(DST,SRC,sizeof(DST))
char *strappend(char *dest, char *src, int size)
{
	int len = strlen(dest);
	strcopy(dest+len, src, size-len);
	return dest;
}

char * unescape(char *input, int size) {
	str_replace_all(input, "&gt;", ">", size);
	str_replace_all(input, "&lt;", "<", size);
	str_replace_all(input, "&quot;", "\"", size);
	str_replace_all(input, "&apos;", "'", size);
	str_replace_all(input, "&amp;", "&", size);
	return input;
}

void wordWrap(char *tmp, char *input, int width, int maxLines, int length)
{
	int maxLength = width * maxLines;
	char *whitespace = NULL;
	unescape(input, length);
	char dots[4] = {0xE2, 0x80, 0xA6, 0};
	str_replace_all(input, dots, "...", length);
	strncpy(tmp, input, maxLength);
	int lineLength = 0;
	int wordLength = 0;
	int lines = 1;
	
	char * ptr=tmp;
	while (*ptr!='\0') {
		if (*ptr == '\n' || *ptr == '\r')
			*ptr = ' ';
		ptr++;
	}
	str_replace_all(tmp, "  ", " ", sizeof(tmp));
	
	for (ptr=tmp;*ptr!=0;ptr++) {
		if (lines >= maxLines) *ptr = 0;
		if (*ptr == ' ') {
			if (lineLength + wordLength > width) {
				if (whitespace) {
					*whitespace = '\n';
					lineLength = ptr - whitespace;
					whitespace = NULL;
				} else {
					*ptr = '\n';
					lineLength = 0;
				}
				lines++;
			} else {
				whitespace = ptr;
				lineLength += wordLength+1;
			}
			wordLength = 0;
		} else if (*ptr == '\n') {
			lineLength = 0;
			wordLength = 0;
		} else {
			wordLength++;
		}
	}
	if (length > maxLength && lineLength <= (width - 3)) { 
		strncat(tmp, "...", 3);
	}
}

//1 = Required, 0 = Usable, -1 = Not Used
//Controller = wiimote, nunchuk, classic, guitar, etc
int getControllerTypes(char *controller, char * gameid)
{
	int id = getIndexFromId(gameid);
	if (id < 0)
		return -1;
	int i = 0;
	for (;i<16;i++) {
		if (!strnicmp(game_info[id].accessoriesReq[i], controller, 20)) {
			return 1;
		} else if (!strnicmp(game_info[id].accessories[i], controller, 20)) {
			return 0;
		}
	}
	return -1;
}

bool hasFeature(char *feature, char *gameid)
{
	int id = getIndexFromId(gameid);
	if (id < 0)
		return 0;
	int i = 0;
	for (;i<8;i++) {
		if (!strnicmp(game_info[id].wififeatures[i], feature, 15)) {
			return 1;
		}
	}
	return 0;
}

bool hasGenre(char *genre, char * gameid)
{
	int id = getIndexFromId(gameid);
	if (id < 0)
		return 0;
	if (strstr(game_info[id].genre, genre)) {
		return 1;
	}
	return 0;
}

bool xml_getCaseColor(u32 *color, char *gameid)
{
	int id = getIndexFromId(gameid);
	if (id < 0) return 0;
	if (game_info[id].caseColor > 0x0) {
		*color = game_info[id].caseColor;
		return 1;
	}
	return 0;
}

bool DatabaseLoaded() {
	return xml_loaded;
}

void CloseXMLDatabase()
{
	//Doesn't work?
    SAFE_FREE(game_info);
	xml_loaded = false;
	array_size = 0;
}

bool OpenXMLFile(char *filename)
{
	gameinfo = gameinfo_reset;
	xml_loaded = false;
	char* strresult = strstr(filename,".zip");
    if (strresult == NULL) {
		FILE *filexml;
		filexml = fopen(filename, "rb");
		if (!filexml)
			return false;
		
		fseek(filexml, 0 , SEEK_END);
		u32 size = ftell(filexml);
		rewind(filexml);
		xmlData = (char*)malloc(size);
		memset(xmlData, 0, size);
		if (xmlData == NULL) {
			fclose(filexml);
			return false;
		}
		u32 ret = fread(xmlData, 1, size, filexml);
		fclose(filexml);
		if (ret != size)
			return false;
	} else {
		unzFile unzfile = unzOpen(filename);
		if (unzfile == NULL)
			return false;
			
		unzOpenCurrentFile(unzfile);
		unz_file_info zipfileinfo;
		unzGetCurrentFileInfo(unzfile, &zipfileinfo, NULL, 0, NULL, 0, NULL, 0);	
		int zipfilebuffersize = zipfileinfo.uncompressed_size;
		if (db_debug) {
			printf("uncompressed xml: %dkb\n", zipfilebuffersize/1024);
		}
		xmlData = (char*)malloc(zipfilebuffersize);
		memset(xmlData, 0, zipfilebuffersize);
		if (xmlData == NULL) {
			unzCloseCurrentFile(unzfile);
			unzClose(unzfile);
			return false;
		}
		unzReadCurrentFile(unzfile, xmlData, zipfilebuffersize);
		unzCloseCurrentFile(unzfile);
		unzClose(unzfile);
	}

	xml_loaded = (xmlData == NULL) ? false : true;
	return xml_loaded;
}

/* convert language text into ISO 639 two-letter language code */
char *ConvertLangTextToCode(char *languagetxt)
{
	if (!strcmp(languagetxt, ""))
		return "EN";
	int i;
	for (i=1;i<=10;i++)
	{
		if (!strcasecmp(languagetxt,langlist[i])) // case insensitive comparison
			return langcodes[i];
	}
	return "EN";
}

char *VerifyLangCode(char *languagetxt)
{
	if (!strcmp(languagetxt, ""))
		return "EN";
	int i;
	for (i=1;i<=10;i++)
	{
		if (!strcasecmp(languagetxt,langcodes[i])) // case insensitive comparison
			return langcodes[i];
	}
	return "EN";
}

char ConvertRatingToIndex(char *ratingtext)
{
	int type = -1;
	if (!strcmp(ratingtext,"CERO")) { type = 0; }
	if (!strcmp(ratingtext,"ESRB")) { type = 1; }
	if (!strcmp(ratingtext,"PEGI")) { type = 2; }
	return type;
}

void ConvertRating(char *ratingvalue, char *fromrating, char *torating, char *destvalue, int destsize)
{
	if(!strcmp(fromrating,torating)) {
		strlcpy(destvalue,ratingvalue,destsize);
		return;
	}

	strcpy(destvalue,"");
	int type = -1;
	int desttype = -1;

	type = ConvertRatingToIndex(fromrating);
	desttype = ConvertRatingToIndex(torating);
	if (type == -1 || desttype == -1)
		return;
	
	/* rating conversion table */
	/* the list is ordered to pick the most likely value first: */
	/* EC and AO are less likely to be used so they are moved down to only be picked up when converting ESRB to PEGI or CERO */
	/* the conversion can never be perfect because ratings can differ between regions for the same game */
	char ratingtable[12][3][4] =
	{
		{{"A"},{"E"},{"3"}},
		{{"A"},{"E"},{"4"}},
		{{"A"},{"E"},{"6"}},
		{{"A"},{"E"},{"7"}},
		{{"A"},{"EC"},{"3"}},
		{{"A"},{"E10"},{"7"}},
		{{"B"},{"T"},{"12"}},
		{{"D"},{"M"},{"18"}},
		{{"D"},{"M"},{"16"}},
		{{"C"},{"T"},{"16"}},
		{{"C"},{"T"},{"15"}},
		{{"Z"},{"AO"},{"18"}},
	};
	
	int i;
	for (i=0;i<=11;i++)
	{
		if (!strcmp(ratingtable[i][type],ratingvalue)) {
			strlcpy(destvalue,ratingtable[i][desttype],destsize);
			return;
		}
	}
}

void strncpySafe(char *out, char *in, int max, int length) {
	strlcpy(out, in, ((max < length+1) ? max : length+1));
}

int intcpy(char *in, int length) {
	char tmp[10];
	strncpy(tmp, in, length);
	return atoi(tmp);
}

void readNode(char * p, char to[], char * open, char * close) {
	char * tmp = strstr(p, open);
	if (tmp == NULL) {
		strcpy(to, "");
	} else {
		char * s = tmp+strlen(open);
		strlcpy(to, s, strstr(tmp, close)-s+1);
	}
}

void readDate(char * tmp, int n) {
	char date[5] = "";
	readNode(tmp, date, "<date year=\"","\" month=\"");
	game_info[n].year = atoi(date);
	strlcpy(date, "", 4);
	readNode(tmp, date, "\" month=\"","\" day=\"");
	game_info[n].month = atoi(date);
	strlcpy(date, "", 4);
	readNode(tmp, date, "\" day=\"","\"/>");
	game_info[n].day = atoi(date);
}

void readRatings(char * start, int n) {
	char *locStart = strstr(start, "<rating type=\"");
	char *locEnd = NULL;
	if (locStart != NULL) {
		locEnd = strstr(locStart, "</rating>");
		if (locEnd == NULL) {
			locEnd = strstr(locStart, "\" value=\"");
			if (locEnd == NULL) return;
			strncpySafe(game_info[n].ratingtype, locStart+14, sizeof(gameinfo.ratingtype), locEnd-(locStart+14));
			locStart = locEnd;
			locEnd = strstr(locStart, "\"/>");
			strncpySafe(game_info[n].ratingvalue, locStart+9, sizeof(gameinfo.ratingvalue), locEnd-(locStart+9));
		} else {
			char * tmp = strndup(locStart, locEnd-locStart);
			char * reset = tmp;
			locEnd = strstr(locStart, "\" value=\"");
			if (locEnd == NULL) return;
			strncpySafe(game_info[n].ratingtype, locStart+14, sizeof(gameinfo.ratingtype), locEnd-(locStart+14));
			locStart = locEnd;
			locEnd = strstr(locStart, "\">");
			strncpySafe(game_info[n].ratingvalue, locStart+9, sizeof(gameinfo.ratingvalue), locEnd-(locStart+9));
			int z = 0;
			while (tmp != NULL) {
				if (z >= 15) break;
				tmp = strstr(tmp, "<descriptor>");
				if (tmp == NULL) {
					break;
				} else {
					char * s = tmp+strlen("<descriptor>");
					tmp = strstr(tmp+1, "</descriptor>");
					strncpySafe(game_info[n].ratingdescriptors[z], s, sizeof(game_info[n].ratingdescriptors[z]), tmp-s);
					z++;
				}
			}
			tmp = reset;
			SAFE_FREE(tmp);
		}
		locStart = strstr(locEnd, "<rating type=\"");
	}
}

void readPlayers(char * start, int n) {
	char *locStart = strstr(start, "<input players=\"");
	char *locEnd = NULL;
	if (locStart != NULL) {
		locEnd = strstr(locStart, "</input>");
		if (locEnd == NULL) {
			locEnd = strstr(locStart, "\"/>");
			if (locEnd == NULL) return;
			game_info[n].players = intcpy(locStart+16, locEnd-(locStart+16));
		} else {
			game_info[n].players = intcpy(locStart+16, strstr(locStart, "\">")-(locStart+16));
		}
	}
}

void readCaseColor(char * start, int n) {
	game_info[n].caseColor = 0x0;
	char *locStart = strstr(start, "<case color=\"");
	if (locStart != NULL) {
		char cc[9];
		int col, len, num;
		strcopy(cc, locStart+13, 7);
		STRAPPEND(cc, "00");  //force alpha to 00
		num = sscanf(cc,"%8x%n", &col, &len);
		if (num == 1 && len == 8)
			game_info[n].caseColor = (u32)col;
	}
}

void readWifi(char * start, int n) {
	char *locStart = strstr(start, "<wi-fi players=\"");
	char *locEnd = NULL;
	if (locStart != NULL) {
		locEnd = strstr(locStart, "</wi-fi>");
		if (locEnd == NULL) {
			locEnd = strstr(locStart, "\"/>");
			if (locEnd == NULL) return;
			game_info[n].wifiplayers = intcpy(locStart+16, locEnd-(locStart+16));
		} else {
			game_info[n].wifiplayers = intcpy(locStart+16, strstr(locStart, "\">")-(locStart+16));
			char * tmp = strndup(locStart, locEnd-locStart);
			char * reset = tmp;
			int z = 0;
			while (tmp != NULL) {
				if (z >= 7) break;
				tmp = strstr(tmp, "<feature>");
				if (tmp == NULL) {
					break;
				} else {
					char * s = tmp+strlen("<feature>");
					tmp = strstr(tmp+1, "</feature>");
					strncpySafe(game_info[n].wififeatures[z], s, sizeof(game_info[n].wififeatures[z]), tmp-s);
					z++;
				}
			}
			tmp = reset;
			SAFE_FREE(tmp);
		}
	}
}

void readTitles(char * start, int n) {
	char *locStart;
	char *locEnd = start;
	char tmpLang[3];
	int found = 0;
	int title = 0;
	int synopsis = 0;
	char *locTmp;
	char *titStart;
	char *titEnd;
	while ((locStart = strstr(locEnd, "<locale lang=\"")) != NULL) {
		strcpy(tmpLang, "");
		locEnd = strstr(locStart, "</locale>");
		if (locEnd == NULL) {
			locEnd = strstr(locStart, "\"/>");
			if (locEnd == NULL) break;
			continue;
		}
		strlcpy(tmpLang, locStart+14, 3);
		if (memcmp(tmpLang, xmlCfgLang, 2) == 0) {
			found = 3;
		} else if (memcmp(tmpLang, "EN", 2) == 0) {
			found = 2;
		} else {
			found = 1;
		}
		// 3. get the configured language text
		// 2. if not found, get english
		// 1. else get whatever is found
		locTmp = strndup(locStart, locEnd-locStart);
		if (title < found) {
			titStart = strstr(locTmp, "<title>");
			if (titStart != NULL) {
				titEnd = strstr(titStart, "</title>");
				strncpySafe(game_info[n].title, titStart+7,
						sizeof(game_info[n].title), titEnd-(titStart+7));
				unescape(game_info[n].title, sizeof(game_info[n].title));
				title = found;
			}
		}
		if (synopsis < found) {
			titStart = strstr(locTmp, "<synopsis>");
			if (titStart != NULL) {
				titEnd = strstr(titStart, "</synopsis>");
				strncpySafe(game_info[n].synopsis, titStart+10,
						sizeof(game_info[n].synopsis), titEnd-(titStart+10));
				unescape(game_info[n].synopsis,sizeof(game_info[n].synopsis));
				synopsis = found;
			}
		}
		SAFE_FREE(locTmp);
		if (synopsis == 3) break;
	}
}

void LoadTitlesFromXML(char *langtxt, bool forcejptoen)
{
	int n = 0;
	char * reset = xmlData;
	char * start;
	char * end;
	char * tmp;
	xmlgameCnt = 0;
	bool forcelang = false;
	if (strcmp(langtxt,""))
		forcelang = true;
	if (forcelang)
		strcpy(xmlCfgLang, (strlen(langtxt) == 2) ? VerifyLangCode(langtxt) : ConvertLangTextToCode(langtxt)); // convert language text into ISO 639 two-letter language code
	if (forcejptoen && (!strcmp(xmlCfgLang,"JA")))
		strcpy(xmlCfgLang,"EN");
	while (1) {
		if (xmlgameCnt >= array_size) {
			void *ptr;
			array_size++;
			ptr = realloc(game_info, (array_size * sizeof(struct gameXMLinfo)));
			if (!ptr) {
				array_size--;
				printf("ERROR: out of memory!\n");
				
				sleep(4);
				break;
			}
			game_info = (struct gameXMLinfo*)ptr;
		}
		start = strstr(xmlData, "<game");
		if (start == NULL) {
			break;
		}
		end = strstr(start, "</game");
		if (end == NULL) {
			break;
		}
		tmp = strndup(start, end-start);
		xmlData = end;
		memset(&game_info[n], 0, sizeof(game_info[n]));
		readNode(tmp, game_info[n].id, "<id>", "</id>");//ok
		if (game_info[n].id[0] == '\0') { //WTF? ERROR
			printf(" ID NULL\n");
			SAFE_FREE(tmp);
			break;
		}

		readTitles(tmp, n);
		readRatings(tmp, n);
		readDate(tmp, n);
		readWifi(tmp, n);
		
		int z = 0;
		int y = 0;
		char * p = tmp;
		while (p != NULL) {
			p = strstr(p, "<control type=\"");
			if (p == NULL) {
				break;
			} else {
				if (y >= 15 && z >= 15) break;
				char * s = p+strlen("<control type=\"");
				p = strstr(p+1, "\" required=\"");
				bool required = (*(p+12) == 't' || *(p+12) == 'T');
				if (!required) {
					if (z < 15) strncpySafe(game_info[n].accessories[z], s, sizeof(game_info[n].accessories[z]), p-s);
					z++;
				} else {
					if (y < 15) strncpySafe(game_info[n].accessoriesReq[y], s, sizeof(game_info[n].accessoriesReq[y]), p-s);
					y++;
				}
			}
		}
		
		readNode(tmp, game_info[n].region, "<region>", "</region>");
		readNode(tmp, game_info[n].developer, "<developer>", "</developer>");
		readNode(tmp, game_info[n].publisher, "<publisher>", "</publisher>");
		readNode(tmp, game_info[n].genre, "<genre>", "</genre>");
		readCaseColor(tmp, n);
		readPlayers(tmp, n);
		//ConvertRating(game_info[n].ratingvalue, gameinfo[n].ratingtype, "ESRB");
		SAFE_FREE(tmp);
		n++;
		xmlgameCnt++;
	}
	xmlData = reset;
	SAFE_FREE(xmlData);
	return;
}

/* load renamed titles from proper names and game info XML, needs to be after cfg_load_games */
bool OpenXMLDatabase(const char* xmlfilepath, char* argdblang, bool argJPtoEN)
{
	if (xml_loaded) {
		return true;
	}

	bool opensuccess = true;

	char pathname[200];
	snprintf(pathname, sizeof(pathname), "%s/wiitdb.zip", xmlfilepath);
	opensuccess = OpenXMLFile(pathname);
	if(!opensuccess)
	{
		snprintf(pathname, sizeof(pathname), "%s/wiitdb.xml", xmlfilepath);
		opensuccess = OpenXMLFile(pathname);
		if (!opensuccess) {
				CloseXMLDatabase();
				return false;
			}
	
	}
	LoadTitlesFromXML(argdblang, argJPtoEN);

	return true;
}

bool ReloadXMLDatabase(const char* xmlfilepath, char* argdblang, bool argJPtoEN)
{
	if (xml_loaded) {
		CloseXMLDatabase();
	}
	return OpenXMLDatabase(xmlfilepath, argdblang, argJPtoEN);
}

int getIndexFromId(char * gameid)
{
	int n = 0;
	if (gameid == NULL)
		return -1;

	for (;n<xmlgameCnt;n++) {
		if (!strcmp(game_info[n].id, gameid)) {
			return n;
		}
	}
	return -1;
}

bool LoadGameInfoFromXML(char * gameid)
/* gameid: full game id */
{
	gameinfo = gameinfo_reset;
	int n = getIndexFromId(gameid);
	if (n >= 0) {
		gameinfo = game_info[n];
		return true;
	}
	return false;
}

int getCountPlayers(char *gameid)
{
	int id = getIndexFromId(gameid);
	if (id < 0) return 0;
	
	return game_info[id].players;
}