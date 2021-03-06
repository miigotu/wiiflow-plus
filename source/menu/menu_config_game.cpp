#include "loader/wbfs.h"
#include "libwbfs/wiidisc.h"
#include "menu.hpp"

#include "loader/sys.h"
#include "gecko.h"

#define ARRAY_SIZE(a)	(sizeof a / sizeof a[0])

using namespace std;

u8 m_gameSettingCategories[12];
u32 g_numGCfPages = 3;

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
	m_btnMgr.hide(m_gameSettingsLblOcarina, instant);
	m_btnMgr.hide(m_gameSettingsBtnOcarina, instant);
	m_btnMgr.hide(m_gameSettingsLblCheat, instant);
	m_btnMgr.hide(m_gameSettingsBtnCheat, instant);
	m_btnMgr.hide(m_gameSettingsLblVipatch, instant);
	m_btnMgr.hide(m_gameSettingsBtnVipatch, instant);
	m_btnMgr.hide(m_gameSettingsLblCountryPatch, instant);
	m_btnMgr.hide(m_gameSettingsBtnCountryPatch, instant);
	m_btnMgr.hide(m_gameSettingsLblCover, instant);
	m_btnMgr.hide(m_gameSettingsBtnCover, instant);
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
	m_btnMgr.hide(m_gameSettingsLblEmulationVal, instant);
	m_btnMgr.hide(m_gameSettingsLblEmulation, instant);
	m_btnMgr.hide(m_gameSettingsBtnEmulationP, instant);
	m_btnMgr.hide(m_gameSettingsBtnEmulationM, instant);
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
		m_btnMgr.show(m_gameSettingsLblCover);
		m_btnMgr.show(m_gameSettingsBtnCover);

		m_btnMgr.show(m_gameSettingsBtnCategoryMain);
		m_btnMgr.show(m_gameSettingsLblCategoryMain);

		m_btnMgr.show(m_gameSettingsLblGameLanguage);
		m_btnMgr.show(m_gameSettingsLblLanguage);
		m_btnMgr.show(m_gameSettingsBtnLanguageP);
		m_btnMgr.show(m_gameSettingsBtnLanguageM);

		m_btnMgr.show(m_gameSettingsLblGameVideo);
		m_btnMgr.show(m_gameSettingsLblVideo);
		m_btnMgr.show(m_gameSettingsBtnVideoP);
		m_btnMgr.show(m_gameSettingsBtnVideoM);

	}
	else
	{
		m_btnMgr.hide(m_gameSettingsLblCover);
		m_btnMgr.hide(m_gameSettingsBtnCover);

		m_btnMgr.hide(m_gameSettingsBtnCategoryMain);
		m_btnMgr.hide(m_gameSettingsLblCategoryMain);

		m_btnMgr.hide(m_gameSettingsLblGameLanguage);
		m_btnMgr.hide(m_gameSettingsLblLanguage);
		m_btnMgr.hide(m_gameSettingsBtnLanguageP);
		m_btnMgr.hide(m_gameSettingsBtnLanguageM);

		m_btnMgr.hide(m_gameSettingsLblGameVideo);
		m_btnMgr.hide(m_gameSettingsLblVideo);
		m_btnMgr.hide(m_gameSettingsBtnVideoP);
		m_btnMgr.hide(m_gameSettingsBtnVideoM);

	}
	if (m_gameSettingsPage == 2)
	{
		m_btnMgr.show(m_gameSettingsLblDebugger);
		m_btnMgr.show(m_gameSettingsLblDebuggerV);
		m_btnMgr.show(m_gameSettingsBtnDebuggerP);
		m_btnMgr.show(m_gameSettingsBtnDebuggerM);

		m_btnMgr.show(m_gameSettingsLblHooktype);
		m_btnMgr.show(m_gameSettingsLblHooktypeVal);
		m_btnMgr.show(m_gameSettingsBtnHooktypeM);
		m_btnMgr.show(m_gameSettingsBtnHooktypeP);

		m_btnMgr.show(m_gameSettingsLblOcarina);
		m_btnMgr.show(m_gameSettingsBtnOcarina);

		m_btnMgr.show(m_gameSettingsLblCheat);
		m_btnMgr.show(m_gameSettingsBtnCheat);
	}
	else
	{
		m_btnMgr.hide(m_gameSettingsLblDebugger);
		m_btnMgr.hide(m_gameSettingsLblDebuggerV);
		m_btnMgr.hide(m_gameSettingsBtnDebuggerP);
		m_btnMgr.hide(m_gameSettingsBtnDebuggerM);

		m_btnMgr.hide(m_gameSettingsLblHooktype);
		m_btnMgr.hide(m_gameSettingsLblHooktypeVal);
		m_btnMgr.hide(m_gameSettingsBtnHooktypeM);
		m_btnMgr.hide(m_gameSettingsBtnHooktypeP);

		m_btnMgr.hide(m_gameSettingsLblOcarina);
		m_btnMgr.hide(m_gameSettingsBtnOcarina);

		m_btnMgr.hide(m_gameSettingsLblCheat);
		m_btnMgr.hide(m_gameSettingsBtnCheat);
	}
	if (m_gameSettingsPage == 3)
	{

		m_btnMgr.show(m_gameSettingsLblPatchVidModes);
		m_btnMgr.show(m_gameSettingsLblPatchVidModesVal);
		m_btnMgr.show(m_gameSettingsBtnPatchVidModesM);
		m_btnMgr.show(m_gameSettingsBtnPatchVidModesP);

		m_btnMgr.show(m_gameSettingsLblVipatch);
		m_btnMgr.show(m_gameSettingsBtnVipatch);

		m_btnMgr.show(m_gameSettingsLblCountryPatch);
		m_btnMgr.show(m_gameSettingsBtnCountryPatch);

		if(m_current_view == COVERFLOW_USB)
		{
			m_btnMgr.show(m_gameSettingsLblEmulationVal);
			m_btnMgr.show(m_gameSettingsLblEmulation);
			m_btnMgr.show(m_gameSettingsBtnEmulationP);
			m_btnMgr.show(m_gameSettingsBtnEmulationM);
		}
	}
	else
	{
		m_btnMgr.hide(m_gameSettingsLblPatchVidModes);
		m_btnMgr.hide(m_gameSettingsLblPatchVidModesVal);
		m_btnMgr.hide(m_gameSettingsBtnPatchVidModesM);
		m_btnMgr.hide(m_gameSettingsBtnPatchVidModesP);

		m_btnMgr.hide(m_gameSettingsLblVipatch);
		m_btnMgr.hide(m_gameSettingsBtnVipatch);

		m_btnMgr.hide(m_gameSettingsLblCountryPatch);
		m_btnMgr.hide(m_gameSettingsBtnCountryPatch);

		m_btnMgr.hide(m_gameSettingsLblEmulationVal);
		m_btnMgr.hide(m_gameSettingsLblEmulation);
		m_btnMgr.hide(m_gameSettingsBtnEmulationP);
		m_btnMgr.hide(m_gameSettingsBtnEmulationM);
	}

	u32 i = 0;

	//Categories Pages
	if (m_gameSettingsPage == 51)
	{
		for (i = 1; i < (u32)min(m_max_categories + 1, 5); ++i)
		{
			m_btnMgr.show(m_gameSettingsBtnCategory[i]);
			m_btnMgr.show(m_gameSettingsLblCategory[i]);
		}
	}
	else
	{
		for (i = 1; i < (u32)min(m_max_categories + 1, 5); ++i)
		{
			m_btnMgr.hide(m_gameSettingsBtnCategory[i]);
			m_btnMgr.hide(m_gameSettingsLblCategory[i]);
		}
	}
	if (m_gameSettingsPage == 52)
	{
		for (i = 5; i < (u32)min(m_max_categories + 1, 9); ++i)
		{
			m_btnMgr.show(m_gameSettingsBtnCategory[i]);
			m_btnMgr.show(m_gameSettingsLblCategory[i]);
		}
	}
	else
	{
		for (i = 5; i < (u32)min(m_max_categories + 1, 9); ++i)
		{
			m_btnMgr.hide(m_gameSettingsBtnCategory[i]);
			m_btnMgr.hide(m_gameSettingsLblCategory[i]);
		}
	}
	if (m_gameSettingsPage == 53)
	{
		for (i = 9; i < (u32)min(m_max_categories + 1, 12); ++i)
		{
			m_btnMgr.show(m_gameSettingsBtnCategory[i]);
			m_btnMgr.show(m_gameSettingsLblCategory[i]);
		}
	}
	else
	{
		for (i = 9; i < (u32)min(m_max_categories + 1, 12); ++i)
		{
			m_btnMgr.hide(m_gameSettingsBtnCategory[i]);
			m_btnMgr.hide(m_gameSettingsLblCategory[i]);
		}
	}

	for (i = 0; i < ARRAY_SIZE(m_gameSettingsLblUser); ++i)
		if (m_gameSettingsLblUser[i] != -1u)
			m_btnMgr.show(m_gameSettingsLblUser[i]);

	string id(m_cf.getId());
	u32 page = m_gameSettingsPage;

	if (page > g_numGCfPages)
		page -= 50;

	m_btnMgr.setText(m_gameSettingsLblPage, wfmt(L"%i / %i", page, g_numGCfPages));
	m_btnMgr.setText(m_gameSettingsBtnOcarina, _optBoolToString(m_gcfg2.getOptBool(id, "cheat")));
	m_btnMgr.setText(m_gameSettingsBtnVipatch, _optBoolToString(m_gcfg2.getOptBool(id, "vipatch")));
	m_btnMgr.setText(m_gameSettingsBtnCountryPatch, _optBoolToString(m_gcfg2.getOptBool(id, "country_patch")));
	i = min((u32)m_gcfg2.getInt(id, "video_mode"), ARRAY_SIZE(CMenu::_videoModes) - 1u);
	m_btnMgr.setText(m_gameSettingsLblVideo, _t(CMenu::_videoModes[i].id, CMenu::_videoModes[i].text));
	i = min((u32)m_gcfg2.getInt(id, "language"), ARRAY_SIZE(CMenu::_languages) - 1u);
	m_btnMgr.setText(m_gameSettingsLblLanguage, _t(CMenu::_languages[i].id, CMenu::_languages[i].text));

	i = min((u32)m_gcfg2.getInt(id, "patch_video_modes"), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
	m_btnMgr.setText(m_gameSettingsLblPatchVidModesVal, _t(CMenu::_vidModePatch[i].id, CMenu::_vidModePatch[i].text));

	i = min((u32)m_gcfg2.getInt(id, "hooktype", 1), ARRAY_SIZE(CMenu::_hooktype) - 1u);
	m_btnMgr.setText(m_gameSettingsLblHooktypeVal, _t(CMenu::_hooktype[i].id, CMenu::_hooktype[i].text));

	int j = EMU_DEFAULT;
	m_gcfg2.getInt(id, "emulation", &j);
	m_btnMgr.setText(m_gameSettingsLblEmulationVal, _t(CMenu::_Emulation[j].id, CMenu::_Emulation[j].text));

	m_btnMgr.setText(m_gameSettingsLblDebuggerV, m_gcfg2.getBool(id, "debugger") ? _t("gecko", L"Gecko") : _t("def", L"Default"));
	m_btnMgr.setText(m_gameSettingsBtnCategoryMain, _fmt("cfgg16",  L"Select").c_str());

	char *categories = (char *) m_cat.getString("CATEGORIES", id, "").c_str();
	memset(&m_gameSettingCategories, '0', sizeof(m_gameSettingCategories));
	if (strlen(categories) == sizeof(m_gameSettingCategories))
		memcpy(&m_gameSettingCategories, categories, sizeof(m_gameSettingCategories));
	for (int i = 0; i < 12; ++i)
		m_btnMgr.setText(m_gameSettingsBtnCategory[i], _optBoolToString(m_gameSettingCategories[i] == '1'));
}

