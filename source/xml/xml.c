/*
Load game information from XML - Lustar
*/
#include <zlib.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <ogc/lwp.h>
#include "../loader/utils.h"
#include "../loader/wbfs_ext.h"
#include "xml.h"

#include "unzip/unzip.h"
#include "unzip/miniunz.h"

#include "gecko.h"

/* config */
char xmlCfgLang[3];
struct gameXMLinfo gameinfo;
FILE *db;
FILE *idx;
int amount_of_games;
void *game_idx = NULL;

bool (*callback)(void *, float) = NULL;
void *userdata = NULL;
f32 progress_step;

int db_debug = 0;

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

bool DatabaseLoaded() {
	return db != NULL && game_idx != NULL;
}

void CloseXMLDatabase()
{
	if (db != NULL)
	{
		fclose(db);
		db = NULL;
	}
	if (idx != NULL)
	{
		fclose(idx);
		idx = NULL;
	}
	if (game_idx != NULL)
	{
		free(game_idx);
		game_idx = NULL;
	}
	memset(&gameinfo, 0, sizeof(struct gameXMLinfo));
}

char * OpenXMLFile(char *filename)
{
	memset(&gameinfo, 0, sizeof(struct gameXMLinfo));

	char *xmlData = NULL;
	char* strresult = strstr(filename,".zip");
    if (strresult == NULL) {
		FILE *filexml;
		filexml = fopen(filename, "rb");
		if (!filexml)
			return NULL;
		
		fseek(filexml, 0 , SEEK_END);
		u32 size = ftell(filexml);
		rewind(filexml);
		xmlData = (char*)malloc(size);
		memset(xmlData, 0, size);
		if (xmlData == NULL) {
			fclose(filexml);
			return NULL;
		}
		u32 ret = fread(xmlData, 1, size, filexml);
		fclose(filexml);
		if (ret != size)
		{
			free(xmlData);
			return NULL;
		}
	} 
	else
	{
		unzFile unzfile = unzOpen(filename);
		if (unzfile == NULL)
			return NULL;
			
		unzOpenCurrentFile(unzfile);
		unz_file_info zipfileinfo;
		unzGetCurrentFileInfo(unzfile, &zipfileinfo, NULL, 0, NULL, 0, NULL, 0);	
		int zipfilebuffersize = zipfileinfo.uncompressed_size;
		if (db_debug) {
			printf("uncompressed xml: %dkb\n", zipfilebuffersize/1024);
		}
		xmlData = (char*)malloc(zipfilebuffersize);
		if (xmlData == NULL) {
			unzCloseCurrentFile(unzfile);
			unzClose(unzfile);
			return NULL;
		}
		memset(xmlData, 0, zipfilebuffersize);
		unzReadCurrentFile(unzfile, xmlData, zipfilebuffersize);
		unzCloseCurrentFile(unzfile);
		unzClose(unzfile);
	}

	return xmlData;
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

void readDate(struct gameXMLinfo *gameinfo, char * tmp) {
	char date[5] = "";
	readNode(tmp, date, "<date year=\"","\" month=\"");
	gameinfo->year = atoi(date);
	strncpy(date, "", 4);
	readNode(tmp, date, "\" month=\"","\" day=\"");
	gameinfo->month = atoi(date);
	strncpy(date, "", 4);
	readNode(tmp, date, "\" day=\"","\"/>");
	gameinfo->day = atoi(date);
}

void readRatings(struct gameXMLinfo *gameinfo, char * start) {
	char *locStart = strstr(start, "<rating type=\"");
	char *locEnd = NULL;
	if (locStart != NULL) {
		locEnd = strstr(locStart, "</rating>");
		if (locEnd == NULL) {
			locEnd = strstr(locStart, "\" value=\"");
			if (locEnd == NULL) return;
			strncpySafe(gameinfo->ratingtype, locStart+14, sizeof(gameinfo->ratingtype), locEnd-(locStart+14));
			locStart = locEnd;
			locEnd = strstr(locStart, "\"/>");
			strncpySafe(gameinfo->ratingvalue, locStart+9, sizeof(gameinfo->ratingvalue), locEnd-(locStart+9));
		} else {
			char * tmp = strndup(locStart, locEnd-locStart);
			char * reset = tmp;
			locEnd = strstr(locStart, "\" value=\"");
			if (locEnd == NULL) return;
			strncpySafe(gameinfo->ratingtype, locStart+14, sizeof(gameinfo->ratingtype), locEnd-(locStart+14));
			locStart = locEnd;
			locEnd = strstr(locStart, "\">");
			strncpySafe(gameinfo->ratingvalue, locStart+9, sizeof(gameinfo->ratingvalue), locEnd-(locStart+9));
			int z = 0;
			while (tmp != NULL) {
				if (z >= 15) break;
				tmp = strstr(tmp, "<descriptor>");
				if (tmp == NULL) {
					break;
				} else {
					char * s = tmp+strlen("<descriptor>");
					tmp = strstr(tmp+1, "</descriptor>");
					strncpySafe(gameinfo->ratingdescriptors[z], s, sizeof(gameinfo->ratingdescriptors[z]), tmp-s);
					z++;
				}
			}
			tmp = reset;
			if (tmp != NULL)
			{
				free(tmp);
				tmp = NULL;
			}
		}
		locStart = strstr(locEnd, "<rating type=\"");
	}
}

void readPlayers(struct gameXMLinfo *gameinfo, char * start) {
	char *locStart = strstr(start, "<input players=\"");
	char *locEnd = NULL;
	if (locStart != NULL) {
		locEnd = strstr(locStart, "</input>");
		if (locEnd == NULL) {
			locEnd = strstr(locStart, "\"/>");
			if (locEnd == NULL) return;
			gameinfo->players = intcpy(locStart+16, locEnd-(locStart+16));
		} else {
			gameinfo->players = intcpy(locStart+16, strstr(locStart, "\">")-(locStart+16));
		}
	}
}

void readCaseColor(struct gameXMLinfo *gameinfo, char * start) {
	gameinfo->caseColor = 0xFFFFFF;
	char *locStart = strstr(start, "<case color=\"");
	if (locStart != NULL) {
		char cc[7];
		memset(cc, 0, 7);
		int col, len, num;
		strncpy(cc, locStart+13, 6);
		num = sscanf(cc,"%6x%n", &col, &len);
		if (num == 1 && len == 6)
			gameinfo->caseColor = (u32)col;
	}
}

void readWifi(struct gameXMLinfo *gameinfo, char * start) {
	char *locStart = strstr(start, "<wi-fi players=\"");
	char *locEnd = NULL;
	if (locStart != NULL) {
		locEnd = strstr(locStart, "</wi-fi>");
		if (locEnd == NULL) {
			locEnd = strstr(locStart, "\"/>");
			if (locEnd == NULL) return;
			gameinfo->wifiplayers = intcpy(locStart+16, locEnd-(locStart+16));
		} else {
			gameinfo->wifiplayers = intcpy(locStart+16, strstr(locStart, "\">")-(locStart+16));
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
					strncpySafe(gameinfo->wififeatures[z], s, sizeof(gameinfo->wififeatures[z]), tmp-s);
					z++;
				}
			}
			tmp = reset;
			if (tmp != NULL)
			{
				free(tmp);
				tmp = NULL;
			}
		}
	}
}

