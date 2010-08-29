
#include "menu.hpp"
#include "loader/wdvd.h"
#include "network/gcard.h"

#include <unistd.h>
#include <fstream>

#include "wbfs.h"
#include "gecko.h"
#include "sys.h"

using namespace std;

extern const u8 btnconfig_png[];
extern const u8 btnconfigs_png[];
extern const u8 btninfo_png[];
extern const u8 btninfos_png[];
extern const u8 btnquit_png[];
extern const u8 btnquits_png[];
extern const u8 btnnext_png[];
extern const u8 btnnexts_png[];
extern const u8 btnprev_png[];
extern const u8 btnprevs_png[];
extern const u8 btnchannel_png[];
extern const u8 btnchannels_png[];
extern const u8 btnusb_png[];
extern const u8 btnusbs_png[];
extern const u8 btndvd_png[];
extern const u8 btndvds_png[];
extern const u8 gradient_png[];
extern const u8 favoriteson_png[];
extern const u8 favoritesons_png[];
extern const u8 favoritesoff_png[];
extern const u8 favoritesoffs_png[];

static inline int loopNum(int i, int s)
{
	return i < 0 ? (s - (-i % s)) % s : i % s;
}

void CMenu::_hideMain(bool instant)
{
	m_btnMgr.hide(m_mainBtnNext, instant);
	m_btnMgr.hide(m_mainBtnPrev, instant);
	m_btnMgr.hide(m_mainBtnConfig, instant);
	m_btnMgr.hide(m_mainBtnInfo, instant);
	m_btnMgr.hide(m_mainBtnQuit, instant);
	m_btnMgr.hide(m_mainBtnChannel, instant);
	m_btnMgr.hide(m_mainBtnUsb, instant);
	m_btnMgr.hide(m_mainBtnDVD, instant);
	m_btnMgr.hide(m_mainBtnInit, instant);
	m_btnMgr.hide(m_mainBtnInit2, instant);
	m_btnMgr.hide(m_mainLblInit, instant);
	m_btnMgr.hide(m_mainBtnFavoritesOn, instant);
	m_btnMgr.hide(m_mainBtnFavoritesOff, instant);
	m_btnMgr.hide(m_mainLblLetter, instant);
	m_btnMgr.hide(m_mainLblNotice, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_mainLblUser); ++i)
		if (m_mainLblUser[i] != -1u)
			m_btnMgr.hide(m_mainLblUser[i], instant);
}

void CMenu::_showMain(void)
{
	_setBg(m_gameBg, m_gameBgLQ);
	m_btnMgr.show(m_mainBtnConfig);
	m_btnMgr.show(m_mainBtnInfo);
	m_btnMgr.show(m_mainBtnQuit);

	if (/*m_channels.CanIdentify() && */m_loaded_ios_base !=57 || m_gameList.empty())
	{
		if (m_current_view == COVERFLOW_USB)
			m_btnMgr.show(m_mainBtnChannel);
		else if (m_current_view == COVERFLOW_CHANNEL)
			m_btnMgr.show(m_mainBtnUsb);
	}

	for (u32 i = 1; i < ARRAY_SIZE(m_mainLblUser); ++i)
		if (m_mainLblUser[i] != -1u)
			m_btnMgr.show(m_mainLblUser[i]);
	if (m_gameList.empty())
	{
		m_btnMgr.show(m_mainBtnInit);
		m_btnMgr.show(m_mainBtnInit2);
		m_btnMgr.show(m_mainLblInit);
	}
}