void CMenu::_gameSettings(void)
{
	m_gcfg2.load(sfmt("%s/gameconfig2.ini", m_settingsDir.c_str()).c_str());
	string id(m_cf.getId());

	m_gameSettingsPage = 1;
	_showGameSettings();
	while (true)
	{
		_mainLoopCommon();
		if (BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if (BTN_UP_PRESSED)
			m_btnMgr.up();
		else if (BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if ((BTN_MINUS_PRESSED || BTN_LEFT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_gameSettingsBtnPageM))) && !m_locked)
		{
			if (m_gameSettingsPage == 1)
				m_gameSettingsPage = g_numGCfPages;
			else if (m_gameSettingsPage == 51)
				m_gameSettingsPage = 53;
			else if ((m_gameSettingsPage > 1 && m_gameSettingsPage <= g_numGCfPages) || m_gameSettingsPage > 51)
				--m_gameSettingsPage;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED) m_btnMgr.click(m_gameSettingsBtnPageM);
			_showGameSettings();
		}
		else if ((BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_gameSettingsBtnPageP))) && !m_locked)
		{
			if (m_gameSettingsPage == g_numGCfPages)
				m_gameSettingsPage = 1;
			else if (m_gameSettingsPage == 53)
				m_gameSettingsPage = 51;
			else if (m_gameSettingsPage < g_numGCfPages || (m_gameSettingsPage > g_numGCfPages && m_gameSettingsPage < 53 && m_max_categories > 8)
				|| (m_gameSettingsPage > g_numGCfPages && m_gameSettingsPage < 52 && m_max_categories > 3))
				++m_gameSettingsPage;
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED) m_btnMgr.click(m_gameSettingsBtnPageP);
			_showGameSettings();
		}
		else if (BTN_A_PRESSED)
		{
			if (m_btnMgr.selected(m_gameSettingsBtnBack))
				break;
			else if (m_btnMgr.selected(m_gameSettingsBtnOcarina))
			{
				int intoption = loopNum(m_gcfg2.getBool(id, "cheat") + 1, 3);
				if (intoption > 1)
					m_gcfg2.remove(id, "cheat");
				else
					m_gcfg2.setOptBool(id, "cheat", intoption);
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnVipatch))
			{
				bool booloption = m_gcfg2.getBool(id, "vipatch");
				if (booloption)
					m_gcfg2.remove(id, "vipatch");
				else
					m_gcfg2.setBool(id, "vipatch", true);
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnCountryPatch))
			{
				bool booloption = m_gcfg2.getBool(id, "country_patch");
				if (booloption)
					m_gcfg2.remove(id, "country_patch");
				else
					m_gcfg2.setBool(id, "country_patch", true);
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnLanguageP) || m_btnMgr.selected(m_gameSettingsBtnLanguageM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnLanguageP) ? 1 : -1;
				int value = (int)loopNum((u32)m_gcfg2.getInt(id, "language") + direction, ARRAY_SIZE(CMenu::_languages));
				if(value)
					m_gcfg2.setInt(id, "language", value);
				else
					m_gcfg2.remove(id, "language");
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnVideoP) || m_btnMgr.selected(m_gameSettingsBtnVideoM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnVideoP) ? 1 : -1;
				int value = (int)loopNum((u32)m_gcfg2.getInt(id, "video_mode") + direction, ARRAY_SIZE(CMenu::_videoModes));
				if(value)
					m_gcfg2.setInt(id, "video_mode", value);
				else
					m_gcfg2.remove(id, "video_mode");
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnPatchVidModesP) || m_btnMgr.selected(m_gameSettingsBtnPatchVidModesM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnPatchVidModesP) ? 1 : -1;
				int value = (int)loopNum((u32)m_gcfg2.getInt(id, "patch_video_modes") + direction, ARRAY_SIZE(CMenu::_vidModePatch));
				if(value)
					m_gcfg2.setInt(id, "patch_video_modes", value);
				else
					m_gcfg2.remove(id, "patch_video_modes");
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnCover))
			{
				m_cf.stopCoverLoader(true);
				_hideGameSettings();
				_download(id);
				_showGameSettings();
				m_cf.startCoverLoader();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnCheat))
			{
				_hideGameSettings();
				_CheatSettings();
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnHooktypeP) || m_btnMgr.selected(m_gameSettingsBtnHooktypeM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnHooktypeP) ? 1 : -1;
				int value = (int)loopNum((u32)m_gcfg2.getInt(id, "hooktype", 1) + direction, ARRAY_SIZE(CMenu::_hooktype));
				if(value)
					m_gcfg2.setInt(id, "hooktype", value);
				else
					m_gcfg2.remove(id, "hooktype");
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnEmulationP) || m_btnMgr.selected(m_gameSettingsBtnEmulationM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnEmulationP) ? 1 : -1;
				int value = (int)loopNum((u32)m_gcfg2.getInt(id, "emulation") + direction, ARRAY_SIZE(CMenu::_Emulation));
				if(value != EMU_DEFAULT)
					m_gcfg2.setInt(id, "emulation", value);
				else
					m_gcfg2.remove(id, "emulation");
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnDebuggerP) || m_btnMgr.selected(m_gameSettingsBtnDebuggerM))
			{
				bool booloption = m_gcfg2.getBool(id, "debugger");
				if (booloption)
					m_gcfg2.remove(id, "debugger");
				else
					m_gcfg2.setBool(id, "debugger", true);
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnCategoryMain))
			{
				_hideGameSettings();
				m_gameSettingsPage = 51;
				_showGameSettings();
			}
			for (int i = 0; i < 12; ++i)
				if (m_btnMgr.selected(m_gameSettingsBtnCategory[i]))
				{
					m_gameSettingCategories[i] = m_gameSettingCategories[i] == '1' ? '0' : '1';
					char categories[13];
					memset(&categories, 0, sizeof(categories));
					memcpy(&categories, &m_gameSettingCategories, sizeof(m_gameSettingCategories));
					m_cat.setString("CATEGORIES", id, categories);
					_showGameSettings();
					break;
				}
		}
		else if (((WBTN_2_HELD && WBTN_1_PRESSED) || (WBTN_1_HELD && WBTN_2_PRESSED)) && m_btnMgr.selected(m_gameSettingsBtnCover))
			m_cf.removeCover(m_cf.getId());

	}
	m_gcfg2.save(true);
	_hideGameSettings();
}

