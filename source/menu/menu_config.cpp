#include "menu.hpp"
#include "nand.hpp"
#include "sys.h"
#include "loader/cios.hpp"
#include "loader/alt_ios.h"
#include "gecko/gecko.h"

using namespace std;

const int CMenu::_nbCfgPages = 6;
static const int g_curPage = 1;

void CMenu::SwitchPartition(bool direction, bool showLabel)
{
	int emu_mode = EMU_DISABLED;
	m_cfg.getInt("NAND", "emulation", &emu_mode);
	bool enabled = (m_current_view == COVERFLOW_CHANNEL && emu_mode) || m_current_view != COVERFLOW_CHANNEL;
	if(enabled)
	{
		Nand::Instance()->Disable_Emu();
		iosinfo_t * iosInfo = cIOSInfo::GetInfo(mainIOS);
		Nand::Instance()->Enable_Emu();
		s8 step = direction ? 1 : -1;
		currentPartition = loopNum(currentPartition + step, (int)USB8);
		while(!DeviceHandler::Instance()->IsInserted(currentPartition) ||
			(m_current_view == COVERFLOW_CHANNEL && (DeviceHandler::Instance()->GetFSType(currentPartition) != PART_FS_FAT ||
				(iosInfo->version < 7 && DeviceHandler::Instance()->PathToDriveType(m_appDir.c_str()) == currentPartition) ||
				(iosInfo->version < 7 && DeviceHandler::Instance()->PathToDriveType(m_dataDir.c_str()) == currentPartition))) ||
			(m_current_view == COVERFLOW_HOMEBREW && DeviceHandler::Instance()->GetFSType(currentPartition) == PART_FS_WBFS))
		{
			currentPartition = loopNum(currentPartition + step, (int)USB8);
			static u8 limiter = SD;
			if (limiter > MAXDEVICES) break;
			limiter++;
		}

		gprintf("Next item: %s\n", DeviceName[currentPartition]);
	}

	m_cfg.setInt(_domainFromView(), "partition", currentPartition);

	if(showLabel)
	{
		char *partition = enabled ? (char *)DeviceName[currentPartition] : (char *)"NAND";

		for(u8 i = 0; strncmp((const char *)&partition[i], "\0", 1) != 0; i++)
			partition[i] = toupper(partition[i]);

		m_showtimer=60; 
		m_btnMgr.setText(m_mainLblNotice, (string)partition);
		m_btnMgr.show(m_mainLblNotice);
	}
}

void CMenu::_hideConfig(bool instant)
{
	m_btnMgr.hide(m_configLblTitle, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_configLblPage, instant);
	m_btnMgr.hide(m_configBtnPageM, instant);
	m_btnMgr.hide(m_configBtnPageP, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_configLblPartitionName, instant);
	m_btnMgr.hide(m_configLblPartition, instant);
	m_btnMgr.hide(m_configBtnPartitionP, instant);
	m_btnMgr.hide(m_configBtnPartitionM, instant);
	m_btnMgr.hide(m_configLblDownload, instant);
	m_btnMgr.hide(m_configBtnDownload, instant);
	m_btnMgr.hide(m_configLblParental, instant);
	m_btnMgr.hide(m_configBtnUnlock, instant);
	m_btnMgr.hide(m_configBtnSetCode, instant);
	m_btnMgr.hide(m_configLblEmulationVal, instant);
	m_btnMgr.hide(m_configLblEmulation, instant);
	m_btnMgr.hide(m_configBtnEmulationP, instant);
	m_btnMgr.hide(m_configBtnEmulationM, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_configLblUser); ++i)
		if (m_configLblUser[i] != -1u)
			m_btnMgr.hide(m_configLblUser[i], instant);
}

