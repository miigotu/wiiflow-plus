

#include "loader/wbfs.h"
#include "libwbfs/wiidisc.h"
#include "menu.hpp"
#include "loader/fs.h"


#define ARRAY_SIZE(a)	(sizeof a / sizeof a[0])

using namespace std;

static inline int loopNum(int i, int s)
{
	return i < 0 ? (s - (-i % s)) % s : i % s;
}

u8 m_gameSettingCategories[12];

void CMenu::_hideGameSettings(bool instant)
{
	m_btnMgr.hide(m_gameSettingsLblPage, instant);
	m_btnMgr.hide(m_gameSettingsBtnPageM, instant);
	m_btnMgr.hide(m_gameSettingsBtnPageP, instant);
	m_btnMgr.hide(m_gameSettingsBtnBack, instant);
	m_btnMgr.hide(m_gameSettingsLblTitle, instant);
	m_btnMgr.hide(m_gameSettingsLblGameLanguage, instant);
	m_btnMgr.hide(m_gameSettingsLblLanguage, instant);
	m_btnMgr.hide(m_gameSettingsBtnLanguageP, instant);
	m_btnMgr.hide(m_gameSettingsBtnLanguageM, instant);
	m_btnMgr.hide(m_gameSettingsLblGameVideo, instant);
	m_btnMgr.hide(m_gameSettingsLblVideo, instant);
	m_btnMgr.hide(m_gameSettingsBtnVideoP, instant);
	m_btnMgr.hide(m_gameSettingsBtnVideoM, instant);
	m_btnMgr.hide(m_gameSettingsLblGameAltDol, instant);
	m_btnMgr.hide(m_gameSettingsLblAltDol, instant);
	m_btnMgr.hide(m_gameSettingsBtnAltDolP, instant);
	m_btnMgr.hide(m_gameSettingsBtnAltDolM, instant);
	m_btnMgr.hide(m_gameSettingsLblOcarina, instant);
	m_btnMgr.hide(m_gameSettingsBtnOcarina, instant);
	m_btnMgr.hide(m_gameSettingsLblCheat, instant);
	m_btnMgr.hide(m_gameSettingsBtnCheat, instant);
	m_btnMgr.hide(m_gameSettingsLblVipatch, instant);
	m_btnMgr.hide(m_gameSettingsBtnVipatch, instant);
	m_btnMgr.hide(m_gameSettingsLblCountryPatch, instant);
	m_btnMgr.hide(m_gameSettingsBtnCountryPatch, instant);
	m_btnMgr.hide(m_gameSettingsLblErr2Fix, instant);
	m_btnMgr.hide(m_gameSettingsBtnErr2Fix, instant);
	m_btnMgr.hide(m_gameSettingsLblGameIOS, instant);
	m_btnMgr.hide(m_gameSettingsLblIOS, instant);
	m_btnMgr.hide(m_gameSettingsBtnIOSP, instant);
	m_btnMgr.hide(m_gameSettingsBtnIOSM, instant);
	m_btnMgr.hide(m_gameSettingsLblCover, instant);
	m_btnMgr.hide(m_gameSettingsBtnCover, instant);
	m_btnMgr.hide(m_gameSettingsLblBlockIOSReload, instant);
	m_btnMgr.hide(m_gameSettingsBtnBlockIOSReload, instant);
	m_btnMgr.hide(m_gameSettingsLblPatchVidModes, instant);
	m_btnMgr.hide(m_gameSettingsLblPatchVidModesVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnPatchVidModesM, instant);
	m_btnMgr.hide(m_gameSettingsBtnPatchVidModesP, instant);
	m_btnMgr.hide(m_gameSettingsLblHooktype, instant);
	m_btnMgr.hide(m_gameSettingsLblHooktypeVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnHooktypeM, instant);
	m_btnMgr.hide(m_gameSettingsBtnHooktypeP, instant);
	m_btnMgr.hide(m_gameSettingsBtnCategoryMain, instant);
	m_btnMgr.hide(m_gameSettingsLblCategoryMain, instant);
	m_btnMgr.hide(m_gameSettingsLblDvdPatch, instant);
	m_btnMgr.hide(m_gameSettingsBtnDvdPatch, instant);
	m_btnMgr.hide(m_gameSettingsBtnReturnTo, instant);
	m_btnMgr.hide(m_gameSettingsLblReturnTo, instant);
	m_btnMgr.hide(m_gameSettingsLblDebugger, instant);
	m_btnMgr.hide(m_gameSettingsLblDebuggerV, instant);
	m_btnMgr.hide(m_gameSettingsBtnDebuggerP, instant);
	m_btnMgr.hide(m_gameSettingsBtnDebuggerM, instant);

	for (int i = 0; i < 12; ++i) {
		m_btnMgr.hide(m_gameSettingsBtnCategory[i], instant); 
		m_btnMgr.hide(m_gameSettingsLblCategory[i], instant); 
	}
	for (u32 i = 0; i < ARRAY_SIZE(m_gameSettingsLblUser); ++i)
		if (m_gameSettingsLblUser[i] != -1u)
			m_btnMgr.hide(m_gameSettingsLblUser[i], instant);
}

wstringEx CMenu::_optBoolToString(int i)
{
	switch (i)
	{
		case 0:
			return _t("off", L"Off");
		case 1:
			return _t("on", L"On");
		default:
			return _t("def", L"Default");
	}
}

