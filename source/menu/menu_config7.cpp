#include "menu.hpp"
#include "gecko.h"

using namespace std;

static const int g_curPage = 7;

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

void CMenu::_hideConfig7(bool instant)
{
	m_btnMgr.hide(m_configLblTitle, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_configLblPage, instant);
	m_btnMgr.hide(m_configBtnPageM, instant);
	m_btnMgr.hide(m_configBtnPageP, instant);
	// 
	m_btnMgr.hide(m_config7LblPartitionName, instant);
	m_btnMgr.hide(m_config7LblPartition, instant);
	m_btnMgr.hide(m_config7BtnPartitionP, instant);
	m_btnMgr.hide(m_config7BtnPartitionM, instant);
	m_btnMgr.hide(m_config7LblAsyncNet, instant);
	m_btnMgr.hide(m_config7BtnAsyncNet, instant);

	for (u32 i = 0; i < ARRAY_SIZE(m_config7LblUser); ++i)
		if (m_config7LblUser[i] != -1u)
			m_btnMgr.hide(m_config7LblUser[i], instant);
}

void CMenu::_showConfig7(void)
{
	_setBg(m_config7Bg, m_config7Bg);
	m_btnMgr.show(m_configLblTitle);
	m_btnMgr.show(m_configBtnBack);
	m_btnMgr.show(m_configLblPage);
	m_btnMgr.show(m_configBtnPageM);
	m_btnMgr.show(m_configBtnPageP);
	// 
	m_btnMgr.show(m_config7LblPartitionName);
	m_btnMgr.show(m_config7LblPartition);
	m_btnMgr.show(m_config7BtnPartitionP);
	m_btnMgr.show(m_config7BtnPartitionM);
	m_btnMgr.show(m_config7LblAsyncNet);
	m_btnMgr.show(m_config7BtnAsyncNet);
	
	for (u32 i = 0; i < ARRAY_SIZE(m_config7LblUser); ++i)
		if (m_config7LblUser[i] != -1u)
			m_btnMgr.show(m_config7LblUser[i]);
	// 
	m_btnMgr.setText(m_configLblPage, wfmt(L"%i / %i", g_curPage, m_locked ? g_curPage : CMenu::_nbCfgPages));

	char *name = (char *)"NULL";
	if(m_current_view == COVERFLOW_USB)
		name = (char *)DeviceName[m_cfg.getInt("GENERAL", "partition", 1)];
	else if(m_current_view == COVERFLOW_HOMEBREW)
		name = (char *)DeviceName[m_cfg.getInt("GENERAL", "homebrew_partition", 1)];
	else if(m_current_view == COVERFLOW_CHANNEL)
	{
		bool disable = m_cfg.getBool("NAND", "Disable_EMU", true);
		name = disable ? (char *)"NAND" : (char *)DeviceName[m_cfg.getInt("GENERAL", "nand_partition", 1)];
	}

	for(u8 i = 0; strncmp((const char *)&name[i], "\0", 1) != 0; i++)
					name[i] = toupper(name[i]);

	m_btnMgr.setText(m_config7LblPartition, (string)name);
	m_btnMgr.setText(m_config7BtnAsyncNet, m_cfg.getBool("GENERAL", "async_network", false) ? _t("on", L"On") : _t("off", L"Off"));
}

