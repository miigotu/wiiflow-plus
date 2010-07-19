
#include "menu.hpp"
#include "wbfs.h"

#include <dirent.h>
#include <sys/types.h> 
#include <sys/param.h> 
#include <sys/stat.h> 
#include <unistd.h> 

using namespace std;

static const int g_curPage = 2;

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

void CMenu::_hideConfigAdv(bool instant)
{
	m_btnMgr.hide(m_configLblTitle, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_configLblPage, instant);
	m_btnMgr.hide(m_configBtnPageM, instant);
	m_btnMgr.hide(m_configBtnPageP, instant);
	// 
	m_btnMgr.hide(m_configAdvLblInstall, instant);
	m_btnMgr.hide(m_configAdvBtnInstall, instant);
	m_btnMgr.hide(m_configAdvLblTheme, instant);
	m_btnMgr.hide(m_configAdvLblCurTheme, instant);
	m_btnMgr.hide(m_configAdvBtnCurThemeM, instant);
	m_btnMgr.hide(m_configAdvBtnCurThemeP, instant);
	m_btnMgr.hide(m_configAdvLblLanguage, instant);
	m_btnMgr.hide(m_configAdvLblCurLanguage, instant);
	m_btnMgr.hide(m_configAdvBtnCurLanguageM, instant);
	m_btnMgr.hide(m_configAdvBtnCurLanguageP, instant);
	m_btnMgr.hide(m_configAdvLblCFTheme, instant);
	m_btnMgr.hide(m_configAdvBtnCFTheme, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_configAdvLblUser); ++i)
		if (m_configAdvLblUser[i] != -1u)
			m_btnMgr.hide(m_configAdvLblUser[i], instant);
}

void CMenu::_showConfigAdv(void)
{
	_setBg(m_configAdvBg, m_configAdvBg);
	m_btnMgr.show(m_configLblTitle);
	m_btnMgr.show(m_configBtnBack);
	m_btnMgr.show(m_configLblPage);
	m_btnMgr.show(m_configBtnPageM);
	m_btnMgr.show(m_configBtnPageP);
	// 
	m_btnMgr.show(m_configAdvLblCurTheme);
	m_btnMgr.show(m_configAdvBtnCurThemeM);
	m_btnMgr.show(m_configAdvBtnCurThemeP);
	m_btnMgr.show(m_configAdvLblTheme);
	if( !m_locked )
	{
		m_btnMgr.show(m_configAdvLblInstall);
		m_btnMgr.show(m_configAdvBtnInstall);
		m_btnMgr.show(m_configAdvLblLanguage);
		m_btnMgr.show(m_configAdvLblCurLanguage);
		m_btnMgr.show(m_configAdvBtnCurLanguageM);
		m_btnMgr.show(m_configAdvBtnCurLanguageP);
		m_btnMgr.show(m_configAdvLblCFTheme);
		m_btnMgr.show(m_configAdvBtnCFTheme);
	}
	for (u32 i = 0; i < ARRAY_SIZE(m_configAdvLblUser); ++i)
		if (m_configAdvLblUser[i] != -1u)
			m_btnMgr.show(m_configAdvLblUser[i]);
	// 
	m_btnMgr.setText(m_configLblPage, wfmt(L"%i / %i", g_curPage, m_locked ? g_curPage : CMenu::_nbCfgPages));
	m_btnMgr.setText(m_configAdvLblCurLanguage, m_curLanguage);
	m_btnMgr.setText(m_configAdvLblCurTheme, m_cfg.getString(" GENERAL", "theme"));
}

static string upperCase(string text)
{
	char c;

	for (string::size_type i = 0; i < text.size(); ++i)
	{
		c = text[i];
		if (c >= 'a' && c <= 'z')
			text[i] = c & 0xDF;
	}
	return text;
}

static void listThemes(const char * path, vector<string> &themes)
{
	DIR *d;
	struct dirent *dir;
	string fileName;
	bool def = false;

	themes.clear();
	d = opendir(path);
	if (d != 0)
	{
		dir = readdir(d);
		while (dir != 0)
		{
			fileName = upperCase(dir->d_name);
			def = def || fileName == "DEFAULT.INI";
			if (fileName.size() > 4 && fileName.substr(fileName.size() - 4, 4) == ".INI")
				themes.push_back(fileName.substr(0, fileName.size() - 4));
			dir = readdir(d);
		}
		closedir(d);
	}
	if (!def)
		themes.push_back("DEFAULT");
	sort(themes.begin(), themes.end());
}