void CMenu::_initGameSettingsMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_gameSettingsLblUser, ARRAY_SIZE(m_gameSettingsLblUser), "GAME_SETTINGS");
	m_gameSettingsBg = _texture(theme.texSet, "GAME_SETTINGS/BG", "texture", theme.bg);
	m_gameSettingsLblTitle = _addTitle(theme, "GAME_SETTINGS/TITLE", 20, 30, 600, 60, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	// Page 1
	m_gameSettingsLblCover = _addLabel(theme, "GAME_SETTINGS/COVER", 40, 130, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCover = _addButton(theme, "GAME_SETTINGS/COVER_BTN", 330, 130, 270, 56);
	m_gameSettingsLblCategoryMain = _addLabel(theme, "GAME_SETTINGS/CAT_MAIN", 40, 190, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategoryMain = _addButton(theme, "GAME_SETTINGS/CAT_MAIN_BTN", 330, 190, 270, 56);

	m_gameSettingsLblGameLanguage = _addLabel(theme, "GAME_SETTINGS/GAME_LANG", 40, 250, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblLanguage = _addLabel(theme, "GAME_SETTINGS/GAME_LANG_BTN", 386, 250, 158, 56, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnLanguageM = _addPicButton(theme, "GAME_SETTINGS/GAME_LANG_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 250, 56, 56);
	m_gameSettingsBtnLanguageP = _addPicButton(theme, "GAME_SETTINGS/GAME_LANG_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 250, 56, 56);

	m_gameSettingsLblGameVideo = _addLabel(theme, "GAME_SETTINGS/VIDEO", 40, 310, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblVideo = _addLabel(theme, "GAME_SETTINGS/VIDEO_BTN", 386, 310, 158, 56, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnVideoM = _addPicButton(theme, "GAME_SETTINGS/VIDEO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 310, 56, 56);
	m_gameSettingsBtnVideoP = _addPicButton(theme, "GAME_SETTINGS/VIDEO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 310, 56, 56);

	// Page 2
	m_gameSettingsLblDebugger = _addLabel(theme, "GAME_SETTINGS/GAME_DEBUGGER", 40, 130, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblDebuggerV = _addLabel(theme, "GAME_SETTINGS/GAME_DEBUGGER_BTN", 386, 130, 158, 56, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnDebuggerM = _addPicButton(theme, "GAME_SETTINGS/GAME_DEBUGGER_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 130, 56, 56);
	m_gameSettingsBtnDebuggerP = _addPicButton(theme, "GAME_SETTINGS/GAME_DEBUGGER_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);
	m_gameSettingsLblHooktype = _addLabel(theme, "GAME_SETTINGS/HOOKTYPE", 40, 190, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblHooktypeVal = _addLabel(theme, "GAME_SETTINGS/HOOKTYPE_BTN", 386, 190, 158, 56, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnHooktypeM = _addPicButton(theme, "GAME_SETTINGS/HOOKTYPE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 190, 56, 56);
	m_gameSettingsBtnHooktypeP = _addPicButton(theme, "GAME_SETTINGS/HOOKTYPE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 190, 56, 56);
	m_gameSettingsLblOcarina = _addLabel(theme, "GAME_SETTINGS/OCARINA", 40, 250, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnOcarina = _addButton(theme, "GAME_SETTINGS/OCARINA_BTN", 330, 250, 270, 56);
	m_gameSettingsLblCheat = _addLabel(theme, "GAME_SETTINGS/CHEAT", 40, 310, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCheat = _addButton(theme, "GAME_SETTINGS/CHEAT_BTN", 330, 310, 270, 56);
	// Page 3
	m_gameSettingsLblCountryPatch = _addLabel(theme, "GAME_SETTINGS/COUNTRY_PATCH", 40, 130, 340, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCountryPatch = _addButton(theme, "GAME_SETTINGS/COUNTRY_PATCH_BTN", 380, 130, 220, 56);
	m_gameSettingsLblVipatch = _addLabel(theme, "GAME_SETTINGS/VIPATCH", 40, 190, 340, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnVipatch = _addButton(theme, "GAME_SETTINGS/VIPATCH_BTN", 380, 190, 220, 56);
	m_gameSettingsLblPatchVidModes = _addLabel(theme, "GAME_SETTINGS/PATCH_VIDEO_MODE", 40, 250, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblPatchVidModesVal = _addLabel(theme, "GAME_SETTINGS/PATCH_VIDEO_MODE_BTN", 436, 250, 108, 56, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnPatchVidModesM = _addPicButton(theme, "GAME_SETTINGS/PATCH_VIDEO_MODE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 380, 250, 56, 56);
	m_gameSettingsBtnPatchVidModesP = _addPicButton(theme, "GAME_SETTINGS/PATCH_VIDEO_MODE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 250, 56, 56);
	m_gameSettingsLblEmulation = _addLabel(theme, "GAME_SETTINGS/EMU", 40, 310, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblEmulationVal = _addLabel(theme, "GAME_SETTINGS/EMU_BTN", 436, 310, 108, 56, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnEmulationM = _addPicButton(theme, "GAME_SETTINGS/EMU_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 380, 310, 56, 56);
	m_gameSettingsBtnEmulationP = _addPicButton(theme, "GAME_SETTINGS/EMU_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 310, 56, 56);
	//Categories Page 1
	//m_gameSettingsLblCategory[0] = _addLabel(theme, "GAME_SETTINGS/CAT_ALL", theme.lblFont, L"All", 40, 130, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	//m_gameSettingsBtnCategory[0] = _addButton(theme, "GAME_SETTINGS/CAT_ALL_BTN", 330, 130, 270, 56);

	m_gameSettingsLblCategory[1] = _addLabel(theme, "GAME_SETTINGS/CAT_1", 40, 130, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[1] = _addButton(theme, "GAME_SETTINGS/CAT_1_BTN", 330, 130, 270, 56);
	m_gameSettingsLblCategory[2] = _addLabel(theme, "GAME_SETTINGS/CAT_2", 40, 190, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[2] = _addButton(theme, "GAME_SETTINGS/CAT_2_BTN", 330, 190, 270, 56);
	m_gameSettingsLblCategory[3] = _addLabel(theme, "GAME_SETTINGS/CAT_3", 40, 250, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[3] = _addButton(theme, "GAME_SETTINGS/CAT_3_BTN", 330, 250, 270, 56);
	m_gameSettingsLblCategory[4] = _addLabel(theme, "GAME_SETTINGS/CAT_4", 40, 310, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[4] = _addButton(theme, "GAME_SETTINGS/CAT_4_BTN", 330, 310, 270, 56);

	//Categories Page 2
	m_gameSettingsLblCategory[5] = _addLabel(theme, "GAME_SETTINGS/CAT_5", 40, 130, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[5] = _addButton(theme, "GAME_SETTINGS/CAT_5_BTN", 330, 130, 270, 56);
	m_gameSettingsLblCategory[6] = _addLabel(theme, "GAME_SETTINGS/CAT_6", 40, 190, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[6] = _addButton(theme, "GAME_SETTINGS/CAT_6_BTN", 330, 190, 270, 56);
	m_gameSettingsLblCategory[7] = _addLabel(theme, "GAME_SETTINGS/CAT_7", 40, 250, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[7] = _addButton(theme, "GAME_SETTINGS/CAT_7_BTN", 330, 250, 270, 56);
	m_gameSettingsLblCategory[8] = _addLabel(theme, "GAME_SETTINGS/CAT_8", 40, 310, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[8] = _addButton(theme, "GAME_SETTINGS/CAT_8_BTN", 330, 310, 270, 56);

	//Categories Page 3
	m_gameSettingsLblCategory[9] = _addLabel(theme, "GAME_SETTINGS/CAT_9", 40, 130, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[9] = _addButton(theme, "GAME_SETTINGS/CAT_9_BTN", 330, 130, 270, 56);
	m_gameSettingsLblCategory[10] = _addLabel(theme, "GAME_SETTINGS/CAT_10", 40, 190, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[10] = _addButton(theme, "GAME_SETTINGS/CAT_10_BTN", 330, 190, 270, 56);
	m_gameSettingsLblCategory[11] = _addLabel(theme, "GAME_SETTINGS/CAT_11", 40, 250, 290, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategory[11] = _addButton(theme, "GAME_SETTINGS/CAT_11_BTN", 330, 250, 270, 56);

	//
	m_gameSettingsLblPage = _addLabel(theme, "GAME_SETTINGS/PAGE_BTN", 76, 410, 80, 56, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnPageM = _addPicButton(theme, "GAME_SETTINGS/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 410, 56, 56);
	m_gameSettingsBtnPageP = _addPicButton(theme, "GAME_SETTINGS/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 156, 410, 56, 56);
	m_gameSettingsBtnBack = _addButton(theme, "GAME_SETTINGS/BACK_BTN", 420, 410, 200, 56);
	//
	_setHideAnim(m_gameSettingsLblTitle, "GAME_SETTINGS/TITLE", 0, -200, 0.f, 1.f);
	_setHideAnim(m_gameSettingsLblGameVideo, "GAME_SETTINGS/VIDEO", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblVideo, "GAME_SETTINGS/VIDEO_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnVideoM, "GAME_SETTINGS/VIDEO_MINUS", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnVideoP, "GAME_SETTINGS/VIDEO_PLUS", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblGameLanguage, "GAME_SETTINGS/GAME_LANG", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblLanguage, "GAME_SETTINGS/GAME_LANG_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnLanguageM, "GAME_SETTINGS/GAME_LANG_MINUS", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnLanguageP, "GAME_SETTINGS/GAME_LANG_PLUS", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblOcarina, "GAME_SETTINGS/OCARINA", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnOcarina, "GAME_SETTINGS/OCARINA_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblCheat, "GAME_SETTINGS/CHEAT", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCheat, "GAME_SETTINGS/CHEAT_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblCountryPatch, "GAME_SETTINGS/COUNTRY_PATCH", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCountryPatch, "GAME_SETTINGS/COUNTRY_PATCH_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblVipatch, "GAME_SETTINGS/VIPATCH", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnVipatch, "GAME_SETTINGS/VIPATCH_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblCover, "GAME_SETTINGS/COVER", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCover, "GAME_SETTINGS/COVER_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblPage, "GAME_SETTINGS/PAGE_BTN", 0, 200, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPageM, "GAME_SETTINGS/PAGE_MINUS", 0, 200, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPageP, "GAME_SETTINGS/PAGE_PLUS", 0, 200, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnBack, "GAME_SETTINGS/BACK_BTN", 0, 200, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblPatchVidModes, "GAME_SETTINGS/PATCH_VIDEO_MODE", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblPatchVidModesVal, "GAME_SETTINGS/PATCH_VIDEO_MODE_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPatchVidModesM, "GAME_SETTINGS/PATCH_VIDEO_MODE_MINUS", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPatchVidModesP, "GAME_SETTINGS/PATCH_VIDEO_MODE_PLUS", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblHooktype, "GAME_SETTINGS/HOOKTYPE", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblHooktypeVal, "GAME_SETTINGS/HOOKTYPE_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnHooktypeM, "GAME_SETTINGS/HOOKTYPE_MINUS", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnHooktypeP, "GAME_SETTINGS/HOOKTYPE_PLUS", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblEmulation, "GAME_SETTINGS/EMU", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblEmulationVal, "GAME_SETTINGS/EMU_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnEmulationP, "GAME_SETTINGS/EMU_PLUS", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnEmulationM, "GAME_SETTINGS/EMU_MINUS", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblDebugger, "GAME_SETTINGS/GAME_DEBUGGER", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblDebuggerV, "GAME_SETTINGS/GAME_DEBUGGER_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDebuggerM, "GAME_SETTINGS/GAME_DEBUGGER_MINUS", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDebuggerP, "GAME_SETTINGS/GAME_DEBUGGER_PLUS", 0, 0, 1.f, 0.f);
	//Categories
	_setHideAnim(m_gameSettingsBtnCategoryMain, "GAME_SETTINGS/CAT_MAIN_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblCategoryMain, "GAME_SETTINGS/CAT_MAIN", 0, 0, 1.f, 0.f);
	//_setHideAnim(m_gameSettingsBtnCategory[0], "GAME_SETTINGS/CAT_ALL_BTN", 0, 0, 1.f, 0.f);
	//_setHideAnim(m_gameSettingsLblCategory[0], "GAME_SETTINGS/CAT_ALL", 0, 0, 1.f, 0.f);
	for (int i = 1; i < 12; ++i) {
		_setHideAnim(m_gameSettingsBtnCategory[i], sfmt("GAME_SETTINGS/CAT_%i_BTN", i).c_str(), 0, 0, 1.f, 0.f);
		_setHideAnim(m_gameSettingsLblCategory[i], sfmt("GAME_SETTINGS/CAT_%i", i).c_str(), 0, 0, 1.f, 0.f);
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
	m_btnMgr.setText(m_gameSettingsLblVipatch, _t("cfgg7", L"Vipatch"));
	m_btnMgr.setText(m_gameSettingsBtnBack, _t("cfgg8", L"Back"));
	m_btnMgr.setText(m_gameSettingsLblCover, _t("cfgg12", L"Download cover"));
	m_btnMgr.setText(m_gameSettingsBtnCover, _t("cfgg13", L"Download"));
	m_btnMgr.setText(m_gameSettingsLblPatchVidModes, _t("cfgg14", L"Patch video modes"));
	m_btnMgr.setText(m_gameSettingsLblCheat, _t("cfgg15", L"Cheat Codes"));
	m_btnMgr.setText(m_gameSettingsBtnCheat, _t("cfgg16", L"Select"));
	m_btnMgr.setText(m_gameSettingsLblCategoryMain, _t("cfgg17", L"Categories"));
	m_btnMgr.setText(m_gameSettingsBtnCategoryMain, _t("cfgg16", L"Select"));
	m_btnMgr.setText(m_gameSettingsLblHooktype, _t("cfgg18", L"Hook Type"));
	m_btnMgr.setText(m_gameSettingsLblDebugger, _t("cfgg22", L"Debugger"));
	m_btnMgr.setText(m_gameSettingsLblEmulation, _t("cfgg24", L"Savegame Emulation"));

	for (int i = 1; i < 12; ++i)
		m_btnMgr.setText(m_gameSettingsLblCategory[i], m_cat.getWString("GENERAL", fmt("cat%d",i), wfmt(L"Category %i",i).c_str()));
}