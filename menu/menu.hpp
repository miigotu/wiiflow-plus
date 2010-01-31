
#ifndef __MENU_HPP
#define __MENU_HPP

#include "cursor.hpp"
#include "gui.hpp"
#include "coverflow.hpp"
#include "loader/disc.h"

#include <wiiuse/wpad.h>
#include <vector>
#include <map>

#include "gct.h"

class CMenu
{
public:
	CMenu(CVideo &vid);
	~CMenu(void) { cleanup(); }
	void init(bool fromHBC);
	void error(const wstringEx &msg);
	int main(void);
	void cleanup(void);
private:
	struct SZone
	{
		int x;
		int y;
		int w;
		int h;
	};
	CVideo &m_vid;
	CCursor m_cur;
	CButtonsMgr m_btnMgr;
	CCoverFlow m_cf;
	std::vector<discHdr> m_gameList;
	Config m_cfg;
	Config m_loc;
	Config m_theme;
	SmartBuf m_music;
	int m_aa;
	bool m_noHBC;
	std::string m_curLanguage;
	bool m_locked;
	bool m_favorites;
	int m_letter;
	std::string m_curGameId;
	STexture m_waitMessage;
	u32 m_numCFVersions;
	bool m_alphaSearch;
	// 
	std::string m_themeDataDir;
	std::string m_dataDir;
	std::string m_picDir;
	std::string m_boxPicDir;
	std::string m_cacheDir;
	std::string m_themeDir;
	std::string m_musicDir;
	// 
	STexture m_prevBg;
	STexture m_nextBg;
	STexture m_curBg;
	STexture m_lqBg;
	u8 m_bgCrossFade;
	// 
	STexture m_errorBg;
	STexture m_mainBg;
	STexture m_configBg;
	STexture m_config2Bg;
	STexture m_config3Bg;
	STexture m_config4Bg;
	STexture m_configAdvBg;
	STexture m_configSndBg;
	STexture m_downloadBg;
	STexture m_gameBg;
	STexture m_codeBg;
	STexture m_aboutBg;
	STexture m_wbfsBg;
	STexture m_gameSettingsBg;
	STexture m_gameBgLQ;
	STexture m_mainBgLQ;
	// 
	u32 m_errorLblMessage;
	u32 m_errorLblIcon;
	u32 m_errorLblUser[4];
	u32 m_mainBtnConfig;
	u32 m_mainBtnInfo;
	u32 m_mainBtnFavoritesOn;
	u32 m_mainBtnFavoritesOff;
	u32 m_mainLblLetter;
	u32 m_mainBtnNext;
	u32 m_mainBtnPrev;
	u32 m_mainBtnQuit;
	u32 m_mainBtnInit;
	u32 m_mainLblInit;
	u32 m_mainLblUser[4];
	u32 m_configLblPage;
	u32 m_configBtnPageM;
	u32 m_configBtnPageP;
	u32 m_configBtnBack;
	u32 m_configLblTitle;
	u32 m_configLblBoxMode;
	u32 m_configBtnBoxMode;
	u32 m_configLblRumble;
	u32 m_configBtnRumble;
	u32 m_configLblDownload;
	u32 m_configBtnDownload; 
	u32 m_configLblParental;
	u32 m_configBtnUnlock;
	u32 m_configBtnSetCode;
	u32 m_configLblUser[4];
	u32 m_configAdvLblTheme;
	u32 m_configAdvLblCurTheme;
	u32 m_configAdvBtnCurThemeM;
	u32 m_configAdvBtnCurThemeP;
	u32 m_configAdvLblLanguage;
	u32 m_configAdvLblCurLanguage;
	u32 m_configAdvBtnCurLanguageM;
	u32 m_configAdvBtnCurLanguageP;
	u32 m_configAdvLblCFTheme;
	u32 m_configAdvBtnCFTheme;
	u32 m_configAdvLblInstall;
	u32 m_configAdvBtnInstall;
	u32 m_configAdvLblUser[4];
	u32 m_config2LblGameLanguage;
	u32 m_config2LblLanguage;
	u32 m_config2BtnLanguageP;
	u32 m_config2BtnLanguageM;
	u32 m_config2LblGameVideo;
	u32 m_config2LblVideo;
	u32 m_config2BtnVideoP;
	u32 m_config2BtnVideoM;
	u32 m_config2LblErr2Fix;
	u32 m_config2BtnErr2Fix;
	u32 m_config2LblOcarina;
	u32 m_config2BtnOcarina;
	u32 m_config2LblUser[4];
	u32 m_config3LblTVHeight;
	u32 m_config3LblTVHeightVal;
	u32 m_config3BtnTVHeightP;
	u32 m_config3BtnTVHeightM;
	u32 m_config3LblTVWidth;
	u32 m_config3LblTVWidthVal;
	u32 m_config3BtnTVWidthP;
	u32 m_config3BtnTVWidthM;
	u32 m_config3LblTVX;
	u32 m_config3LblTVXVal;
	u32 m_config3BtnTVXM;
	u32 m_config3BtnTVXP;
	u32 m_config3LblTVY;
	u32 m_config3LblTVYVal;
	u32 m_config3BtnTVYM;
	u32 m_config3BtnTVYP;
	u32 m_config3LblUser[4];
	u32 m_config4LblHome;
	u32 m_config4BtnHome;
	u32 m_config4LblSaveFavMode;
	u32 m_config4BtnSaveFavMode;
	u32 m_config4LblSearchMode;
	u32 m_config4BtnSearchMode;
	u32 m_config4LblUser[4];
	u32 m_configSndLblBnrVol;
	u32 m_configSndLblBnrVolVal;
	u32 m_configSndBtnBnrVolP;
	u32 m_configSndBtnBnrVolM;
	u32 m_configSndLblMusicVol;
	u32 m_configSndLblMusicVolVal;
	u32 m_configSndBtnMusicVolP;
	u32 m_configSndBtnMusicVolM;
	u32 m_configSndLblGuiVol;
	u32 m_configSndLblGuiVolVal;
	u32 m_configSndBtnGuiVolP;
	u32 m_configSndBtnGuiVolM;
	u32 m_configSndLblCFVol;
	u32 m_configSndLblCFVolVal;
	u32 m_configSndBtnCFVolP;
	u32 m_configSndBtnCFVolM;
	u32 m_configSndLblUser[4];
	u32 m_downloadLblTitle;
	u32 m_downloadPBar;
	u32 m_downloadBtnCancel;
	u32 m_downloadBtnAll;
	u32 m_downloadBtnMissing;
	u32 m_downloadBtnAllTitles;
	u32 m_downloadBtnMissingTitles;
	u32 m_downloadLblMessage[2];
	u32 m_downloadLblCovers;
	u32 m_downloadLblGameTitles;
	u32 m_downloadLblWiiTDB;
	u32 m_downloadLblUser[4];
	u32 m_gameLblInfo;
	u32 m_gameBtnFavoriteOn;
	u32 m_gameBtnFavoriteOff;
	u32 m_gameBtnAdultOn;
	u32 m_gameBtnAdultOff;
	u32 m_gameBtnPlay;
	u32 m_gameBtnDelete;
	u32 m_gameBtnSettings;
	u32 m_gameBtnBack;
	u32 m_gameLblUser[4];
	u32 m_codeLblTitle;
	u32 m_codeBtnKey[10];
	u32 m_codeBtnBack;
	u32 m_codeBtnErase;
	u32 m_codeLblUser[4];
	u32 m_aboutLblTitle;
	u32 m_aboutLblOrigAuthor;
	u32 m_aboutLblAuthor;
	u32 m_aboutLblInfo;
	u32 m_aboutLblUser[4];
	u32 m_aboutLblIOS;
	u32 m_wbfsLblTitle;
	u32 m_wbfsPBar;
	u32 m_wbfsBtnBack;
	u32 m_wbfsBtnGo;
	u32 m_wbfsLblDialog;
	u32 m_wbfsLblMessage;
	u32 m_wbfsLblUser[4];
	u32 m_cfThemeBtnAlt;
	u32 m_cfThemeBtnSelect;
	u32 m_cfThemeBtnWide;
	u32 m_cfThemeLblParam;
	u32 m_cfThemeBtnParamM;
	u32 m_cfThemeBtnParamP;
	u32 m_cfThemeBtnCopy;
	u32 m_cfThemeBtnPaste;
	u32 m_cfThemeBtnSave;
	u32 m_cfThemeBtnCancel;
	u32 m_cfThemeLblVal[4 * 4];
	u32 m_cfThemeBtnValM[4 * 4];
	u32 m_cfThemeBtnValP[4 * 4];
	u32 m_cfThemeLblValTxt[4];
	u32 m_gameSettingsLblPage;
	u32 m_gameSettingsBtnPageM;
	u32 m_gameSettingsBtnPageP;
	u32 m_gameSettingsBtnBack;
	u32 m_gameSettingsLblTitle;
	u32 m_gameSettingsLblGameLanguage;
	u32 m_gameSettingsLblLanguage;
	u32 m_gameSettingsBtnLanguageP;
	u32 m_gameSettingsBtnLanguageM;
	u32 m_gameSettingsLblGameVideo;
	u32 m_gameSettingsLblVideo;
	u32 m_gameSettingsBtnVideoP;
	u32 m_gameSettingsBtnVideoM;
	u32 m_gameSettingsLblGameAltDol;
	u32 m_gameSettingsLblAltDol;
	u32 m_gameSettingsBtnAltDolP;
	u32 m_gameSettingsBtnAltDolM;
	u32 m_gameSettingsLblOcarina;
	u32 m_gameSettingsBtnOcarina;
	u32 m_gameSettingsLblVipatch;
	u32 m_gameSettingsBtnVipatch;
	u32 m_gameSettingsLblCountryPatch;
	u32 m_gameSettingsBtnCountryPatch;
	u32 m_gameSettingsLblErr2Fix;
	u32 m_gameSettingsBtnErr2Fix;
	u32 m_gameSettingsLblGameIOS;
	u32 m_gameSettingsLblIOS;
	u32 m_gameSettingsBtnIOSP;
	u32 m_gameSettingsBtnIOSM;
	u32 m_gameSettingsLblCover;
	u32 m_gameSettingsBtnCover;
	u32 m_gameSettingsLblBlockIOSReload;
	u32 m_gameSettingsBtnBlockIOSReload;
	u32 m_gameSettingsLblPatchVidModes;
	u32 m_gameSettingsLblPatchVidModesVal;
	u32 m_gameSettingsBtnPatchVidModesM;
	u32 m_gameSettingsBtnPatchVidModesP;
	u32 m_gameSettingsLblUser[3 * 2];
	