void CMenu::_showConfig(void)
{
	_setBg(m_configBg, m_configBg);
	m_btnMgr.show(m_configLblTitle);
	m_btnMgr.show(m_configBtnBack);
	if(!m_locked)
	{
		m_btnMgr.show(m_configLblPartitionName);
		m_btnMgr.show(m_configLblPartition);
		m_btnMgr.show(m_configBtnPartitionP);
		m_btnMgr.show(m_configBtnPartitionM);
		m_btnMgr.show(m_configLblDownload);
		m_btnMgr.show(m_configBtnDownload);
	}
	m_btnMgr.show(m_configLblParental);
	m_btnMgr.show(m_configLblPage);
	m_btnMgr.show(m_configBtnPageM);
	m_btnMgr.show(m_configBtnPageP);

	m_btnMgr.show(m_locked ? m_configBtnUnlock : m_configBtnSetCode);

	int emu_mode = EMU_DISABLED;
	m_cfg.getInt("NAND", "emulation", &emu_mode);
	bool enabled = (m_current_view == COVERFLOW_CHANNEL && emu_mode) || m_current_view != COVERFLOW_CHANNEL;
	char *partitionname = enabled ? (char *)DeviceName[m_cfg.getInt(_domainFromView(), "partition", currentPartition)] : (char *)"NAND";

	for(u8 i = 0; strncmp((const char *)&partitionname[i], "\0", 1) != 0; i++)
		partitionname[i] = toupper(partitionname[i]);

	for (u32 i = 0; i < ARRAY_SIZE(m_configLblUser); ++i)
		if (m_configLblUser[i] != -1u)
			m_btnMgr.show(m_configLblUser[i]);

	m_btnMgr.setText(m_configLblPartition, (string)partitionname);
	m_btnMgr.setText(m_configLblPage, wfmt(L"%i / %i", g_curPage, m_locked ? g_curPage : CMenu::_nbCfgPages));

	if ((m_current_view == COVERFLOW_CHANNEL || m_current_view == COVERFLOW_USB) && !m_locked)
	{
		int i = EMU_DISABLED;
		m_cfg.getInt(_domainFromView(), "emulation", &i);
		m_btnMgr.setText(m_configLblEmulationVal, _t(CMenu::_Emulation[i].id, CMenu::_Emulation[i].text));
		m_btnMgr.setText(m_configLblEmulation, _t("cfg11", L"Emulation"));

		m_btnMgr.show(m_configLblEmulation);
		m_btnMgr.show(m_configLblEmulationVal);
		m_btnMgr.show(m_configBtnEmulationP);
		m_btnMgr.show(m_configBtnEmulationM);
	}
}

void CMenu::_config(int page)
{
	m_curGameId = m_cf.getId();
	m_cf.clear();
	while (page > 0 && page <= CMenu::_nbCfgPages)
		switch (page)
		{
			case 1:
				page = _config1();
				break;
			case 2:
				page = _configAdv();
				break;
			case 3:
				page = _config3();
				break;
			case 4:
				page = _config4();
				break;
			case 5:
				page = _configSnd();
				break;
			case 6:
				page = _configScreen();
				break;
		}
	m_cfg.save();
	_initCF();
}

int CMenu::_config1(void)
{
	int nextPage = 0;
	SetupInput();

	s32 bCurrentPartition = currentPartition;

	gprintf("Current Partition: %d\n", currentPartition);
	
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
		if (BTN_LEFT_PRESSED || BTN_MINUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configBtnPageM)))
		{
			nextPage = m_locked ? 1 : loopNum(g_curPage - 1, CMenu::_nbCfgPages + 1);
			if(nextPage <= 0) nextPage = CMenu::_nbCfgPages;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED) m_btnMgr.click(m_configBtnPageM);
			break;
		}
		if (BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configBtnPageP)))
		{
			nextPage = m_locked ? 1 : loopNum(g_curPage + 1, CMenu::_nbCfgPages + 1);
			if(nextPage <= 0) nextPage = 1;
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED) m_btnMgr.click(m_configBtnPageP);
			break;
		}
		if (BTN_A_PRESSED)
		{
			if (m_btnMgr.selected(m_configBtnBack))
				break;
			else if (m_btnMgr.selected(m_configBtnDownload))
			{
				m_cf.stopCoverLoader(true);
				_hideConfig();
				_download();
				_showConfig();
				m_cf.startCoverLoader();
			}
			else if (m_btnMgr.selected(m_configBtnUnlock))
			{
				char code[4];
				_hideConfig();
				if (_code(code) && memcmp(code, m_cfg.getString("GENERAL", "parent_code").c_str(), 4) == 0)
					m_locked = false;
				else
					error(_t("cfgg25",L"Password incorrect."));
				_showConfig();
			}
			else if (m_btnMgr.selected(m_configBtnSetCode))
			{
				char code[4];
				_hideConfig();
				if (_code(code, true))
				{
					m_cfg.setString("GENERAL", "parent_code", string(code, 4).c_str());
					m_locked = true;
				}
				_showConfig();
			}
			else if (!m_locked && (m_btnMgr.selected(m_configBtnPartitionP) || m_btnMgr.selected(m_configBtnPartitionM)))
			{
				SwitchPartition(m_btnMgr.selected(m_configBtnPartitionP));
				_showConfig();
			}
			else if (!m_locked && (m_btnMgr.selected(m_configBtnEmulationP) || m_btnMgr.selected(m_configBtnEmulationM)))
			{
				s8 direction = m_btnMgr.selected(m_configBtnEmulationP) ? 1 : -1;
				int value = (int)loopNum((u32)m_cfg.getInt(_domainFromView(), "emulation", EMU_DISABLED) + direction, ARRAY_SIZE(CMenu::_Emulation) - 1u);
				if(value > EMU_FULL) value = EMU_DISABLED;

				if(value)
					m_cfg.setInt(_domainFromView(), "emulation", value);
				else
					m_cfg.remove(_domainFromView(), "emulation");
				_showConfig();
			}
		}
	}
	int emu_mode = EMU_DISABLED;
	m_cfg.getInt("NAND", "emulation", &emu_mode);
	if(currentPartition != bCurrentPartition && (m_current_view != COVERFLOW_CHANNEL || (m_current_view == COVERFLOW_CHANNEL && emu_mode)))
	{
		_showWaitMessage();
		_loadList();
		_hideWaitMessage();
	}

	_hideConfig();
	
	return nextPage;
}

