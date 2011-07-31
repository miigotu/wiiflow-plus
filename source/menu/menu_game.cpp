
#include "menu.hpp"
#include "loader/patchcode.h"

#include "loader/sys.h"
#include "loader/wdvd.h"
#include "loader/alt_ios.h"
#include "loader/playlog.h"
#include <ogc/machine/processor.h>
#include <unistd.h>
#include <time.h>
#include "network/http.h"
#include "network/gcard.h"
#include "DeviceHandler.hpp"
#include "loader/wbfs.h"
#include "wip.h"

#include "loader/frag.h"
#include "loader/fst.h"

#include "gui/WiiMovie.hpp"
#include "gui/WiiTDB.hpp"
#include "channels.h"
#include "nand.hpp"
#include "alt_ios.h"
#include "gecko.h"
#include "homebrew.h"
#include "defines.h"

using namespace std;

extern const u8 btngamecfg_png[];
extern const u8 btngamecfgs_png[];
extern const u8 stopkidon_png[];
extern const u8 stopkidons_png[];
extern const u8 stopkidoff_png[];
extern const u8 stopkidoffs_png[];
extern const u8 favoriteson_png[];
extern const u8 favoritesons_png[];
extern const u8 favoritesoff_png[];
extern const u8 favoritesoffs_png[];
extern const u8 delete_png[];
extern const u8 deletes_png[];

extern u32 sector_size;
extern int mainIOS;
static u64 sm_title_id  ATTRIBUTE_ALIGN(32);

const string CMenu::_translations[23] = {
	"Default",
	"Arab",
	"Brazilian",
	"Chinese_S",
	"Chinese_T",
	"Danish",
	"Dutch",
	"English",
	"Finnish",
	"French",
	"Gallego",
	"German",
	"Hungarian",
	"Italian",
	"Japanese",
	"Norwegian",
	"Polish",
	"Portuguese",
	"Russian",
	"Spanish",
	"Swedish",
	"Tagalog",
	"Turkish"
};

const CMenu::SOption CMenu::_languages[11] = {
	{ "lngdef", L"Default" },
	{ "lngjap", L"Japanese" },
	{ "lngeng", L"English" },
	{ "lngger", L"German" },
	{ "lngfre", L"French" },
	{ "lngspa", L"Spanish" },
	{ "lngita", L"Italian" },
	{ "lngdut", L"Dutch" },
	{ "lngsch", L"S. Chinese" },
	{ "lngtch", L"T. Chinese" },
	{ "lngkor", L"Korean" }
};

const CMenu::SOption CMenu::_videoModes[7] = {
	{ "viddef", L"Default" },
	{ "vidp50", L"PAL 50Hz" },
	{ "vidp60", L"PAL 60Hz" },
	{ "vidntsc", L"NTSC" },
	{ "vidpatch", L"Auto Patch" },
	{ "vidsys", L"System" },	
	{ "vidprog", L"Progressive" }
};

const CMenu::SOption CMenu::_vidModePatch[4] = {
	{ "vmpnone", L"None" },
	{ "vmpnormal", L"Normal" },
	{ "vmpmore", L"More" },
	{ "vmpall", L"All" }
};


const CMenu::SOption CMenu::_hooktype[8] = {
	{ "disabled", L"Disabled" },
	{ "hooktype1", L"VBI" },
	{ "hooktype2", L"KPAD read" },
	{ "hooktype3", L"Joypad" },
	{ "hooktype4", L"GXDraw" },
	{ "hooktype5", L"GXFlush" },
	{ "hooktype6", L"OSSleepThread" },
	{ "hooktype7", L"AXNextFrame" },
};
/*
0 No Hook
1 VBI
2 KPAD read
3 Joypad Hook
4 GXDraw Hook
5 GXFlush Hook
6 OSSleepThread Hook
7 AXNextFrame Hook
*/

const int CMenu::_ios[6] = {0, 249, 250, 222, 223, 224};

safe_vector<u32> CMenu::_installed_cios;

u8 banner_title[84];

static inline int loopNum(int i, int s)
{
	return i < 0 ? (s - (-i % s)) % s : i % s;
}

static void _extractBannerTitle(Banner *bnr, int language)
{
	if (bnr != NULL)
	{
		memset(banner_title, 0, 84);
		bnr->GetName(banner_title, language);
	}
}

static Banner *_extractChannelBnr(const u64 chantitle)
{
	return Channels::GetBanner(chantitle);
}

static Banner *_extractBnr(dir_discHdr *hdr)
{
	Banner *banner = NULL;
	wbfs_disc_t *disc = WBFS_OpenDisc((u8 *) &hdr->hdr.id, (char *) hdr->path);
	if (disc != NULL)
	{
		void *bnr = NULL;
		if (wbfs_extract_file(disc, (char *) "opening.bnr", &bnr) > 0)
		{
			banner = new Banner((u8 *) bnr);
		}
		WBFS_CloseDisc(disc);
	}
	return banner;
}

