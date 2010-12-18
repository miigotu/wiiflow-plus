
#include "menu.hpp"
#include "loader/patchcode.h"

#include "loader/sys.h"
#include "loader/wdvd.h"
#include "loader/mload_modules.h"
#include "loader/alt_ios.h"
#include "loader/playlog.h"
#include <ogc/machine/processor.h>
#include <unistd.h>
#include <time.h>
#include "network/http.h"
#include "network/gcard.h"
#include "DeviceHandler.hpp"
#include "loader/mload_modules.h"
#include "loader/wbfs.h"
#include "loader/wbfs_ext.h"
#include "loader/usbstorage.h"
#include "libwbfs/wiidisc.h"
#include "loader/frag.h"
#include "loader/fst.h"
#include "loader/wdm.h"
#include "loader/wip.h"
#include "list.hpp"

#include "gui/WiiMovie.hpp"
#include "channels.h"

#include "gecko.h"
#include "homebrew.h"
#include "sys.h"
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

safe_vector<CMenu::SIOS> CMenu::_installed_cios;

wdm_entry_t *wdm_entry = NULL;
u32 current_wdm = 0;
u8 banner_title[84];
bool wdm_loaded;

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
	m_btnMgr.hide(m_gameLblWdm, instant);
	m_btnMgr.hide(m_gameBtnWdmM, instant);
	m_btnMgr.hide(m_gameBtnWdmP, instant);
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
		
	// Load WDM file
	wdm_loaded = load_wdm(m_wdmDir.c_str(), m_cf.getId().c_str()) == 0;
	if (m_current_view == COVERFLOW_USB && wdm_loaded && wdm_count > 1)
	{
		m_btnMgr.show(m_gameLblWdm);
		m_btnMgr.show(m_gameBtnWdmM);
		m_btnMgr.show(m_gameBtnWdmP);
	
		m_gcfg1.getInt("WDM", m_cf.getId(), (int *) &current_wdm);
		
		if (current_wdm > wdm_count - 1) current_wdm = 0;
		wdm_entry = &wdm_entries[current_wdm];
		
		m_btnMgr.setText(m_gameLblWdm, wstringEx(wdm_entry->name));
	}
	else
	{
		m_btnMgr.hide(m_gameLblWdm, true);
		m_btnMgr.hide(m_gameBtnWdmM, true);
		m_btnMgr.hide(m_gameBtnWdmP, true);
	}

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

