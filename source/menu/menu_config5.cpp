
#include "menu.hpp"
#include "wbfs.h"


#include "gecko.h"

using namespace std;

static const int g_curPage = 7;

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

void CMenu::_hideConfig5(bool instant)
{
	m_btnMgr.hide(m_configLblTitle, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_configLblPage, instant);
	m_btnMgr.hide(m_configBtnPageM, instant);
	m_btnMgr.hide(m_configBtnPageP, instant);
	// 
	m_btnMgr.hide(m_config5LblPartitionName, instant);
	m_btnMgr.hide(m_config5LblPartition, instant);
	m_btnMgr.hide(m_config5BtnPartitionP, instant);
	m_btnMgr.hide(m_config5BtnPartitionM, instant);
	m_btnMgr.hide(m_config5LblAsyncNet, instant);
	m_btnMgr.hide(m_config5BtnAsyncNet, instant);

	for (u32 i = 0; i < ARRAY_SIZE(m_config5LblUser); ++i)
		if (m_config5LblUser[i] != -1u)
			m_btnMgr.hide(m_config5LblUser[i], instant);
}

void CMenu::_showConfig5(void)
{
	_setBg(m_config5Bg, m_config5Bg);
	m_btnMgr.show(m_configLblTitle);
	m_btnMgr.show(m_configBtnBack);
	m_btnMgr.show(m_configLblPage);
	m_btnMgr.show(m_configBtnPageM);
	m_btnMgr.show(m_configBtnPageP);
	// 
	m_btnMgr.show(m_config5LblPartitionName);
	m_btnMgr.show(m_config5LblPartition);
	m_btnMgr.show(m_config5BtnPartitionP);
	m_btnMgr.show(m_config5BtnPartitionM);
	m_btnMgr.show(m_config5LblAsyncNet);
	m_btnMgr.show(m_config5BtnAsyncNet);
	
	for (u32 i = 0; i < ARRAY_SIZE(m_config5LblUser); ++i)
		if (m_config5LblUser[i] != -1u)
			m_btnMgr.show(m_config5LblUser[i]);
	// 
	m_btnMgr.setText(m_configLblPage, wfmt(L"%i / %i", g_curPage, m_locked ? g_curPage : CMenu::_nbCfgPages));
	m_btnMgr.setText(m_config5LblPartition, m_cfg.getString("GENERAL", "partition", "WBFS1"));
	m_btnMgr.setText(m_config5BtnAsyncNet, m_cfg.getBool("GENERAL", "async_network", false) ? _t("on", L"On") : _t("off", L"Off"));
}

int CMenu::_config5(void)
{
	int nextPage = 0;
	SetupInput();
	
	s32 amountOfPartitions = WBFS_GetPartitionCount();
	s32 currentPartition = WBFS_GetCurrentPartition();
	s32 bCurrentPartition = currentPartition;

	gprintf("Current Partition: %d\n", currentPartition);
	gprintf("Amount of partitions: %d\n", amountOfPartitions);

	_showConfig5();
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
		else if (buttonHeld != (u32)-1 && m_btnMgr.selected(buttonHeld) && repeatButton >= 16 && (repeatButton % 4 == 0))
			wii_btnsPressed |= WBTN_A;
		if (BTN_LEFT_PRESSED || BTN_MINUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configBtnPageM)))
		{
			nextPage = g_curPage == 1 && !m_locked ? 7 : max(1, m_locked ? 1 : g_curPage - 1);
			m_btnMgr.click(m_wmote, m_configBtnPageM);
			break;
		}
		if (!m_locked && (BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configBtnPageP))))
		{
			nextPage = (g_curPage == CMenu::_nbCfgPages) ? 1 : min(g_curPage + 1, CMenu::_nbCfgPages);
			m_btnMgr.click(m_wmote, m_configBtnPageP);
			break;
		}
		if (BTN_A_PRESSED)
		{
			m_btnMgr.click(m_wmote);
			if (m_btnMgr.selected(m_configBtnBack))
				break;
			else if (m_btnMgr.selected(m_config5BtnPartitionP))
			{
				char buf[5];
				currentPartition = loopNum(currentPartition + 1, amountOfPartitions);
				gprintf("Next item: %d\n", currentPartition);
				WBFS_GetPartitionName(currentPartition, (char *) &buf);
				gprintf("Which is: %s\n", buf);
				m_cfg.setString("GENERAL", "partition", buf);
				_showConfig5();
			}
			else if (m_btnMgr.selected(m_config5BtnPartitionM))
			{
				char buf[5];
				currentPartition = loopNum(currentPartition - 1, amountOfPartitions);
				gprintf("Next item: %d\n", currentPartition);
				WBFS_GetPartitionName(currentPartition, (char *) &buf);
				gprintf("Which is: %s\n", buf);
				m_cfg.setString("GENERAL", "partition", buf);
				_showConfig5();
			}
			else if (m_btnMgr.selected(m_config5BtnAsyncNet))
			{
				m_cfg.setBool("GENERAL", "async_network", !m_cfg.getBool("GENERAL", "async_network", false));
				_showConfig5();
			}
		}
	}
	if (currentPartition != bCurrentPartition)
	{
		gprintf("Switching partition to %s\n", m_cfg.getString("GENERAL", "partition").c_str());
		_loadList();
	}
	
	_hideConfig5();
	return nextPage;
}

void CMenu::_initConfig5Menu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_config5LblUser, ARRAY_SIZE(m_config5LblUser), "CONFIG5");
	m_config5Bg = _texture(theme.texSet, "CONFIG5/BG", "texture", theme.bg);
	m_config5LblPartitionName = _addLabel(theme, "CONFIG5/PARTITION", theme.lblFont, L"", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config5LblPartition = _addLabel(theme, "CONFIG5/PARTITION_BTN", theme.lblFont, L"", 386, 130, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config5BtnPartitionM = _addPicButton(theme, "CONFIG5/PARTITION_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 130, 56, 56);
	m_config5BtnPartitionP = _addPicButton(theme, "CONFIG5/PARTITION_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);
	m_config5LblAsyncNet = _addLabel(theme, "CONFIG5/ASYNCNET", theme.lblFont, L"", 40, 250, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config5BtnAsyncNet = _addButton(theme, "CONFIG5/ASYNCNET_BTN", theme.btnFont, L"", 330, 250, 270, 56, theme.btnFontColor);
	// 
	_setHideAnim(m_config5LblPartitionName, "CONFIG5/PARTITION", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config5LblPartition, "CONFIG5/PARTITION_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config5BtnPartitionM, "CONFIG5/PARTITION_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config5BtnPartitionP, "CONFIG5/PARTITION_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config5LblAsyncNet, "CONFIG5/ASYNCNET", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config5BtnAsyncNet, "CONFIG5/ASYNCNET_BTN", 0, 0, 1.f, -1.f);
	_hideConfig5(true);
	_textConfig5();
}

void CMenu::_textConfig5(void)
{
	m_btnMgr.setText(m_config5LblPartitionName, _t("cfgp1", L"Game Partition"));
	m_btnMgr.setText(m_config5LblAsyncNet, _t("cfgp3", L"Init network on boot"));
}
