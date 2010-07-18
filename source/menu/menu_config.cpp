
#include "menu.hpp"
#include "sys.h"


using namespace std;

const int CMenu::_nbCfgPages = 7;
static const int g_curPage = 1;

void CMenu::_hideConfig(bool instant)
{
	m_btnMgr.hide(m_configLblTitle, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_configLblPage, instant);
	m_btnMgr.hide(m_configBtnPageM, instant);
	m_btnMgr.hide(m_configBtnPageP, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_configLblRumble, instant);
	m_btnMgr.hide(m_configBtnRumble, instant);
	m_btnMgr.hide(m_configLblBoxMode, instant);
	m_btnMgr.hide(m_configBtnBoxMode, instant);
	m_btnMgr.hide(m_configLblDownload, instant);
	m_btnMgr.hide(m_configBtnDownload, instant);
	m_btnMgr.hide(m_configLblParental, instant);
	m_btnMgr.hide(m_configBtnUnlock, instant);
	m_btnMgr.hide(m_configBtnSetCode, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_configLblUser); ++i)
		if (m_configLblUser[i] != -1u)
			m_btnMgr.hide(m_configLblUser[i], instant);
}

void CMenu::_showConfig(void)
{
	_setBg(m_configBg, m_configBg);
	m_btnMgr.show(m_configLblTitle);
	m_btnMgr.show(m_configBtnBack);
	m_btnMgr.show(m_configLblRumble);
	m_btnMgr.show(m_configBtnRumble);
	m_btnMgr.show(m_configLblBoxMode);
	m_btnMgr.show(m_configBtnBoxMode);
	m_btnMgr.show(m_configLblDownload);
	m_btnMgr.show(m_configBtnDownload);
	m_btnMgr.show(m_configLblParental);
	m_btnMgr.show(m_configLblPage);
	m_btnMgr.show(m_configBtnPageM);
	m_btnMgr.show(m_configBtnPageP);
	if (m_locked)
		m_btnMgr.show(m_configBtnUnlock);
	else
	{
		m_btnMgr.show(m_configBtnSetCode);
	}
	for (u32 i = 0; i < ARRAY_SIZE(m_configLblUser); ++i)
		if (m_configLblUser[i] != -1u)
			m_btnMgr.show(m_configLblUser[i]);
	// 
	m_btnMgr.setText(m_configLblPage, wfmt(L"%i / %i", g_curPage, m_locked ? g_curPage + 1 : CMenu::_nbCfgPages));
	m_btnMgr.setText(m_configBtnRumble, m_cfg.getBool(" GENERAL", "rumble") ? _t("on", L"On") : _t("off", L"Off"));
	m_btnMgr.setText(m_configBtnBoxMode, m_cfg.getBool(" GENERAL", "box_mode") ? _t("on", L"On") : _t("off", L"Off"));
}

void CMenu::_config(int page)
{
	int nextPage = page;

	SetupInput();
	m_curGameId = m_cf.getId();
	m_cf.clear();
	while (nextPage > 0 && nextPage <= CMenu::_nbCfgPages)
		switch (nextPage)
		{
			case 1:
				SetupInput();
				nextPage = _config1();
				break;
			case 2:
				SetupInput();
				nextPage = _configAdv();
				break;
			case 3:
				SetupInput();
				nextPage = _config4();
				break;
			case 4:
				SetupInput();
				nextPage = _configSnd();
				break;
			case 5:
				SetupInput();
				nextPage = _config2();
				break;
			case 6:
				SetupInput();
				nextPage = _config3();
				break;
			case 7:
				SetupInput();
				nextPage = _config5();
				break;
		}
	SetupInput();
	m_cfg.save();
	m_cf.setBoxMode(m_cfg.getBool(" GENERAL", "box_mode"));
	_initCF();
}

int CMenu::_config1(void)
{
	int nextPage = 0;

	_showConfig();
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
		if (BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected() == m_configBtnPageP))
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
			else if (m_btnMgr.selected() == m_configBtnDownload)
			{
				_hideConfig();
				_download();
				_showConfig();
			}
			else if (m_btnMgr.selected() == m_configBtnUnlock)
			{
				char code[4];
				_hideConfig();
				if (_code(code) && memcmp(code, m_cfg.getString(" GENERAL", "parent_code", "").c_str(), 4) == 0)
					m_locked = false;
				_showConfig();
			}
			else if (m_btnMgr.selected() == m_configBtnSetCode)
			{
				char code[4];
				_hideConfig();
				if (_code(code, true))
				{
					m_cfg.setString(" GENERAL", "parent_code", string(code, 4).c_str());
					m_locked = true;
				}
				_showConfig();
			}
			else if (m_btnMgr.selected() == m_configBtnBoxMode)
			{
				m_cfg.setBool(" GENERAL", "box_mode", !m_cfg.getBool(" GENERAL", "box_mode"));
				_showConfig();
			}
			else if (m_btnMgr.selected() == m_configBtnRumble)
			{
				m_cfg.setBool(" GENERAL", "rumble", !m_cfg.getBool(" GENERAL", "rumble"));
				m_btnMgr.setRumble(m_cfg.getBool(" GENERAL", "rumble"));
				_showConfig();
			}
		}
	}
	_hideConfig();
	
	return nextPage;
}

