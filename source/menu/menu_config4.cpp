
#include "menu.hpp"
#include "loader/sys.h"
#include "channels.h"


using namespace std;

static const int g_curPage = 3;

static inline int loopNum(int i, int s)
{
	return i < 0 ? (s - (-i % s)) % s : i % s;
}

int currentChannelIndex = -1;
int amountOfChannels = -1;

void CMenu::_hideConfig4(bool instant)
{
	m_btnMgr.hide(m_configLblTitle, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_configLblPage, instant);
	m_btnMgr.hide(m_configBtnPageM, instant);
	m_btnMgr.hide(m_configBtnPageP, instant);
	// 
	m_btnMgr.hide(m_config4LblHome, instant);
	m_btnMgr.hide(m_config4BtnHome, instant);
	m_btnMgr.hide(m_config4LblSaveFavMode, instant);
	m_btnMgr.hide(m_config4BtnSaveFavMode, instant);
	m_btnMgr.hide(m_config4LblSearchMode, instant);
	m_btnMgr.hide(m_config4BtnSearchMode, instant);
	m_btnMgr.hide(m_config4LblReturnTo, instant);
	m_btnMgr.hide(m_config4LblReturnToVal, instant);
	m_btnMgr.hide(m_config4BtnReturnToM, instant);
	m_btnMgr.hide(m_config4BtnReturnToP, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_config4LblUser); ++i)
		if (m_config4LblUser[i] != -1u)
			m_btnMgr.hide(m_config4LblUser[i], instant);
}

void CMenu::_showConfig4(void)
{
	_setBg(m_config4Bg, m_config4Bg);
	m_btnMgr.show(m_configLblTitle);
	m_btnMgr.show(m_configBtnBack);
	m_btnMgr.show(m_configLblPage);
	m_btnMgr.show(m_configBtnPageM);
	m_btnMgr.show(m_configBtnPageP);
	// 
	m_btnMgr.show(m_config4LblHome);
	m_btnMgr.show(m_config4BtnHome);
	m_btnMgr.show(m_config4LblSaveFavMode);
	m_btnMgr.show(m_config4BtnSaveFavMode);
	m_btnMgr.show(m_config4LblSearchMode);
	m_btnMgr.show(m_config4BtnSearchMode);
	if (/*m_channels.CanIdentify() && */m_loaded_ios_base !=57)
	{
		m_btnMgr.show(m_config4LblReturnTo);
		m_btnMgr.show(m_config4LblReturnToVal);
		m_btnMgr.show(m_config4BtnReturnToM);
		m_btnMgr.show(m_config4BtnReturnToP);
	}
	for (u32 i = 0; i < ARRAY_SIZE(m_config4LblUser); ++i)
		if (m_config4LblUser[i] != -1u)
			m_btnMgr.show(m_config4LblUser[i]);
 
	m_btnMgr.setText(m_configLblPage, wfmt(L"%i / %i", g_curPage, m_locked ? g_curPage : CMenu::_nbCfgPages));
	m_alphaSearch = m_cfg.getBool(" GENERAL", "alphabetic_search_on_plus_minus", false);
	m_btnMgr.setText(m_config4BtnHome, m_cfg.getBool(" GENERAL", "exit_to_wii_menu") ? _t("on", L"On") : _t("off", L"Off"));
	m_btnMgr.setText(m_config4BtnSaveFavMode, m_cfg.getBool(" GENERAL", "favorites_on_startup") ? _t("on", L"On") : _t("off", L"Off"));
	m_btnMgr.setText(m_config4BtnSearchMode, m_cfg.getBool(" GENERAL", "alphabetic_search_on_plus_minus") ? _t("smalpha", L"Alphabetic") : _t("smpage", L"Pages"));

	wstringEx channelName = m_loc.getWString(m_curLanguage, "disabled", L"Disabled");

	string langCode = m_loc.getString(m_curLanguage, "wiitdb_code", "EN");
	m_channels.Init(0x00010001, langCode);
	amountOfChannels = m_channels.Count();

	string currentChanId = m_cfg.getString(" GENERAL", "returnto" );
	if (currentChanId.size() > 0)
	{
		for (int i = 0; i < amountOfChannels; i++)
		{
			if (currentChanId == m_channels.GetId(i))
			{
				channelName = m_channels.GetName(i);
				break;
			}
		}
	}

	m_btnMgr.setText(m_config4LblReturnToVal, channelName);
}

