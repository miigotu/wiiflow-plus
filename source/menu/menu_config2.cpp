
#include "menu.hpp"


#define ARRAY_SIZE(a)	(sizeof a / sizeof a[0])

using namespace std;

static const int g_curPage = 5;

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

void CMenu::_hideConfig2(bool instant)
{
	m_btnMgr.hide(m_configLblTitle, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_configLblPage, instant);
	m_btnMgr.hide(m_configBtnPageM, instant);
	m_btnMgr.hide(m_configBtnPageP, instant);
	// 
	m_btnMgr.hide(m_config2LblGameLanguage, instant);
	m_btnMgr.hide(m_config2LblLanguage, instant);
	m_btnMgr.hide(m_config2BtnLanguageP, instant);
	m_btnMgr.hide(m_config2BtnLanguageM, instant);
	m_btnMgr.hide(m_config2LblGameVideo, instant);
	m_btnMgr.hide(m_config2LblVideo, instant);
	m_btnMgr.hide(m_config2BtnVideoP, instant);
	m_btnMgr.hide(m_config2BtnVideoM, instant);
	m_btnMgr.hide(m_config2LblErr2Fix, instant);
	m_btnMgr.hide(m_config2BtnErr2Fix, instant);
	m_btnMgr.hide(m_config2LblOcarina, instant);
	m_btnMgr.hide(m_config2BtnOcarina, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_config2LblUser); ++i)
		if (m_config2LblUser[i] != -1u)
			m_btnMgr.hide(m_config2LblUser[i], instant);
}

void CMenu::_showConfig2(void)
{
	_setBg(m_config2Bg, m_config2Bg);
	m_btnMgr.show(m_configLblTitle);
	m_btnMgr.show(m_configBtnBack);
	m_btnMgr.show(m_configLblPage);
	m_btnMgr.show(m_configBtnPageM);
	m_btnMgr.show(m_configBtnPageP);
	// 
	m_btnMgr.show(m_config2LblGameLanguage);
	m_btnMgr.show(m_config2LblLanguage);
	m_btnMgr.show(m_config2BtnLanguageP);
	m_btnMgr.show(m_config2BtnLanguageM);
	m_btnMgr.show(m_config2LblGameVideo);
	m_btnMgr.show(m_config2LblVideo);
	m_btnMgr.show(m_config2BtnVideoP);
	m_btnMgr.show(m_config2BtnVideoM);
	m_btnMgr.show(m_config2LblErr2Fix);
	m_btnMgr.show(m_config2BtnErr2Fix);
	m_btnMgr.show(m_config2LblOcarina);
	m_btnMgr.show(m_config2BtnOcarina);
	for (u32 i = 0; i < ARRAY_SIZE(m_config2LblUser); ++i)
		if (m_config2LblUser[i] != -1u)
			m_btnMgr.show(m_config2LblUser[i]);
	// 
	int i;
	m_btnMgr.setText(m_configLblPage, wfmt(L"%i / %i", g_curPage, m_locked ? g_curPage : CMenu::_nbCfgPages));
	i = min(max(0, m_cfg.getInt(" GENERAL", "video_mode", 0)), (int)ARRAY_SIZE(CMenu::_videoModes) - 1);
	m_btnMgr.setText(m_config2LblVideo, _t(CMenu::_videoModes[i].id, CMenu::_videoModes[i].text));
	i = min(max(0, m_cfg.getInt(" GENERAL", "game_language", 0)), (int)ARRAY_SIZE(CMenu::_languages) - 1);
	m_btnMgr.setText(m_config2LblLanguage, _t(CMenu::_languages[i].id, CMenu::_languages[i].text));
	m_btnMgr.setText(m_config2BtnErr2Fix, m_cfg.getBool(" GENERAL", "error_002_fix", true) ? _t("on", L"On") : _t("off", L"Off"));
	m_btnMgr.setText(m_config2BtnOcarina, m_cfg.getBool(" GENERAL", "cheat") ? _t("on", L"On") : _t("off", L"Off"));
}