void CMenu::_showGameSettings(void)
{
	int i;
	wstringEx title(_t("cfgg1", L"Settings"));
	title += L" [";
	title += wstringEx(m_cf.getId());
	title += L"]";
	m_btnMgr.setText(m_gameSettingsLblTitle, title);
	_setBg(m_gameSettingsBg, m_gameSettingsBg);
	m_btnMgr.show(m_gameSettingsLblPage);
	m_btnMgr.show(m_gameSettingsBtnPageM);
	m_btnMgr.show(m_gameSettingsBtnPageP);
	m_btnMgr.show(m_gameSettingsBtnBack);
	m_btnMgr.show(m_gameSettingsLblTitle);
	if (m_gameSettingsPage == 1)
	{
		m_btnMgr.show(m_gameSettingsLblGameLanguage);
		m_btnMgr.show(m_gameSettingsLblLanguage);
		m_btnMgr.show(m_gameSettingsBtnLanguageP);
		m_btnMgr.show(m_gameSettingsBtnLanguageM);
		m_btnMgr.show(m_gameSettingsLblOcarina);
		m_btnMgr.show(m_gameSettingsBtnOcarina);
		m_btnMgr.show(m_gameSettingsLblGameAltDol);
		m_btnMgr.show(m_gameSettingsLblAltDol);
		m_btnMgr.show(m_gameSettingsBtnAltDolP);
		m_btnMgr.show(m_gameSettingsBtnAltDolM);
		m_btnMgr.show(m_gameSettingsLblCheat);
		m_btnMgr.show(m_gameSettingsBtnCheat);
	}
	else
	{
		m_btnMgr.hide(m_gameSettingsLblGameLanguage);
		m_btnMgr.hide(m_gameSettingsLblLanguage);
		m_btnMgr.hide(m_gameSettingsBtnLanguageP);
		m_btnMgr.hide(m_gameSettingsBtnLanguageM);
		m_btnMgr.hide(m_gameSettingsLblOcarina);
		m_btnMgr.hide(m_gameSettingsBtnOcarina);
		m_btnMgr.hide(m_gameSettingsLblGameAltDol);
		m_btnMgr.hide(m_gameSettingsLblAltDol);
		m_btnMgr.hide(m_gameSettingsBtnAltDolP);
		m_btnMgr.hide(m_gameSettingsBtnAltDolM);
		m_btnMgr.hide(m_gameSettingsLblCheat);
		m_btnMgr.hide(m_gameSettingsBtnCheat);
	}
	if (m_gameSettingsPage == 2)
	{
		m_btnMgr.show(m_gameSettingsLblVipatch);
		m_btnMgr.show(m_gameSettingsBtnVipatch);
		m_btnMgr.show(m_gameSettingsLblCountryPatch);
		m_btnMgr.show(m_gameSettingsBtnCountryPatch);
		m_btnMgr.show(m_gameSettingsLblErr2Fix);
		m_btnMgr.show(m_gameSettingsBtnErr2Fix);
		m_btnMgr.show(m_gameSettingsLblGameIOS);
		m_btnMgr.show(m_gameSettingsLblIOS);
		m_btnMgr.show(m_gameSettingsBtnIOSP);
		m_btnMgr.show(m_gameSettingsBtnIOSM);
	}
	else
	{
		m_btnMgr.hide(m_gameSettingsLblVipatch);
		m_btnMgr.hide(m_gameSettingsBtnVipatch);
		m_btnMgr.hide(m_gameSettingsLblCountryPatch);
		m_btnMgr.hide(m_gameSettingsBtnCountryPatch);
		m_btnMgr.hide(m_gameSettingsLblErr2Fix);
		m_btnMgr.hide(m_gameSettingsBtnErr2Fix);
		m_btnMgr.hide(m_gameSettingsLblGameIOS);
		m_btnMgr.hide(m_gameSettingsLblIOS);
		m_btnMgr.hide(m_gameSettingsBtnIOSP);
		m_btnMgr.hide(m_gameSettingsBtnIOSM);
	}
	if (m_gameSettingsPage == 3)
	{
		m_btnMgr.show(m_gameSettingsLblCover);
		m_btnMgr.show(m_gameSettingsBtnCover);
		m_btnMgr.show(m_gameSettingsLblBlockIOSReload);
		m_btnMgr.show(m_gameSettingsBtnBlockIOSReload);
		m_btnMgr.show(m_gameSettingsLblPatchVidModes);
		m_btnMgr.show(m_gameSettingsLblPatchVidModesVal);
		m_btnMgr.show(m_gameSettingsBtnPatchVidModesM);
		m_btnMgr.show(m_gameSettingsBtnPatchVidModesP);
		m_btnMgr.show(m_gameSettingsLblGameVideo);
		m_btnMgr.show(m_gameSettingsLblVideo);
		m_btnMgr.show(m_gameSettingsBtnVideoP);
		m_btnMgr.show(m_gameSettingsBtnVideoM);
	}
	else
	{
		m_btnMgr.hide(m_gameSettingsLblCover);
		m_btnMgr.hide(m_gameSettingsBtnCover);
		m_btnMgr.hide(m_gameSettingsLblBlockIOSReload);
		m_btnMgr.hide(m_gameSettingsBtnBlockIOSReload);
		m_btnMgr.hide(m_gameSettingsLblPatchVidModes);
		m_btnMgr.hide(m_gameSettingsLblPatchVidModesVal);
		m_btnMgr.hide(m_gameSettingsBtnPatchVidModesM);
		m_btnMgr.hide(m_gameSettingsBtnPatchVidModesP);
		m_btnMgr.hide(m_gameSettingsLblGameVideo);
		m_btnMgr.hide(m_gameSettingsLblVideo);
		m_btnMgr.hide(m_gameSettingsBtnVideoP);
		m_btnMgr.hide(m_gameSettingsBtnVideoM);
	}
	if (m_gameSettingsPage == 4)
	{
		m_btnMgr.show(m_gameSettingsBtnCategoryMain);
		m_btnMgr.show(m_gameSettingsLblCategoryMain);
		m_btnMgr.show(m_gameSettingsLblHooktype);
		m_btnMgr.show(m_gameSettingsLblHooktypeVal);
		m_btnMgr.show(m_gameSettingsBtnHooktypeM);
		m_btnMgr.show(m_gameSettingsBtnHooktypeP);
		m_btnMgr.show(m_gameSettingsLblDvdPatch);
		m_btnMgr.show(m_gameSettingsBtnDvdPatch);
		m_btnMgr.show(m_gameSettingsBtnReturnTo);
		m_btnMgr.show(m_gameSettingsLblReturnTo);

	}
	else
	{
		m_btnMgr.hide(m_gameSettingsBtnCategoryMain);
		m_btnMgr.hide(m_gameSettingsLblCategoryMain);
		m_btnMgr.hide(m_gameSettingsLblHooktype);
		m_btnMgr.hide(m_gameSettingsLblHooktypeVal);
		m_btnMgr.hide(m_gameSettingsBtnHooktypeM);
		m_btnMgr.hide(m_gameSettingsBtnHooktypeP);
		m_btnMgr.hide(m_gameSettingsLblDvdPatch);
		m_btnMgr.hide(m_gameSettingsBtnDvdPatch);
		m_btnMgr.hide(m_gameSettingsBtnReturnTo);
		m_btnMgr.hide(m_gameSettingsLblReturnTo);
	}
	if (m_gameSettingsPage == 5)
	{
		m_btnMgr.show(m_gameSettingsLblDebugger);
		m_btnMgr.show(m_gameSettingsLblDebuggerV);
		m_btnMgr.show(m_gameSettingsBtnDebuggerP);
		m_btnMgr.show(m_gameSettingsBtnDebuggerM);
	}
	else
	{
		m_btnMgr.hide(m_gameSettingsLblDebugger);
		m_btnMgr.hide(m_gameSettingsLblDebuggerV);
		m_btnMgr.hide(m_gameSettingsBtnDebuggerP);
		m_btnMgr.hide(m_gameSettingsBtnDebuggerM);
	}


	//Categories Pages
	if (m_gameSettingsPage == 51)
	{
		for (u32 i = 1;i < (u32)min(m_max_categories+1, 5);++i) {
			m_btnMgr.show(m_gameSettingsBtnCategory[i]);
			m_btnMgr.show(m_gameSettingsLblCategory[i]);
		}		
	}
	else
	{
		for (int i = 1;i < 5;++i) {
			m_btnMgr.hide(m_gameSettingsBtnCategory[i]);
			m_btnMgr.hide(m_gameSettingsLblCategory[i]);
		}		
	}
	if (m_gameSettingsPage == 52)
	{
		for (u32 i = 5;i < (u32)min(m_max_categories+1, 9);++i) {
			m_btnMgr.show(m_gameSettingsBtnCategory[i]);
			m_btnMgr.show(m_gameSettingsLblCategory[i]);
		}		
	}
	else
	{
		for (int i = 5;i < 9;++i) {
			m_btnMgr.hide(m_gameSettingsBtnCategory[i]);
			m_btnMgr.hide(m_gameSettingsLblCategory[i]);
		}		
	}
	if (m_gameSettingsPage == 53)
	{
		for (u32 i = 9;i < (u32)min(m_max_categories+1, 12);++i) {
			m_btnMgr.show(m_gameSettingsBtnCategory[i]);
			m_btnMgr.show(m_gameSettingsLblCategory[i]);
		}		
	}
	else
	{
		for (int i = 9;i < 12;++i) {
			m_btnMgr.hide(m_gameSettingsBtnCategory[i]);
			m_btnMgr.hide(m_gameSettingsLblCategory[i]);
		}	
	}
	
	for (u32 i = 0; i < ARRAY_SIZE(m_gameSettingsLblUser); ++i)
		if (m_gameSettingsLblUser[i] != -1u)
			m_btnMgr.show(m_gameSettingsLblUser[i]);

	string id(m_cf.getId());
	int page = m_gameSettingsPage;
	int maxpage = 5;
	if (m_gameSettingsPage > 5)
		page = m_gameSettingsPage-50;
	if (m_gameSettingsPage > 5)
		maxpage = 3;

	m_btnMgr.setText(m_gameSettingsLblPage, wfmt(L"%i / %i", page, maxpage));
	m_btnMgr.setText(m_gameSettingsBtnOcarina, _optBoolToString(m_cfg.getOptBool(id, "cheat")));
	m_btnMgr.setText(m_gameSettingsBtnVipatch, _optBoolToString(m_cfg.getOptBool(id, "vipatch", 0)));
	m_btnMgr.setText(m_gameSettingsBtnCountryPatch, _optBoolToString(m_cfg.getOptBool(id, "country_patch", 0)));
	m_btnMgr.setText(m_gameSettingsBtnErr2Fix, _optBoolToString(m_cfg.getOptBool(id, "error_002_fix")));
	m_btnMgr.setText(m_gameSettingsBtnBlockIOSReload, m_cfg.getBool(id, "block_ios_reload", false) ? _t("on", L"On") : _t("off", L"Off"));
	i = min((u32)m_cfg.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_videoModes) - 1u);
	m_btnMgr.setText(m_gameSettingsLblVideo, _t(CMenu::_videoModes[i].id, CMenu::_videoModes[i].text));
	i = min((u32)m_cfg.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
	m_btnMgr.setText(m_gameSettingsLblLanguage, _t(CMenu::_languages[i].id, CMenu::_languages[i].text));
	m_btnMgr.setText(m_gameSettingsLblAltDol, m_cfg.getString(id, "dol", "main.dol"));
	i = min((u32)m_cfg.getInt(id, "ios", 0), ARRAY_SIZE(CMenu::_ios) - 1u);
	if (i == 0)
		m_btnMgr.setText(m_gameSettingsLblIOS, _t("def", L"Default"));
	else
		m_btnMgr.setText(m_gameSettingsLblIOS, wstringEx(sfmt("%i", CMenu::_ios[i])));
	i = min((u32)m_cfg.getInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
	m_btnMgr.setText(m_gameSettingsLblPatchVidModesVal, _t(CMenu::_vidModePatch[i].id, CMenu::_vidModePatch[i].text));

	i = min((u32)m_cfg.getInt(id, "hooktype", 1), ARRAY_SIZE(CMenu::_hooktype) - 1u);
	m_btnMgr.setText(m_gameSettingsLblHooktypeVal, _t(CMenu::_hooktype[i].id, CMenu::_hooktype[i].text));

	m_btnMgr.setText(m_gameSettingsBtnCategoryMain, _fmt("cfgg16",  wfmt(L"Select",i).c_str() )); 
	
	char *categories = (char *) m_cfg.getString(id, "categories", "").c_str();
	memset(&m_gameSettingCategories, '0', sizeof(m_gameSettingCategories));
	if (strlen(categories) == sizeof(m_gameSettingCategories))
	{
		memcpy(&m_gameSettingCategories, categories, sizeof(m_gameSettingCategories));
	}
	for (int i=0; i<12; ++i) {
		m_btnMgr.setText(m_gameSettingsBtnCategory[i], _optBoolToString(m_gameSettingCategories[i] == '1'));
	}
	m_btnMgr.setText(m_gameSettingsBtnDvdPatch,  m_cfg.getBool(id, "disable_dvd_patch", false) ? _t("on", L"On") : _t("def", L"Default"));
	m_btnMgr.setText(m_gameSettingsBtnReturnTo,  m_cfg.getBool(id, "returnto", true) ? _t("def", L"Default") : _t("on", L"On"));		
	m_btnMgr.setText(m_gameSettingsLblDebuggerV, m_cfg.getBool(id, "debugger", false) ? _t("gecko", L"Gecko") : _t("def", L"Default"));		
}

void CMenu::_gameSettings(void)
{
	bool dolsListed = false;
	vector<string> dols;
	string id(m_cf.getId());

	m_gameSettingsPage = 1;
	dols.push_back(m_cfg.getString(id, "dol", "main.dol"));
	_showGameSettings();
	while (true)
	{
		ScanInput();
		if ((padsState & (WPAD_BUTTON_HOME | WPAD_BUTTON_B)) != 0)
			break;
		if (wd->ir.valid)
			m_btnMgr.mouse(wd->ir.x - m_cur.width() / 2, wd->ir.y - m_cur.height() / 2);
		else if ((padsState & WPAD_BUTTON_UP) != 0)
			m_btnMgr.up();
		else if ((padsState & WPAD_BUTTON_DOWN) != 0)
			m_btnMgr.down();
		if ((padsState & WPAD_BUTTON_MINUS) != 0 || (btn & WPAD_BUTTON_LEFT) != 0)
		{
			if ((m_gameSettingsPage > 1 && m_gameSettingsPage < 6) || m_gameSettingsPage > 51)
				--m_gameSettingsPage;
			_showGameSettings();
			m_btnMgr.click(m_gameSettingsBtnPageM);
		}
		else if ((padsState & WPAD_BUTTON_PLUS) != 0 || (btn & WPAD_BUTTON_RIGHT) != 0)
		{
			if (m_gameSettingsPage < 5 || (m_gameSettingsPage > 5 && m_gameSettingsPage < 53 && m_max_categories > 8)
				|| (m_gameSettingsPage > 5 && m_gameSettingsPage < 52 && m_max_categories > 5))
				++m_gameSettingsPage;
			_showGameSettings();
			m_btnMgr.click(m_gameSettingsBtnPageP);
		}
		else if ((padsState & WPAD_BUTTON_A) != 0)
		{
			m_btnMgr.click();
			if (m_btnMgr.selected() == m_gameSettingsBtnBack)
				break;
			else if (m_btnMgr.selected() == m_gameSettingsBtnPageM)
			{
				if ((m_gameSettingsPage > 1 && m_gameSettingsPage < 6) || m_gameSettingsPage > 51)
					--m_gameSettingsPage;
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnPageP && !m_locked)
			{
				if (m_gameSettingsPage < 5 || (m_gameSettingsPage > 5 && m_gameSettingsPage < 53 && m_max_categories > 8)
				|| (m_gameSettingsPage > 5 && m_gameSettingsPage < 52 && m_max_categories > 5))
					++m_gameSettingsPage;
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnOcarina)
			{
				int intoption = loopNum(m_cfg.getOptBool(id, "cheat") + 1, 3);
				if (intoption !=0 && intoption !=1)
					m_cfg.remove(id, "cheat");
				else
					m_cfg.setOptBool(id, "cheat", intoption);
					_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnVipatch)
			{
//				m_cfg.setOptBool(id, "vipatch", loopNum(m_cfg.getOptBool(id, "vipatch") + 1, 3));
				bool booloption = m_cfg.getBool(id, "vipatch");
				if (booloption != false)
					m_cfg.remove(id, "vipatch");
				else
					m_cfg.setBool(id, "vipatch", true);
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnCountryPatch)
			{
//				m_cfg.setOptBool(id, "country_patch", loopNum(m_cfg.getOptBool(id, "country_patch") + 1, 3));
				bool booloption = m_cfg.getBool(id, "country_patch");
				if (booloption != false)
					m_cfg.remove(id, "country_patch");
				else
					m_cfg.setBool(id, "country_patch", true);
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnErr2Fix)
			{
				int intoption = loopNum(m_cfg.getOptBool(id, "error_002_fix") + 1, 3);
				if (intoption !=0 && intoption !=1)
					m_cfg.remove(id, "error_002_fix");
				else
					m_cfg.setOptBool(id, "error_002_fix", intoption);
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnBlockIOSReload)
			{
				bool booloption = m_cfg.getBool(id, "block_ios_reload");
				if (booloption != false)
					m_cfg.remove(id, "block_ios_reload");
				else
				m_cfg.setBool(id, "block_ios_reload", true);
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnLanguageP)
			{
				m_cfg.setInt(id, "language", (int)loopNum((u32)m_cfg.getInt(id, "language", 0) + 1, ARRAY_SIZE(CMenu::_languages)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnLanguageM)
			{
				m_cfg.setInt(id, "language", (int)loopNum((u32)m_cfg.getInt(id, "language", 0) - 1, ARRAY_SIZE(CMenu::_languages)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnVideoP)
			{
				m_cfg.setInt(id, "video_mode", (int)loopNum((u32)m_cfg.getInt(id, "video_mode", 0) + 1, ARRAY_SIZE(CMenu::_videoModes)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnVideoM)
			{
				m_cfg.setInt(id, "video_mode", (int)loopNum((u32)m_cfg.getInt(id, "video_mode", 0) - 1, ARRAY_SIZE(CMenu::_videoModes)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnIOSP)
			{
				m_cfg.setInt(id, "ios", (int)loopNum((u32)m_cfg.getInt(id, "ios", 0) + 1, ARRAY_SIZE(CMenu::_ios)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnIOSM)
			{
				m_cfg.setInt(id, "ios", (int)loopNum((u32)m_cfg.getInt(id, "ios", 0) - 1, ARRAY_SIZE(CMenu::_ios)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnPatchVidModesP)
			{
				m_cfg.setInt(id, "patch_video_modes", (int)loopNum((u32)m_cfg.getInt(id, "patch_video_modes", 0) + 1, ARRAY_SIZE(CMenu::_vidModePatch)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnPatchVidModesM)
			{
				m_cfg.setInt(id, "patch_video_modes", (int)loopNum((u32)m_cfg.getInt(id, "patch_video_modes", 0) - 1, ARRAY_SIZE(CMenu::_vidModePatch)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnAltDolP || m_btnMgr.selected() == m_gameSettingsBtnAltDolM)
			{
				if (!dolsListed)
				{
					m_cf.stopPicLoader();	// Don't keep a thread that may be reading and writing to the USB HD while we access it a different way
					dols.clear();
					dols.push_back("main.dol");
					_listDOL(dols, id);
					sort(dols.begin(), dols.end());
					dolsListed = true;
					m_cf.startPicLoader();
				}
				int i = find(dols.begin(), dols.end(), m_cfg.getString(id, "dol")) - dols.begin();
				if (m_btnMgr.selected() == m_gameSettingsBtnAltDolP)
					++i;
				else
					--i;
				m_cfg.setString(id, "dol", dols[loopNum(i, (int)dols.size())]);
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnCover)
			{
				m_cf.stopPicLoader(true);	// Empty cover cache
				_hideGameSettings();
				_download(id);
				_showGameSettings();
				m_cf.startPicLoader();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnCheat)
			{
				//Handler for Cheat Button
				Fat_Mount();
				_hideGameSettings();
				_CheatSettings();
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnHooktypeP)
			{
				m_cfg.setInt(id, "hooktype", (int)loopNum((u32)m_cfg.getInt(id, "hooktype", 1) + 1, ARRAY_SIZE(CMenu::_hooktype)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnHooktypeM)
			{
				m_cfg.setInt(id, "hooktype", (int)loopNum((u32)m_cfg.getInt(id, "hooktype", 1) - 1, ARRAY_SIZE(CMenu::_hooktype)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnDvdPatch)
			{
				m_cfg.setBool(id, "disable_dvd_patch", !m_cfg.getBool(id, "disable_dvd_patch", false));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnReturnTo)
			{
				_hideGameSettings();
				m_cfg.setBool(id, "returnto", !m_cfg.getBool(id, "returnto", false));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnDebuggerP)
			{
				bool booloption = m_cfg.getBool(id, "debugger");
				if (booloption != false)
					m_cfg.remove(id, "debugger");
				else
				m_cfg.setBool(id, "debugger", true);
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnDebuggerM)
			{
				bool booloption = m_cfg.getBool(id, "debugger");
				if (booloption != false)
					m_cfg.remove(id, "debugger");
				else
				m_cfg.setBool(id, "debugger", true);
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnCategoryMain)
			{
				_hideGameSettings();
				m_gameSettingsPage = 51;
				_showGameSettings();
			}
			for (int i = 0; i < 12; ++i)
				if (m_btnMgr.selected() == m_gameSettingsBtnCategory[i])
				{
					m_gameSettingCategories[i] = m_gameSettingCategories[i] == '1' ? '0' : '1';
					char categories[13];
					memset(&categories, 0, sizeof(categories));
					memcpy(&categories, &m_gameSettingCategories, sizeof(m_gameSettingCategories));
					m_cfg.setString(id, "categories", categories);
					_showGameSettings();
					break;
				}
		}
		_mainLoopCommon(wd, false);
	}
	_hideGameSettings();
}

void CMenu::_initGameSettingsMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_gameSettingsLblUser, ARRAY_SIZE(m_gameSettingsLblUser), "GAME_SETTINGS");
	m_gameSettingsBg = _texture(theme.texSet, "GAME_SETTINGS/BG", "texture", theme.bg);
	m_gameSettingsLblTitle = _addLabel(theme, "GAME_SETTINGS/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	// Page 1
	m_gameSettingsLblGameLanguage = _addLabel(theme, "GAME_SETTINGS/GAME_LANG", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblLanguage = _addLabel(theme, "GAME_SETTINGS/GAME_LANG_BTN", theme.btnFont, L"", 386, 130, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnLanguageM = _addPicButton(theme, "GAME_SETTINGS/GAME_LANG_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 130, 56, 56);
	m_gameSettingsBtnLanguageP = _addPicButton(theme, "GAME_SETTINGS/GAME_LANG_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);
	m_gameSettingsLblGameAltDol = _addLabel(theme, "GAME_SETTINGS/ALT_DOL", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblAltDol = _addLabel(theme, "GAME_SETTINGS/ALT_DOL_BTN", theme.btnFont, L"", 386, 190, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnAltDolM = _addPicButton(theme, "GAME_SETTINGS/ALT_DOL_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 190, 56, 56);
	m_gameSettingsBtnAltDolP = _addPicButton(theme, "GAME_SETTINGS/ALT_DOL_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 190, 56, 56);
	m_gameSettingsLblOcarina = _addLabel(theme, "GAME_SETTINGS/OCARINA", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnOcarina = _addButton(theme, "GAME_SETTINGS/OCARINA_BTN", theme.btnFont, L"", 330, 250, 270, 56, theme.btnFontColor);
	m_gameSettingsLblCheat = _addLabel(theme, "GAME_SETTINGS/CHEAT", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCheat = _addButton(theme, "GAME_SETTINGS/CHEAT_BTN", theme.btnFont, L"", 330, 310, 270, 56, theme.btnFontColor);
	// Page 2
	m_gameSettingsLblCountryPatch = _addLabel(theme, "GAME_SETTINGS/COUNTRY_PATCH", theme.lblFont, L"", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCountryPatch = _addButton(theme, "GAME_SETTINGS/COUNTRY_PATCH_BTN", theme.btnFont, L"", 380, 130, 220, 56, theme.btnFontColor);
	m_gameSettingsLblErr2Fix = _addLabel(theme, "GAME_SETTINGS/ERR2FIX", theme.lblFont, L"", 40, 190, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnErr2Fix = _addButton(theme, "GAME_SETTINGS/ERR2FIX_BTN", theme.btnFont, L"", 380, 190, 220, 56, theme.btnFontColor);
	m_gameSettingsLblVipatch = _addLabel(theme, "GAME_SETTINGS/VIPATCH", theme.lblFont, L"", 40, 250, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnVipatch = _addButton(theme, "GAME_SETTINGS/VIPATCH_BTN", theme.btnFont, L"", 380, 250, 220, 56, theme.btnFontColor);
	m_gameSettingsLblGameIOS = _addLabel(theme, "GAME_SETTINGS/IOS", theme.lblFont, L"", 40, 310, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblIOS = _addLabel(theme, "GAME_SETTINGS/IOS_BTN", theme.btnFont, L"", 436, 310, 108, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnIOSM = _addPicButton(theme, "GAME_SETTINGS/IOS_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 380, 310, 56, 56);
	m_gameSettingsBtnIOSP = _addPicButton(theme, "GAME_SETTINGS/IOS_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 310, 56, 56);
	// Page 3
	m_gameSettingsLblCover = _addLabel(theme, "GAME_SETTINGS/COVER", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCover = _addButton(theme, "GAME_SETTINGS/COVER_BTN", theme.btnFont, L"", 330, 130, 270, 56, theme.btnFontColor);
	m_gameSettingsLblBlockIOSReload = _addLabel(theme, "GAME_SETTINGS/BLOCK_IOSRELOAD", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnBlockIOSReload = _addButton(theme, "GAME_SETTINGS/BLOCK_IOSRELOAD_BTN", theme.btnFont, L"", 330, 190, 270, 56, theme.btnFontColor);
	m_gameSettingsLblPatchVidModes = _addLabel(theme, "GAME_SETTINGS/PATCH_VIDEO_MODE", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblPatchVidModesVal = _addLabel(theme, "GAME_SETTINGS/PATCH_VIDEO_MODE_BTN", theme.btnFont, L"", 386, 250, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnPatchVidModesM = _addPicButton(theme, "GAME_SETTINGS/PATCH_VIDEO_MODE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 250, 56, 56);
	m_gameSettingsBtnPatchVidModesP = _addPicButton(theme, "GAME_SETTINGS/PATCH_VIDEO_MODE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 250, 56, 56);
	m_gameSettingsLblGameVideo = _addLabel(theme, "GAME_SETTINGS/VIDEO", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblVideo = _addLabel(theme, "GAME_SETTINGS/VIDEO_BTN", theme.btnFont, L"", 386, 310, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnVideoM = _addPicButton(theme, "GAME_SETTINGS/VIDEO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 310, 56, 56);
	m_gameSettingsBtnVideoP = _addPicButton(theme, "GAME_SETTINGS/VIDEO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 310, 56, 56);
	//Page 4
	m_gameSettingsLblCategoryMain = _addLabel(theme, "GAME_SETTINGS/CAT_MAIN", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategoryMain = _addButton(theme, "GAME_SETTINGS/CAT_MAIN_BTN", theme.btnFont, L"", 330, 130, 270, 56, theme.btnFontColor);
	m_gameSettingsLblHooktype = _addLabel(theme, "GAME_SETTINGS/HOOKTYPE", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblHooktypeVal = _addLabel(theme, "GAME_SETTINGS/HOOKTYPE_BTN", theme.btnFont, L"", 386, 190, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnHooktypeM = _addPicButton(theme, "GAME_SETTINGS/HOOKTYPE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 190, 56, 56);
	m_gameSettingsBtnHooktypeP = _addPicButton(theme, "GAME_SETTINGS/HOOKTYPE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 190, 56, 56);
	m_gameSettingsLblDvdPatch = _addLabel(theme, "GAME_SETTINGS/DVD_PATCH", theme.lblFont, L"", 40, 250, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnDvdPatch = _addButton(theme, "GAME_SETTINGS/DVD_PATCH_BTN", theme.btnFont, L"", 330, 250, 270, 56, theme.btnFontColor);
	m_gameSettingsLblReturnTo = _addLabel(theme, "GAME_SETTINGS/RETURN_TO", theme.lblFont, L"", 40, 310, 270, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnReturnTo = _addButton(theme, "GAME_SETTINGS/RETURN_TO_BTN", theme.btnFont, L"", 330, 310, 270, 56, theme.btnFontColor);
	//Page 5
	m_gameSettingsLblDebugger = _addLabel(theme, "GAME_SETTINGS/GAME_DEBUGGER", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblDebuggerV = _addLabel(theme, "GAME_SETTINGS/GAME_DEBUGGER_BTN", theme.btnFont, L"", 386, 130, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnDebuggerM = _addPicButton(theme, "GAME_SETTINGS/GAME_DEBUGGER_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 130, 56, 56);
	m_gameSettingsBtnDebuggerP = _addPicButton(theme, "GAME_SETTINGS/GAME_DEBUGGER_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);
	//Categories Page 1 
	//m_gameSettingsLblCategory[0] = _addLabel(theme, "GAME_SETTINGS/CAT_ALL", theme.lblFont, L"All", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	//m_gameSettingsBtnCategory[0] = _addButton(theme, "GAME_SETTINGS/CAT_ALL_BTN", theme.btnFont, L"", 330, 130, 270, 56, theme.btnFontColor);

	m_gameSettingsLblCategory[1] = _addLabel(theme, "GAME_SETTINGS/CAT_1", theme.lblFont, L"", 40, 130, 190, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[1] = _addButton(theme, "GAME_SETTINGS/CAT_1_BTN", theme.btnFont, L"", 330, 130, 270, 56, theme.btnFontColor);
	m_gameSettingsLblCategory[2] = _addLabel(theme, "GAME_SETTINGS/CAT_2", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[2] = _addButton(theme, "GAME_SETTINGS/CAT_2_BTN", theme.btnFont, L"", 330, 190, 270, 56, theme.btnFontColor);
	m_gameSettingsLblCategory[3] = _addLabel(theme, "GAME_SETTINGS/CAT_3", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[3] = _addButton(theme, "GAME_SETTINGS/CAT_3_BTN", theme.btnFont, L"", 330, 250, 270, 56, theme.btnFontColor);
	m_gameSettingsLblCategory[4] = _addLabel(theme, "GAME_SETTINGS/CAT_4", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[4] = _addButton(theme, "GAME_SETTINGS/CAT_4_BTN", theme.btnFont, L"", 330, 310, 270, 56, theme.btnFontColor);
	
	//Categories Page 2
	m_gameSettingsLblCategory[5] = _addLabel(theme, "GAME_SETTINGS/CAT_5", theme.lblFont, L"", 40, 130, 190, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[5] = _addButton(theme, "GAME_SETTINGS/CAT_5_BTN", theme.btnFont, L"", 330, 130, 270, 56, theme.btnFontColor);
	m_gameSettingsLblCategory[6] = _addLabel(theme, "GAME_SETTINGS/CAT_6", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[6] = _addButton(theme, "GAME_SETTINGS/CAT_6_BTN", theme.btnFont, L"", 330, 190, 270, 56, theme.btnFontColor);
	m_gameSettingsLblCategory[7] = _addLabel(theme, "GAME_SETTINGS/CAT_7", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[7] = _addButton(theme, "GAME_SETTINGS/CAT_7_BTN", theme.btnFont, L"", 330, 250, 270, 56, theme.btnFontColor);
	m_gameSettingsLblCategory[8] = _addLabel(theme, "GAME_SETTINGS/CAT_8", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[8] = _addButton(theme, "GAME_SETTINGS/CAT_8_BTN", theme.btnFont, L"", 330, 310, 270, 56, theme.btnFontColor);

	//Categories Page 3 
	m_gameSettingsLblCategory[9] = _addLabel(theme, "GAME_SETTINGS/CAT_9", theme.lblFont, L"", 40, 130, 190, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[9] = _addButton(theme, "GAME_SETTINGS/CAT_9_BTN", theme.btnFont, L"", 330, 130, 270, 56, theme.btnFontColor);
	m_gameSettingsLblCategory[10] = _addLabel(theme, "GAME_SETTINGS/CAT_10", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[10] = _addButton(theme, "GAME_SETTINGS/CAT_10_BTN", theme.btnFont, L"", 330, 190, 270, 56, theme.btnFontColor);
	m_gameSettingsLblCategory[11] = _addLabel(theme, "GAME_SETTINGS/CAT_11", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[11] = _addButton(theme, "GAME_SETTINGS/CAT_11_BTN", theme.btnFont, L"", 330, 250, 270, 56, theme.btnFontColor);

	// 
	m_gameSettingsLblPage = _addLabel(theme, "GAME_SETTINGS/PAGE_BTN", theme.btnFont, L"", 76, 410, 80, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnPageM = _addPicButton(theme, "GAME_SETTINGS/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 410, 56, 56);
	m_gameSettingsBtnPageP = _addPicButton(theme, "GAME_SETTINGS/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 156, 410, 56, 56);
	m_gameSettingsBtnBack = _addButton(theme, "GAME_SETTINGS/BACK_BTN", theme.btnFont, L"", 420, 410, 200, 56, theme.btnFontColor);
	// 
	_setHideAnim(m_gameSettingsLblTitle, "GAME_SETTINGS/TITLE", 0, -200, 0.f, 1.f);
	_setHideAnim(m_gameSettingsLblGameVideo, "GAME_SETTINGS/VIDEO", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblVideo, "GAME_SETTINGS/VIDEO_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnVideoM, "GAME_SETTINGS/VIDEO_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnVideoP, "GAME_SETTINGS/VIDEO_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblGameLanguage, "GAME_SETTINGS/GAME_LANG", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblLanguage, "GAME_SETTINGS/GAME_LANG_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnLanguageM, "GAME_SETTINGS/GAME_LANG_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnLanguageP, "GAME_SETTINGS/GAME_LANG_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblGameAltDol, "GAME_SETTINGS/ALT_DOL", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblAltDol, "GAME_SETTINGS/ALT_DOL_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnAltDolM, "GAME_SETTINGS/ALT_DOL_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnAltDolP, "GAME_SETTINGS/ALT_DOL_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblOcarina, "GAME_SETTINGS/OCARINA", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnOcarina, "GAME_SETTINGS/OCARINA_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblCheat, "GAME_SETTINGS/CHEAT", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCheat, "GAME_SETTINGS/CHEAT_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblCountryPatch, "GAME_SETTINGS/COUNTRY_PATCH", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCountryPatch, "GAME_SETTINGS/COUNTRY_PATCH_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblErr2Fix, "GAME_SETTINGS/ERR2FIX", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnErr2Fix, "GAME_SETTINGS/ERR2FIX_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblVipatch, "GAME_SETTINGS/VIPATCH", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnVipatch, "GAME_SETTINGS/VIPATCH_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblGameIOS, "GAME_SETTINGS/IOS", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblIOS, "GAME_SETTINGS/IOS_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnIOSM, "GAME_SETTINGS/IOS_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnIOSP, "GAME_SETTINGS/IOS_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblCover, "GAME_SETTINGS/COVER", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCover, "GAME_SETTINGS/COVER_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblBlockIOSReload, "GAME_SETTINGS/BLOCK_IOSRELOAD", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnBlockIOSReload, "GAME_SETTINGS/BLOCK_IOSRELOAD_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblPage, "GAME_SETTINGS/PAGE_BTN", 0, 200, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPageM, "GAME_SETTINGS/PAGE_MINUS", 0, 200, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPageP, "GAME_SETTINGS/PAGE_PLUS", 0, 200, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnBack, "GAME_SETTINGS/BACK_BTN", 0, 200, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblPatchVidModes, "GAME_SETTINGS/PATCH_VIDEO_MODE", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblPatchVidModesVal, "GAME_SETTINGS/PATCH_VIDEO_MODE_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPatchVidModesM, "GAME_SETTINGS/PATCH_VIDEO_MODE_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPatchVidModesP, "GAME_SETTINGS/PATCH_VIDEO_MODE_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblHooktype, "GAME_SETTINGS/HOOKTYPE", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblHooktypeVal, "GAME_SETTINGS/HOOKTYPE_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnHooktypeM, "GAME_SETTINGS/HOOKTYPE_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnHooktypeP, "GAME_SETTINGS/HOOKTYPE_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblDvdPatch, "GAME_SETTINGS/DVD_PATCH", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDvdPatch, "GAME_SETTINGS/DVD_PATCH_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblReturnTo, "GAME_SETTINGS/RETURN_TO", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnReturnTo, "GAME_SETTINGS/RETURN_TO_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblDebugger, "GAME_SETTINGS/GAME_DEBUGGER", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblDebuggerV, "GAME_SETTINGS/GAME_DEBUGGER_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDebuggerM, "GAME_SETTINGS/GAME_DEBUGGER_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDebuggerP, "GAME_SETTINGS/GAME_DEBUGGER_PLUS", 200, 0, 1.f, 0.f);
	//Categories
	_setHideAnim(m_gameSettingsBtnCategoryMain, "GAME_SETTINGS/CAT_MAIN_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblCategoryMain, "GAME_SETTINGS/CAT_MAIN", -200, 0, 1.f, 0.f);
	//_setHideAnim(m_gameSettingsBtnCategory[0], "GAME_SETTINGS/CAT_ALL_BTN", 200, 0, 1.f, 0.f);
	//_setHideAnim(m_gameSettingsLblCategory[0], "GAME_SETTINGS/CAT_ALL", -200, 0, 1.f, 0.f);
	for (int i = 1; i < 12; ++i) {
		_setHideAnim(m_gameSettingsBtnCategory[i], sfmt("GAME_SETTINGS/CAT_%i_BTN", i).c_str(), 200, 0, 1.f, 0.f);
		_setHideAnim(m_gameSettingsLblCategory[i], sfmt("GAME_SETTINGS/CAT_%i", i).c_str(), -200, 0, 1.f, 0.f);
	}

	_hideGameSettings(true);
	_textGameSettings();
}

void CMenu::_textGameSettings(void)
{
	m_btnMgr.setText(m_gameSettingsLblTitle, _t("cfgg1", L"Settings"));
	m_btnMgr.setText(m_gameSettingsLblGameVideo, _t("cfgg2", L"Video mode"));
	m_btnMgr.setText(m_gameSettingsLblGameLanguage, _t("cfgg3", L"Language"));
	m_btnMgr.setText(m_gameSettingsLblCountryPatch, _t("cfgg4", L"Patch country strings"));
	m_btnMgr.setText(m_gameSettingsLblOcarina, _t("cfgg5", L"Ocarina"));
	m_btnMgr.setText(m_gameSettingsLblErr2Fix, _t("cfgg6", L"Error 002 fix"));
	m_btnMgr.setText(m_gameSettingsLblVipatch, _t("cfgg7", L"Vipatch"));
	m_btnMgr.setText(m_gameSettingsBtnBack, _t("cfgg8", L"Back"));
	m_btnMgr.setText(m_gameSettingsLblGameAltDol, _t("cfgg9", L"DOL"));
	m_btnMgr.setText(m_gameSettingsLblGameIOS, _t("cfgg10", L"IOS"));
	m_btnMgr.setText(m_gameSettingsLblBlockIOSReload, _t("cfgg11", L"Block IOS reload"));
	m_btnMgr.setText(m_gameSettingsLblCover, _t("cfgg12", L"Download cover"));
	m_btnMgr.setText(m_gameSettingsBtnCover, _t("cfgg13", L"Download"));
	m_btnMgr.setText(m_gameSettingsLblPatchVidModes, _t("cfgg14", L"Patch video modes"));
	m_btnMgr.setText(m_gameSettingsLblCheat, _t("cfgg15", L"Cheat Codes"));
	m_btnMgr.setText(m_gameSettingsBtnCheat, _t("cfgg16", L"Select"));
	m_btnMgr.setText(m_gameSettingsLblCategoryMain, _t("cfgg17", L"Categories"));
	m_btnMgr.setText(m_gameSettingsBtnCategoryMain, _t("cfgg16", L"Select"));
	m_btnMgr.setText(m_gameSettingsLblHooktype, _t("cfgg18", L"Hook Type"));
	m_btnMgr.setText(m_gameSettingsLblDvdPatch, _t("cfgg19", L"Disable DVD Patch"));
	m_btnMgr.setText(m_gameSettingsLblReturnTo, _t("cfgg20", L"Disable Return To"));
	m_btnMgr.setText(m_gameSettingsLblDebugger, _t("cfgg22", L"Debugger"));
	for (int i = 1; i < 12; ++i)
		m_btnMgr.setText(m_gameSettingsLblCategory[i], m_cfg.getWString(" GENERAL", fmt("cat%d",i), m_loc.getWString(m_curLanguage, fmt("cat%d", i), wfmt(L"Category %i",i).c_str())));
}

static void addDolToList(void *o, const char *fileName)
{
	vector<string> &v = *(vector<string> *)o;
	v.push_back(fileName);
}

void CMenu::_listDOL(vector<string> &v, const string &gameId)
{
	wbfs_disc_t *disc = WBFS_OpenDisc((u8 *)gameId.c_str());
	wiidisc_t *wdisc = wd_open_disc((int (*)(void *, u32, u32, void *))wbfs_disc_read, disc);
	wd_list_dols(wdisc, ALL_PARTITIONS, addDolToList, (void *)&v);
	wd_close_disc(wdisc);
	WBFS_CloseDisc(disc);
}