static int GetLanguage(const char *lang)
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


void CMenu::_hideGame(bool instant)
{
	m_gameSelected = false;
	m_fa.unload();
	m_cf.showCover();
	
	m_btnMgr.hide(m_gameBtnPlay, instant);
	m_btnMgr.hide(m_gameBtnDelete, instant);
	m_btnMgr.hide(m_gameBtnSettings, instant);
	m_btnMgr.hide(m_gameBtnBack, instant);
	m_btnMgr.hide(m_gameBtnFavoriteOn, instant);
	m_btnMgr.hide(m_gameBtnFavoriteOff, instant);
	m_btnMgr.hide(m_gameBtnAdultOn, instant);
	m_btnMgr.hide(m_gameBtnAdultOff, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_gameLblUser); ++i)
		if (m_gameLblUser[i] != -1u)
			m_btnMgr.hide(m_gameLblUser[i], instant);
}

void CMenu::_showGame(void)
{
	m_cf.showCover();
	
	if (m_fa.load(m_cfg, m_fanartDir.c_str(), m_cf.getId().c_str()))
	{
		STexture bg, bglq;
		m_fa.getBackground(bg, bglq);
		_setBg(bg, bglq);

		if (m_fa.hideCover())
			m_cf.hideCover();
	}
	else
		_setBg(m_mainBg, m_mainBgLQ);
		
	m_btnMgr.show(m_gameBtnPlay);
	m_btnMgr.show(m_gameBtnBack);
	for (u32 i = 0; i < ARRAY_SIZE(m_gameLblUser); ++i)
		if (m_gameLblUser[i] != -1u)
			m_btnMgr.show(m_gameLblUser[i]);
}

static void setLanguage(int l)
{
	if (l > 0 && l <= 10)
		configbytes[0] = l - 1;
	else
		configbytes[0] = 0xCD;
}

