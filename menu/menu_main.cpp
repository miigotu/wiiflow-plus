
#include "menu.hpp"
#include "loader/wdvd.h"

#include <wiiuse/wpad.h>
#include <unistd.h>
#include <fstream>

#include "wbfs.h"
#include "gecko.h"

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
	m_btnMgr.hide(m_mainBtnInit, instant);
	m_btnMgr.hide(m_mainLblInit, instant);
	m_btnMgr.hide(m_mainBtnFavoritesOn, instant);
	m_btnMgr.hide(m_mainBtnFavoritesOff, instant);
	m_btnMgr.hide(m_mainLblLetter, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_mainLblUser); ++i)
		if (m_mainLblUser[i] != -1u)
			m_btnMgr.hide(m_mainLblUser[i], instant);
}

void CMenu::_showMain(void)
{
	_setBg(m_mainBg, m_mainBgLQ);
	m_btnMgr.show(m_mainBtnConfig);
	m_btnMgr.show(m_mainBtnInfo);
	m_btnMgr.show(m_mainBtnQuit);
	for (u32 i = 1; i < ARRAY_SIZE(m_mainLblUser); ++i)
		if (m_mainLblUser[i] != -1u)
			m_btnMgr.show(m_mainLblUser[i]);
	if (m_gameList.empty())
	{
		m_btnMgr.show(m_mainBtnInit);
		m_btnMgr.show(m_mainLblInit);
	}
}