void readTitles(struct gameXMLinfo *gameinfo, char * start) {
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
		strncpy(tmpLang, locStart+14, 3);
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
				strncpySafe(gameinfo->title, titStart+7,
						sizeof(gameinfo->title), titEnd-(titStart+7));
				unescape(gameinfo->title, sizeof(gameinfo->title));
				title = found;
			}
		}
		if (synopsis < found) {
			titStart = strstr(locTmp, "<synopsis>");
			if (titStart != NULL) {
				titEnd = strstr(titStart, "</synopsis>");
				strncpySafe(gameinfo->synopsis, titStart+10,
						sizeof(gameinfo->synopsis), titEnd-(titStart+10));
				unescape(gameinfo->synopsis,sizeof(gameinfo->synopsis));
				synopsis = found;
			}
		}
		if (locTmp != NULL)
		{
			free(locTmp);
			locTmp = NULL;
		}
		if (synopsis == 3) break;
	}
}

int OpenDbFiles(const char *xmlfilepath, bool writing)
{
	if (db != NULL)
	{
		fclose(db);
		db = NULL;
	}
	if (idx != NULL)
	{
		fclose(idx);
		idx = NULL;
	}

	char pathname[200];
	sprintf(pathname, "%s/wiitdb.db", xmlfilepath);
	db = fopen(pathname, writing ? "wb" : "rb");

	sprintf(pathname, "%s/wiitdb.idx", xmlfilepath);
	idx = fopen(pathname, writing ? "wb" : "rb");

	return (db != NULL && idx != NULL) ? 1 : 0;
}