void CMenu::_game(bool launch)
{
	m_gcfg1.load(sfmt("%s/gameconfig1.ini", m_settingsDir.c_str()).c_str());
	if (!launch)
	{
		SetupInput();
		_playGameSound();
		_showGame();
		m_gameSelected = true;
	}
	
	s8 startGameSound = 1;
	while (true)
	{
		if(startGameSound < 1) startGameSound++;

		string id(m_cf.getId());
		string title(m_cf.getTitle());
		u64 chantitle = m_cf.getChanTitle();

		if (startGameSound == -5)
		{
			_playGameSound();
			_showGame();
		}
		_mainLoopCommon(true);

		if (startGameSound == 0)
		{
			m_gameSelected = true;
			startGameSound = 1;
		}

		if (BTN_HOME_PRESSED || BTN_B_PRESSED)
		{
			m_gameSound.Stop();
			break;
		}
		else if (BTN_PLUS_PRESSED)
		{
			_hideGame();
			m_gameSelected = true;
			_gameinfo();
			_showGame();
			if (!m_gameSound.IsPlaying()) startGameSound = -6;
		}
		else if (BTN_MINUS_PRESSED)
		{
			string videoPath = sfmt("%s/%.3s.thp", m_videoDir.c_str(), id.c_str());
		
			FILE *file = fopen(videoPath.c_str(), "rb");
			if (file)
			{
				SAFE_CLOSE(file);
				
				_hideGame();
				WiiMovie movie(videoPath.c_str());
				movie.SetScreenSize(m_cfg.getInt("GENERAL", "tv_width", 640), m_cfg.getInt("GENERAL", "tv_height", 480), m_cfg.getInt("GENERAL", "tv_x", 0), m_cfg.getInt("GENERAL", "tv_y", 0));
				movie.SetVolume(m_cfg.getInt("GENERAL", "sound_volume_bnr", 255));
				//_stopSounds();		
				movie.Play();
				
				m_video_playing = true;
				
				STexture videoBg;
				while (!BTN_B_PRESSED && !BTN_A_PRESSED && !BTN_HOME_PRESSED && movie.GetNextFrame(&videoBg))
				{
					_setBg(videoBg, videoBg);
					m_bgCrossFade = 10;
					_mainLoopCommon(); // Redraw the background every frame
				}
				movie.Stop();
				_showGame();
				m_video_playing = false;
				//m_gameSound.play(m_bnrSndVol);
			}
		}
		else if (BTN_1_PRESSED)
		{
			int cfVersion = 1 + loopNum(m_cfg.getInt("GENERAL", m_current_view == COVERFLOW_USB ? "last_cf_mode" : "last_chan_cf_mode" , 1), m_numCFVersions);
			_loadCFLayout(cfVersion);
			m_cf.applySettings();
			m_cfg.setInt("GENERAL", m_current_view == COVERFLOW_USB ? "last_cf_mode" : "last_chan_cf_mode" , cfVersion);
		}
		else if (BTN_2_PRESSED)
		{
			int cfVersion = 1 + loopNum(m_cfg.getInt("GENERAL", m_current_view == COVERFLOW_USB ? "last_cf_mode" : "last_chan_cf_mode" , 1) - 2, m_numCFVersions);
			_loadCFLayout(cfVersion);
			m_cf.applySettings();
			m_cfg.setInt("GENERAL", m_current_view == COVERFLOW_USB ? "last_cf_mode" : "last_chan_cf_mode" , cfVersion);
		}
		else if (launch || BTN_A_PRESSED)
		{
			if (m_btnMgr.selected(m_mainBtnQuit))
				break;
			else if (m_btnMgr.selected(m_gameBtnDelete))
			{
				if (!m_locked)
				{
					_hideGame();
					CheckGameSoundThread(true);
					if (_wbfsOp(CMenu::WO_REMOVE_GAME))
					{
						m_gameSound.Stop();
						break;
					}
					_showGame();
				}
			}
			else if (m_btnMgr.selected(m_gameBtnFavoriteOn) || m_btnMgr.selected(m_gameBtnFavoriteOff))
				m_gcfg1.setBool("FAVORITES", id, !m_gcfg1.getBool("FAVORITES", id, false));
			else if (m_btnMgr.selected(m_gameBtnAdultOn) || m_btnMgr.selected(m_gameBtnAdultOff))
				m_gcfg1.setBool("ADULTONLY", id, !m_gcfg1.getBool("ADULTONLY", id, false));
			else if (m_btnMgr.selected(m_gameBtnBack))
			{
				m_gameSound.Stop();
				break;
			}
			else if (m_btnMgr.selected(m_gameBtnSettings))
			{
				_hideGame();
				m_gameSelected = true;
				_gameSettings();
				_showGame();
				if (!m_gameSound.IsPlaying()) startGameSound = -6;
			}
			else if (launch || m_btnMgr.selected(m_gameBtnPlay) || (!WPadIR_Valid(0) && !WPadIR_Valid(1) && !WPadIR_Valid(2) && !WPadIR_Valid(3) && m_btnMgr.selected((u32)-1)))
			{
				_hideGame();
				dir_discHdr *hdr = m_cf.getHdr();

				m_cf.clear();
				_showWaitMessage();

				if (m_current_view != COVERFLOW_HOMEBREW && m_cfg.getBool("GENERAL", "write_playlog", true))
				{
					if (banner_title[0] == 0) // No title set?
					{					
						// Get banner_title
						Banner * banner = m_current_view == COVERFLOW_CHANNEL ? _extractChannelBnr(chantitle) : m_current_view == COVERFLOW_USB ? _extractBnr(hdr) : NULL;
						if (banner != NULL)
						{						
							if (banner->IsValid())
								_extractBannerTitle(banner, GetLanguage(m_loc.getString(m_curLanguage, "wiitdb_code", "EN").c_str()));
							delete banner;
						}
						banner = NULL;
					}

					if (Playlog_Update(id.c_str(), banner_title) < 0)
						Playlog_Delete();
				}

				gprintf("Launching game\n");
				_launch(hdr);

				if(m_exit || bootHB) break;

				_hideWaitMessage();
				launch = false;

				for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
					WPAD_SetVRes(chan, m_vid.width() + m_cursor[chan].width(), m_vid.height() + m_cursor[chan].height());

				_showGame();
				_initCF();
				m_cf.select();
			}
			else 
			{
				for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
					if (m_cf.mouseOver(m_vid, m_cursor[chan].x(), m_cursor[chan].y()))
						m_cf.flip();
			}
		}
		if ((startGameSound == 1 || startGameSound < -8) && (BTN_UP_REPEAT || RIGHT_STICK_UP))
		{
			m_cf.up();
			startGameSound = -10;
		}
		if ((startGameSound == 1 || startGameSound < -8) && (BTN_RIGHT_REPEAT || RIGHT_STICK_RIGHT))
		{
			m_cf.right();
			startGameSound = -10;
		}
		if ((startGameSound == 1 || startGameSound < -8) && (BTN_DOWN_REPEAT || RIGHT_STICK_DOWN))
		{
			m_cf.down();
			startGameSound = -10;
		}
		if ((startGameSound == 1 || startGameSound < -8) && (BTN_LEFT_REPEAT || RIGHT_STICK_LEFT))
		{
			m_cf.left();
			startGameSound = -10;
		}
		if (startGameSound == -10)
		{
			m_gameSound.Stop();
			m_gameSelected = false;
			m_fa.unload();
			_setBg(m_mainBg, m_mainBgLQ);			
		}
		if (m_show_zone_game)
		{
			bool b = m_gcfg1.getBool("FAVORITES", id, false);
			m_btnMgr.show(b ? m_gameBtnFavoriteOn : m_gameBtnFavoriteOff);
			m_btnMgr.hide(b ? m_gameBtnFavoriteOff : m_gameBtnFavoriteOn);
			m_btnMgr.show(m_gameBtnPlay);
			m_btnMgr.show(m_gameBtnBack);
			for (u32 i = 0; i < ARRAY_SIZE(m_gameLblUser); ++i)
				if (m_gameLblUser[i] != -1u)
					m_btnMgr.show(m_gameLblUser[i]);

			if (!m_locked)
			{
				b = m_gcfg1.getBool("ADULTONLY", id, false);
				m_btnMgr.show(b ? m_gameBtnAdultOn : m_gameBtnAdultOff);
				m_btnMgr.hide(b ? m_gameBtnAdultOff : m_gameBtnAdultOn);
				m_btnMgr.show(m_gameBtnSettings);
			}

			if (m_current_view == COVERFLOW_USB && !m_locked)
				m_btnMgr.show(m_gameBtnDelete);
		}
		else
		{
			m_btnMgr.hide(m_gameBtnFavoriteOn);
			m_btnMgr.hide(m_gameBtnFavoriteOff);
			m_btnMgr.hide(m_gameBtnAdultOn);
			m_btnMgr.hide(m_gameBtnAdultOff);
			m_btnMgr.hide(m_gameBtnDelete);
			m_btnMgr.hide(m_gameBtnSettings);
			m_btnMgr.hide(m_gameBtnPlay);
			m_btnMgr.hide(m_gameBtnBack);
			for (u32 i = 0; i < ARRAY_SIZE(m_gameLblUser); ++i)
				if (m_gameLblUser[i] != -1u)
					m_btnMgr.hide(m_gameLblUser[i]);
		}
	}
	m_gcfg1.save();
	m_gcfg1.unload();
	
	_hideGame();
}

