
#include "menu.hpp"


using namespace std;

static const int g_curPage = 6;

void CMenu::_hideConfig3(bool instant)
{
	m_btnMgr.hide(m_configLblTitle, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_configLblPage, instant);
	m_btnMgr.hide(m_configBtnPageM, instant);
	m_btnMgr.hide(m_configBtnPageP, instant);
	// 
	m_btnMgr.hide(m_config3LblTVHeight, instant);
	m_btnMgr.hide(m_config3LblTVHeightVal, instant);
	m_btnMgr.hide(m_config3BtnTVHeightP, instant);
	m_btnMgr.hide(m_config3BtnTVHeightM, instant);
	m_btnMgr.hide(m_config3LblTVWidth, instant);
	m_btnMgr.hide(m_config3LblTVWidthVal, instant);
	m_btnMgr.hide(m_config3BtnTVWidthP, instant);
	m_btnMgr.hide(m_config3BtnTVWidthM, instant);
	m_btnMgr.hide(m_config3LblTVX, instant);
	m_btnMgr.hide(m_config3LblTVXVal, instant);
	m_btnMgr.hide(m_config3BtnTVXM, instant);
	m_btnMgr.hide(m_config3BtnTVXP, instant);
	m_btnMgr.hide(m_config3LblTVY, instant);
	m_btnMgr.hide(m_config3LblTVYVal, instant);
	m_btnMgr.hide(m_config3BtnTVYM, instant);
	m_btnMgr.hide(m_config3BtnTVYP, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_config3LblUser); ++i)
		if (m_config3LblUser[i] != -1u)
			m_btnMgr.hide(m_config3LblUser[i], instant);
}

void CMenu::_showConfig3(void)
{
	_setBg(m_config3Bg, m_config3Bg);
	m_btnMgr.show(m_configLblTitle);
	m_btnMgr.show(m_configBtnBack);
	m_btnMgr.show(m_configLblPage);
	m_btnMgr.show(m_configBtnPageM);
	m_btnMgr.show(m_configBtnPageP);
	// 
	m_btnMgr.show(m_config3LblTVHeight);
	m_btnMgr.show(m_config3LblTVHeightVal);
	m_btnMgr.show(m_config3BtnTVHeightP);
	m_btnMgr.show(m_config3BtnTVHeightM);
	m_btnMgr.show(m_config3LblTVWidth);
	m_btnMgr.show(m_config3LblTVWidthVal);
	m_btnMgr.show(m_config3BtnTVWidthP);
	m_btnMgr.show(m_config3BtnTVWidthM);
	m_btnMgr.show(m_config3LblTVX);
	m_btnMgr.show(m_config3LblTVXVal);
	m_btnMgr.show(m_config3BtnTVXM);
	m_btnMgr.show(m_config3BtnTVXP);
	m_btnMgr.show(m_config3LblTVY);
	m_btnMgr.show(m_config3LblTVYVal);
	m_btnMgr.show(m_config3BtnTVYM);
	m_btnMgr.show(m_config3BtnTVYP);
	for (u32 i = 0; i < ARRAY_SIZE(m_config3LblUser); ++i)
		if (m_config3LblUser[i] != -1u)
			m_btnMgr.show(m_config3LblUser[i]);
	// 
	m_btnMgr.setText(m_configLblPage, wfmt(L"%i / %i", g_curPage, m_locked ? g_curPage : CMenu::_nbCfgPages));
	m_btnMgr.setText(m_config3LblTVWidthVal, wfmt(L"%i", 640 * 640 / max(1, m_cfg.getInt("GENERAL", "tv_width", 640))));
	m_btnMgr.setText(m_config3LblTVHeightVal, wfmt(L"%i", 480 * 480 / max(1, m_cfg.getInt("GENERAL", "tv_height", 480))));
	m_btnMgr.setText(m_config3LblTVXVal, wfmt(L"%i", -m_cfg.getInt("GENERAL", "tv_x", 0)));
	m_btnMgr.setText(m_config3LblTVYVal, wfmt(L"%i", m_cfg.getInt("GENERAL", "tv_y", 0)));
}