int CMenu::main(void)
{
	s32 padsState;
	WPADData *wd;
	u32 btn;
	wstringEx curLetter;
	int repeatButton = 0;
	u32 buttonHeld = (u32)-1;
	string prevTheme = m_cfg.getString(" GENERAL", "theme", "default");
	bool reload = false;
	float angle = 0;
	float mag = 0;
	static u32 disc_check = 0, olddisc_check = 0;

	WPAD_Rumble(WPAD_CHAN_0, 0);
	m_padLeftDelay = 0;
	m_padRightDelay = 0;
	_loadGameList();
	_showMain();
	m_curGameId.clear();
	_initCF();
	_startMusic();

	WDVD_GetCoverStatus(&disc_check);
	
	while (true)
	{
		olddisc_check = disc_check;
		WDVD_GetCoverStatus(&disc_check);
				
		WPAD_ScanPads();
		padsState = WPAD_ButtonsDown(0);
		wd = WPAD_Data(0);
		btn = _btnRepeat(wd->btns_h);
		//Get Nunchuk values
		angle = wd->exp.nunchuk.js.ang;
		mag = wd->exp.nunchuk.js.mag;
		
		//check if Disc was inserted
		if ((disc_check & 0x2) && (disc_check!=olddisc_check) && !m_locked) {
		_hideMain();
		_wbfsOp(CMenu::WO_ADD_GAME);
		_showMain();
		}
		
		if ((padsState & WPAD_BUTTON_HOME) != 0)
		{
			reload = (wd->btns_h & WPAD_BUTTON_B) != 0;
			break;
		}
		if (wd->ir.valid)
			m_btnMgr.mouse(wd->ir.x - m_cur.width() / 2, wd->ir.y - m_cur.height() / 2);
		++repeatButton;
		if ((wd->btns_h & WPAD_BUTTON_A) == 0)
			buttonHeld = (u32)-1;
		else if (buttonHeld != (u32)-1 && buttonHeld == m_btnMgr.selected() && repeatButton >= 16)
			padsState |= WPAD_BUTTON_A;
		if ((padsState & WPAD_BUTTON_1) != 0)
		{
			int cfVersion = 1 + loopNum(m_cfg.getInt(" GENERAL", "last_cf_mode", 1), m_numCFVersions);
			_loadCFLayout(cfVersion);
			m_cf.applySettings();
			m_cfg.setInt(" GENERAL", "last_cf_mode", cfVersion);
		}
		else if ((padsState & WPAD_BUTTON_2) != 0)
		{
			int cfVersion = 1 + loopNum(m_cfg.getInt(" GENERAL", "last_cf_mode", 1) - 2, m_numCFVersions);
			_loadCFLayout(cfVersion);
			m_cf.applySettings();
			m_cfg.setInt(" GENERAL", "last_cf_mode", cfVersion);
		}
		if (((padsState & (WPAD_BUTTON_DOWN | WPAD_BUTTON_RIGHT)) != 0 && (wd->btns_h & WPAD_BUTTON_B) != 0)
			|| ((padsState & WPAD_BUTTON_PLUS) != 0 && m_alphaSearch == ((wd->btns_h & WPAD_BUTTON_B) == 0)))
		{
			curLetter.resize(1);
			curLetter[0] = m_cf.nextLetter();
			m_letter = 60;
			m_btnMgr.setText(m_mainLblLetter, curLetter);
			m_btnMgr.show(m_mainLblLetter);

		}
		else if (((padsState & (WPAD_BUTTON_UP | WPAD_BUTTON_LEFT)) != 0 && (wd->btns_h & WPAD_BUTTON_B) != 0)
			|| ((padsState & WPAD_BUTTON_MINUS) != 0 && m_alphaSearch == ((wd->btns_h & WPAD_BUTTON_B) == 0)))
		{
			curLetter.resize(1);
			curLetter[0] = m_cf.prevLetter();
			m_letter = 60;
			m_btnMgr.setText(m_mainLblLetter, curLetter);
			m_btnMgr.show(m_mainLblLetter);
		}
		else if ((padsState & WPAD_BUTTON_UP) != 0)
			m_cf.up();
		else if ((padsState & WPAD_BUTTON_DOWN) != 0)
			m_cf.down();
		else if ((padsState & WPAD_BUTTON_MINUS) != 0)
			m_cf.pageUp();
		else if ((padsState & WPAD_BUTTON_PLUS) != 0)
			m_cf.pageDown();
		else if ((btn & WPAD_BUTTON_LEFT) != 0 || ((angle > 255 && angle < 285) && mag > 0.75))
			m_cf.left();
		else if ((btn & WPAD_BUTTON_RIGHT) != 0 || ((angle > 75 && angle < 105) && mag > 0.75))
			m_cf.right();
		if ((padsState & WPAD_BUTTON_A) != 0)
		{
			if (buttonHeld != m_btnMgr.selected())
				m_btnMgr.click();
			if (m_btnMgr.selected() == m_mainBtnQuit)
				break;
			else if (m_btnMgr.selected() == m_mainBtnInit)
			{
				_hideMain();
				_config(2);
				if (prevTheme != m_cfg.getString(" GENERAL", "theme"))
				{
					reload = true;
					break;
				}
				_showMain();
			}
			else if (m_btnMgr.selected() == m_mainBtnConfig)
			{
				_hideMain();
				_config(1);
				if (prevTheme != m_cfg.getString(" GENERAL", "theme"))
				{
					reload = true;
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
				m_cfg.setInt(" GENERAL", "favorites", m_favorites);
				m_curGameId = m_cf.getId();
				_initCF();
			}
			else if (!m_cf.empty())
			{
				if (m_cf.select())
				{
					_hideMain();
					_game((wd->btns_h & WPAD_BUTTON_B) != 0);
					m_cf.cancel();
					_showMain();
				}
			}
		}
		if (m_letter > 0)
			if (--m_letter == 0)
				m_btnMgr.hide(m_mainLblLetter);
		// 
		if (!m_gameList.empty() && wd->ir.valid && m_cur.x() >= m_mainPrevZone.x && m_cur.y() >= m_mainPrevZone.y
			&& m_cur.x() < m_mainPrevZone.x + m_mainPrevZone.w && m_cur.y() < m_mainPrevZone.y + m_mainPrevZone.h)
			m_btnMgr.show(m_mainBtnPrev);
		else
			m_btnMgr.hide(m_mainBtnPrev);
		if (!m_gameList.empty() && wd->ir.valid && m_cur.x() >= m_mainNextZone.x && m_cur.y() >= m_mainNextZone.y
			&& m_cur.x() < m_mainNextZone.x + m_mainNextZone.w && m_cur.y() < m_mainNextZone.y + m_mainNextZone.h)
			m_btnMgr.show(m_mainBtnNext);
		else
			m_btnMgr.hide(m_mainBtnNext);
		if (!m_gameList.empty() && wd->ir.valid && m_cur.x() >= m_mainButtonsZone.x && m_cur.y() >= m_mainButtonsZone.y
			&& m_cur.x() < m_mainButtonsZone.x + m_mainButtonsZone.w && m_cur.y() < m_mainButtonsZone.y + m_mainButtonsZone.h)
		{
			m_btnMgr.show(m_mainLblUser[0]);
			m_btnMgr.show(m_mainBtnConfig);
			m_btnMgr.show(m_mainBtnInfo);
			m_btnMgr.show(m_mainBtnQuit);
			m_btnMgr.show(m_favorites ? m_mainBtnFavoritesOn : m_mainBtnFavoritesOff);
			m_btnMgr.hide(m_favorites ? m_mainBtnFavoritesOff : m_mainBtnFavoritesOn);
		}
		else
		{
			m_btnMgr.hide(m_mainLblUser[0]);
			m_btnMgr.hide(m_mainBtnConfig);
			m_btnMgr.hide(m_mainBtnInfo);
			m_btnMgr.hide(m_mainBtnQuit);
			m_btnMgr.hide(m_mainBtnFavoritesOn);
			m_btnMgr.hide(m_mainBtnFavoritesOff);
		}
		// 
		if (!wd->ir.valid || m_btnMgr.selected() != (u32)-1)
			m_cf.mouse(m_vid, -1, -1);
		else
			m_cf.mouse(m_vid, m_cur.x(), m_cur.y());
		_mainLoopCommon(wd, true);
	}
	WPAD_Rumble(WPAD_CHAN_0, 0);
	// 
	GX_InvVtxCache();
	GX_InvalidateTexAll();
	m_cf.clear();
	m_cfg.save();
//	m_loc.save();
	_stopSounds();
	if (reload)
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
	m_mainBtnNext = _addPicButton(theme, "MAIN/NEXT_BTN", texNext, texNextS, 540, 146, 80, 80);
	m_mainBtnPrev = _addPicButton(theme, "MAIN/PREV_BTN", texPrev, texPrevS, 20, 146, 80, 80);
	m_mainBtnInit = _addButton(theme, "MAIN/BIG_SETTINGS_BTN", theme.titleFont, L"", 60, 180, 520, 120, CColor(0xFFFFFFFF));
	m_mainLblInit = _addLabel(theme, "MAIN/MESSAGE", theme.lblFont, L"", 40, 40, 560, 140, CColor(0xFFFFFFFF), FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_mainBtnFavoritesOn = _addPicButton(theme, "MAIN/FAVORITES_ON", texFavOn, texFavOnS, 300, 412, 56, 56);
	m_mainBtnFavoritesOff = _addPicButton(theme, "MAIN/FAVORITES_OFF", texFavOff, texFavOffS, 300, 412, 56, 56);
	m_mainLblLetter = _addLabel(theme, "MAIN/LETTER", theme.titleFont, L"", 540, 40, 80, 80, CColor(0xFFFFFFFF), FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, emptyTex);
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
	//
	_setHideAnim(m_mainBtnNext, "MAIN/NEXT_BTN", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainBtnPrev, "MAIN/PREV_BTN", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainBtnConfig, "MAIN/CONFIG_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnInfo, "MAIN/INFO_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnQuit, "MAIN/QUIT_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnFavoritesOn, "MAIN/FAVORITES_ON", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnFavoritesOff, "MAIN/FAVORITES_OFF", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnInit, "MAIN/BIG_SETTINGS_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_mainLblInit, "MAIN/MESSAGE", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainLblLetter, "MAIN/LETTER", 0, 0, 0.f, 0.f);
	_hideMain(true);
	_textMain();
}

void CMenu::_textMain(void)
{
	m_btnMgr.setText(m_mainBtnInit, _t("main1", L"Settings"));
	m_btnMgr.setText(m_mainLblInit, _t("main2", L"Welcome to WiiFlow. I have not found any game. Click on Settings to install games."), true);
}