void CMenu::_directlaunch(const string &id)
{
	m_directLaunch = true;

	for (int i = USB1; i < USB8; i++)
	{
		if(!DeviceHandler::Instance()->IsInserted(i)) continue;

		DeviceHandler::Instance()->Open_WBFS(i);
		CList<dir_discHdr> list;
		string path = sfmt(GAMES_DIR, DeviceName[i]);
		safe_vector<string> pathlist;
		list.GetPaths(pathlist, id.c_str(), path,
			strncasecmp(DeviceHandler::Instance()->PathToFSName(path.c_str()), "WBFS", 4) == 0);

		m_gameList.clear();
		list.GetHeaders(pathlist, m_gameList, m_settingsDir, m_curLanguage);
		if(m_gameList.size() > 0)
		{
			gprintf("Game found on partition #%i\n", i);
			_launch(&m_gameList[0]); // Launch will exit wiiflow
		}
	}
	
	error(sfmt("Cannot find the game with ID: %s", id.c_str()));
}

void CMenu::_launch(dir_discHdr *hdr)
{
	m_gcfg2.load(sfmt("%s/gameconfig2.ini", m_settingsDir.c_str()).c_str());
	switch(m_current_view)
	{
		case COVERFLOW_HOMEBREW:
			_launchHomebrew((char *)hdr->path, m_homebrewArgs);
			break;
		case COVERFLOW_CHANNEL:
			_launchChannel(hdr);
			break;
		case COVERFLOW_USB:
		default:
			_launchGame(hdr, false);
			break;
	}
}

extern "C" {extern void USBStorage_Deinit(void);}

void CMenu::_launchHomebrew(const char *filepath, safe_vector<std::string> arguments)
{
	//MEM1_cleanup();	MEM1_clear();
	if(LoadHomebrew(filepath))
	{
		m_gcfg1.save();
		m_gcfg2.save();
		m_cat.save();
		m_cfg.save();

		AddBootArgument(filepath);
		for(u32 i = 0; i < arguments.size(); ++i)
			AddBootArgument(arguments[i].c_str());

		Playlog_Delete();

		cleanup();
		Close_Inputs();
		USBStorage_Deinit();
		bootHB = true;
	}
	m_exit = true;
}