int CMenu::_config3(void)
{
	int nextPage = 0;
	SetupInput();

	_showConfig3();
	while (true)
	{
		_mainLoopCommon();
		if (BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if (BTN_UP_PRESSED)
			m_btnMgr.up();
		else if (BTN_DOWN_PRESSED)
			m_btnMgr.down();
		++repeatButton;
		if ((wii_btnsHeld & WBTN_A) == 0)
			buttonHeld = (u32)-1;
		else if (buttonHeld != (u32)-1 && buttonHeld == m_btnMgr.selected() && repeatButton >= 16 && (repeatButton % 4 == 0))
			wii_btnsPressed |= WBTN_A;
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
			else if (m_btnMgr.selected() == m_config3BtnTVWidthP || m_btnMgr.selected() == m_config3BtnTVWidthM
				|| m_btnMgr.selected() == m_config3BtnTVHeightP || m_btnMgr.selected() == m_config3BtnTVHeightM
				|| m_btnMgr.selected() == m_config3BtnTVXP || m_btnMgr.selected() == m_config3BtnTVXM
				|| m_btnMgr.selected() == m_config3BtnTVYP || m_btnMgr.selected() == m_config3BtnTVYM)
			{
				int step = 0;
				if (m_btnMgr.selected() == m_config3BtnTVWidthM || m_btnMgr.selected() == m_config3BtnTVHeightM)
					step = 2;
				else if (m_btnMgr.selected() == m_config3BtnTVWidthP || m_btnMgr.selected() == m_config3BtnTVHeightP)
					step = -2;
				else if (m_btnMgr.selected() == m_config3BtnTVXP || m_btnMgr.selected() == m_config3BtnTVYM)
					step = -1;
				else if (m_btnMgr.selected() == m_config3BtnTVXM || m_btnMgr.selected() == m_config3BtnTVYP)
					step = 1;
				if (m_btnMgr.selected() == m_config3BtnTVWidthM || m_btnMgr.selected() == m_config3BtnTVWidthP)
					m_cfg.setInt("GENERAL", "tv_width", min(max(512, m_cfg.getInt("GENERAL", "tv_width", 640) + step), 800));
				else if (m_btnMgr.selected() == m_config3BtnTVHeightM || m_btnMgr.selected() == m_config3BtnTVHeightP)
					m_cfg.setInt("GENERAL", "tv_height", min(max(384, m_cfg.getInt("GENERAL", "tv_height", 480) + step), 600));
				else if (m_btnMgr.selected() == m_config3BtnTVXP || m_btnMgr.selected() == m_config3BtnTVXM)
					m_cfg.setInt("GENERAL", "tv_x", min(max(-50, m_cfg.getInt("GENERAL", "tv_x", 0) + step), 50));
				else if (m_btnMgr.selected() == m_config3BtnTVYP || m_btnMgr.selected() == m_config3BtnTVYM)
					m_cfg.setInt("GENERAL", "tv_y", min(max(-30, m_cfg.getInt("GENERAL", "tv_y", 0) + step), 30));
				_showConfig3();
				m_vid.set2DViewport(m_cfg.getInt("GENERAL", "tv_width", 640), m_cfg.getInt("GENERAL", "tv_height", 480), m_cfg.getInt("GENERAL", "tv_x", 0), m_cfg.getInt("GENERAL", "tv_y", 0));
				if (buttonHeld != m_btnMgr.selected())
				{
					repeatButton = 0;
					buttonHeld = m_btnMgr.selected();
				}
			}
		}
	}
	_hideConfig3();
	return nextPage;
}