void CMenu::_initConfigMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_configLblUser, ARRAY_SIZE(m_configLblUser), "CONFIG");
	m_configBg = _texture(theme.texSet, "CONFIG/BG", "texture", theme.bg);
	m_configLblTitle = _addLabel(theme, "CONFIG/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_configLblDownload = _addLabel(theme, "CONFIG/DOWNLOAD", theme.lblFont, L"", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtnDownload = _addButton(theme, "CONFIG/DOWNLOAD_BTN", theme.btnFont, L"", 400, 130, 200, 56, theme.btnFontColor);
	m_configLblParental = _addLabel(theme, "CONFIG/PARENTAL", theme.lblFont, L"", 40, 190, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtnUnlock = _addButton(theme, "CONFIG/UNLOCK_BTN", theme.btnFont, L"", 400, 190, 200, 56, theme.btnFontColor);
	m_configBtnSetCode = _addButton(theme, "CONFIG/SETCODE_BTN", theme.btnFont, L"", 400, 190, 200, 56, theme.btnFontColor);
	m_configLblBoxMode = _addLabel(theme, "CONFIG/BOXMODE", theme.lblFont, L"", 40, 250, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtnBoxMode = _addButton(theme, "CONFIG/BOXMODE_BTN", theme.btnFont, L"", 400, 250, 200, 56, theme.btnFontColor);
	m_configLblRumble = _addLabel(theme, "CONFIG/RUMBLE", theme.lblFont, L"", 40, 310, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtnRumble = _addButton(theme, "CONFIG/RUMBLE_BTN", theme.btnFont, L"", 400, 310, 200, 56, theme.btnFontColor);
	m_configLblPage = _addLabel(theme, "CONFIG/PAGE_BTN", theme.btnFont, L"", 76, 410, 80, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configBtnPageM = _addPicButton(theme, "CONFIG/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 410, 56, 56);
	m_configBtnPageP = _addPicButton(theme, "CONFIG/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 156, 410, 56, 56);
	m_configBtnBack = _addButton(theme, "CONFIG/BACK_BTN", theme.btnFont, L"", 420, 410, 200, 56, theme.btnFontColor);
	// 
	_setHideAnim(m_configLblTitle, "CONFIG/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_configLblBoxMode, "CONFIG/BOXMODE", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnBoxMode, "CONFIG/BOXMODE_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_configLblRumble, "CONFIG/RUMBLE", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnRumble, "CONFIG/RUMBLE_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_configLblDownload, "CONFIG/DOWNLOAD", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnDownload, "CONFIG/DOWNLOAD_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_configLblParental, "CONFIG/PARENTAL", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnUnlock, "CONFIG/UNLOCK_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnSetCode, "CONFIG/SETCODE_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnBack, "CONFIG/BACK_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_configLblPage, "CONFIG/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnPageM, "CONFIG/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnPageP, "CONFIG/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_hideConfig(true);
	_textConfig();
}

void CMenu::_textConfig(void)
{
	m_btnMgr.setText(m_configLblTitle, _t("cfg1", L"Settings"));
	m_btnMgr.setText(m_configLblBoxMode, _t("cfg2", L"3D boxes"));
	m_btnMgr.setText(m_configLblDownload, _t("cfg3", L"Download covers & titles"));
	m_btnMgr.setText(m_configBtnDownload, _t("cfg4", L"Download"));
	m_btnMgr.setText(m_configLblParental, _t("cfg5", L"Parental control"));
	m_btnMgr.setText(m_configBtnUnlock, _t("cfg6", L"Unlock"));
	m_btnMgr.setText(m_configBtnSetCode, _t("cfg7", L"Set code"));
	m_btnMgr.setText(m_configBtnBack, _t("cfg10", L"Back"));
	m_btnMgr.setText(m_configLblRumble, _t("cfg11", L"Rumble"));
}