void CMenu::_launchChannel(dir_discHdr *hdr)
{
	Channels channel;
	u32 *data = channel.Load(hdr->hdr.chantitle, (char *)hdr->hdr.id);
	if(data != NULL)
	{
		string id = string((const char *) hdr->hdr.id);

		bool vipatch = m_gcfg2.testOptBool(id, "vipatch", m_cfg.getBool("GENERAL", "vipatch", false));
		bool cheat = m_gcfg2.testOptBool(id, "cheat", m_cfg.getBool("GENERAL", "cheat", false));
		bool countryPatch = m_gcfg2.testOptBool(id, "country_patch", m_cfg.getBool("GENERAL", "country_patch", false));
		u8 videoMode = (u8)min((u32)m_gcfg2.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_videoModes) - 1u);
		int language = min((u32)m_gcfg2.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
		const char *rtrn = m_gcfg2.getBool(id, "returnto", true) ? m_cfg.getString("GENERAL", "returnto").c_str() : NULL;

		u8 patchVidMode = min((u32)m_gcfg2.getInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
		hooktype = (u32) m_gcfg2.getInt(id, "hooktype", 0);
		debuggerselect = m_gcfg2.getBool(id, "debugger", false) ? 1 : 0;

		if ((debuggerselect || cheat) && hooktype == 0) hooktype = 1;
		if (!debuggerselect && !cheat) hooktype = 0;

		if (videoMode == 0)	videoMode = (u8)min((u32)m_cfg.getInt("GENERAL", "video_mode", 0), ARRAY_SIZE(CMenu::_videoModes) - 1);
		if (language == 0)	language = min((u32)m_cfg.getInt("GENERAL", "game_language", 0), ARRAY_SIZE(CMenu::_languages) - 1);

		m_cfg.setString("GENERAL", "current_channel", id);
		m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1); 
		m_gcfg1.setUInt("LASTPLAYED", id, time(NULL)); 

		if (rtrn != NULL && strlen(rtrn) == 4)
		{			
			int rtrnID = rtrn[0] << 24 | rtrn[1] << 16 | rtrn[2] << 8 | rtrn[3];
			
			static ioctlv vector[1] __attribute__((aligned(0x20)));			
			sm_title_id = (((u64)(0x00010001) << 32) | (rtrnID&0xFFFFFFFF));
			
			vector[0].data = &sm_title_id;
			vector[0].len = 8;
			
			s32 ESHandle = IOS_Open("/dev/es", 0);
			gprintf("Return to channel %s. Using new d2x way\n", IOS_Ioctlv(ESHandle, 0xA1, 1, 0, vector) != -101 ? "Succeeded" : "Failed!" );
			IOS_Close(ESHandle);
		}
			
		if (has_enabled_providers() && _initNetwork() == 0)
			add_game_to_card(id.c_str());
		
		m_gcfg1.save();
		m_gcfg2.save();
		m_cat.save();
		m_cfg.save();

		setLanguage(language);

		SmartBuf cheatFile;
		u32 cheatSize = 0;
		if (cheat) _loadFile(cheatFile, cheatSize, m_cheatDir.c_str(), fmt("%s.gct", hdr->hdr.id));
		ocarina_load_code((u8 *) &hdr->hdr.id, cheatFile.get(), cheatSize);

		CheckGameSoundThread(true);
		//MEM1_cleanup();		MEM1_clear();
		cleanup();
		Close_Inputs();
		USBStorage_Deinit();

		if(!channel.Launch(data, hdr->hdr.chantitle, videoMode, vipatch, countryPatch, patchVidMode))
			Sys_LoadMenu();
	}
}

