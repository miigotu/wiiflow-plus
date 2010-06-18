/*
Comment:
--------
New class

Class shows system menu and initiates downloading of a new .dol

comment: This code is not very elegant as it uses directly items of menu_download
Todo in later releases: clean up this code and separate download and system code

Revision:
---------
Update
*/

#include "svnrev.h"
#include "menu.hpp"
#include "fs.h"
#include "libs/libfat/file_allocation_table.h"
#include <wiiuse/wpad.h>
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "gecko.h"

extern int mainIOS;
extern int mainIOSRev;

class LockMutex
{
	mutex_t &m_mutex;
public:
	LockMutex(mutex_t &m) : m_mutex(m) { LWP_MutexLock(m_mutex); }
	~LockMutex(void) { LWP_MutexUnlock(m_mutex); }
};

void CMenu::_system()
{
	s32 padsState;
	WPADData *wd;
	u32 btn;
	lwp_t thread = 0;
	int msg = 0;
	wstringEx prevMsg;

	WPAD_Rumble(WPAD_CHAN_0, 0);
	_showSystem();
	m_btnMgr.setText(m_systemBtnBack, _t("dl1", L"Cancel"));
	m_thrdStop = false;
	m_thrdMessageAdded = false;

	while (true)
	{
		WPAD_ScanPads();
		padsState = WPAD_ButtonsDown(0);
		wd = WPAD_Data(0);
		btn = _btnRepeat(wd->btns_h);
		if ((padsState & (WPAD_BUTTON_HOME | WPAD_BUTTON_B)) != 0 && !m_thrdWorking)
			break;
		if (wd->ir.valid)
			m_btnMgr.mouse(wd->ir.x - m_cur.width() / 2, wd->ir.y - m_cur.height() / 2);
		else if ((padsState & WPAD_BUTTON_UP) != 0)
			m_btnMgr.up();
		else if ((padsState & WPAD_BUTTON_DOWN) != 0)
			m_btnMgr.down();
		if (((padsState & WPAD_BUTTON_A) != 0) && !(m_thrdWorking && m_thrdStop))
		{
			m_btnMgr.click();
			if ((m_btnMgr.selected() == m_systemBtnDownload) && !m_thrdWorking)
			{
				// Download new version
				m_btnMgr.show(m_downloadPBar);
				m_btnMgr.setProgress(m_downloadPBar, 0.f);
				m_btnMgr.hide(m_systemLblVersionTxt);
				m_btnMgr.hide(m_systemLblVersion);
				m_btnMgr.hide(m_systemLblVersionRev);
				m_btnMgr.hide(m_systemLblIOSTxt);
				m_btnMgr.hide(m_systemLblIOS);
				m_btnMgr.hide(m_systemLblIOSbase);
				m_btnMgr.hide(m_systemBtnDownload);
				m_thrdStop = false;
				m_thrdWorking = true;
				LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_versionDownloaderInit, (void *)this, 0, 8192, 40);
			}
			else if (m_btnMgr.selected() == m_systemBtnBack)
			{
				LockMutex lock(m_mutex);
				m_thrdStop = true;
				m_thrdMessageAdded = true;
				m_thrdMessage = _t("dlmsg6", L"Canceling...");
			}
		}
		if (Sys_Exiting())
		{
			LockMutex lock(m_mutex);
			m_thrdStop = true;
			m_thrdMessageAdded = true;
			m_thrdMessage = _t("dlmsg6", L"Canceling...");
		}
		// 
		if (m_thrdMessageAdded)
		{
			LockMutex lock(m_mutex);
			m_thrdMessageAdded = false;
			m_btnMgr.setProgress(m_downloadPBar, m_thrdProgress);
			if (m_thrdProgress == 1.f)
				m_btnMgr.setText(m_systemBtnBack, _t("dl2", L"Back"));
			if (prevMsg != m_thrdMessage)
			{
				prevMsg = m_thrdMessage;
				m_btnMgr.setText(m_downloadLblMessage[msg], m_thrdMessage, false);
				m_btnMgr.hide(m_downloadLblMessage[msg], -200, 0, 1.f, 0.5f, true);
				m_btnMgr.show(m_downloadLblMessage[msg]);
				msg ^= 1;
				m_btnMgr.hide(m_downloadLblMessage[msg], +400, 0, 1.f, 1.f);
			}
		}
		_mainLoopCommon(wd, false, m_thrdWorking);
		if (m_thrdStop && !m_thrdWorking)
			break;
	}
	WPAD_Rumble(WPAD_CHAN_0, 0);
	_hideSystem();
}

void CMenu::_hideSystem(bool instant)
{
	m_btnMgr.hide(m_systemLblTitle, instant);
	m_btnMgr.hide(m_systemLblVersionTxt, instant);
	m_btnMgr.hide(m_systemLblVersion, instant);
	m_btnMgr.hide(m_systemLblVersionRev, instant);
	m_btnMgr.hide(m_systemLblIOSTxt, instant);
	m_btnMgr.hide(m_systemLblIOS, instant);
	m_btnMgr.hide(m_systemLblIOSbase, instant);
	m_btnMgr.hide(m_systemBtnBack, instant);
	m_btnMgr.hide(m_systemBtnDownload, instant);
	m_btnMgr.hide(m_downloadPBar, instant);
	m_btnMgr.hide(m_downloadLblMessage[0], 0, 0, -2.f, 0.f, instant);
	m_btnMgr.hide(m_downloadLblMessage[1], 0, 0, -2.f, 0.f, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_systemLblUser); ++i)
	if (m_systemLblUser[i] != -1u)
		m_btnMgr.hide(m_systemLblUser[i], instant);
}