int CMenu::_config7(void)
{
	int nextPage = 0;
	SetupInput();
	
	s32 bCurrentPartition = currentPartition;

	gprintf("Current Partition: %d\n", currentPartition);

	_showConfig7();
	while (true)
	{
		_mainLoopCommon();
		if (BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if (BTN_UP_PRESSED)
			m_btnMgr.up();
		else if (BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if (BTN_LEFT_PRESSED || BTN_MINUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configBtnPageM)))
		{
			nextPage = g_curPage == 1 && !m_locked ? 7 : max(1, m_locked ? 1 : g_curPage - 1);
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED) m_btnMgr.click(m_configBtnPageM);
			break;
		}
		if (!m_locked && (BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configBtnPageP))))
		{
			nextPage = (g_curPage == CMenu::_nbCfgPages) ? 1 : min(g_curPage + 1, CMenu::_nbCfgPages);
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED) m_btnMgr.click(m_configBtnPageP);
			break;
		}
		if (BTN_A_PRESSED)
		{
			if (m_btnMgr.selected(m_configBtnBack))
				break;
			else if (m_btnMgr.selected(m_config7BtnPartitionP) || m_btnMgr.selected(m_config7BtnPartitionM))
			{
				bool disable = m_current_view == COVERFLOW_CHANNEL && m_cfg.getBool("NAND", "Disable_EMU", true);
				if(!disable)
				{
					s8 offset = m_btnMgr.selected(m_config7BtnPartitionP) ? 1 : -1;
					currentPartition = loopNum(currentPartition + offset, (int)USB8);
					while(!DeviceHandler::Instance()->IsInserted(currentPartition) ||
						(m_current_view == COVERFLOW_CHANNEL && (DeviceHandler::Instance()->GetFSType(currentPartition) != PART_FS_FAT ||
							DeviceHandler::Instance()->PathToDriveType(m_appDir.c_str()) == currentPartition ||
							DeviceHandler::Instance()->PathToDriveType(m_dataDir.c_str()) == currentPartition)) ||
						(m_current_view == COVERFLOW_HOMEBREW && DeviceHandler::Instance()->GetFSType(currentPartition) == PART_FS_WBFS))
							currentPartition = loopNum(currentPartition + offset, (int)USB8);

					gprintf("Next item: %s\n", DeviceName[currentPartition]);
					if(m_current_view == COVERFLOW_USB)
						m_cfg.setInt("GENERAL", "partition", currentPartition);
					else if(m_current_view == COVERFLOW_HOMEBREW)
						m_cfg.setInt("GENERAL", "homebrew_partition", currentPartition);
					else if (m_current_view == COVERFLOW_CHANNEL)
						m_cfg.setInt("NAND", "nand_partition", currentPartition);
				}
					_showConfig7();
			}
			else if (m_btnMgr.selected(m_config7BtnAsyncNet))
			{
				m_cfg.setBool("GENERAL", "async_network", !m_cfg.getBool("GENERAL", "async_network", false));
				_showConfig7();
			}
		}
	}
	if (currentPartition != bCurrentPartition)
	{
		bool disable = m_current_view == COVERFLOW_CHANNEL && m_cfg.getBool("NAND", "Disable_EMU", true);
		if(!disable)
		{
			char *newpartition = (char *)"NULL";
			if(m_current_view == COVERFLOW_USB)
				newpartition = (char *)DeviceName[m_cfg.getInt("GENERAL", "partition", currentPartition)];
			else if(m_current_view == COVERFLOW_HOMEBREW)
				newpartition = (char *)DeviceName[m_cfg.getInt("GENERAL", "homebrew_partition", currentPartition)];
			else if(m_current_view == COVERFLOW_CHANNEL)
				newpartition = disable ? (char *)"NAND" : (char *)DeviceName[m_cfg.getInt("NAND", "nand_partition", currentPartition)];
				
			for(u8 i = 0; strncmp((const char *)&newpartition[i], "\0", 1) != 0; i++)
				newpartition[i] = toupper(newpartition[i]);

			gprintf("Switching partition to %s\n", newpartition);
			_loadList();
		}
	}
	
	_hideConfig7();
	return nextPage;
}

void CMenu::_initConfig7Menu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_config7LblUser, ARRAY_SIZE(m_config7LblUser), "CONFIG7");
	m_config7Bg = _texture(theme.texSet, "CONFIG7/BG", "texture", theme.bg);
	m_config7LblPartitionName = _addLabel(theme, "CONFIG7/PARTITION", theme.lblFont, L"", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config7LblPartition = _addLabel(theme, "CONFIG7/PARTITION_BTN", theme.lblFont, L"", 386, 130, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config7BtnPartitionM = _addPicButton(theme, "CONFIG7/PARTITION_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 130, 56, 56);
	m_config7BtnPartitionP = _addPicButton(theme, "CONFIG7/PARTITION_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);
	m_config7LblAsyncNet = _addLabel(theme, "CONFIG7/ASYNCNET", theme.lblFont, L"", 40, 250, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config7BtnAsyncNet = _addButton(theme, "CONFIG7/ASYNCNET_BTN", theme.btnFont, L"", 330, 250, 270, 56, theme.btnFontColor);
	// 
	_setHideAnim(m_config7LblPartitionName, "CONFIG7/PARTITION", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config7LblPartition, "CONFIG7/PARTITION_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config7BtnPartitionM, "CONFIG7/PARTITION_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config7BtnPartitionP, "CONFIG7/PARTITION_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config7LblAsyncNet, "CONFIG7/ASYNCNET", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config7BtnAsyncNet, "CONFIG7/ASYNCNET_BTN", 0, 0, 1.f, -1.f);
	_hideConfig7(true);
	_textConfig7();
}

void CMenu::_textConfig7(void)
{
	m_btnMgr.setText(m_config7LblPartitionName, _t("cfgp1", L"Game Partition"));
	m_btnMgr.setText(m_config7LblAsyncNet, _t("cfgp3", L"Init network on boot"));
}