void CMenu::_launchGame(dir_discHdr *hdr, bool dvd)
{
	string id = string((const char *) hdr->hdr.id);

	bool gc = false;
	if (dvd)
	{
		u32 cover = 0;


		Disc_SetUSB(NULL);
		if (WDVD_GetCoverStatus(&cover) < 0)
		{
			error(L"WDVDGetCoverStatus Failed!");
			if (BTN_B_PRESSED) return;
		}
		if (!(cover & 0x2))
		{
			error(L"Please insert a game disc.");
			do {
				WDVD_GetCoverStatus(&cover);
				if (BTN_B_PRESSED) return;
			} while(!(cover & 0x2));
		}
		/* Open Disc */
		if (Disc_Open() < 0) 
		{
			error(L"Cannot Read DVD.");
			if (BTN_B_PRESSED) return;
		} 
		/* Check disc */
		if (Disc_IsWii() < 0)
		{
			if (Disc_IsGC() < 0) 
			{
				error(L"This is not a Wii or GC disc");
				if (BTN_B_PRESSED) return;
			}
			else
				gc = true;
		}
		/* Read header */
		struct discHdr *header = (struct discHdr *)MEM2_alloc(sizeof(struct discHdr));
		Disc_ReadHeader(header);
		for (int i = 0;i < 6; i++)
			id[i] = header->id[i];
		SAFE_FREE(header);
	}
	bool vipatch = m_gcfg2.testOptBool(id, "vipatch", m_cfg.getBool("GENERAL", "vipatch", false));
	bool cheat = m_gcfg2.testOptBool(id, "cheat", m_cfg.getBool("GENERAL", "cheat", false));
	bool countryPatch = m_gcfg2.testOptBool(id, "country_patch", m_cfg.getBool("GENERAL", "country_patch", false));
	u8 videoMode = (u8)min((u32)m_gcfg2.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_videoModes) - 1u);
	int language = min((u32)m_gcfg2.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
	const char *rtrn = m_gcfg2.getBool(id, "returnto", true) ? m_cfg.getString("GENERAL", "returnto").c_str() : NULL;
	
	int iosIdx = 0;
	if (m_gcfg2.getInt(id, "ios", &iosIdx) && (u32) iosIdx < ARRAY_SIZE(CMenu::_ios))
		m_gcfg2.setInt(id, "ios", CMenu::_ios[iosIdx]);
	
	int gameIOS = mainIOS;
	if (m_gcfg2.getInt(id, "ios", &gameIOS) && find(_installed_cios.begin(), _installed_cios.end(), gameIOS) == _installed_cios.end())
		gameIOS = mainIOS;

	u8 patchVidMode = min((u32)m_gcfg2.getInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
	hooktype = (u32) m_gcfg2.getInt(id, "hooktype", 0); // hooktype is defined in patchcode.h
	debuggerselect = m_gcfg2.getBool(id, "debugger", false) ? 1 : 0; // debuggerselect is defined in fst.h

	if ((debuggerselect || cheat) && hooktype == 0) hooktype = 1;
	if (!debuggerselect && !cheat) hooktype = 0;

	if (id == "RPWE41" || id == "RPWZ41" || id == "SPXP41") // Prince of Persia, Rival Swords
		debuggerselect = false;

	SmartBuf cheatFile, gameconfig;
	u32 cheatSize = 0, gameconfigSize = 0;
	bool iosLoaded = false;

	CheckGameSoundThread(true);
	if (videoMode == 0)	videoMode = (u8)min((u32)m_cfg.getInt("GENERAL", "video_mode", 0), ARRAY_SIZE(CMenu::_videoModes) - 1);
	if (language == 0)	language = min((u32)m_cfg.getInt("GENERAL", "game_language", 0), ARRAY_SIZE(CMenu::_languages) - 1);
	m_cfg.setString("GENERAL", "current_game", id);
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1);
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));
	
	if (has_enabled_providers() && _initNetwork() == 0)
		add_game_to_card(id.c_str());

	m_gcfg1.save();
	m_gcfg2.save();
	m_cat.save();
	m_cfg.save();
	setLanguage(language);

	// Do every disc related action before reloading IOS
	if (!dvd && get_frag_list((u8 *) hdr->hdr.id, (char *) hdr->path, sector_size) < 0)
		return;

	if (cheat) _loadFile(cheatFile, cheatSize, m_cheatDir.c_str(), fmt("%s.gct", hdr->hdr.id));

	_loadFile(gameconfig, gameconfigSize, m_txtCheatDir.c_str(), "gameconfig.txt");

	load_wip_patches((u8 *) m_wipDir.c_str(), (u8 *) &hdr->hdr.id);	
	app_gameconfig_load((u8 *) &hdr->hdr.id, gameconfig.get(), gameconfigSize);
	ocarina_load_code((u8 *) &hdr->hdr.id, cheatFile.get(), cheatSize);

	net_wc24cleanup();
		
	// Reload IOS, if requested
	if ((gameIOS != mainIOS))
	{
		gprintf("Reloading IOS into %d\n", gameIOS);
		cleanup(true);
		if (!loadIOS(gameIOS, true))
		{
			error(sfmt("Couldn't load IOS %i", gameIOS));
			return;
		}
		iosLoaded = true;
	}
	
	//MEM1_cleanup();	MEM1_clear();
	if (IOS_GetRevision() < D2X_MIN_REV)
	{
		error(sfmt("d2x rev %i or higher is required.\nPlease install the latest version.", gameIOS, D2X_MIN_REV));
		Sys_LoadMenu();
	}

	if (!m_directLaunch)
	{
		if (rtrn != NULL && strlen(rtrn) == 4)
		{			
			int rtrnID = rtrn[0] << 24 | rtrn[1] << 16 | rtrn[2] << 8 | rtrn[3];
			
			static ioctlv vector[1] __attribute__((aligned(0x20)));			

			sm_title_id = (((u64)(0x00010001) << 32) | (rtrnID&0xFFFFFFFF));
			
			vector[0].data = &sm_title_id;
			vector[0].len = 8;
			
			s32 ESHandle = IOS_Open("/dev/es", 0);
			gprintf("Return to channel %s. Using new d2x way\n", IOS_Ioctlv(ESHandle, 0xA1, 1, 0, vector) != -101 ? "succeeded" : "failed!");
			IOS_Close(ESHandle);
		}
	}

	if (!dvd)
	{
		s32 ret = Disc_SetUSB((u8 *) hdr->hdr.id);
		if (ret < 0)
		{
			gprintf("Set USB failed: %d\n", ret);
			error(L"Disc_SetUSB failed");
			if (iosLoaded) Sys_LoadMenu();
			return;
		}
		
		
		if (Disc_Open() < 0)
		{
			error(L"Disc_Open failed");
			if (iosLoaded) Sys_LoadMenu();
			return;
		}
	}
	cleanup();
	Close_Inputs();
	USBStorage_Deinit();
	if (gc)
	{
		WII_Initialize();
		if (WII_LaunchTitle(0x0000000100000100ULL)<0)
			Sys_LoadMenu();
	} 
	else 
	{
		gprintf("Booting game\n");
		if (Disc_WiiBoot(videoMode, vipatch, countryPatch, patchVidMode, gameIOS) < 0)
			Sys_LoadMenu();
	}
}

