
#include "menu.hpp"
#include "loader/wbfs.h"
#include "lockMutex.hpp"

using namespace std;

void CMenu::_hideWBFS(bool instant)
{
	m_btnMgr.hide(m_wbfsLblTitle, instant);
	m_btnMgr.hide(m_wbfsPBar, instant);
	m_btnMgr.hide(m_wbfsBtnBack, instant);
	m_btnMgr.hide(m_wbfsBtnGo, instant);
	m_btnMgr.hide(m_wbfsLblDialog);
	m_btnMgr.hide(m_wbfsLblMessage);
	for (u32 i = 0; i < ARRAY_SIZE(m_wbfsLblUser); ++i)
		if (m_wbfsLblUser[i] != -1u)
			m_btnMgr.hide(m_wbfsLblUser[i], instant);
}

void CMenu::_showWBFS(CMenu::WBFS_OP op)
{
	_setBg(m_wbfsBg, m_wbfsBg);
	switch (op)
	{
		case CMenu::WO_ADD_GAME:
				m_btnMgr.setText(m_wbfsLblTitle, _t("wbfsop1", L"Install Game"));
				break;
		case CMenu::WO_REMOVE_GAME:
				m_btnMgr.setText(m_wbfsLblTitle, _t("wbfsop2", L"Delete Game"));
				break;
		case CMenu::WO_FORMAT:
//				m_btnMgr.setText(m_wbfsLblTitle, _t("wbfsop3", L"Format"));
				break;
	};
	m_btnMgr.show(m_wbfsLblTitle);
	m_btnMgr.show(m_wbfsBtnBack);
	m_btnMgr.show(m_wbfsBtnGo);
	m_btnMgr.show(m_wbfsLblDialog);
	for (u32 i = 0; i < ARRAY_SIZE(m_wbfsLblUser); ++i)
		if (m_wbfsLblUser[i] != -1u)
			m_btnMgr.show(m_wbfsLblUser[i]);
}

static void slotLight(bool state)
{
	if (state)
		*(u32 *)0xCD0000C0 |= 0x20;
	else
		*(u32 *)0xCD0000C0 &= ~0x20;
}

void CMenu::_addDiscProgress(int status, int total, void *user_data)
{
	CMenu &m = *(CMenu *)user_data;
	float progress = total == 0 ? 0.f : (float)status / (float)total;
	// Don't synchronize too often
	if (progress - m.m_thrdProgress >= 0.01f)
		m._setThrdMsg(L"", progress);
}

int CMenu::_gameInstaller(void *obj)
{
	CMenu &m = *(CMenu *)obj;
	int ret;

	if (!WBFS_Mounted())
	{
		m.m_thrdWorking = false;
		return -1;
	}

	u64 comp_size = 0, real_size = 0;
	f32 free, used;
	WBFS_DiskSpace(&used, &free);
	WBFS_DVD_Size(&comp_size, &real_size);

	if ((f32)comp_size + (f32)128*1024 >= free * GB_SIZE)
	{
		m._setThrdMsg(wfmt(m._fmt("wbfsop10", L"Not enough space : %lld blocks needed, %i available"), comp_size, free), 0.f);
		ret = -1;
	}
	else
	{
		m._setThrdMsg(L"", 0);

		ret = WBFS_AddGame(CMenu::_addDiscProgress, obj);
		if (ret == 0)
			m._setThrdMsg(m._t("wbfsop8", L"Game installed"), 1.f);
		else
			m._setThrdMsg(m._t("wbfsop9", L"An error has occurred"), 1.f);
		slotLight(true);
	}
	m.m_thrdWorking = false;
	return ret;
}

