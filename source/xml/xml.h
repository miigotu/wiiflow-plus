#ifndef _XML_H_
#define _XML_H_

#include <gctypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct gameXMLinfo
{	
	char id[7];
	char region[7];
	char title[80];
	char synopsis[3062];
	char developer[75];
	char publisher[75];
	int year;
	int month;
	int day;
	char genre[75];
	char ratingtype[5];
	char ratingvalue[5];
	char ratingdescriptors[16][40];
	int wifiplayers;
	char wififeatures[8][15];
	u8 players;
	char accessories[16][20];
	char accessoriesReq[16][20];
	long caseColor;
};

struct gameXMLinfo *game_info;
char * getLang(int lang);
bool ReloadXMLDatabase(const char* xmlfilepath, char* argdblang, bool argJPtoEN);
void CloseXMLDatabase();
bool LoadGameInfoFromXML(char * gameid);
char *ConvertLangTextToCode(char *langtext);
char *VerifyLangCode(char *langtext);
char ConvertRatingToIndex(char *ratingtext);
void ConvertRating(char *ratingvalue, char *fromrating, char *torating, char *destvalue, int destsize);
int getIndexFromId(char * gameid);
int getCountPlayers(char * gameid);

bool DatabaseLoaded(void);
int getControllerTypes(char *controller, char * gameid);
bool hasGenre(char *genre, char * gameid);
bool hasFeature(char *feature, char *gameid);
bool xml_getCaseColor(u32 *color, char *gameid);

#ifdef __cplusplus
}
#endif

#endif