int CMenu::_config4(void)
{
	int nextPage = 0;

	_showConfig4();
	while (true)
	{
		_mainLoopCommon();
		if ((btnsPressed & (WBTN_HOME | WBTN_B)) != 0)
			break;
		else if ((btnsPressed & WBTN_UP) != 0)
			m_btnMgr.up();
		else if ((btnsPressed & WBTN_DOWN) != 0)
			m_btnMgr.down();
		if ((btn & WBTN_LEFT) != 0 || (btnsPressed & WBTN_MINUS) != 0 || ((btnsPressed & WBTN_A) != 0 && m_btnMgr.selected() == m_configBtnPageM))
		{
			nextPage = max(1, m_locked ? 1 : g_curPage - 1);
			m_btnMgr.click(m_configBtnPageM);
			break;
		}
		if (!m_locked && ((btn & WBTN_RIGHT) != 0 || (btnsPressed & WBTN_PLUS) != 0 || ((btnsPressed & WBTN_A) != 0 && m_btnMgr.selected() == m_configBtnPageP)))
		{
			nextPage = min(g_curPage + 1, CMenu::_nbCfgPages);
			m_btnMgr.click(m_configBtnPageP);
			break;
		}
		if ((btnsPressed & WBTN_A) != 0)
		{
			m_btnMgr.click();
			if (m_btnMgr.selected() == m_configBtnBack)
				break;
			else if (m_btnMgr.selected() == m_config4BtnHome)
			{
				m_cfg.setBool(" GENERAL", "exit_to_wii_menu", !m_cfg.getBool(" GENERAL", "exit_to_wii_menu"));
				Sys_ExitToWiiMenu(m_noHBC || m_cfg.getBool(" GENERAL", "exit_to_wii_menu", false));
				_showConfig4();
			}
			else if (m_btnMgr.selected() == m_config4BtnSaveFavMode)
			{
				m_cfg.setBool(" GENERAL", "favorites_on_startup", !m_cfg.getBool(" GENERAL", "favorites_on_startup"));
				_showConfig4();
			}
			else if (m_btnMgr.selected() == m_config4BtnSearchMode)
			{
				m_alphaSearch = !m_cfg.getBool(" GENERAL", "alphabetic_search_on_plus_minus", false);
				m_cfg.setBool(" GENERAL", "alphabetic_search_on_plus_minus", m_alphaSearch);
				_showConfig4();
			}
			else if (m_btnMgr.selected() == m_config4BtnReturnToP)
			{
				currentChannelIndex = (currentChannelIndex >= amountOfChannels - 1) ? -1 : currentChannelIndex + 1;
				if (currentChannelIndex == -1)
					m_cfg.remove(" GENERAL", "returnto");
				else
					m_cfg.setString(" GENERAL", "returnto", m_channels.GetId(currentChannelIndex));
				_showConfig4();
			}
			else if (m_btnMgr.selected() == m_config4BtnReturnToM)
			{
				if (currentChannelIndex == -1) currentChannelIndex = amountOfChannels;
				currentChannelIndex--;
				if (currentChannelIndex == -1)
					m_cfg.remove(" GENERAL", "returnto");
				else
					m_cfg.setString(" GENERAL", "returnto", m_channels.GetId(currentChannelIndex));
				_showConfig4();
			}
		}
	}
	_hideConfig4();
	return nextPage;
}

void CMenu::_initConfig4Menu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_config4LblUser, ARRAY_SIZE(m_config4LblUser), "CONFIG4");
	m_config4Bg = _texture(theme.texSet, "CONFIG4/BG", "texture", theme.bg);
	m_config4LblHome = _addLabel(theme, "CONFIG4/WIIMENU", theme.lblFont, L"", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config4BtnHome = _addButton(theme, "CONFIG4/WIIMENU_BTN", theme.btnFont, L"", 400, 130, 200, 56, theme.btnFontColor);
	m_config4LblSaveFavMode = _addLabel(theme, "CONFIG4/SAVE_FAVMODE", theme.lblFont, L"", 40, 190, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config4BtnSaveFavMode = _addButton(theme, "CONFIG4/SAVE_FAVMODE_BTN", theme.btnFont, L"", 400, 190, 200, 56, theme.btnFontColor);
	m_config4LblSearchMode = _addLabel(theme, "CONFIG4/SEARCH_MODE", theme.lblFont, L"", 40, 250, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config4BtnSearchMode = _addButton(theme, "CONFIG4/SEARCH_MODE_BTN", theme.btnFont, L"", 400, 250, 200, 56, theme.btnFontColor);
	m_config4LblReturnTo = _addLabel(theme, "CONFIG4/RETURN_TO", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config4LblReturnToVal = _addLabel(theme, "CONFIG4/RETURN_TO_BTN", theme.btnFont, L"", 426, 310, 118, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config4BtnReturnToM = _addPicButton(theme, "CONFIG4/RETURN_TO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 370, 310, 56, 56);
	m_config4BtnReturnToP = _addPicButton(theme, "CONFIG4/RETURN_TO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 310, 56, 56);
//
	_setHideAnim(m_config4LblHome, "CONFIG4/WIIMENU", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config4BtnHome, "CONFIG4/WIIMENU_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_config4LblSaveFavMode, "CONFIG4/SAVE_FAVMODE", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config4BtnSaveFavMode, "CONFIG4/SAVE_FAVMODE_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_config4LblSearchMode, "CONFIG4/SEARCH_MODE", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config4BtnSearchMode, "CONFIG4/SEARCH_MODE_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_config4LblReturnTo, "CONFIG4/RETURN_TO", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config4LblReturnToVal, "CONFIG4/RETURN_TO_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config4BtnReturnToM, "CONFIG4/RETURN_TO_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config4BtnReturnToP, "CONFIG4/RETURN_TO_PLUS", 0, 0, 1.f, -1.f);
	_hideConfig4(true);
	_textConfig4();
}

void CMenu::_textConfig4(void)
{
	m_btnMgr.setText(m_config4LblHome, _t("cfgc1", L"Exit to Wii Menu"));
	m_btnMgr.setText(m_config4LblSaveFavMode, _t("cfgd5", L"Save favorite mode state"));
	m_btnMgr.setText(m_config4LblSearchMode, _t("cfgd6", L"Default search mode"));
	m_btnMgr.setText(m_config4LblReturnTo, _t("cfgg21", L"Return To Channel"));
}