int CMenu::main(void)
{
	wstringEx curLetter;
	string prevTheme = m_cfg.getString("GENERAL", "theme", "default");
	m_reload = false;
	static u32 disc_check = 0, olddisc_check = 0;
	int done = 0;

	// Start network asynchronious, if configured and required
	if (m_cfg.getBool("GENERAL", "async_network", true) || has_enabled_providers())
	{
		_initAsyncNetwork();
	}

	SetupInput();
	_loadList();
	_searchMusic();
	_startMusic();
	_updateWiiTDB();
	_showMain();
	m_curGameId.clear();
	_initCF();

	WDVD_GetCoverStatus(&disc_check);
	
	while (true)
	{
		_mainLoopCommon(true);

		//check if Disc was inserted
		olddisc_check = disc_check;
		WDVD_GetCoverStatus(&disc_check);

		//Check for exit or reload request
		if (BTN_HOME_PRESSED)
		{
			m_reload = BTN_B_HELD;
			break;
		}
		++repeatButton;
		if ((wii_btnsHeld & WBTN_A) == 0)
			buttonHeld = (u32)-1;
		else if (buttonHeld != (u32)-1 && buttonHeld == m_btnMgr.selected() && repeatButton >= 16)
			wii_btnsPressed |= WBTN_A;
		//Normal coverflow movement
		for(int wmote=0;wmote<4;wmote++)
		{
			if ((BTN_UP_REPEAT || RIGHT_STICK_UP) && (wii_btnsHeld & WBTN_B) == 0)
				m_cf.up();
			if ((BTN_RIGHT_REPEAT || RIGHT_STICK_RIGHT) && (wii_btnsHeld & WBTN_B) == 0)
				m_cf.right();
			if ((BTN_DOWN_REPEAT ||  RIGHT_STICK_DOWN) && (wii_btnsHeld & WBTN_B) == 0)
				m_cf.down();
			if ((BTN_LEFT_REPEAT || RIGHT_STICK_LEFT) && (wii_btnsHeld & WBTN_B) == 0)
				m_cf.left();
		}
		//CF Layout select
		if (BTN_1_PRESSED && (wii_btnsHeld & WBTN_B) == 0)
		{
			int cfVersion = 1 + loopNum(m_cfg.getInt("GENERAL", "last_cf_mode", 1), m_numCFVersions);
			_loadCFLayout(cfVersion);
			m_cf.applySettings();
			m_cfg.setInt("GENERAL", "last_cf_mode", cfVersion);
		}
		else if (BTN_2_PRESSED && (wii_btnsHeld & WBTN_B) == 0)
		{
			int cfVersion = 1 + loopNum(m_cfg.getInt("GENERAL", "last_cf_mode", 1) - 2, m_numCFVersions);
			_loadCFLayout(cfVersion);
			m_cf.applySettings();
			m_cfg.setInt("GENERAL", "last_cf_mode", cfVersion);
		}
		if (BTN_B_HELD)
		{
			//Search by Alphabet
			if (BTN_DOWN_PRESSED)
			{
				if (m_cfg.getInt("GENERAL", "sort", SORT_ALPHA) != SORT_ALPHA && m_titles_loaded)
				{
					m_cf.setSorting((Sorting)SORT_ALPHA);
					m_cfg.setInt("GENERAL", "sort", SORT_ALPHA);
				}
				curLetter.resize(1);
				curLetter[0] = m_cf.nextLetter();
				m_showtimer = 60;
				m_btnMgr.setText(m_mainLblLetter, curLetter);
				m_btnMgr.show(m_mainLblLetter);
			}
			else if (BTN_UP_PRESSED)
			{
				if (m_cfg.getInt("GENERAL", "sort", SORT_ALPHA) != SORT_ALPHA && m_titles_loaded)
				{
					m_cf.setSorting((Sorting)SORT_ALPHA);
					m_cfg.setInt("GENERAL", "sort", SORT_ALPHA);
				}
				curLetter.resize(1);
				curLetter[0] = m_cf.prevLetter();
				m_showtimer = 60;
				m_btnMgr.setText(m_mainLblLetter, curLetter);
				m_btnMgr.show(m_mainLblLetter);
			}
			//Search by pages
			else if (BTN_LEFT_PRESSED)
				m_cf.pageUp();
			else if (BTN_RIGHT_PRESSED)
				m_cf.pageDown();
			//Sorting Selection
			else if (BTN_PLUS_PRESSED && !m_locked && m_titles_loaded)
			{
				u32 sort = 0;
				sort = m_cfg.getInt("GENERAL", "sort", 0);
				++sort;
				if (sort >= 4) sort = 0;
				m_cf.setSorting((Sorting)sort);
				m_cfg.setInt("GENERAL", "sort", sort);
				wstringEx curSort ;
				if (sort == SORT_ALPHA)
					curSort = m_loc.getWString(m_curLanguage, "alphabetically", L"Alphabetically");
				else if (sort == SORT_PLAYCOUNT)
					curSort = m_loc.getWString(m_curLanguage, "byplaycount", L"By Play Count");
				else if (sort == SORT_LASTPLAYED)
					curSort = m_loc.getWString(m_curLanguage, "bylastplayed", L"By Last Played");
				else if (sort == SORT_GAMEID)
					curSort = m_loc.getWString(m_curLanguage, "bygameid", L"By Game I.D.");
				m_showtimer=60; 
				m_btnMgr.setText(m_mainLblNotice, curSort);
				m_btnMgr.show(m_mainLblNotice);
			}
			//Partition Selection
			else if (BTN_MINUS_PRESSED && !m_locked)
			{
				_hideMain();
				s32 amountOfPartitions = WBFS_GetPartitionCount();
				s32 currentPartition = WBFS_GetCurrentPartition();
				char buf[5];
				currentPartition = loopNum(currentPartition + 1, amountOfPartitions);
				gprintf("Next item: %d\n", currentPartition);
				WBFS_GetPartitionName(currentPartition, (char *) &buf);
				gprintf("Which is: %s\n", buf);
				m_cfg.setString("GENERAL", "partition", buf);
				m_showtimer=60; 
				m_btnMgr.setText(m_mainLblNotice, (string)buf);
				m_btnMgr.show(m_mainLblNotice);
				_loadList();
				_showMain();
				_initCF();
			}
		}
		if (BTN_B_PRESSED)
		{
			if (buttonHeld != m_btnMgr.selected())
				m_btnMgr.click();
			//Events to Show Categories
			if (m_btnMgr.selected() == m_mainBtnFavoritesOn || m_btnMgr.selected() == m_mainBtnFavoritesOff)
			{
				if (m_current_view == COVERFLOW_USB) // Only supported in game mode (not for channels, since you don't have options for channels yet)
				{
					// Event handler to show categories for selection
					_hideMain();
					_CategorySettings();
					_showMain();
					m_curGameId = m_cf.getId();
					_initCF();
				}
			}
			/*//Events to Switch off/on nand emu
			if (m_btnMgr.selected() == m_mainBtnChannel || m_btnMgr.selected() == m_mainBtnUsb)
			{
				//switch to nand emu here.
			}
			*/
		}
		else if (done==0 && m_current_view == COVERFLOW_USB && m_cat.getBool("GENERAL", "category_on_start", false)) // Only supported in game mode (not for channels, since you don't have options for channels yet)
		{
			done = 1; //set done so it doesnt keep doing it
			// show categories menu
			_hideMain();
			_CategorySettings();
			_showMain();
			m_curGameId = m_cf.getId();
			_initCF();
		}
		//Handling input when other gui buttons are selected
		else if (BTN_A_PRESSED)
		{
			if (buttonHeld != m_btnMgr.selected())
				m_btnMgr.click();
			if (m_btnMgr.selected() == m_mainBtnQuit) {
				bool hbc = *((u32 *) 0x80001804) == 0x53545542 && *((u32 *) 0x80001808) == 0x48415858;
				if (!hbc)
					Sys_ExitToWiiMenu(true);
				break;
			}
			else if (m_btnMgr.selected() == m_mainBtnChannel || m_btnMgr.selected() == m_mainBtnUsb)
			{
				if (m_btnMgr.selected() == m_mainBtnChannel) 
				{
					m_current_view = COVERFLOW_CHANNEL;
					m_category = 0;
				}
				else if (m_btnMgr.selected() == m_mainBtnUsb)
				{
					m_current_view = COVERFLOW_USB;
					m_category = m_cat.getInt("GENERAL", "category", 0);
				}
				_hideMain();
				//m_cfg.setInt("GENERAL", "currentview", m_current_view);
				_loadList();
				_initCF();
				_showMain();
			}
			else if (m_btnMgr.selected() == m_mainBtnInit)
			{
				if (!WBFS_IsReadOnly())
				{
					_hideMain();
					_wbfsOp(CMenu::WO_ADD_GAME);
					if (prevTheme != m_cfg.getString("GENERAL", "theme"))
					{
						m_reload = true;
						break;
					}
					_showMain();
				}
				else
				{
					error(_t("wbfsop11", L"The currently selected filesystem is read-only. You cannot install games or remove them."));
					if (prevTheme != m_cfg.getString("GENERAL", "theme"))
					{
						m_reload = true;
						break;
					}
					_showMain();
				}
			}
			else if (m_btnMgr.selected() == m_mainBtnInit2)
			{
				_hideMain();
				//_config(2);
				_config(7);
				if (prevTheme != m_cfg.getString("GENERAL", "theme"))
				{
					m_reload = true;
					break;
				}
				_showMain();
			}
			else if (m_btnMgr.selected() == m_mainBtnConfig)
			{
				_hideMain();
				_config(1);
				if (prevTheme != m_cfg.getString("GENERAL", "theme") || m_reload == true)
				{
					m_reload = true;
					break;
				}
				_showMain();
			}
			else if (m_btnMgr.selected() == m_mainBtnInfo)
			{
				_hideMain();
				_about();
				_showMain();
			}
			else if (m_btnMgr.selected() == m_mainBtnDVD)
			{
				_hideMain();
				string dvddvd = "dvddvd";
				m_vid.waitMessage(m_waitMessage);
				_launchGame(dvddvd, true);
				_showMain();
			}
			else if (m_btnMgr.selected() == m_mainBtnNext)
			{
				m_cf.right();
				if (buttonHeld != m_btnMgr.selected())
				{
					repeatButton = 0;
					buttonHeld = m_btnMgr.selected();
				}
			}
			else if (m_btnMgr.selected() == m_mainBtnPrev)
			{
				m_cf.left();
				if (buttonHeld != m_btnMgr.selected())
				{
					repeatButton = 0;
					buttonHeld = m_btnMgr.selected();
				}
			}
			else if (m_btnMgr.selected() == m_mainBtnFavoritesOn || m_btnMgr.selected() == m_mainBtnFavoritesOff)
			{
				m_favorites = !m_favorites;
				m_cfg.setInt("GENERAL", "favorites", m_favorites);
				m_curGameId = m_cf.getId();
				_initCF();
			}
			else if (!m_cf.empty())
			{
				if (m_cf.select())
				{
					_hideMain();
					_game(BTN_B_HELD);
					m_cf.cancel();
					_showMain();
				}
			}
		}
		if (m_showtimer > 0)
			if (--m_showtimer == 0){
				m_btnMgr.hide(m_mainLblLetter);
				m_btnMgr.hide(m_mainLblNotice);
			}
		//zones, showing and hiding buttons
		if (!m_gameList.empty() && m_show_zone_prev)
			m_btnMgr.show(m_mainBtnPrev);
		else
			m_btnMgr.hide(m_mainBtnPrev);
		if (!m_gameList.empty() && m_show_zone_next)
			m_btnMgr.show(m_mainBtnNext);
		else
			m_btnMgr.hide(m_mainBtnNext);
		if (!m_gameList.empty() && m_show_zone_main)
		{
			m_btnMgr.show(m_mainLblUser[0]);
			m_btnMgr.show(m_mainLblUser[1]);
			m_btnMgr.show(m_mainBtnConfig);
			m_btnMgr.show(m_mainBtnInfo);
			m_btnMgr.show(m_mainBtnQuit);
			m_btnMgr.show(m_favorites ? m_mainBtnFavoritesOn : m_mainBtnFavoritesOff);
			m_btnMgr.hide(m_favorites ? m_mainBtnFavoritesOff : m_mainBtnFavoritesOn);
		}
		else
		{
			m_btnMgr.hide(m_mainLblUser[0]);
			m_btnMgr.hide(m_mainLblUser[1]);
			m_btnMgr.hide(m_mainBtnConfig);
			m_btnMgr.hide(m_mainBtnInfo);
			m_btnMgr.hide(m_mainBtnQuit);
			m_btnMgr.hide(m_mainBtnFavoritesOn);
			m_btnMgr.hide(m_mainBtnFavoritesOff);
		}
		bool hideChannels, showDVD;
		hideChannels = (m_cfg.getBool("GENERAL", "hidechannelsbutton", false) || m_loaded_ios_base == 57);
		if (!hideChannels && (m_gameList.empty() || m_show_zone_main2))
		{
			if (m_current_view == COVERFLOW_USB)
				m_btnMgr.show(m_mainBtnChannel);
			else if (m_current_view == COVERFLOW_CHANNEL)
				m_btnMgr.show(m_mainBtnUsb);
			m_btnMgr.show(m_mainLblUser[2]);
			m_btnMgr.show(m_mainLblUser[3]);
		}
		else
		{
			m_btnMgr.hide(m_mainBtnChannel);
			m_btnMgr.hide(m_mainBtnUsb);
			m_btnMgr.hide(m_mainLblUser[2]);
			m_btnMgr.hide(m_mainLblUser[3]);
		}
		showDVD = (disc_check & 0x2);
		if (showDVD && (m_gameList.empty() || m_show_zone_main3))
		{
			m_btnMgr.show(m_mainBtnDVD);
			m_btnMgr.show(m_mainLblUser[4]);
			m_btnMgr.show(m_mainLblUser[5]);
		}
		else
		{
			m_btnMgr.hide(m_mainBtnDVD);
			m_btnMgr.hide(m_mainLblUser[4]);
			m_btnMgr.hide(m_mainLblUser[5]);
		}
		//
		if ((m_shown_pointer == 1 && WPadIR_Valid(0)) || (m_shown_pointer == 10 && !WPadIR_Valid(0)))
			m_cf.mouse(m_vid, 0, m_cursor1.x(), m_cursor1.y());
		else if ((m_shown_pointer == 2 && WPadIR_Valid(1)) || (m_shown_pointer == 20 && !WPadIR_Valid(1)))
			m_cf.mouse(m_vid, 1, m_cursor2.x(), m_cursor2.y());
		else if ((m_shown_pointer == 3 && WPadIR_Valid(2)) || (m_shown_pointer == 30 && !WPadIR_Valid(2)))
			m_cf.mouse(m_vid, 2, m_cursor3.x(), m_cursor3.y());
		else if ((m_shown_pointer == 4 && WPadIR_Valid(3)) || (m_shown_pointer == 40 && !WPadIR_Valid(3)))
			m_cf.mouse(m_vid, 3, m_cursor4.x(), m_cursor4.y());	
		else
			m_cf.mouse(m_vid, 0, -1, -1);
	}
	// 
	GX_InvVtxCache();
	GX_InvalidateTexAll();
	m_cf.clear();
	m_cfg.save();
	m_cat.save();
//	m_loc.save();
	_stopSounds();
	if (m_reload)
	{
		m_vid.waitMessage(m_waitMessage);
		return 1;
	}
	return 0;
}