void LoadTitlesFromXML(const char *xmlfilepath, char *xmlData, char *langtxt, int forcejptoen)
{
	int n = 0;
	char * start;
	char * end;
	char * tmp;
	char * coverColors = NULL;
	int nrCoverColors = 0;
	int forcelang = 0;
	if (strcmp(langtxt,""))
		forcelang = 1;
	if (forcelang)
		strcpy(xmlCfgLang, (strlen(langtxt) == 2) ? VerifyLangCode(langtxt) : ConvertLangTextToCode(langtxt)); // convert language text into ISO 639 two-letter language code
	if (forcejptoen && (!strcmp(xmlCfgLang,"JA")))
		strcpy(xmlCfgLang,"EN");

	// First check if the database file is up to date...
	OpenDbFiles(xmlfilepath, true);

	// First find the amount of games
	start = strstr(xmlData, "<WiiTDB");
	if (start == NULL) {
		goto abort;
	}
	end = strstr(start, "/>");
	if (end == NULL) { 
		goto abort;
	}
	tmp = strndup(start, end-start);
	if (tmp == NULL)
		goto abort;
	
	char *slash = strstr(tmp, "/");
	if (slash != NULL)
	{
		slash = strndup(start, slash-tmp);
		free(tmp);
		tmp = slash;
	}
	
	char games[7] = "";
	readNode(tmp, games, "\" games=\"","\"");
	free(tmp);
	
	amount_of_games = atoi(games);
	progress_step = 1.f / amount_of_games;

	char titlespath[200];
	sprintf(titlespath, "%s/titles.new", xmlfilepath);
	FILE *titles = fopen(titlespath, "w");
	fwrite("[TITLES]\n", 9, 1, titles);

	bool abort = false;
	while (!abort) {
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
		memset(&gameinfo, 0, sizeof(struct gameXMLinfo));
		readNode(tmp, gameinfo.id, "<id>", "</id>");//ok
		if (gameinfo.id[0] == '\0') { //WTF? ERROR
			printf(" ID NULL\n");
			if(tmp != NULL)
			{
				free(tmp);
				tmp = NULL;
			}
			break;
		}

		readTitles(&gameinfo, tmp);
		readRatings(&gameinfo, tmp);
		readDate(&gameinfo, tmp);
		readWifi(&gameinfo, tmp);
		
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
				int required = (*(p+12) == 't' || *(p+12) == 'T');
				if (!required) {
					if (z < 15) strncpySafe(gameinfo.accessories[z], s, sizeof(gameinfo.accessories[z]), p-s);
					z++;
				} else {
					if (y < 15) strncpySafe(gameinfo.accessoriesReq[y], s, sizeof(gameinfo.accessoriesReq[y]), p-s);
					y++;
				}
			}
		}
		
		readNode(tmp, gameinfo.region, "<region>", "</region>");
		readNode(tmp, gameinfo.developer, "<developer>", "</developer>");
		readNode(tmp, gameinfo.publisher, "<publisher>", "</publisher>");
		readNode(tmp, gameinfo.genre, "<genre>", "</genre>");
		readCaseColor(&gameinfo, tmp);
		readPlayers(&gameinfo, tmp);
		//ConvertRating(gameinfo.ratingvalue, gameinfo.ratingtype, "ESRB");
		if (tmp != NULL)
		{
			free(tmp);
			tmp = NULL;
		}

		// Write game id to idx file
		fwrite(gameinfo.id, 6, 1, idx);
		fwrite(&gameinfo, sizeof(struct gameXMLinfo), 1, db);
		
		fwrite(gameinfo.id, strlen(gameinfo.id), 1, titles);
		fwrite("=", 1, 1, titles);
		fwrite(gameinfo.title, strlen(gameinfo.title), 1, titles);
		fwrite("\n", 1, 1, titles);

		if (gameinfo.caseColor != 0xFFFFFF)
		{
			nrCoverColors++;
			
			int new_size = nrCoverColors * 18;
			
			char *cc = realloc(coverColors, new_size);
			if (cc != NULL)
			{
				char *ptr = cc + ((nrCoverColors - 1) * 18);
				memset(ptr, 0, 18);
				sprintf(ptr, "%s=0x%.6lxff\n", gameinfo.id, gameinfo.caseColor);
				coverColors = cc;
			}
		}

		n++;
		if (callback != NULL)
		{
			abort = !callback(userdata, n * progress_step);
		}
	}

	fflush(idx);
	fflush(db);
	
	if (coverColors != NULL)
	{
		gprintf("Writing coverColors to titles.new");
	
		fwrite("\n[COVERS]\n", 10, 1, titles);
		fwrite(coverColors, strlen(coverColors), 1, titles);
		
		ghexdump(coverColors, strlen(coverColors));
		
		free(coverColors);
		coverColors = NULL;
	}

	fclose(titles);
	titles = NULL;
	
	char pathname[200];
	if (abort && n != amount_of_games)
	{
abort:
		gprintf("Aborting wiitdb\n");

		// Delete intermediate files, if the files are not complete
		fclose(idx);
		fclose(db);
		
		idx = NULL;
		db = NULL;

		char pathname[200];
		sprintf(pathname, "%s/wiitdb.db", xmlfilepath);
		remove(pathname);
		sprintf(pathname, "%s/wiitdb.idx", xmlfilepath);
		remove(pathname);
		
		remove(titlespath);
	}
	else
	{
		sprintf(pathname, "%s/titles.ini", xmlfilepath);
		// Move titles.new to titles.ini
		remove(pathname);
		rename(titlespath, pathname);
	}
	
	OpenDbFiles(xmlfilepath, false);
}