	u32 m_gameSettingsLblCheat;
	u32 m_gameSettingsBtnCheat;
	//Cheat menu
	u32 m_cheatBtnBack;
	u32 m_cheatBtnApply;
	u32 m_cheatBtnDownload;
	u32 m_cheatLblTitle;
	u32 m_cheatLblPage;
	u32 m_cheatBtnPageM;
	u32 m_cheatBtnPageP;
	u32 m_cheatLblItem[6];
	u32 m_cheatBtnItem[6];
	u32 m_cheatSettingsPage;
	STexture m_cheatBg;
	GCTCheats m_cheatfile;
	 
	SZone m_mainPrevZone;
	SZone m_mainNextZone;
	SZone m_mainButtonsZone;
	 
	u32 m_padLeftDelay;
	u32 m_padRightDelay;
	 
	u32 m_gameSettingsPage;
	 
	volatile bool m_networkInit;
	volatile bool m_thrdStop;
	volatile bool m_thrdWorking;
	float m_thrdStep;
	float m_thrdStepLen;
	std::string m_coverDLGameId;
	mutex_t m_mutex;
	wstringEx m_thrdMessage;
	volatile float m_thrdProgress;
	volatile bool m_thrdMessageAdded;
	SSoundEffect m_gameSound;
	SSoundEffect m_gameSoundTmp;
	std::string m_gameSoundId;
	lwp_t m_gameSoundThread;
	mutex_t m_gameSndMutex;
	u8 m_bnrSndVol;
private:
	enum WBFS_OP { WO_ADD_GAME, WO_REMOVE_GAME, WO_FORMAT };
	typedef std::pair<std::string, u32> FontDesc;
	typedef std::map<FontDesc, SFont> FontSet;
	typedef std::map<std::string, STexture> TexSet;
	typedef std::map<std::string, SSoundEffect> SoundSet;
	struct SThemeData
	{
		TexSet texSet;
		FontSet fontSet;
		SoundSet soundSet;
		SFont btnFont;
		SFont lblFont;
		SFont titleFont;
		CColor btnFontColor;
		CColor lblFontColor;
		CColor txtFontColor;
		CColor titleFontColor;
		STexture bg;
		STexture btnTexL;
		STexture btnTexR;
		STexture btnTexC;
		STexture btnTexLS;
		STexture btnTexRS;
		STexture btnTexCS;
		STexture pbarTexL;
		STexture pbarTexR;
		STexture pbarTexC;
		STexture pbarTexLS;
		STexture pbarTexRS;
		STexture pbarTexCS;
		STexture btnTexPlus;
		STexture btnTexPlusS;
		STexture btnTexMinus;
		STexture btnTexMinusS;
		SSoundEffect clickSound;
		SSoundEffect hoverSound;
	};
	struct SCFParamDesc
	{
		enum { PDT_EMPTY, PDT_FLOAT, PDT_V3D, PDT_COLOR, PDT_BOOL, PDT_INT, PDT_TXTSTYLE } paramType[4];
		enum { PDD_BOTH, PDD_NORMAL, PDD_SELECTED } domain;
		bool scrnFmt;
		const char name[32];
		const char valName[4][64];
		const char key[4][48];
		float step[4];
		float minMaxVal[4][2];
	};
	u32 _btnRepeat(u32 btn);
	// 
	bool _loadGameList(void);
	void _initCF(void);
	// 
	void _initMainMenu(SThemeData &theme);
	void _initErrorMenu(SThemeData &theme);
	void _initYesNoMenu(SThemeData &theme);
	void _initConfigMenu(SThemeData &theme);
	void _initConfig2Menu(SThemeData &theme);
	void _initConfig3Menu(SThemeData &theme);
	void _initConfig4Menu(SThemeData &theme);
	void _initConfigAdvMenu(SThemeData &theme);
	void _initConfigSndMenu(SThemeData &theme);
	void _initGameMenu(SThemeData &theme);
	void _initDownloadMenu(SThemeData &theme);
	void _initCodeMenu(SThemeData &theme);
	void _initAboutMenu(SThemeData &theme);
	void _initWBFSMenu(SThemeData &theme);
	void _initCFThemeMenu(SThemeData &theme);
	void _initGameSettingsMenu(SThemeData &theme);
	//Cheat Menu
	void _CheatSettings();
	void _hideCheatSettings(bool instant = false);
	void _showCheatSettings(void);
	void _initCheatSettingsMenu(SThemeData &theme);
	void _textCheatSettings(void);
	