void CMenu::_initGameMenu(CMenu::SThemeData &theme)
{
	CColor fontColor(0xD0BFDFFF);
	STexture texFavOn;
	STexture texFavOnSel;
	STexture texFavOff;
	STexture texFavOffSel;
	STexture texAdultOn;
	STexture texAdultOnSel;
	STexture texAdultOff;
	STexture texAdultOffSel;
	STexture texDelete;
	STexture texDeleteSel;
	STexture texSettings;
	STexture texSettingsSel;
	STexture bgLQ;

	texFavOn.fromPNG(favoriteson_png);
	texFavOnSel.fromPNG(favoritesons_png);
	texFavOff.fromPNG(favoritesoff_png);
	texFavOffSel.fromPNG(favoritesoffs_png);
	texAdultOn.fromPNG(stopkidon_png);
	texAdultOnSel.fromPNG(stopkidons_png);
	texAdultOff.fromPNG(stopkidoff_png);
	texAdultOffSel.fromPNG(stopkidoffs_png);
	texDelete.fromPNG(delete_png);
	texDeleteSel.fromPNG(deletes_png);
	texSettings.fromPNG(btngamecfg_png);
	texSettingsSel.fromPNG(btngamecfgs_png);
	_addUserLabels(theme, m_gameLblUser, ARRAY_SIZE(m_gameLblUser), "GAME");
	m_gameBg = _texture(theme.texSet, "GAME/BG", "texture", theme.bg);
	if (m_theme.loaded() && STexture::TE_OK == bgLQ.fromPNGFile(sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString("GAME/BG", "texture").c_str()).c_str(), GX_TF_CMPR, ALLOC_MEM2, 64, 64))
		m_gameBgLQ = bgLQ;

	m_gameBtnPlay = _addButton(theme, "GAME/PLAY_BTN", theme.btnFont, L"", 420, 354, 200, 56, fontColor);
	m_gameBtnBack = _addButton(theme, "GAME/BACK_BTN", theme.btnFont, L"", 420, 410, 200, 56, fontColor);
	m_gameBtnFavoriteOn = _addPicButton(theme, "GAME/FAVORITE_ON", texFavOn, texFavOnSel, 460, 170, 48, 48);
	m_gameBtnFavoriteOff = _addPicButton(theme, "GAME/FAVORITE_OFF", texFavOff, texFavOffSel, 460, 170, 48, 48);
	m_gameBtnAdultOn = _addPicButton(theme, "GAME/ADULTONLY_ON", texAdultOn, texAdultOnSel, 532, 170, 48, 48);
	m_gameBtnAdultOff = _addPicButton(theme, "GAME/ADULTONLY_OFF", texAdultOff, texAdultOffSel, 532, 170, 48, 48);
	m_gameBtnSettings = _addPicButton(theme, "GAME/SETTINGS_BTN", texSettings, texSettingsSel, 460, 242, 48, 48);
	m_gameBtnDelete = _addPicButton(theme, "GAME/DELETE_BTN", texDelete, texDeleteSel, 532, 242, 48, 48);

	m_gameButtonsZone.x = m_theme.getInt("GAME/ZONES", "buttons_x", 0);
	m_gameButtonsZone.y = m_theme.getInt("GAME/ZONES", "buttons_y", 0);
	m_gameButtonsZone.w = m_theme.getInt("GAME/ZONES", "buttons_w", 640);
	m_gameButtonsZone.h = m_theme.getInt("GAME/ZONES", "buttons_h", 480);
	m_gameButtonsZone.hide = m_theme.getBool("GAME/ZONES", "buttons_hide", true);

	// 
	_setHideAnim(m_gameBtnPlay, "GAME/PLAY_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameBtnBack, "GAME/BACK_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameBtnFavoriteOn, "GAME/FAVORITE_ON", 0, 0, -1.5f, -1.5f);
	_setHideAnim(m_gameBtnFavoriteOff, "GAME/FAVORITE_OFF", 0, 0, -1.5f, -1.5f);
	_setHideAnim(m_gameBtnAdultOn, "GAME/ADULTONLY_ON", 0, 0, -1.5f, -1.5f);
	_setHideAnim(m_gameBtnAdultOff, "GAME/ADULTONLY_OFF", 0, 0, -1.5f, -1.5f);
	_setHideAnim(m_gameBtnSettings, "GAME/SETTINGS_BTN", 0, 0, -1.5f, -1.5f);
	_setHideAnim(m_gameBtnDelete, "GAME/DELETE_BTN", 0, 0, -1.5f, -1.5f);
	_hideGame(true);
	_textGame();
}