void CMenu::_initConfigMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_configLblUser, ARRAY_SIZE(m_configLblUser), "CONFIG");
	m_configBg = _texture(theme.texSet, "CONFIG/BG", "texture", theme.bg);
	m_configLblTitle = _addTitle(theme, "CONFIG/TITLE", 20, 30, 600, 60, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_configLblDownload = _addLabel(theme, "CONFIG/DOWNLOAD", 40, 130, 340, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtnDownload = _addButton(theme, "CONFIG/DOWNLOAD_BTN", 400, 130, 200, 56);
	m_configLblParental = _addLabel(theme, "CONFIG/PARENTAL", 40, 190, 340, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtnUnlock = _addButton(theme, "CONFIG/UNLOCK_BTN", 400, 190, 200, 56);
	m_configBtnSetCode = _addButton(theme, "CONFIG/SETCODE_BTN", 400, 190, 200, 56);
	m_configLblPartitionName = _addLabel(theme, "CONFIG/PARTITION", 40, 250, 340, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configLblPartition = _addLabel(theme, "CONFIG/PARTITION_BTN", 456, 250, 88, 56, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configBtnPartitionM = _addPicButton(theme, "CONFIG/PARTITION_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 400, 250, 56, 56);
	m_configBtnPartitionP = _addPicButton(theme, "CONFIG/PARTITION_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 250, 56, 56);
	m_configLblEmulation = _addLabel(theme, "CONFIG/EMU", 40, 310, 340, 56, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configLblEmulationVal = _addLabel(theme, "CONFIG/EMU_BTN", 456, 310, 88, 56, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configBtnEmulationM = _addPicButton(theme, "CONFIG/EMU_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 400, 310, 56, 56);
	m_configBtnEmulationP = _addPicButton(theme, "CONFIG/EMU_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 310, 56, 56);
	m_configLblPage = _addLabel(theme, "CONFIG/PAGE_BTN", 76, 410, 80, 56, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configBtnPageM = _addPicButton(theme, "CONFIG/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 410, 56, 56);
	m_configBtnPageP = _addPicButton(theme, "CONFIG/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 156, 410, 56, 56);
	m_configBtnBack = _addButton(theme, "CONFIG/BACK_BTN", 420, 410, 200, 56);
	// 
	_setHideAnim(m_configLblTitle, "CONFIG/TITLE", 0, 0, 1.f, 0.f);

	_setHideAnim(m_configLblDownload, "CONFIG/DOWNLOAD", 0, 0, 1.f, 0.f);
	_setHideAnim(m_configBtnDownload, "CONFIG/DOWNLOAD_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_configLblParental, "CONFIG/PARENTAL", 0, 0, 1.f, 0.f);
	_setHideAnim(m_configBtnUnlock, "CONFIG/UNLOCK_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_configBtnSetCode, "CONFIG/SETCODE_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_configLblPartitionName, "CONFIG/PARTITION", 0, 0, 1.f, 0.f);
	_setHideAnim(m_configLblPartition, "CONFIG/PARTITION_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnPartitionM, "CONFIG/PARTITION_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnPartitionP, "CONFIG/PARTITION_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configLblEmulation, "CONFIG/EMU", 0, 0, 1.f, 0.f);
	_setHideAnim(m_configLblEmulationVal, "CONFIG/EMU_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_configBtnEmulationM, "CONFIG/EMU_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnEmulationP, "CONFIG/EMU_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnBack, "CONFIG/BACK_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_configLblPage, "CONFIG/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnPageM, "CONFIG/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnPageP, "CONFIG/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_hideConfig(true);
	_textConfig();
}

void CMenu::_textConfig(void)
{
	m_btnMgr.setText(m_configLblTitle, _t("cfg1", L"Settings"));
	m_btnMgr.setText(m_configLblDownload, _t("cfg3", L"Download covers & titles"));
	m_btnMgr.setText(m_configBtnDownload, _t("cfg4", L"Download"));
	m_btnMgr.setText(m_configLblParental, _t("cfg5", L"Parental control"));
	m_btnMgr.setText(m_configBtnUnlock, _t("cfg6", L"Unlock"));
	m_btnMgr.setText(m_configBtnSetCode, _t("cfg7", L"Set code"));

	m_btnMgr.setText(m_configLblPartitionName, _t("cfgp1", L"Game Partition"));
	m_btnMgr.setText(m_configBtnBack, _t("cfg10", L"Back"));
}