void CMenu::_initMainMenu(CMenu::SThemeData &theme)
{
	STexture texQuit;
	STexture texQuitS;
	STexture texInfo;
	STexture texInfoS;
	STexture texConfig;
	STexture texConfigS;
	STexture texChannel;
	STexture texChannels;
	STexture texDVD;
	STexture texDVDs;
	STexture texUsb;
	STexture texUsbs;
	STexture texPrev;
	STexture texPrevS;
	STexture texNext;
	STexture texNextS;
	STexture texFavOn;
	STexture texFavOnS;
	STexture texFavOff;
	STexture texFavOffS;
	STexture bgLQ;
	STexture emptyTex;

	m_mainBg = _texture(theme.texSet, "MAIN/BG", "texture", theme.bg);
	if (m_theme.loaded() && STexture::TE_OK == bgLQ.fromPNGFile(sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString("MAIN/BG", "texture").c_str()).c_str(), GX_TF_CMPR, ALLOC_MEM2, 64, 64))
		m_mainBgLQ = bgLQ;
	texQuit.fromPNG(btnquit_png);
	texQuitS.fromPNG(btnquits_png);
	texInfo.fromPNG(btninfo_png);
	texInfoS.fromPNG(btninfos_png);
	texConfig.fromPNG(btnconfig_png);
	texConfigS.fromPNG(btnconfigs_png);
	texChannel.fromPNG(btnchannel_png);
	texChannels.fromPNG(btnchannels_png);
	texDVD.fromPNG(btndvd_png);
	texDVDs.fromPNG(btndvds_png);
	texUsb.fromPNG(btnusb_png);
	texUsbs.fromPNG(btnusbs_png);
	texPrev.fromPNG(btnprev_png);
	texPrevS.fromPNG(btnprevs_png);
	texNext.fromPNG(btnnext_png);
	texNextS.fromPNG(btnnexts_png);
	texFavOn.fromPNG(favoriteson_png);
	texFavOnS.fromPNG(favoritesons_png);
	texFavOff.fromPNG(favoritesoff_png);
	texFavOffS.fromPNG(favoritesoffs_png);
	_addUserLabels(theme, m_mainLblUser, ARRAY_SIZE(m_mainLblUser), "MAIN");
	m_mainBtnInfo = _addPicButton(theme, "MAIN/INFO_BTN", texInfo, texInfoS, 20, 412, 48, 48);
	m_mainBtnConfig = _addPicButton(theme, "MAIN/CONFIG_BTN", texConfig, texConfigS, 70, 412, 48, 48);
	m_mainBtnQuit = _addPicButton(theme, "MAIN/QUIT_BTN", texQuit, texQuitS, 570, 412, 48, 48);
	m_mainBtnChannel = _addPicButton(theme, "MAIN/CHANNEL_BTN", texChannel, texChannels, 520, 412, 48, 48);
	m_mainBtnDVD = _addPicButton(theme, "MAIN/DVD_BTN", texDVD, texDVDs, 470, 412, 48, 48);
	m_mainBtnUsb = _addPicButton(theme, "MAIN/USB_BTN", texUsb, texUsbs, 520, 412, 48, 48);
	m_mainBtnNext = _addPicButton(theme, "MAIN/NEXT_BTN", texNext, texNextS, 540, 146, 80, 80);
	m_mainBtnPrev = _addPicButton(theme, "MAIN/PREV_BTN", texPrev, texPrevS, 20, 146, 80, 80);
	m_mainBtnInit = _addButton(theme, "MAIN/BIG_SETTINGS_BTN", theme.titleFont, L"", 72, 180, 496, 96, CColor(0xFFFFFFFF));
	m_mainBtnInit2 = _addButton(theme, "MAIN/BIG_SETTINGS_BTN2", theme.titleFont, L"", 72, 290, 496, 96, CColor(0xFFFFFFFF));
	m_mainLblInit = _addLabel(theme, "MAIN/MESSAGE", theme.lblFont, L"", 40, 40, 560, 140, CColor(0xFFFFFFFF), FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_mainBtnFavoritesOn = _addPicButton(theme, "MAIN/FAVORITES_ON", texFavOn, texFavOnS, 300, 412, 56, 56);
	m_mainBtnFavoritesOff = _addPicButton(theme, "MAIN/FAVORITES_OFF", texFavOff, texFavOffS, 300, 412, 56, 56);
	m_mainLblLetter = _addLabel(theme, "MAIN/LETTER", theme.titleFont, L"", 540, 40, 80, 80, CColor(0xFFFFFFFF), FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, emptyTex);
	m_mainLblNotice = _addLabel(theme, "MAIN/NOTICE", theme.titleFont, L"", 340, 40, 280, 80, CColor(0xFFFFFFFF), FTGX_JUSTIFY_RIGHT | FTGX_ALIGN_MIDDLE, emptyTex);
	// 
	m_mainPrevZone.x = m_theme.getInt("MAIN/ZONES", "prev_x", -32);
	m_mainPrevZone.y = m_theme.getInt("MAIN/ZONES", "prev_y", -32);
	m_mainPrevZone.w = m_theme.getInt("MAIN/ZONES", "prev_w", 182);
	m_mainPrevZone.h = m_theme.getInt("MAIN/ZONES", "prev_h", 382);
	m_mainNextZone.x = m_theme.getInt("MAIN/ZONES", "next_x", 490);
	m_mainNextZone.y = m_theme.getInt("MAIN/ZONES", "next_y", -32);
	m_mainNextZone.w = m_theme.getInt("MAIN/ZONES", "next_w", 182);
	m_mainNextZone.h = m_theme.getInt("MAIN/ZONES", "next_h", 382);
	m_mainButtonsZone.x = m_theme.getInt("MAIN/ZONES", "buttons_x", -32);
	m_mainButtonsZone.y = m_theme.getInt("MAIN/ZONES", "buttons_y", 350);
	m_mainButtonsZone.w = m_theme.getInt("MAIN/ZONES", "buttons_w", 704);
	m_mainButtonsZone.h = m_theme.getInt("MAIN/ZONES", "buttons_h", 162);
	m_mainButtonsZone2.x = m_theme.getInt("MAIN/ZONES", "buttons2_x", -32);
	m_mainButtonsZone2.y = m_theme.getInt("MAIN/ZONES", "buttons2_y", 350);
	m_mainButtonsZone2.w = m_theme.getInt("MAIN/ZONES", "buttons2_w", 704);
	m_mainButtonsZone2.h = m_theme.getInt("MAIN/ZONES", "buttons2_h", 162);
	m_mainButtonsZone3.x = m_theme.getInt("MAIN/ZONES", "buttons3_x", -32);
	m_mainButtonsZone3.y = m_theme.getInt("MAIN/ZONES", "buttons3_y", 350);
	m_mainButtonsZone3.w = m_theme.getInt("MAIN/ZONES", "buttons3_w", 704);
	m_mainButtonsZone3.h = m_theme.getInt("MAIN/ZONES", "buttons3_h", 162);
	//
	_setHideAnim(m_mainBtnNext, "MAIN/NEXT_BTN", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainBtnPrev, "MAIN/PREV_BTN", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainBtnConfig, "MAIN/CONFIG_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnInfo, "MAIN/INFO_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnQuit, "MAIN/QUIT_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnChannel, "MAIN/CHANNEL_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnDVD, "MAIN/DVD_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnUsb, "MAIN/USB_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnFavoritesOn, "MAIN/FAVORITES_ON", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnFavoritesOff, "MAIN/FAVORITES_OFF", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnInit, "MAIN/BIG_SETTINGS_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_mainBtnInit2, "MAIN/BIG_SETTINGS_BTN2", 0, 0, -2.f, 0.f);
	_setHideAnim(m_mainLblInit, "MAIN/MESSAGE", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainLblLetter, "MAIN/LETTER", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainLblNotice, "MAIN/NOTICE", 0, 0, 0.f, 0.f);
	_hideMain(true);
	_textMain();
}

void CMenu::_textMain(void)
{
	m_btnMgr.setText(m_mainBtnInit, _t("main1", L"Install Game"));
	m_btnMgr.setText(m_mainBtnInit2, _t("main3", L"Select Partition"));
	m_btnMgr.setText(m_mainLblInit, _t("main2", L"Welcome to WiiFlow. I have not found any games. Click Install to install games, or Select partition to select your partition type."), true);
}

