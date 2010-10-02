#include "menu.hpp"
#include "loader/sys.h"
#include "xml/xml.h"
#include "lockMutex.hpp"

void CMenu::_hideWiiTDBUpdate(bool instant)
{
	m_btnMgr.hide(m_downloadBtnCancel, instant);
	m_btnMgr.hide(m_downloadPBar, instant);
	m_btnMgr.hide(m_downloadLblMessage[0], 0, 0, -2.f, 0.f, instant);
	m_btnMgr.hide(m_downloadLblMessage[1], 0, 0, -2.f, 0.f, instant);
}

void CMenu::_showWiiTDBUpdate(void)
{
	_setBg(m_downloadBg, m_downloadBg);
	m_btnMgr.show(m_downloadBtnCancel);
	m_btnMgr.show(m_downloadPBar);
}

bool CMenu::_updateProgress(void *obj, float progress)
{
	CMenu *m = (CMenu *)obj;
	LWP_MutexLock(m->m_mutex);
	m->_setThrdMsg(L"...", progress);
	LWP_MutexUnlock(m->m_mutex);
	return !m->m_thrdStop;
}

u32 CMenu::_updateWiiTDBAsync(void *obj)
{
	CMenu *m = (CMenu *)obj;
	if (!m->m_thrdWorking)
		return 0;
		
	// Update wiitdb here
	LWP_MutexLock(m->m_mutex);
	m->_setThrdMsg(m->_t("wtmsg1", L"Creating WiiTDB database..."), 0);
	LWP_MutexUnlock(m->m_mutex);

	rebuild_database(m->m_settingsDir.c_str(), (char *) m->m_curLanguage.c_str(), 1, (bool (*)(void *, float)) CMenu::_updateProgress, (void *) m);
	
	m->m_thrdStop = true;
	m->m_thrdWorking = false;
	return 0;
}

void CMenu::_updateWiiTDB()
{
	int msg = 0;
	wstringEx prevMsg;

	if (wiitdb_requires_update(m_settingsDir.c_str()))
	{
		m_btnMgr.setProgress(m_downloadPBar, 0.f);
		_showWiiTDBUpdate();
		m_btnMgr.setText(m_downloadBtnCancel, _t("dl1", L"Cancel"));
		m_thrdStop = false;
		m_thrdMessageAdded = false;

		m_thrdWorking = true;
		lwp_t thread = LWP_THREAD_NULL;
		LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_updateWiiTDBAsync, (void *)this, 0, 8192, 40);		
		while (true)
		{
			_mainLoopCommon(false, m_thrdWorking);
			if ((BTN_HOME_PRESSED || BTN_B_PRESSED) && !m_thrdWorking)
				break;
			if (BTN_A_PRESSED && !(m_thrdWorking && m_thrdStop))
			{
				if (m_btnMgr.selected(m_downloadBtnCancel))
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
				m_thrdWorking = false;
			}

			if (m_thrdMessageAdded)
			{
				LockMutex lock(m_mutex);
				m_thrdMessageAdded = false;
				m_btnMgr.setProgress(m_downloadPBar, m_thrdProgress);
				if (m_thrdProgress >= 1.f) {
					// m_btnMgr.setText(m_downloadBtnCancel, _t("dl2", L"Back"));
					break;
				}
				if (prevMsg != m_thrdMessage)
				{
					prevMsg = m_thrdMessage;
					m_btnMgr.setText(m_downloadLblMessage[msg], m_thrdMessage, false);
					m_btnMgr.hide(m_downloadLblMessage[msg], 0, 0, -1.f, -1.f, true);
					m_btnMgr.show(m_downloadLblMessage[msg]);
					msg ^= 1;
					m_btnMgr.hide(m_downloadLblMessage[msg], 0, 0, -1.f, -1.f);
				}
			}
			if (m_thrdStop && !m_thrdWorking)
				break;
		}
		if (thread != LWP_THREAD_NULL)
		{
			LWP_JoinThread(thread, NULL);
			thread = LWP_THREAD_NULL;
		}
		_hideWiiTDBUpdate();
	}
	else
	{
		ReloadXMLDatabase(m_settingsDir.c_str(), (char *) m_curLanguage.c_str(), 1);
	}
}