	void _textMain(void);
	void _textError(void);
	void _textYesNo(void);
	void _textConfig(void);
	void _textConfig2(void);
	void _textConfig3(void);
	void _textConfig4(void);
	void _textConfigAdv(void);
	void _textConfigSnd(void);
	void _textGame(void);
	void _textDownload(void);
	void _textCode(void);
	void _textAbout(void);
	void _textWBFS(void);
	void _textGameSettings(void);
	void _hideError(bool instant = false);
	void _hideMain(bool instant = false);
	void _hideConfig(bool instant = false);
	void _hideConfig2(bool instant = false);
	void _hideConfig3(bool instant = false);
	void _hideConfig4(bool instant = false);
	void _hideConfigAdv(bool instant = false);
	void _hideConfigSnd(bool instant = false);
	void _hideGame(bool instant = false);
	void _hideDownload(bool instant = false);
	void _hideCode(bool instant = false);
	void _hideAbout(bool instant = false);
	void _hideWBFS(bool instant = false);
	void _hideCFTheme(bool instant = false);
	void _hideGameSettings(bool instant = false);
	void _showError(void);
	void _showMain(void);
	void _showConfig(void);
	void _showConfig2(void);
	void _showConfig3(void);
	void _showConfig4(void);
	void _showConfigAdv(void);
	void _showConfigSnd(void);
	void _showGame(void);
	void _showDownload(void);
	void _showCode(void);
	void _showAbout(void);
	void _showWBFS(WBFS_OP op);
	void _showCFTheme(u32 curParam, int version, bool wide);
	void _showGameSettings(void);
	void _setBg(const STexture &tex, const STexture &lqTex);
	void _updateBg(void);
	void _drawBg(void);
	void _updateText(void);
	// 
	void _config(int page);
	int _config1(void);
	int _config2(void);
	int _config3(void);
	int _config4(void);
	int _configAdv(void);
	int _configSnd(void);
	void _game(bool launch = false);
	void _download(std::string gameId = std::string());
	bool _code(char code[4], bool erase = false);
	void _about(void);
	bool _wbfsOp(WBFS_OP op);
	void _cfTheme(void);
	void _gameSettings(void);
	void _mainLoopCommon(const WPADData *wd, bool withCF = false, bool blockReboot = false, bool adjusting = false);
	// 
	void _launchGame(const std::string &id);
	bool _networkFix(void);
	void _setAA(int aa);
	void _loadCFCfg(void);
	void _loadCFLayout(int version, bool forceAA = false, bool otherScrnFmt = false);
	Vector3D _getCFV3D(const std::string &domain, const std::string &key, const Vector3D &def, bool otherScrnFmt = false);
	int _getCFInt(const std::string &domain, const std::string &key, int def, bool otherScrnFmt = false);
	float _getCFFloat(const std::string &domain, const std::string &key, float def, bool otherScrnFmt = false);
	void _cfParam(bool inc, int i, const SCFParamDesc &p, int cfVersion, bool wide);
	void _buildMenus(void);
	SFont _font(FontSet &fontSet, const char *domain, const char *key, SFont def);
	STexture _texture(TexSet &texSet, const char *domain, const char *key, STexture def);
	SSoundEffect _sound(CMenu::SoundSet &soundSet, const char *domain, const char *key, SSoundEffect def);
	u16 _textStyle(const char *domain, const char *key, u16 def);
	u32 _addButton(SThemeData &theme, const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color);
	u32 _addPicButton(SThemeData &theme, const char *domain, STexture &texNormal, STexture &texSelected, int x, int y, u32 width, u32 height);
	u32 _addLabel(SThemeData &theme, const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, u16 style);
	u32 _addLabel(SThemeData &theme, const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, u16 style, STexture &bg);
	u32 _addProgressBar(SThemeData &theme, const char *domain, int x, int y, u32 width, u32 height);
	void _setHideAnim(u32 id, const char *domain, int dx, int dy, float scaleX, float scaleY);
	void _addUserLabels(CMenu::SThemeData &theme, u32 *ids, u32 size, const char *domain);
	// 
	const wstringEx _t(const char *key, const wchar_t *def = L"") { return m_loc.getWString(m_curLanguage, key, def); }
	const wstringEx _fmt(const char *key, const wchar_t *def);
	// 
	void _setThrdMsg(const wstringEx &msg, float progress);
	int _coverDownloader(bool missingOnly);
	static int _coverDownloaderAll(CMenu *m);
	static int _coverDownloaderMissing(CMenu *m);
	static bool _downloadProgress(void *obj, int size, int position);
	int _titleDownloader(bool missingOnly);
	int _updateDownloader(void);
	static int _titleDownloaderAll(CMenu *m);
	static int _titleDownloaderMissing(CMenu *m);
	static int _update(CMenu *m);
	int _initNetwork(char *ip);
	static void _addDiscProgress(int status, int total, void *user_data);
	static int _gameInstaller(void *obj);
	wstringEx _optBoolToString(int b);
	void _listDOL(std::vector<std::string> &v, const std::string &gameId);
	void _startMusic(void);
	void _stopMusic(void);
	void _pauseMusic(void);
	void _resumeMusic(void);
	void _stopSounds(void);
	// 
	void _playGameSound(void);
	void _loadGameSound(const std::string &id);
	void _waitForGameSoundExtract(void);
	static int _loadGameSoundThrd(CMenu *m);
	// 
	struct SOption { const char id[10]; const wchar_t text[16]; };
	static const SOption _languages[11];
	static const SOption _videoModes[7];
	static const SOption _vidModePatch[4];
	static const int _ios[5];
	static const SCFParamDesc _cfParams[];
	static const int _nbCfgPages;
};

#define ARRAY_SIZE(a)		(sizeof a / sizeof a[0])

#endif // !defined(__MENU_HPP)