void CMenu::_initConfig3Menu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_config3LblUser, ARRAY_SIZE(m_config3LblUser), "CONFIG3");
	m_config3Bg = _texture(theme.texSet, "CONFIG3/BG", "texture", theme.bg);
	m_config3LblTVWidth = _addLabel(theme, "CONFIG3/TVWIDTH", theme.lblFont, L"", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3LblTVWidthVal = _addLabel(theme, "CONFIG3/TVWIDTH_BTN", theme.btnFont, L"", 426, 130, 118, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config3BtnTVWidthM = _addPicButton(theme, "CONFIG3/TVWIDTH_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 370, 130, 56, 56);
	m_config3BtnTVWidthP = _addPicButton(theme, "CONFIG3/TVWIDTH_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);
	m_config3LblTVHeight = _addLabel(theme, "CONFIG3/TVHEIGHT", theme.lblFont, L"", 40, 190, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3LblTVHeightVal = _addLabel(theme, "CONFIG3/TVHEIGHT_BTN", theme.btnFont, L"", 426, 190, 118, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config3BtnTVHeightM = _addPicButton(theme, "CONFIG3/TVHEIGHT_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 370, 190, 56, 56);
	m_config3BtnTVHeightP = _addPicButton(theme, "CONFIG3/TVHEIGHT_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 190, 56, 56);
	m_config3LblTVX = _addLabel(theme, "CONFIG3/TVX", theme.lblFont, L"", 40, 250, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3LblTVXVal = _addLabel(theme, "CONFIG3/TVX_BTN", theme.btnFont, L"", 426, 250, 118, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config3BtnTVXM = _addPicButton(theme, "CONFIG3/TVX_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 370, 250, 56, 56);
	m_config3BtnTVXP = _addPicButton(theme, "CONFIG3/TVX_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 250, 56, 56);
	m_config3LblTVY = _addLabel(theme, "CONFIG3/TVY", theme.lblFont, L"", 40, 310, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3LblTVYVal = _addLabel(theme, "CONFIG3/TVY_BTN", theme.btnFont, L"", 426, 310, 118, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config3BtnTVYM = _addPicButton(theme, "CONFIG3/TVY_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 370, 310, 56, 56);
	m_config3BtnTVYP = _addPicButton(theme, "CONFIG3/TVY_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 310, 56, 56);
	// 
	_setHideAnim(m_config3LblTVWidth, "CONFIG3/TVWIDTH", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config3LblTVWidthVal, "CONFIG3/TVWIDTH_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3BtnTVWidthM, "CONFIG3/TVWIDTH_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3BtnTVWidthP, "CONFIG3/TVWIDTH_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3LblTVHeight, "CONFIG3/TVHEIGHT", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config3LblTVHeightVal, "CONFIG3/TVHEIGHT_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3BtnTVHeightM, "CONFIG3/TVHEIGHT_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3BtnTVHeightP, "CONFIG3/TVHEIGHT_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3LblTVX, "CONFIG3/TVX", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config3LblTVXVal, "CONFIG3/TVX_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3BtnTVXM, "CONFIG3/TVX_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3BtnTVXP, "CONFIG3/TVX_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3LblTVY, "CONFIG3/TVY", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config3LblTVYVal, "CONFIG3/TVY_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3BtnTVYM, "CONFIG3/TVY_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3BtnTVYP, "CONFIG3/TVY_PLUS", 0, 0, 1.f, -1.f);
	_hideConfig3(true);
	_textConfig3();
}

void CMenu::_textConfig3(void)
{
	m_btnMgr.setText(m_config3LblTVWidth, _t("cfgc2", L"Adjust TV width"));
	m_btnMgr.setText(m_config3LblTVHeight, _t("cfgc3", L"Adjust TV height"));
	m_btnMgr.setText(m_config3LblTVX, _t("cfgc6", L"Horizontal offset"));
	m_btnMgr.setText(m_config3LblTVY, _t("cfgc7", L"Vertical offset"));
}