extern "C" { bool DatabaseLoaded(void); }

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
			m_gameSound.stop();
			break;
		}
		else if (BTN_PLUS_PRESSED)
		{
			if (DatabaseLoaded())
			{
				_hideGame();
				_gameinfo();
				_showGame();
			}
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
					_waitForGameSoundExtract();
					if (_wbfsOp(CMenu::WO_REMOVE_GAME))
					{
						m_gameSound.stop();
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
				m_gameSound.stop();
				break;
			}
			else if (m_btnMgr.selected(m_gameBtnSettings))
			{
				_hideGame();
				_waitForGameSoundExtract();
				_gameSettings();
				_showGame();
			}
			else if (launch || m_btnMgr.selected(m_gameBtnPlay) || (!WPadIR_Valid(0) && !WPadIR_Valid(1) && !WPadIR_Valid(2) && !WPadIR_Valid(3) && m_btnMgr.selected((u32)-1)))
			{
				_hideGame();
				dir_discHdr *hdr = m_cf.getHdr();

				m_cf.clear();
				_showWaitMessage();

				if (wdm_count > 1) m_gcfg1.setInt("WDM", id, current_wdm);

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
			else if (m_btnMgr.selected(m_gameBtnWdmM))
			{
				current_wdm = (current_wdm == 0) ? wdm_count - 1 : current_wdm - 1;
				wdm_entry = &wdm_entries[current_wdm];
				m_btnMgr.setText(m_gameLblWdm, wstringEx(wdm_entry->name));
			}
			else if (m_btnMgr.selected(m_gameBtnWdmP))
			{
				current_wdm = (current_wdm == wdm_count - 1) ? 0 : current_wdm + 1;
				wdm_entry = &wdm_entries[current_wdm];
				m_btnMgr.setText(m_gameLblWdm, wstringEx(wdm_entry->name));
			}
			else 
			{
				for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
					if (m_cf.mouseOver(m_vid, m_cursor[chan].x(), m_cursor[chan].y()))
						m_cf.flip();
			}
		}
		if (m_gameSoundThread == 0 && (startGameSound == 1 || startGameSound < -8) && (BTN_UP_REPEAT || RIGHT_STICK_UP))
		{
			m_cf.up();
			startGameSound = -10;
		}
		if (m_gameSoundThread == 0 && (startGameSound == 1 || startGameSound < -8) && (BTN_RIGHT_REPEAT || RIGHT_STICK_RIGHT))
		{
			m_cf.right();
			startGameSound = -10;
		}
		if (m_gameSoundThread == 0 && (startGameSound == 1 || startGameSound < -8) && (BTN_DOWN_REPEAT || RIGHT_STICK_DOWN))
		{
			m_cf.down();
			startGameSound = -10;
		}
		if (m_gameSoundThread == 0 && (startGameSound == 1 || startGameSound < -8) && (BTN_LEFT_REPEAT || RIGHT_STICK_LEFT))
		{
			m_cf.left();
			startGameSound = -10;
		}
		if (startGameSound == -10)
		{
			m_gameSound.stop();
			wdm_count = 0;
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
			}

			if (m_current_view == COVERFLOW_USB)
			{
	
				if (!m_locked)
				{
					m_btnMgr.show(m_gameBtnDelete);
					m_btnMgr.show(m_gameBtnSettings);
				}
				if (wdm_count > 1 )//&& m_gameSelected == true)
				{
					m_btnMgr.show(m_gameLblWdm);
					m_btnMgr.show(m_gameBtnWdmM);
					m_btnMgr.show(m_gameBtnWdmP);
				}
				else
				{
					m_btnMgr.hide(m_gameLblWdm, true);
					m_btnMgr.hide(m_gameBtnWdmM, true);
					m_btnMgr.hide(m_gameBtnWdmP, true);
				}
			}
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
			m_btnMgr.hide(m_gameLblWdm);
			m_btnMgr.hide(m_gameBtnWdmM);
			m_btnMgr.hide(m_gameBtnWdmP);					
			for (u32 i = 0; i < ARRAY_SIZE(m_gameLblUser); ++i)
				if (m_gameLblUser[i] != -1u)
					m_btnMgr.hide(m_gameLblUser[i]);
		}
	}
	m_gcfg1.save();
	m_gcfg1.unload();
	
	free_wdm();
	_waitForGameSoundExtract();
	_hideGame();
}

static u32 stringcompare(const string &s1, const string &s2)
{
	u32 index = 0;
	while (true)
	{
		if (s1[index] == 0 || s2[index] == 0)
				return 0;
		if (s1[index] != '?' && s2[index] != '?' && toupper((u8)s1[index]) != toupper((u8)s2[index]))
				return -1;
		index++;
	}
}

static SmartBuf extractDOL(const char *dolName, u32 &size, dir_discHdr *hdr, s32 source, bool dvd)
{
	void *dol = NULL;
	if (!dvd && source == ALT_DOL_DISC)
	{
		wbfs_disc_t *disc = WBFS_OpenDisc((u8 *)hdr->hdr.id, (char *) hdr->path);
		size = wbfs_extract_file(disc, (char *) dolName, &dol);
		WBFS_CloseDisc(disc);
	}
	return SmartBuf((u8 *) dol);
}

static void addDolToList(void *o, const char *fileName)
{
	safe_vector<string> &v = *(safe_vector<string> *)o;
	v.push_back(fileName);
}

static bool findDOL(const char *dolNameToMatch, string &altdol, dir_discHdr *hdr)
{
	safe_vector<string> dols;
	dols.push_back("main.dol");

	wbfs_disc_t *disc = WBFS_OpenDisc((u8 *)hdr->hdr.id, (char *)hdr->path);
	wiidisc_t *wdisc = wd_open_disc((int (*)(void *, u32, u32, void *))wbfs_disc_read, disc);
	wd_list_dols(wdisc, ALL_PARTITIONS, addDolToList, (void *)&dols);
	wd_close_disc(wdisc);
	WBFS_CloseDisc(disc);
	
	// Now loop through the dols, and find the correct name
	for (safe_vector<string>::iterator itr = dols.begin(); itr != dols.end(); itr++)
	{
		if (stringcompare(dolNameToMatch, (*itr)) == 0)
		{
			altdol = (*itr);
			return true;
		}
	}
	return false;
}