/* load renamed titles from proper names and game info XML, needs to be after cfg_load_games */
bool OpenXMLDatabase(const char* xmlfilepath, char* argdblang, bool argJPtoEN)
{
	if (db != NULL && game_idx != NULL) {
		return 1;
	} 
	else if (db == NULL || idx == NULL) {
		CloseXMLDatabase();
		OpenDbFiles(xmlfilepath, false);
	}

	if (db == NULL || idx == NULL)
	{
		return 0;
	}
	else
	{
		fseek(idx, 0, SEEK_END);
		amount_of_games = ftell(idx) / 6;
		fseek(idx, 0, SEEK_SET);
	}

	game_idx = malloc(amount_of_games * 6);
	memset(game_idx, 0, amount_of_games * 6);
	fread(game_idx, 6, amount_of_games, idx);
	fclose(idx);
	idx = NULL;

	return 1;
}

bool ReloadXMLDatabase(const char* xmlfilepath, char* argdblang, bool argJPtoEN)
{
	if (db != NULL && game_idx != NULL) {
		CloseXMLDatabase();
	}
	return OpenXMLDatabase(xmlfilepath, argdblang, argJPtoEN);
}

bool LoadGameInfoFromXML(char * gameid)
/* gameid: full game id */
{
	memset(&gameinfo, 0, sizeof(struct gameXMLinfo));

	int i;
	char *ptr = (char *) game_idx;
	for (i = 0; i < amount_of_games; i++)
	{
		if (strncmp(ptr, gameid, strlen(gameid)) == 0)
		{
			// Index = i
			fseek(db, i * sizeof(struct gameXMLinfo), SEEK_SET);
			fread(&gameinfo, sizeof(struct gameXMLinfo), 1, db);
			return 1;
		}
		ptr += 6;
	}
	return 0;
}

u8 rebuild_database(const char *xmlfilepath, char *argdblang, bool argJPtoEN, bool (*f)(void *, float), void *ud)
{
	callback = f;
	userdata = ud;

	char pathname[200];
	sprintf(pathname, "%s/wiitdb.zip", xmlfilepath);
	char * xmlData = OpenXMLFile(pathname);
	if(xmlData == NULL)
	{
		sprintf(pathname, "%s/wiitdb.xml", xmlfilepath);
		xmlData = OpenXMLFile(pathname);
		if (xmlData == NULL) {
			CloseXMLDatabase();
			return 1;
		}
	}
	LoadTitlesFromXML(xmlfilepath, xmlData, argdblang, argJPtoEN);
	free(xmlData);
	xmlData = NULL;

	OpenXMLDatabase(xmlfilepath, argdblang, argJPtoEN);
	
	callback = NULL;
	userdata = NULL;
	
	return 0;
}

u8 wiitdb_requires_update(const char *xmlfilepath)
{
	char pathname[200];
	struct stat orig;
	memset(&orig, 0, sizeof(struct stat));
	sprintf(pathname, "%s/wiitdb.zip", xmlfilepath);
	if (stat(pathname, &orig) == -1)
	{
		gprintf("Can't find %s\n", pathname);
		// try wiitdb.xml
		sprintf(pathname, "%s/wiitdb.xml", xmlfilepath);
		if (stat(pathname, &orig) == -1)
		{
			gprintf("Can't find %s\n", pathname);
			gprintf("Deleting database files\n");

			// Delete .db and .idx file
			sprintf(pathname, "%s/wiitdb.db", xmlfilepath);
			remove(pathname);
			sprintf(pathname, "%s/wiitdb.idx", xmlfilepath);
			remove(pathname);
			
			return 0; // Cannot stat original files, no update required
		}
	}
	gprintf("Found either .zip or .xml file, date is %i\n", orig.st_mtime);

	struct stat db_files;
	memset(&db_files, 0, sizeof(struct stat));
	
	sprintf(pathname, "%s/wiitdb.db", xmlfilepath);
	if (stat(pathname, &db_files) == -1 || orig.st_mtime > db_files.st_mtime)
	{
		gprintf("db file not found found or older, recreating databases: %i\n", db_files.st_mtime);
		return 1; // XML or ZIP file is newer, force creation of new files
	}
	gprintf("db file not older: %i\n", db_files.st_mtime);
	
	sprintf(pathname, "%s/wiitdb.idx", xmlfilepath);
	if (stat(pathname, &db_files) == -1 || orig.st_mtime > db_files.st_mtime)
	{
		gprintf("idx file not found or older, recreating databases: %i\n", db_files.st_mtime);
		return 1; // XML or ZIP file is newer, force creation of new files
	}
	gprintf("idx file not older: %i\n", db_files.st_mtime);
	
	return 0;
}
