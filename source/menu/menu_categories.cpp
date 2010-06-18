#include "menu.hpp"

#include <wiiuse/wpad.h>
#include <string.h>
#include <gccore.h>

void CMenu::_CategorySettings() {
	s32 padsState;
	WPADData *wd;
	u32 btn;
	WPAD_Rumble(WPAD_CHAN_0, 0);
	bool exitloop = false;

	_showCategorySettings();
	
	while (true)
	{
		exitloop=false;
		WPAD_ScanPads();
		padsState = WPAD_ButtonsDown(0);
		wd = WPAD_Data(0);
		btn = _btnRepeat(wd->btns_h);
		if ((padsState & (WPAD_BUTTON_HOME | WPAD_BUTTON_B)) != 0)
			break;
		if (wd->ir.valid)
			m_btnMgr.mouse(wd->ir.x - m_cur.width() / 2, wd->ir.y - m_cur.height() / 2);
		else if ((padsState & WPAD_BUTTON_UP) != 0)
			m_btnMgr.up();
		else if ((padsState & WPAD_BUTTON_DOWN) != 0)
			m_btnMgr.down();
		if ((padsState & WPAD_BUTTON_A) != 0)
		{
			m_btnMgr.click();
			if (m_btnMgr.selected() == m_categoryBtnBack)
				break;
			for (int i = 0; i < 12; ++i) {
				if (m_btnMgr.selected() == m_categoryBtn[i]) {
					// handling code for clicked favorite
					m_category = i;
					m_cfg.setInt(" GENERAL", "category", i);
					exitloop = true;
					break;
				}
			}
			if (exitloop) {
				break;
			}
		}
		_mainLoopCommon(wd);
	}
	_hideCategorySettings();
}

void CMenu::_hideCategorySettings(bool instant)
{
	m_btnMgr.hide(m_categoryBtnBack,instant);
	for (int i = 0; i < 12; ++i)
		m_btnMgr.hide(m_categoryBtn[i],instant);
}

void CMenu::_showCategorySettings(void)
{
	m_btnMgr.show(m_categoryBtnBack);
	for (int i = 0; i < 12; ++i)
		m_btnMgr.show(m_categoryBtn[i]);
}


void CMenu::_initCategorySettingsMenu(CMenu::SThemeData &theme)
{
	m_categoryBtnBack = _addButton(theme, "CATEGORY/BACK_BTN", theme.btnFont, L"", 420, 410, 200, 56, theme.btnFontColor);
	m_categoryBtn[0] = _addButton(theme, "CATEGORY/ALL_BTN", theme.btnFont, L"",  60, 40, 200, 50, theme.btnFontColor);
	m_categoryBtn[1] = _addButton(theme, "CATEGORY/1_BTN", theme.btnFont, L"", 340, 40, 200, 50, theme.btnFontColor);
	m_categoryBtn[2] = _addButton(theme, "CATEGORY/2_BTN", theme.btnFont, L"",  60, 100, 200, 50, theme.btnFontColor);
	m_categoryBtn[3] = _addButton(theme, "CATEGORY/3_BTN", theme.btnFont, L"", 340, 100, 200, 50, theme.btnFontColor);
	m_categoryBtn[4] = _addButton(theme, "CATEGORY/4_BTN", theme.btnFont, L"",  60, 160, 200, 50, theme.btnFontColor);
	m_categoryBtn[5] = _addButton(theme, "CATEGORY/5_BTN", theme.btnFont, L"", 340, 160, 200, 50, theme.btnFontColor);
	m_categoryBtn[6] = _addButton(theme, "CATEGORY/6_BTN", theme.btnFont, L"",  60, 220, 200, 50, theme.btnFontColor);
	m_categoryBtn[7] = _addButton(theme, "CATEGORY/7_BTN", theme.btnFont, L"", 340, 220, 200, 50, theme.btnFontColor);
	m_categoryBtn[8] = _addButton(theme, "CATEGORY/8_BTN", theme.btnFont, L"",  60, 280, 200, 50, theme.btnFontColor);
	m_categoryBtn[9] = _addButton(theme, "CATEGORY/9_BTN", theme.btnFont, L"", 340, 280, 200, 50, theme.btnFontColor);
	m_categoryBtn[10] = _addButton(theme, "CATEGORY/10_BTN", theme.btnFont, L"", 60, 340, 200, 50, theme.btnFontColor);
	m_categoryBtn[11] = _addButton(theme, "CATEGORY/11_BTN", theme.btnFont, L"",340, 340, 200, 50, theme.btnFontColor);

	_setHideAnim(m_categoryBtnBack, "CATEGORY/BACK_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_categoryBtn[0], "CATEGORY/ALL_BTN", 0, 0, 0.f, 0.f);
	for (int i = 1; i < 12; ++i)
		_setHideAnim(m_categoryBtn[i], sfmt("CATEGORY/%i_BTN", i).c_str(), 0, 0, 0.f, 0.f);
	
	_hideCategorySettings(true);
	_textCategorySettings();
}

void CMenu::_textCategorySettings(void)
{
	m_btnMgr.setText(m_categoryBtnBack, _t("cd1", L"Back"));
	m_btnMgr.setText(m_categoryBtn[0], _t("dl3", L"All"));
	for (int i = 1; i < 12; i++)
		m_btnMgr.setText(m_categoryBtn[i], m_cfg.getWString(" GENERAL", fmt("cat%d",i), m_loc.getWString(m_curLanguage, fmt("cat%d", i), wfmt(L"Category %i",i).c_str())));
}

