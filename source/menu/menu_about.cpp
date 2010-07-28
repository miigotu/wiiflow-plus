
#include "menu.hpp"
#include "svnrev.h"

#include "sys.h"
#include "alt_ios.h"

#define LOADER_AUTHOR	"Kwiirk & Waninkoko, Hermes"
#define GUI_AUTHOR		"Hibernatus, Narolez, r-win, Miigotu"
#define THANKS			"Lustar, CedWii, Benjay, Domi78, Oops, Celtiore, Jiiwah, FluffyKiwi, Roku93, Spayrosam, Bluescreen81, Chappy23, BlindDude, Bubba, DJTaz, OggZee, Usptactical, WiiPower, Hermes, Spidy1000"
#define THANKS_SITES	"devkitpro.org, wiibrew.org, wiitdb.com, ohloh.net, wiifanart.com, wiiflowiki.com"
#define THANKS_CODE		"CFG Loader, uLoader, USB Loader GX, NeoGamma"

extern int mainIOS;
extern int mainIOSRev;

void CMenu::_about(void)
{
	SetupInput();
	_showAbout();
	do
	{
		_mainLoopCommon();
		if ((BTN_HOME_PRESSED || BTN_B_PRESSED) && !(m_thrdWorking && m_thrdStop))
		{
			m_btnMgr.click();
			if (m_btnMgr.selected() == m_aboutBtnSystem) {
				// show system menu
				_hideAbout(false);
				_system();
				remove(m_ver.c_str());
				_showAbout();
			}
		}
	} while ((wii_btnsPressed & (WBTN_HOME | WBTN_B)) == 0);
	_hideAbout(false);
}

void CMenu::_hideAbout(bool instant)
{
	m_btnMgr.hide(m_aboutLblTitle, instant);
	m_btnMgr.hide(m_aboutLblIOS, instant);
	m_btnMgr.hide(m_aboutLblOrigAuthor, instant);
	m_btnMgr.hide(m_aboutLblAuthor, instant);
	m_btnMgr.hide(m_aboutLblInfo, instant);
	m_btnMgr.hide(m_aboutBtnSystem, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_aboutLblUser); ++i)
		if (m_aboutLblUser[i] != -1u)
			m_btnMgr.hide(m_aboutLblUser[i], instant);
}

void CMenu::_showAbout(void)
{
	_setBg(m_aboutBg, m_aboutBg);
	m_btnMgr.show(m_aboutLblTitle);
	m_btnMgr.show(m_aboutLblIOS);
	m_btnMgr.show(m_aboutLblOrigAuthor);
	m_btnMgr.show(m_aboutLblAuthor);
	m_btnMgr.show(m_aboutLblInfo);
	m_btnMgr.show(m_aboutBtnSystem);
	for (u32 i = 0; i < ARRAY_SIZE(m_aboutLblUser); ++i)
		if (m_aboutLblUser[i] != -1u)
			m_btnMgr.show(m_aboutLblUser[i]);
	_textAbout();
}

void CMenu::_initAboutMenu(CMenu::SThemeData &theme)
{
	STexture emptyTex;
	_addUserLabels(theme, m_aboutLblUser, ARRAY_SIZE(m_aboutLblUser), "ABOUT");
	m_aboutBg = _texture(theme.texSet, "ABOUT/BG", "texture", theme.bg);
	m_aboutLblTitle = _addLabel(theme, "ABOUT/TITLE", theme.titleFont, L"", 170, 25, 300, 75, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, emptyTex);
	m_aboutLblOrigAuthor = _addLabel(theme, "ABOUT/ORIG_AUTHOR", theme.lblFont, L"", 40, 110, 560, 56, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_aboutLblAuthor = _addLabel(theme, "ABOUT/AUTHOR", theme.lblFont, L"", 40, 150, 560, 56, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_aboutLblInfo = _addLabel(theme, "ABOUT/INFO", theme.thxFont, L"", 40, 210, 460, 56, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_aboutBtnSystem = _addButton(theme, "ABOUT/SYSTEM_BTN", theme.btnFont, L"", 20, 410, 200, 56, theme.btnFontColor); // New element
	m_aboutLblIOS = _addLabel(theme, "ABOUT/IOS", theme.lblFont, L"", 40, 400, 560, 56, theme.txtFontColor, FTGX_JUSTIFY_RIGHT | FTGX_ALIGN_BOTTOM);
	// 
	_setHideAnim(m_aboutLblTitle, "ABOUT/TITLE", 0, 100, 0.f, 0.f);
	_setHideAnim(m_aboutLblOrigAuthor, "ABOUT/ORIG_AUTHOR", -100, 0, 0.f, 0.f);
	_setHideAnim(m_aboutLblAuthor, "ABOUT/AUTHOR", 100, 0, 0.f, 0.f);
	_setHideAnim(m_aboutLblInfo, "ABOUT/INFO", 0, -100, 0.f, 0.f);
	_setHideAnim(m_aboutBtnSystem, "ABOUT/SYSTEM_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_aboutLblIOS, "ABOUT/IOS", 0, 100, 0.f, 0.f);
	// 
	_hideAbout(true);
	_textAbout();
}

void CMenu::_textAbout(void)
{
	m_btnMgr.setText(m_aboutBtnSystem, _t("sys4", L"Update"));
	m_btnMgr.setText(m_aboutLblTitle, wfmt(_fmt("appname", L"%s v%s r%s"), APP_NAME, APP_VERSION, SVN_REV), true);
	m_btnMgr.setText(m_aboutLblOrigAuthor, wfmt(_fmt("about1", L"Loader by %s"), LOADER_AUTHOR), true);
	m_btnMgr.setText(m_aboutLblAuthor, wfmt(_fmt("about2", L"GUI by %s"), GUI_AUTHOR), true);
	wstringEx thanksTo(m_cfg.getWString(" GENERAL", "insertnamehere"));
	if (!thanksTo.empty())
		thanksTo.append(L", ");
	wstringEx translator(m_loc.getWString(m_curLanguage, "translation_author"));
	if (!translator.empty())
		translator.append(L", ");
	m_btnMgr.setText(m_aboutLblInfo, wfmt(_fmt("about3", L"Thanks to :\n\n%s%s%s\n\n%s\n%s"), thanksTo.toUTF8().c_str(), translator.toUTF8().c_str(), THANKS, THANKS_SITES, THANKS_CODE), true);
	if ((is_ios_type(IOS_TYPE_WANIN) && IOS_GetRevision() >= 18) ||
		(is_ios_type(IOS_TYPE_HERMES) && IOS_GetRevision() >= 5))
		m_btnMgr.setText(m_aboutLblIOS, wfmt(_fmt("ios", L"IOS%i rev%i, base IOS%i"), mainIOS, mainIOSRev, get_ios_base()), true);
	else
		m_btnMgr.setText(m_aboutLblIOS, wfmt(_fmt("ios", L"IOS%i rev%i"), mainIOS, mainIOSRev), true);
}