int CMenu::_config2(void)
{
	int nextPage = 0;

	_showConfig2();
	while (true)
	{
		ScanInput();
		for(int wmote=0;wmote<4;wmote++)
			if (WPadIR_Valid(wmote))
				m_btnMgr.mouse(wd[wmote]->ir.x - m_cur.width() / 2, wd[wmote]->ir.y - m_cur.height() / 2);
		if ((wpadsState & (WPAD_BUTTON_HOME | WPAD_BUTTON_B)) != 0)
			break;
		else if ((wpadsState & WPAD_BUTTON_UP) != 0)
			m_btnMgr.up();
		else if ((wpadsState & WPAD_BUTTON_DOWN) != 0)
			m_btnMgr.down();
		if ((btn & WPAD_BUTTON_LEFT) != 0 || (wpadsState & WPAD_BUTTON_MINUS) != 0 || ((wpadsState & WPAD_BUTTON_A) != 0 && m_btnMgr.selected() == m_configBtnPageM))
		{
			nextPage = max(1, m_locked ? 1 : g_curPage - 1);
			m_btnMgr.click(m_configBtnPageM);
			break;
		}
		if (!m_locked && ((btn & WPAD_BUTTON_RIGHT) != 0 || (wpadsState & WPAD_BUTTON_PLUS) != 0 || ((wpadsState & WPAD_BUTTON_A) != 0 && m_btnMgr.selected() == m_configBtnPageP)))
		{
			nextPage = min(g_curPage + 1, CMenu::_nbCfgPages);
			m_btnMgr.click(m_configBtnPageP);
			break;
		}
		if ((wpadsState & WPAD_BUTTON_A) != 0)
		{
			m_btnMgr.click();
			if (m_btnMgr.selected() == m_configBtnBack)
				break;
			else if (m_btnMgr.selected() == m_config2BtnLanguageP)
			{
				m_cfg.setInt(" GENERAL", "game_language", (int)loopNum((u32)m_cfg.getInt(" GENERAL", "game_language", 0) + 1, ARRAY_SIZE(CMenu::_languages)));
				_showConfig2();
			}
			else if (m_btnMgr.selected() == m_config2BtnLanguageM)
			{
				m_cfg.setInt(" GENERAL", "game_language", (int)loopNum((u32)m_cfg.getInt(" GENERAL", "game_language", 0) - 1, ARRAY_SIZE(CMenu::_languages)));
				_showConfig2();
			}
			else if (m_btnMgr.selected() == m_config2BtnVideoP)
			{
				m_cfg.setInt(" GENERAL", "video_mode", (int)loopNum((u32)m_cfg.getInt(" GENERAL", "video_mode", 0) + 1, ARRAY_SIZE(CMenu::_videoModes)));
				_showConfig2();
			}
			else if (m_btnMgr.selected() == m_config2BtnVideoM)
			{
				m_cfg.setInt(" GENERAL", "video_mode", (int)loopNum((u32)m_cfg.getInt(" GENERAL", "video_mode", 0) - 1, ARRAY_SIZE(CMenu::_videoModes)));
				_showConfig2();
			}
			else if (m_btnMgr.selected() == m_config2BtnErr2Fix)
			{
				m_cfg.setBool(" GENERAL", "error_002_fix", !m_cfg.getBool(" GENERAL", "error_002_fix", true));
				_showConfig2();
			}
			else if (m_btnMgr.selected() == m_config2BtnOcarina)
			{
				m_cfg.setBool(" GENERAL", "cheat", !m_cfg.getBool(" GENERAL", "cheat"));
				_showConfig2();
			}
		}
		_mainLoopCommon(wd);
	}
	_hideConfig2();
	return nextPage;
}

void CMenu::_initConfig2Menu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_config2LblUser, ARRAY_SIZE(m_config2LblUser), "CONFIG2");
	m_config2Bg = _texture(theme.texSet, "CONFIG2/BG", "texture", theme.bg);
	m_config2LblGameVideo = _addLabel(theme, "CONFIG2/VIDEO", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config2LblVideo = _addLabel(theme, "CONFIG2/VIDEO_BTN", theme.btnFont, L"", 386, 130, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config2BtnVideoM = _addPicButton(theme, "CONFIG2/VIDEO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 130, 56, 56);
	m_config2BtnVideoP = _addPicButton(theme, "CONFIG2/VIDEO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);
	m_config2LblGameLanguage = _addLabel(theme, "CONFIG2/GAME_LANG", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config2LblLanguage = _addLabel(theme, "CONFIG2/GAME_LANG_BTN", theme.btnFont, L"", 386, 190, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config2BtnLanguageM = _addPicButton(theme, "CONFIG2/GAME_LANG_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 190, 56, 56);
	m_config2BtnLanguageP = _addPicButton(theme, "CONFIG2/GAME_LANG_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 190, 56, 56);
	m_config2LblErr2Fix = _addLabel(theme, "CONFIG2/ERR2FIX", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config2BtnErr2Fix = _addButton(theme, "CONFIG2/ERR2FIX_BTN", theme.btnFont, L"", 330, 250, 270, 56, theme.btnFontColor);
	m_config2LblOcarina = _addLabel(theme, "CONFIG2/OCARINA", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config2BtnOcarina = _addButton(theme, "CONFIG2/OCARINA_BTN", theme.btnFont, L"", 330, 310, 270, 56, theme.btnFontColor);
	// 
	_setHideAnim(m_config2LblGameVideo, "CONFIG2/VIDEO", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config2LblVideo, "CONFIG2/VIDEO_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config2BtnVideoM, "CONFIG2/VIDEO_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config2BtnVideoP, "CONFIG2/VIDEO_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config2LblGameLanguage, "CONFIG2/GAME_LANG", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config2LblLanguage, "CONFIG2/GAME_LANG_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config2BtnLanguageM, "CONFIG2/GAME_LANG_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config2BtnLanguageP, "CONFIG2/GAME_LANG_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config2LblErr2Fix, "CONFIG2/ERR2FIX", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config2BtnErr2Fix, "CONFIG2/ERR2FIX_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config2LblOcarina, "CONFIG2/OCARINA", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config2BtnOcarina, "CONFIG2/OCARINA_BTN", 0, 0, 1.f, -1.f);
	_hideConfig2(true);
	_textConfig2();
}

void CMenu::_textConfig2(void)
{
	m_btnMgr.setText(m_config2LblGameVideo, _t("cfgb3", L"Default video mode"));
	m_btnMgr.setText(m_config2LblGameLanguage, _t("cfgb4", L"Default game language"));
	m_btnMgr.setText(m_config2LblErr2Fix, _t("cfgd2", L"Error 002 fix"));
	m_btnMgr.setText(m_config2LblOcarina, _t("cfgb1", L"Ocarina"));
}