void CMenu::_showSystem(void)
{
	_setBg(m_systemBg, m_systemBg);
	m_btnMgr.show(m_systemLblTitle);
	m_btnMgr.show(m_systemLblVersionTxt);
	m_btnMgr.show(m_systemLblVersion);
	m_btnMgr.show(m_systemLblVersionRev);
	m_btnMgr.show(m_systemLblIOSTxt);
	m_btnMgr.show(m_systemLblIOS);
	m_btnMgr.show(m_systemLblIOSbase);
	m_btnMgr.show(m_systemBtnBack);
	//m_btnMgr.show(m_systemBtnDownload);
	for (u32 i = 0; i < ARRAY_SIZE(m_systemLblUser); ++i)
		if (m_systemLblUser[i] != -1u)
			m_btnMgr.show(m_systemLblUser[i]);
	_textSystem();
}

void CMenu::_initSystemMenu(CMenu::SThemeData &theme)
{
	STexture emptyTex;

	_addUserLabels(theme, m_systemLblUser, ARRAY_SIZE(m_systemLblUser), "SYSTEM");		
	m_systemBg = _texture(theme.texSet, "SYSTEM/BG", "texture", theme.bg);
	m_systemLblTitle = _addLabel(theme, "SYSTEM/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_systemLblVersionTxt = _addLabel(theme, "SYSTEM/VERSION_TXT", theme.lblFont, L"", 40, 110, 220, 56, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_systemLblVersion = _addLabel(theme, "SYSTEM/VERSION", theme.lblFont, L"", 260, 110, 200, 56, theme.titleFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_systemLblVersionRev = _addLabel(theme, "SYSTEM/VERSION_REV", theme.lblFont, L"", 310, 110, 200, 56, theme.titleFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_systemLblIOSTxt = _addLabel(theme, "SYSTEM/IOS_TXT", theme.lblFont, L"", 40, 146, 220, 56, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_systemLblIOS = _addLabel(theme, "SYSTEM/IOS", theme.lblFont, L"", 260, 146, 200, 56, theme.titleFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_systemLblIOSbase = _addLabel(theme, "SYSTEM/IOSBASE", theme.lblFont, L"", 260, 184, 200, 56, theme.titleFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_systemBtnDownload = _addButton(theme, "SYSTEM/DOWNLOAD_BTN", theme.btnFont, L"", 20, 410, 200, 56, theme.btnFontColor);
	m_systemBtnBack = _addButton(theme, "SYSTEM/BACK_BTN", theme.btnFont, L"", 420, 410, 200, 56, theme.btnFontColor); 
	// 
	_setHideAnim(m_systemLblTitle, "SYSTEM/TITLE", 0, 100, 0.f, 0.f);
	_setHideAnim(m_systemBtnDownload, "SYSTEM/DOWNLOAD_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_systemBtnBack, "SYSTEM/BACK_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_systemLblVersionTxt, "SYSTEM/VERSION_TXT", -100, 0, 0.f, 0.f);
	_setHideAnim(m_systemLblVersion, "SYSTEM/VERSION", 200, 0, 0.f, 0.f);
	_setHideAnim(m_systemLblVersionRev, "SYSTEM/VERSION_REV", 200, 0, 0.f, 0.f);
	_setHideAnim(m_systemLblIOSTxt, "SYSTEM/IOS_TXT", -100, 0, 0.f, 0.f);
	_setHideAnim(m_systemLblIOS, "SYSTEM/IOS", 200, 0, 0.f, 0.f);
	_setHideAnim(m_systemLblIOSbase, "SYSTEM/IOSBASE", 200, 0, 0.f, 0.f);
	// 
	_hideSystem(true);
	_textSystem();
}

void CMenu::_textSystem(void)
{
	int ReV = atoi(SVN_REV);
	m_btnMgr.setText(m_systemLblTitle, _t("sys1", L"System"));
	// version info
	m_btnMgr.setText(m_systemLblVersionTxt, _t("sys2", L"Version:"));
	m_btnMgr.setText(m_systemLblVersion, wfmt(L"%s",APP_VERSION).c_str() );
	m_btnMgr.setText(m_systemLblVersionRev, wfmt(L"%i",ReV).c_str() );
	m_btnMgr.setText(m_systemLblIOSTxt, _t("sys6", L"IOS:"));
	m_btnMgr.setText(m_systemLblIOS, wfmt(_fmt("ios", L"%i v%i"), mainIOS, mainIOSRev).c_str(), true);
	m_btnMgr.setText(m_systemLblIOSbase, wfmt(_fmt("base", L"Base %i"), m_loaded_ios_base).c_str(), true);
	m_btnMgr.setText(m_systemBtnBack, _t("sys3", L"Cancel"));
	m_btnMgr.setText(m_systemBtnDownload, _t("sys4", L"Upgrade"));
}