bool CMenu::_wbfsOp(CMenu::WBFS_OP op)
{
	lwp_t thread = LWP_THREAD_NULL;
	static discHdr header ATTRIBUTE_ALIGN(32);
	bool done = false;
	bool out = false;
	struct AutoLight { AutoLight(void) { } ~AutoLight(void) { slotLight(false); } } aw;
	string cfPos = m_cf.getNextId();

	SetupInput();

	_showWBFS(op);
	switch (op)
	{
		case CMenu::WO_ADD_GAME:
			m_btnMgr.setText(m_wbfsLblDialog, _t("wbfsadddlg", L"Please insert the disc you want to copy, then click on Go."));
			break;
		case CMenu::WO_REMOVE_GAME:
			m_btnMgr.setText(m_wbfsLblDialog, wfmt(_fmt("wbfsremdlg", L"To permanently remove the game : %s, click on Go."), m_cf.getTitle().toUTF8().c_str()));
			break;
		case CMenu::WO_FORMAT:
			break;
	}
	m_thrdStop = false;
	m_thrdMessageAdded = false;
	while (true)
	{
		_mainLoopCommon(false, m_thrdWorking);
		if ((BTN_HOME_PRESSED || BTN_B_PRESSED) && !m_thrdWorking)
			break;
		else if (BTN_UP_PRESSED)
			m_btnMgr.up();
		else if (BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if (BTN_A_PRESSED && !m_thrdWorking)
		{
			if (m_btnMgr.selected(m_wbfsBtnBack))
				break;
			else if (m_btnMgr.selected(m_wbfsBtnGo))
			{
				switch (op)
				{
					case CMenu::WO_ADD_GAME:
						m_btnMgr.show(m_wbfsPBar);
						m_btnMgr.setProgress(m_wbfsPBar, 0.f);
						m_btnMgr.hide(m_wbfsBtnGo);
						m_btnMgr.hide(m_wbfsBtnBack);
						m_btnMgr.show(m_wbfsLblMessage);
						m_btnMgr.setText(m_wbfsLblMessage, L"");
						Disc_SetUSB(NULL);
						if (Disc_Wait() < 0)
						{
							error(_t("wbfsoperr1", L"Disc_Wait failed"));
							out = true;
							break;
						}
						if (Disc_Open() < 0)
						{
							error(_t("wbfsoperr2", L"Disc_Open failed"));
							out = true;
							break;
						}
						if (Disc_IsWii() < 0)
						{
							error(_t("wbfsoperr3", L"This is not a Wii disc!"));
							out = true;
							break;
						}
						Disc_ReadHeader(&header);

						if (_searchGamesByID(header.id).size() != 0)
						{
							error(_t("wbfsoperr4", L"Game already installed"));
							out = true;
							break;
						}
						cfPos = string(header.id);
						m_btnMgr.setText(m_wbfsLblDialog, wfmt(_fmt("wbfsop6", L"Installing [%s] %s..."), header.id, header.title));
						done = true;
						m_thrdWorking = true;
						m_thrdProgress = 0.f;
						m_thrdMessageAdded = false;
						LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_gameInstaller, (void *)this, 0, 8 * 1024, 64);
						break;
					case CMenu::WO_REMOVE_GAME:
						done = WBFS_RemoveGame((char *)m_cf.getId().c_str(), m_cf.getHdr()->path) == 0;
						m_btnMgr.setText(m_wbfsLblMessage, done ? _t("wbfsop7", L"Game deleted") : _t("wbfsop11", L"Deleting game failed!"));
						out = !done;
						m_btnMgr.show(m_wbfsPBar);
						m_btnMgr.setProgress(m_wbfsPBar, 0.f, true);
						m_btnMgr.setProgress(m_wbfsPBar, 1.f);
						m_btnMgr.hide(m_wbfsLblDialog);
						m_btnMgr.hide(m_wbfsBtnGo);
						m_btnMgr.show(m_wbfsLblMessage);
						break;
					case CMenu::WO_FORMAT:
						break;
				}
				if (out)
					break;
			}
		}
		//
		if (m_thrdMessageAdded)
		{
			LockMutex lock(m_mutex);
			m_thrdMessageAdded = false;
			if (!m_thrdMessage.empty())
				m_btnMgr.setText(m_wbfsLblDialog, m_thrdMessage);
			m_btnMgr.setProgress(m_wbfsPBar, m_thrdProgress);
			m_btnMgr.setText(m_wbfsLblMessage, wfmt(_fmt("wbfsprogress", L"%i%%"), (int)(m_thrdProgress * 100.f)));
			if (!m_thrdWorking)
				m_btnMgr.show(m_wbfsBtnBack);
		}
	}
	_hideWBFS();
	if (done && (op == CMenu::WO_REMOVE_GAME || op == CMenu::WO_ADD_GAME))
	{
		m_gameList.SetLanguage(m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str());
		UpdateCache(COVERFLOW_USB);

		_loadList();
		_initCF();
		m_cf.findId(cfPos.c_str(), true);
	}
	return done;
}

void CMenu::_initWBFSMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_wbfsLblUser, ARRAY_SIZE(m_wbfsLblUser), "WBFS");
	m_wbfsBg = _texture(theme.texSet, "WBFS/BG", "texture", theme.bg);
	m_wbfsLblTitle = _addTitle(theme, "WBFS/TITLE", 20, 30, 600, 60, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_wbfsLblDialog = _addLabel(theme, "WBFS/DIALOG", 40, 90, 560, 200, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_wbfsLblMessage = _addLabel(theme, "WBFS/MESSAGE", 40, 300, 560, 100, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_TOP);
	m_wbfsPBar = _addProgressBar(theme, "WBFS/PROGRESS_BAR", 40, 270, 560, 20);
	m_wbfsBtnBack = _addButton(theme, "WBFS/BACK_BTN", 420, 410, 200, 56);
	m_wbfsBtnGo = _addButton(theme, "WBFS/GO_BTN", 245, 260, 150, 56);
	//
	_setHideAnim(m_wbfsLblTitle, "WBFS/TITLE", 0, 0, 1.f, 0.f);
	_setHideAnim(m_wbfsLblDialog, "WBFS/DIALOG", 0, 0, 1.f, 0.f);
	_setHideAnim(m_wbfsLblMessage, "WBFS/MESSAGE", 0, 0, 1.f, 0.f);
	_setHideAnim(m_wbfsPBar, "WBFS/PROGRESS_BAR", 0, 0, 1.f, 0.f);
	_setHideAnim(m_wbfsBtnBack, "WBFS/BACK_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_wbfsBtnGo, "WBFS/GO_BTN", 0, 0, 1.f, 0.f);
	_hideWBFS(true);
	_textWBFS();
}

void CMenu::_textWBFS(void)
{
	m_btnMgr.setText(m_wbfsBtnBack, _t("wbfsop4", L"Back"));
	m_btnMgr.setText(m_wbfsBtnGo, _t("wbfsop5", L"Go"));
}