int CMenu::_configAdv(void)
{
	int nextPage = 0;
	vector<string> themes;
	int curTheme;
	string prevTheme = m_cfg.getString(" GENERAL", "theme");

	listThemes(m_themeDir.c_str(), themes);
	curTheme = 0;
	for (u32 i = 0; i < themes.size(); ++i)
		if (themes[i] == prevTheme)
		{
			curTheme = i;
			break;
		}
	_showConfigAdv();
	while (true)
	{
		_mainLoopCommon();
		if (BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if (BTN_UP_PRESSED)
			m_btnMgr.up();
		else if (BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if (BTN_LEFT_PRESSED || BTN_MINUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected() == m_configBtnPageM))
		{
			nextPage = max(1, m_locked ? 1 : g_curPage - 1);
			m_btnMgr.click(m_configBtnPageM);
			break;
		}
		if (!m_locked && (BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected() == m_configBtnPageP)))
		{
			nextPage = min(g_curPage + 1, CMenu::_nbCfgPages);
			m_btnMgr.click(m_configBtnPageP);
			break;
		}
		if (BTN_A_PRESSED)
		{
			m_btnMgr.click();
			if (m_btnMgr.selected() == m_configBtnBack)
				break;
			else if (m_btnMgr.selected() == m_configAdvBtnInstall)
			{
				if (!WBFS_IsReadOnly())
				{
					_hideConfigAdv();
					_wbfsOp(CMenu::WO_ADD_GAME);
					_showConfigAdv();
				}
				else
				{
					error(_t("wbfsop11", L"The currently selected filesystem is read-only. You cannot install games or remove them."));
				}
			}
			else if (m_btnMgr.selected() == m_configAdvBtnCurThemeP)
			{
				curTheme = loopNum(curTheme + 1, (int)themes.size());
				m_cfg.setString(" GENERAL", "theme", themes[curTheme]);
				_showConfigAdv();
			}
			else if (m_btnMgr.selected() == m_configAdvBtnCurThemeM)
			{
				curTheme = loopNum(curTheme - 1, (int)themes.size());
				m_cfg.setString(" GENERAL", "theme", themes[curTheme]);
				_showConfigAdv();
			}
			else if (m_btnMgr.selected() == m_configAdvBtnCurLanguageP)
			{
				m_curLanguage = m_loc.nextDomain(m_curLanguage);
				m_cfg.setString(" GENERAL", "language", m_curLanguage);
				_updateText();
				_showConfigAdv();
			}
			else if (m_btnMgr.selected() == m_configAdvBtnCurLanguageM)
			{
				m_curLanguage = m_loc.prevDomain(m_curLanguage);
				m_cfg.setString(" GENERAL", "language", m_curLanguage);
				_updateText();
				_showConfigAdv();
			}
			else if (m_btnMgr.selected() == m_configAdvBtnCFTheme)
			{
				_hideConfigAdv();
				_cfTheme();
				_showConfigAdv();
			}
		}
	}
	_hideConfigAdv();
	if (m_gameList.empty())
		_loadList();
	return nextPage;
}

void CMenu::_initConfigAdvMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_configAdvLblUser, ARRAY_SIZE(m_configAdvLblUser), "CONFIG_ADV");
	m_configAdvBg = _texture(theme.texSet, "CONFIG_ADV/BG", "texture", theme.bg);
	m_configAdvLblTheme = _addLabel(theme, "CONFIG_ADV/THEME", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configAdvLblCurTheme = _addLabel(theme, "CONFIG_ADV/THEME_BTN", theme.btnFont, L"", 386, 130, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configAdvBtnCurThemeM = _addPicButton(theme, "CONFIG_ADV/THEME_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 130, 56, 56);
	m_configAdvBtnCurThemeP = _addPicButton(theme, "CONFIG_ADV/THEME_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);
	m_configAdvLblLanguage = _addLabel(theme, "CONFIG_ADV/LANGUAGE", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configAdvLblCurLanguage = _addLabel(theme, "CONFIG_ADV/LANGUAGE_BTN", theme.btnFont, L"", 386, 190, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configAdvBtnCurLanguageM = _addPicButton(theme, "CONFIG_ADV/LANGUAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 190, 56, 56);
	m_configAdvBtnCurLanguageP = _addPicButton(theme, "CONFIG_ADV/LANGUAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 190, 56, 56);
	m_configAdvLblCFTheme = _addLabel(theme, "CONFIG_ADV/CUSTOMIZE_CF", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configAdvBtnCFTheme = _addButton(theme, "CONFIG_ADV/CUSTOMIZE_CF_BTN", theme.btnFont, L"", 330, 250, 270, 56, theme.btnFontColor);
	m_configAdvLblInstall = _addLabel(theme, "CONFIG_ADV/INSTALL", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configAdvBtnInstall = _addButton(theme, "CONFIG_ADV/INSTALL_BTN", theme.btnFont, L"", 330, 310, 270, 56, theme.btnFontColor);
	// 
	_setHideAnim(m_configAdvLblTheme, "CONFIG_ADV/THEME", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configAdvLblCurTheme, "CONFIG_ADV/THEME_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configAdvBtnCurThemeM, "CONFIG_ADV/THEME_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configAdvBtnCurThemeP, "CONFIG_ADV/THEME_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configAdvLblLanguage, "CONFIG_ADV/LANGUAGE", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configAdvLblCurLanguage, "CONFIG_ADV/LANGUAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configAdvBtnCurLanguageM, "CONFIG_ADV/LANGUAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configAdvBtnCurLanguageP, "CONFIG_ADV/LANGUAGE_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configAdvLblCFTheme, "CONFIG_ADV/CUSTOMIZE_CF", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configAdvBtnCFTheme, "CONFIG_ADV/CUSTOMIZE_CF_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configAdvLblInstall, "CONFIG_ADV/INSTALL", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configAdvBtnInstall, "CONFIG_ADV/INSTALL_BTN", 0, 0, 1.f, -1.f);
	_hideConfigAdv(true);
	_textConfigAdv();
}

void CMenu::_textConfigAdv(void)
{
	m_btnMgr.setText(m_configAdvLblTheme, _t("cfga7", L"Theme"));
	m_btnMgr.setText(m_configAdvLblLanguage, _t("cfga6", L"Language"));
	m_btnMgr.setText(m_configAdvLblCFTheme, _t("cfgc4", L"Adjust Coverflow"));
	m_btnMgr.setText(m_configAdvBtnCFTheme, _t("cfgc5", L"Go"));
	m_btnMgr.setText(m_configAdvLblInstall, _t("cfga2", L"Install game"));
	m_btnMgr.setText(m_configAdvBtnInstall, _t("cfga3", L"Install"));
}
