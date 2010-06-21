/*
 * gct.h
 * Class to handle Ocarina TXT Cheatfiles
 * 
 */

#ifndef _GCT_H
#define _GCT_H

#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <cstdio>
#include <string.h>
#include <algorithm>
#include <cctype>
//#include <mxml.h>

using namespace std;

//!Handles Ocarina TXT Cheatfiles
class GCTCheats 
{
private:
    string sGameID;
    string sGameTitle;
    vector<string> sCheatName;
    vector<string> sCheats;
    vector<string> sCheatComment;
    unsigned int iCntCheats;
	
public:
	//!Array Shows which cheat is selected 
	vector<bool> bCheatSelected;
    //!Constructor
    GCTCheats(void);
    //!Destructor
    ~GCTCheats(void);
    //!Open TXT Cheatfile
    //!\param TXT filename
    //!\return error code
    int OpenTXTFile(const char * filename);
	//!Update TXT file
    //!\param TXT filename
    //!\return error code
    int UpdateTXT(const char * filename);
	//!Open XML Cheat file 
    //!\param XML filename
	//!\param GameID
    //!\return error code
	//int OpenXMLFile(const char * filename,const char *gameid);
	//!Update XML Cheat file
    //!\param XML filename
	//!\param GameID
    //!\return error code
	//int UpdateXMLFile(const char * filename,const char *gameid);
    //!Creates GCT file
    //!\param filename name of GCT file
    //!\return error code
    int CreateGCT(const char * filename);
	//!Creates directly gct in memory
    //!\param filename name of TXT file
    //!\return GCT buffer
    string createGCTbuff(int nr[],int cnt);
    //!Gets Count cheats
    //!\return Count cheats
    unsigned int GetCount(void);
    //!Gets Game Name
    //!\return Game Name
    string GetGamename(void);
    //!Gets GameID
    //!\return GameID
    string GetGameID(void);
    //!Gets cheat data
    //!\return cheat data
    string GetCheat(unsigned int nr);
    //!Gets Cheat Name
    //!\return Cheat Name
    string GetCheatname(unsigned int nr);
    //!Gets Cheat Comment
    //!\return Cheat Comment
    string GetCheatcomment(unsigned int nr);
	//!Check if string is a code
    //!\return true/false
	bool IsCode(const std::string& s);
	//!Check if string is a code
    //!\return 0=ok, 1="X", 2=no code
	int IsCodeEx(const std::string& s);
};

#endif  /* _GCT_H */