void CMenu::_directlaunch(const string &id)
{
	m_directLaunch = true;

	for (int i = USB1; i < USB8; i++)
	{
		if(!DeviceHandler::Instance()->IsInserted(i)) continue;

		DeviceHandler::Instance()->Open_WBFS(i);

		safe_vector<string> pathlist;
		CList::Instance()->GetPaths(pathlist, id.c_str(), sfmt(GAMES_DIR, DeviceName[i]));
		m_gameList.clear();
		CList::Instance()->GetHeaders(pathlist, m_gameList);
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

void CMenu::_launchHomebrew(const char *filepath, safe_vector<std::string> arguments)
{
	COVER_clear();
	if(LoadHomebrew(filepath))
	{
		m_gcfg1.save();
		m_gcfg2.save();
		m_cat.save();
		m_cfg.save();

		AddBootArgument(filepath);
		for(u32 i = 0; i < arguments.size(); ++i)
			AddBootArgument(arguments[i].c_str());

		Close_Inputs();
		Playlog_Delete();

		DeviceHandler::Instance()->UnMountAll();
		cleanup();
		Close_Inputs();
		USBStorage_Deinit();
		bootHB = true;
	}
	m_exit = true;
}

void CMenu::_launchChannel(dir_discHdr *hdr)
{
	string id = string((const char *) hdr->hdr.id);

	m_cfg.setString("GENERAL", "current_channel", id);
	m_gcfg2.setInt("PLAYCOUNT", id, m_gcfg2.getInt("PLAYCOUNT", id, 0) + 1);
	m_gcfg2.setUInt("LASTPLAYED", id, time(NULL));
	
	if (has_enabled_providers() && _initNetwork() == 0)
		add_game_to_card(id.c_str());
	
	m_gcfg1.save();
	m_gcfg2.save();
	m_cat.save();
	m_cfg.save();

	COVER_clear();
	
	DeviceHandler::Instance()->UnMountAll();
	cleanup();
	Close_Inputs();
	USBStorage_Deinit();

	WII_LaunchTitle(hdr->hdr.chantitle);
}

void CMenu::_launchGame(dir_discHdr *hdr, bool dvd)
{
	string id = string((const char *) hdr->hdr.id);

	bool gc = false;
	if (dvd)
	{
		u32 cover = 0;

		struct discHdr *header;
		header = (struct discHdr *)memalign(32, sizeof(struct discHdr));

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
		Disc_ReadHeader(header);
		for (int i = 0;i < 6; i++)
			id[i] = header->id[i];
	}
	bool vipatch = m_gcfg2.testOptBool(id, "vipatch", m_cfg.getBool("GENERAL", "vipatch", false));
	bool cheat = m_gcfg2.testOptBool(id, "cheat", m_cfg.getBool("GENERAL", "cheat", false));
	bool countryPatch = m_gcfg2.testOptBool(id, "country_patch", m_cfg.getBool("GENERAL", "country_patch", false));
	bool err002Fix = m_gcfg2.testOptBool(id, "error_002_fix", m_cfg.getBool("GENERAL", "error_002_fix", true));
	bool patchDiscCheck = !m_gcfg2.testOptBool(id, "disable_dvd_patch", false);
	u8 videoMode = (u8)min((u32)m_gcfg2.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_videoModes) - 1u);
	string altdol = m_gcfg2.getString(id, "dol");
	int language = min((u32)m_gcfg2.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
	const char *rtrn = m_gcfg2.getBool(id, "returnto", true) ? m_cfg.getString("GENERAL", "returnto").c_str() : NULL;
	
	int iosIdx = 0;
	if (m_gcfg2.getInt(id, "ios", &iosIdx) && (u32) iosIdx < ARRAY_SIZE(CMenu::_ios))
		m_gcfg2.setInt(id, "ios", CMenu::_ios[iosIdx]);
	
	int iosNum = mainIOS;
	if (m_gcfg2.getInt(id, "ios", &iosNum) && find(_installed_cios.begin(), _installed_cios.end(), iosNum) == _installed_cios.end())
		iosNum = mainIOS;
	
	u8 patchVidMode = min((u32)m_gcfg2.getInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
	hooktype = (u32) m_gcfg2.getInt(id, "hooktype", 1); // hooktype is defined in patchcode.h
	debuggerselect = m_gcfg2.getBool(id, "debugger", false) ? 1 : 0; // debuggerselect is defined in fst.h
	m_locDol = ALT_DOL_DISC;

	/* enable_ffs:
	bit 0   -> 0 SD 1-> USB
	bit 1-2 -> 0-> /nand, 1-> /nand2, 2-> /nand3, 3-> /nand4
	bit 3   -> led on in save operations
	bit 4-5 -> verbose level: 0-> disabled, 1-> logs from FAT operations, 2 -> logs FFS operations
	bit 6   -> 0: diary to NAND 1: diary to device (used to block the diary for now)
	bit 7   -> FFS emulation enabled/disabled

	bit 8-9 -> Emulation mode: 0->default 1-> DLC redirected to device:/nand, 2-> Full Mode  3-> Full Mode with DLC redirected to device:/nand
	*/
/*
	// NAND emulation settings
	int nand_device = 1;
	int nand_dir = 0;
	int led_on_save = 1;
	int verbose = 3;
	int playlog = m_cfg.getBool("GENERAL", "write_playlog", true) ? 0 : 1;
	int ffs_emulation = 0;
	int emulation_mode = 3;
	
	m_gcfg2.getInt(id, "nand_emulation", &ffs_emulation);
	m_gcfg2.getInt(id, "nand_emulation_device", &nand_device);
	m_gcfg2.getInt(id, "nand_dir", &nand_dir);

	ffs_emulation = 1;
	nand_device = 1;
	nand_dir = 0;

	global_mount |= 2; // USB Device (2 for SD)
	global_mount |= 128; // Enable emulation

	int nand_mode = (nand_device & 1) | 
					(nand_dir << 1) | 
					(led_on_save << 3) | 
					(verbose << 4) | 
					(playlog << 6) | 
					(ffs_emulation << 7) |
					(emulation_mode << 8);
*/
	if (id == "RPWE41" || id == "RPWZ41" || id == "SPXP41") // Prince of Persia, Rival Swords
	{
		//cheat = false;
		//hooktype = 0;
		debuggerselect = false;
		patchDiscCheck = false;
		rtrn = NULL;
	}

	int rtrnID = 0;
	if (!m_directLaunch)
		if (rtrn != NULL && strlen(rtrn) == 4)
			rtrnID = rtrn[0] << 24 | rtrn[1] << 16 | rtrn[2] << 8 | rtrn[3];


	SmartBuf cheatFile, gameconfig, dolFile;
	u32 cheatSize = 0, gameconfigSize = 0, dolSize = 0;
	bool iosLoaded = false;

	_waitForGameSoundExtract();
	if (videoMode == 0)	videoMode = (u8)min((u32)m_cfg.getInt("GENERAL", "video_mode", 0), ARRAY_SIZE(CMenu::_videoModes) - 1);
	if (language == 0)	language = min((u32)m_cfg.getInt("GENERAL", "game_language", 0), ARRAY_SIZE(CMenu::_languages) - 1);
	m_cfg.setString("GENERAL", "current_game", id);
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg2.getInt("PLAYCOUNT", id, 0) + 1);
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));
	
	if (has_enabled_providers() && _initNetwork() == 0)
		add_game_to_card(id.c_str());


	if (load_wdm(m_wdmDir.c_str(), id.c_str()) == 0)
	{
		wdm_entry_t *wdm_entry = NULL;
		s32 current_wdm = -1;
		m_gcfg1.getInt("WDM", id, (int *) &current_wdm);
		if (wdm_count == 1)
			wdm_entry = &wdm_entries[wdm_count-1];
		else
			wdm_entry = current_wdm == -1 || current_wdm > (s32) wdm_count - 1 ? NULL : &wdm_entries[current_wdm];

		if (wdm_entry != NULL)
		{
			gprintf("WDM Entry found, searching for dol with name '%s'\n", wdm_entry->dolname);
			findDOL(wdm_entry->dolname, altdol, hdr);
		}

		free_wdm();
	}
	if (strcasecmp(altdol.c_str(), "main.dol") == 0)
		altdol.clear();
	
	if (!altdol.empty()) dolFile = extractDOL(altdol.c_str(), dolSize, hdr, m_locDol, dvd);
	
	m_gcfg1.save();
	m_gcfg2.save();
	m_cat.save();
	m_cfg.save();
	setLanguage(language);

	// Do every disc related action before reloading IOS
	if (!dvd && get_frag_list((u8 *) hdr->hdr.id, (char *) hdr->path) < 0)
		return;

	#ifdef DBG_FRAG
	extern FragList *frag_list;
	frag_dump(frag_list);
	#endif /* DBG_FRAG */

	if (cheat) _loadFile(cheatFile, cheatSize, m_cheatDir.c_str(), fmt("%s.gct", hdr->hdr.id));

	_loadFile(gameconfig, gameconfigSize, m_txtCheatDir.c_str(), "gameconfig.txt");
	
	load_bca_code((u8 *) m_bcaDir.c_str(), (u8 *) &hdr->hdr.id);
	load_wip_patches((u8 *) m_wipDir.c_str(), (u8 *) &hdr->hdr.id);
	app_gameconfig_load((u8 *) &hdr->hdr.id, gameconfig.get(), gameconfigSize);
	ocarina_load_code((u8 *) &hdr->hdr.id, cheatFile.get(), cheatSize);

	net_wc24cleanup();
	
	// Reload IOS, if requested
	if ((iosNum != mainIOS))
	{
		gprintf("Reloading IOS into %d\n", iosNum);
		if (!loadIOS(iosNum, true))
		{
			error(sfmt("Couldn't load IOS %i", iosNum));
			return;
		}
		iosLoaded = true;
	}
	bool mload = is_ios_type(IOS_TYPE_HERMES);
	int minIOSRev = 0;
	switch (IOS_GetVersion())
	{
		case 222: minIOSRev = IOS_222_MIN_REV; break;
		case 223: minIOSRev = IOS_223_MIN_REV; break;
		case 224: minIOSRev = IOS_224_MIN_REV; break;
		case 249: minIOSRev = IOS_249_MIN_REV; break;
		case 250: minIOSRev = IOS_250_MIN_REV; break;
		default:  minIOSRev = IOS_ODD_MIN_REV; break;
	}

	COVER_clear();
	if (IOS_GetRevision() < minIOSRev && minIOSRev != 0)
	{
		error(sfmt("IOS %i rev %i or higher is required.\nPlease install the latest version.", iosNum, minIOSRev));
		Sys_LoadMenu();
	}
	else if (minIOSRev == 0)
	{
		error(sfmt("IOS %i is not supported.\nPlease install a supported version.", iosNum));
		Sys_LoadMenu();
	}

	bool blockIOSReload = m_gcfg2.getBool((const char *) hdr->hdr.id, "block_ios_reload", false);
	if (mload && blockIOSReload) disableIOSReload();
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
	DeviceHandler::Instance()->UnMountAll();
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
		// hide cios devices
 		/* if (shadow_mload())
			disable_ffs_patch(); */
		/*if (ffs_emulation)
			 if (load_fatffs_module((u8 *)id.c_str()) != -1)
				enable_ffs(nand_mode); */
		
		gprintf("Booting game\n");
		char *altDolDir = (char *) m_altDolDir.c_str();
		if (Disc_WiiBoot(dvd, videoMode, cheatFile.get(), cheatSize, vipatch, countryPatch, err002Fix, dolFile.get(), dolSize, patchVidMode, rtrnID, patchDiscCheck, altDolDir, wdm_entry == NULL ? 1 : wdm_entry->parameter) < 0)
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
	m_gameLblWdm = _addLabel(theme, "GAME/WDM_NAME", theme.btnFont, L"", 68, 410, 200, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);;
	m_gameBtnWdmM = _addPicButton(theme, "GAME/WDM_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 410, 48, 48);
	m_gameBtnWdmP = _addPicButton(theme, "GAME/WDM_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 268, 410, 48, 48);

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
	_setHideAnim(m_gameLblWdm, "GAME/WDM_NAME", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnWdmM, "GAME/WDM_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnWdmP, "GAME/WDM_PLUS", 0, 0, 1.f, -1.f);
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

struct LZ77Info
{
	u16 length : 4;
	u16 offset : 12;
} __attribute__((packed));

inline u32 le32(u32 i)
{
	return ((i & 0xFF) << 24) | ((i & 0xFF00) << 8) | ((i & 0xFF0000) >> 8) | ((i & 0xFF000000) >> 24);
}

SmartBuf uncompressLZ77(u32 &size, const u8 *inputBuf, u32 inputLength)
{
	SmartBuf buffer;
	if (inputLength <= 0x8 || *(const u32 *)inputBuf != 'LZ77' || inputBuf[4] != 0x10)
		return buffer;
	u32 uncSize = le32(((const u32 *)inputBuf)[1] << 8);
	const u8 *inBuf = inputBuf + 8;
	const u8 *inBufEnd = inputBuf + inputLength;

	buffer = smartMem2Alloc(uncSize);
	if (!buffer) return buffer;

	u8 *bufCur = buffer.get();
	u8 *bufEnd = buffer.get() + uncSize;
	while (bufCur < bufEnd && inBuf < inBufEnd)
	{
		u8 flags = *inBuf;
		++inBuf;
		for (int i = 0; i < 8 && bufCur < bufEnd && inBuf < inBufEnd; ++i)
		{
			if ((flags & 0x80) != 0)
			{
				const LZ77Info &info = *(const LZ77Info *)inBuf;
				inBuf += sizeof (LZ77Info);
				int length = info.length + 3;
				if (bufCur - info.offset - 1 < buffer.get() || bufCur + length > bufEnd)
					return buffer;
				memcpy(bufCur, bufCur - info.offset - 1, length);
				bufCur += length;
			}
			else
			{
				*bufCur = *inBuf;
				++inBuf;
				++bufCur;
			}
			flags <<= 1;
		}
	}
	size = uncSize;
	return buffer;
}

void CMenu::_loadGameSound(dir_discHdr *hdr)
{
	u32 sndSize = 0;

	Banner *banner = m_current_view == COVERFLOW_USB ? _extractBnr(hdr) : m_current_view == COVERFLOW_CHANNEL ? _extractChannelBnr(hdr->hdr.chantitle) : NULL;

	if (banner == NULL || !banner->IsValid())
	{
		gprintf("no valid banner found\n");
		return;
	}
	_extractBannerTitle(banner, GetLanguage(m_loc.getString(m_curLanguage, "wiitdb_code", "EN").c_str()));
	
	const u8 *soundBin = banner->GetFile((char *) "sound.bin", &sndSize);

	if (soundBin == NULL || ((IMD5Header *)soundBin)->fcc != 'IMD5')
		return;
	const u8 *soundChunk = soundBin + sizeof (IMD5Header);
	u32 sndType = *(u32 *)soundChunk;
	u32 soundChunkSize = sndSize - sizeof (IMD5Header);
	SmartBuf uncompressed;
	if (sndType == 'LZ77')
	{
		u32 uncSize;
		uncompressed = uncompressLZ77(uncSize, soundChunk, soundChunkSize);
		if (!uncompressed) return;

		soundChunk = uncompressed.get();
		soundChunkSize = uncSize;
		sndType = *(u32 *)soundChunk;
	}
	switch (sndType)
	{
		case 'RIFF':
			LWP_MutexLock(m_gameSndMutex);
			m_gameSoundTmp.fromWAV(soundChunk, soundChunkSize);
			LWP_MutexUnlock(m_gameSndMutex);
			break;
		case 'BNS ':
			LWP_MutexLock(m_gameSndMutex);
			m_gameSoundTmp.fromBNS(soundChunk, soundChunkSize);
			LWP_MutexUnlock(m_gameSndMutex);
			break;
		case 'FORM':
			LWP_MutexLock(m_gameSndMutex);
			m_gameSoundTmp.fromAIFF(soundChunk, soundChunkSize);
			LWP_MutexUnlock(m_gameSndMutex);
			break;
	}
	delete banner;
}

int CMenu::_loadGameSoundThrd(CMenu *m)
{
	dir_discHdr *hdr = m->m_gameSoundHdr;

	LWP_MutexLock(m->m_gameSndMutex);
	m->m_gameSoundHdr = NULL;
	LWP_MutexUnlock(m->m_gameSndMutex);

	m->_loadGameSound(hdr);

	LWP_MutexLock(m->m_gameSndMutex);
	m->m_gameSoundHdr = NULL;
	LWP_MutexUnlock(m->m_gameSndMutex);

	m->m_gameSoundThread = 0;
	return 0;
}

void CMenu::_playGameSound(void)
{
	if (m_bnrSndVol == 0) return;

	LWP_MutexLock(m_gameSndMutex);
	m_gameSoundHdr = m_cf.getHdr();
	LWP_MutexUnlock(m_gameSndMutex);

	m_cf.stopPicLoader();
	if (m_gameSoundThread == 0)
		LWP_CreateThread(&m_gameSoundThread, (void *(*)(void *))CMenu::_loadGameSoundThrd, (void *)this, 0, 8 * 1024, 40);
}

void CMenu::_waitForGameSoundExtract(void)
{
	for (int i = 0; i < 30 && m_gameSoundThread != 0; ++i)	// 3 s
		usleep(100000);
	if (m_gameSoundThread != 0)
	{
		error(L"Error while reading a game disc");
		Sys_Reboot();
	}
}