void CMenu::_textGame(void)
{
	m_btnMgr.setText(m_gameBtnPlay, _t("gm1", L"Play"));
	m_btnMgr.setText(m_gameBtnBack, _t("gm2", L"Back"));
}

//

struct IMD5Header
{
	u32 fcc;
	u32 filesize;
	u8 zeroes[8];
	u8 crypto[16];
} __attribute__((packed));

inline u32 le32(u32 i)
{
	return ((i & 0xFF) << 24) | ((i & 0xFF00) << 8) | ((i & 0xFF0000) >> 8) | ((i & 0xFF000000) >> 24);
}

SmartBuf gameSoundThreadStack;

void CMenu::_gameSoundThread(CMenu *m)
{
	m->m_gamesound_changed = false;
	u32 sndSize = 0;
	m->m_gameSoundHdr = m->m_cf.getHdr();

	Banner *banner = m->m_current_view == COVERFLOW_USB ?
		_extractBnr(m->m_gameSoundHdr) : m->m_current_view == COVERFLOW_CHANNEL ?
		_extractChannelBnr(m->m_gameSoundHdr->hdr.chantitle) : NULL;

	if (banner == NULL || !banner->IsValid())
	{
		gprintf("no valid banner found\n");
		SAFE_DELETE(banner);
		m->m_gameSoundHdr = NULL;
		return;
	}
	_extractBannerTitle(banner, GetLanguage(m->m_loc.getString(m->m_curLanguage, "wiitdb_code", "EN").c_str()));
	
	const u8 *soundBin = banner->GetFile((char *) "sound.bin", &sndSize);

	if (soundBin == NULL || (((IMD5Header *)soundBin)->fcc != 'IMD5' && ((IMD5Header *)soundBin)->fcc != 'RIFF'))
	{
		gprintf("Failed to load banner sound!\n\n");
		SAFE_DELETE(banner);
		m->m_gameSoundHdr = NULL;
		return;
	}

	m->m_gameSound.Load(soundBin, sndSize, false);
	SAFE_DELETE(banner);
	m->m_gamesound_changed = true;
	m->m_gameSoundHdr = NULL;
}

void CMenu::_playGameSound(void)
{
	m_gamesound_changed = false;
	if (m_bnrSndVol == 0 || m_gameSoundHdr != NULL || m_gameSoundThread != LWP_THREAD_NULL) return;

	m_cf.stopCoverLoader();

	unsigned int stack_size = (unsigned int)32768;
	SMART_FREE(gameSoundThreadStack);
	gameSoundThreadStack = smartMem2Alloc(stack_size);
	LWP_CreateThread(&m_gameSoundThread, (void *(*)(void *))CMenu::_gameSoundThread, (void *)this, gameSoundThreadStack.get(), stack_size, 40);
}

void CMenu::CheckGameSoundThread(bool force)
{
	if (force || (m_gameSoundHdr == NULL && m_gameSoundThread != LWP_THREAD_NULL))
	{
		if(LWP_ThreadIsSuspended(m_gameSoundThread))
			LWP_ResumeThread(m_gameSoundThread);

		LWP_JoinThread(m_gameSoundThread, NULL);

		SMART_FREE(gameSoundThreadStack);
		m_gameSoundThread = LWP_THREAD_NULL;
	}
}

void CMenu::CheckThreads(bool force)
{
	CheckGameSoundThread(force);
	m_vid.CheckWaitThread(force);
}
