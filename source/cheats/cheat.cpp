#include <string.h>
#include <gccore.h>
#include "cheat.hpp"
#include "loader/fs.h"
#include "text.hpp"

#include "menu.hpp"
#include "http.h"

#define FILEDIR	"sd:/codes/%s.gct"
#define FILEDIR_USB	"usb:/codes/%s.gct"
#define FILEDIR_GECKO "sd:/codes/%s.gct"
#define FILEDIR_GECKO_USB "usb:/codes/%s.gct"

#define TXTDIR_FLOW "sd:/txtcodes/%s.txt"
#define TXTDIR_FLOW_USB "usb:/txtcodes/%s.txt"

#define GECKOURL "http://www.geckocodes.org//codes//R//%s.txt"

extern const DISC_INTERFACE __io_wiisd;
extern DISC_INTERFACE __io_usbstorage;

void loadCheatFile(SmartBuf &buffer, u32 &size, const char *gameId)
{
	FILE *fp = 0;
	u32 fileSize;
	SmartBuf fileBuf;

	buffer.release();
	size = 0;
	if (Fat_SDAvailable())
	{
		fp = fopen(sfmt(FILEDIR, gameId).c_str(), "rb");
		if (fp == 0) fp = fopen(sfmt(FILEDIR_GECKO, gameId).c_str(), "rb");
	}
		
	if (fp == 0 && Fat_USBAvailable())
	{
		fp = fopen(sfmt(FILEDIR_USB, gameId).c_str(), "rb");
		if (fp == 0) fp = fopen(sfmt(FILEDIR_GECKO_USB, gameId).c_str(), "rb");
	}

	if (fp == 0)
		return;

	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fileBuf = smartCoverAlloc(fileSize);
	if (!fileBuf)
	{
		fclose(fp);
		return;
	}
	if (fread(fileBuf.get(), 1, fileSize, fp) != fileSize)
	{
		fclose(fp);
		return;
	}
	fclose(fp);
	buffer = fileBuf;
	size = fileSize;
}

void CMenu::_CheatSettings() {
	s32 padsState;
	WPADData *wd;
	u32 btn;
	WPAD_Rumble(WPAD_CHAN_0, 0);

	// try to load cheat file
	int txtavailable=0;
	m_cheatSettingsPage = 1; // init page
	
	if (Fat_SDAvailable())
	{
		txtavailable = m_cheatfile.openTxtfile(sfmt(TXTDIR_FLOW, m_cf.getId().c_str()).c_str()); 
	}
		
	if (txtavailable <= 0 && Fat_USBAvailable())
	{
		txtavailable = m_cheatfile.openTxtfile(sfmt(TXTDIR_FLOW_USB, m_cf.getId().c_str()).c_str()); 
	}
	
	_showCheatSettings();
	_textCheatSettings();
	
	if (txtavailable)
		m_btnMgr.setText(m_cheatLblTitle,wfmt(L"%s",m_cheatfile.getGameName().c_str()));
	else 
		m_btnMgr.setText(m_cheatLblTitle,L"");
	
	while (true)
	{
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
		if ((padsState & WPAD_BUTTON_MINUS) != 0)
		{
			if (m_cheatSettingsPage > 1)
				--m_cheatSettingsPage;
			_showCheatSettings();
			m_btnMgr.click(m_cheatBtnPageM);
		}
		else if ((padsState & WPAD_BUTTON_PLUS) != 0)
		{
			if (m_cheatSettingsPage < (m_cheatfile.getCnt()+4)/5)
				++m_cheatSettingsPage;
			_showCheatSettings();
			m_btnMgr.click(m_cheatBtnPageP);
		}		
		if ((padsState & WPAD_BUTTON_A) != 0)
		{
			m_btnMgr.click();
			if (m_btnMgr.selected() == m_cheatBtnBack)
				break;
			else if (m_btnMgr.selected() == m_cheatBtnPageM)
			{
				if (m_cheatSettingsPage > 1)
					--m_cheatSettingsPage;
				_showCheatSettings();
			}
			else if (m_btnMgr.selected() == m_cheatBtnPageP)
			{
				if (m_cheatSettingsPage < (m_cheatfile.getCnt()+4)/5)
					++m_cheatSettingsPage;
				_showCheatSettings();
			}
				
			for (int i = 0; i < 5; ++i)
				if (m_btnMgr.selected() == m_cheatBtnItem[i]) {
					// handling code for clicked cheat
					m_cheatfile.sCheatSelected[(m_cheatSettingsPage-1)*5 + i] = !m_cheatfile.sCheatSelected[(m_cheatSettingsPage-1)*5 + i];
					_showCheatSettings();
				}
			
			if (m_btnMgr.selected() == m_cheatBtnApply)
			{
				int operation_ok,check = 0;
				//checks if at least one cheat is selected
				for (unsigned int i=0; i < m_cheatfile.getCnt(); ++i) {
					if (m_cheatfile.sCheatSelected[i] == true) 
						{
						check = 1;
						break;
						}
					
					}
					
				if (Fat_SDAvailable() && check)
				{
					operation_ok = m_cheatfile.createGCT(sfmt(FILEDIR, m_cf.getId().c_str()).c_str()); 
					operation_ok = m_cheatfile.createTXT(sfmt(TXTDIR_FLOW, m_cf.getId().c_str()).c_str()); 
					
				}
					
				if (!Fat_SDAvailable() <= 0 && Fat_USBAvailable() && check)
				{
					operation_ok = m_cheatfile.createGCT(sfmt(FILEDIR_USB, m_cf.getId().c_str()).c_str()); 
					operation_ok = m_cheatfile.createTXT(sfmt(TXTDIR_FLOW_USB, m_cf.getId().c_str()).c_str()); 
				}
				m_cfg.setOptBool(m_cf.getId(), "cheat",1);
				if (operation_ok)
					break;
			}

			if (m_btnMgr.selected() == m_cheatBtnDownload)
			{
				// Download cheat code
				m_btnMgr.hide(m_cheatLblTitle);
				
				u32 bufferSize = 0x100000;	// Maximum download size 1 MB
				SmartBuf buffer;
				block cheatfile;
				FILE *file;
				char ip[16];

				
				if (!m_networkInit && _initNetwork(ip) < 0) {
					m_btnMgr.hide(m_cheatLblTitle);
					break;
				}
				m_networkInit = true;

				buffer = smartCoverAlloc(bufferSize);
				cheatfile = downloadfile(buffer.get(), bufferSize, sfmt(GECKOURL, m_cf.getId().c_str()).c_str(),CMenu::_downloadProgress, this);
				if (cheatfile.data != NULL && cheatfile.size > 65 && cheatfile.data[0] != '<') {
					// cheat file was downloaded and presumably no 404
					if (Fat_SDAvailable())
						file = fopen(sfmt(TXTDIR_FLOW, m_cf.getId().c_str()).c_str(), "wb");
					else	
						if (!Fat_SDAvailable() && Fat_USBAvailable())
							file = fopen(sfmt(TXTDIR_FLOW_USB, m_cf.getId().c_str()).c_str(), "wb");
						else
							break;
							
					if (file != NULL)
					{
						fwrite(cheatfile.data, 1, cheatfile.size, file);
						fclose(file);
						break;
					}
				}
				else
				{
					// cheat code not found, show result
					m_btnMgr.setText(m_cheatLblItem[0], _t("cheat4", L"Download not found."));
					m_btnMgr.setText(m_cheatLblItem[1], wfmt(L"http://www.geckocodes.org/codes/R/%s.txt",m_cf.getId().c_str()));
					m_btnMgr.show(m_cheatLblItem[1]);
				}
			}
		}
		_mainLoopCommon(wd);
	}
	_hideCheatSettings();
}

void CMenu::_hideCheatSettings(bool instant)
{

	m_btnMgr.hide(m_cheatBtnBack, instant);
	m_btnMgr.hide(m_cheatBtnApply, instant);
	m_btnMgr.hide(m_cheatBtnDownload, instant);
	m_btnMgr.hide(m_cheatLblTitle, instant);

	m_btnMgr.hide(m_cheatLblPage, instant);
	m_btnMgr.hide(m_cheatBtnPageM, instant);
	m_btnMgr.hide(m_cheatBtnPageP, instant);
	
	for (int i=0;i<5;++i) {
		m_btnMgr.hide(m_cheatBtnItem[i], instant);
		m_btnMgr.hide(m_cheatLblItem[i], instant);
	}
	
	for (u32 i = 0; i < ARRAY_SIZE(m_cheatLblUser); ++i)
		if (m_cheatLblUser[i] != -1u)
			m_btnMgr.hide(m_cheatLblUser[i]);
}

// CheatMenu
// check for cheat txt file
// if it exists, load it and show cheat texts on screen
// if it does not exist, show download button
void CMenu::_showCheatSettings(void)
{
	_setBg(m_cheatBg, m_cheatBg);
	m_btnMgr.show(m_cheatBtnBack);
	m_btnMgr.show(m_cheatLblTitle);

	for (u32 i = 0; i < ARRAY_SIZE(m_cheatLblUser); ++i)
		if (m_cheatLblUser[i] != -1u)
			m_btnMgr.show(m_cheatLblUser[i]);

	if (m_cheatfile.getCnt() > 0) {

		// cheat found, show apply
		m_btnMgr.show(m_cheatBtnApply);
		m_btnMgr.show(m_cheatLblPage);
		m_btnMgr.show(m_cheatBtnPageM);
		m_btnMgr.show(m_cheatBtnPageP);
		m_btnMgr.setText(m_cheatLblPage, wfmt(L"%i / %i", m_cheatSettingsPage, (m_cheatfile.getCnt()+4)/5)); 
		
		// Show cheats if available, else hide
		for (u32 i=0; i < 5; ++i) {
			// cheat in range?
			if (((m_cheatSettingsPage-1)*5 + i + 1) <= m_cheatfile.getCnt()) 
			{
				//Limit to 70 characters otherwise the Cheatnames overlap
				char tempcheatname[71];
				strncpy(tempcheatname, m_cheatfile.getCheatName((m_cheatSettingsPage-1)*5 + i).c_str(),70);
				tempcheatname[70] = '\0';
				
				// cheat avaiable, show elements and text
				m_btnMgr.setText(m_cheatLblItem[i], wstringEx(tempcheatname));
				//m_btnMgr.setText(m_cheatLblItem[i], m_cheatfile.getCheseleatName((m_cheatSettingsPage-1)*5 + i));
				m_btnMgr.setText(m_cheatBtnItem[i], _optBoolToString(m_cheatfile.sCheatSelected[(m_cheatSettingsPage-1)*5 + i]));
				
				m_btnMgr.show(m_cheatLblItem[i]);
				m_btnMgr.show(m_cheatBtnItem[i]);
			}
			else
			{
				// cheat out of range, hide elements
				m_btnMgr.hide(m_cheatLblItem[i]);
				m_btnMgr.hide(m_cheatBtnItem[i]);
			}
		}


	}
	else
	{
		// no cheat found, allow downloading
		m_btnMgr.show(m_cheatBtnDownload);
		m_btnMgr.setText(m_cheatLblItem[0], _t("cheat3", L"Cheat file for game not found."));
		m_btnMgr.show(m_cheatLblItem[0]);
		
	}
}


void CMenu::_initCheatSettingsMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_cheatLblUser, ARRAY_SIZE(m_cheatLblUser), "CHEAT");
	m_cheatBg = _texture(theme.texSet, "CHEAT/BG", "texture", theme.bg);
	m_cheatLblTitle = _addLabel(theme, "CHEAT/TITLE", theme.lblFont, L"Cheats", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_cheatBtnBack = _addButton(theme, "CHEAT/BACK_BTN", theme.btnFont, L"", 460, 410, 150, 56, theme.btnFontColor);
	m_cheatBtnApply = _addButton(theme, "CHEAT/APPLY_BTN", theme.btnFont, L"", 240, 410, 150, 56, theme.btnFontColor);
	m_cheatBtnDownload = _addButton(theme, "CHEAT/DOWNLOAD_BTN", theme.btnFont, L"", 240, 410, 200, 56, theme.btnFontColor);

	m_cheatLblPage = _addLabel(theme, "CHEAT/PAGE_BTN", theme.btnFont, L"", 76, 410, 80, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_cheatBtnPageM = _addPicButton(theme, "CHEAT/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 410, 56, 56);
	m_cheatBtnPageP = _addPicButton(theme, "CHEAT/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 156, 410, 56, 56);

	m_cheatLblItem[0] = _addLabel(theme, "CHEAT/ITEM_0", theme.lblFont, L"", 40, 100, 460, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cheatBtnItem[0] = _addButton(theme, "CHEAT/ITEM_0_BTN", theme.btnFont, L"", 500, 100, 120, 56, theme.btnFontColor);
	m_cheatLblItem[1] = _addLabel(theme, "CHEAT/ITEM_1", theme.lblFont, L"", 40, 160, 460, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cheatBtnItem[1] = _addButton(theme, "CHEAT/ITEM_1_BTN", theme.btnFont, L"", 500, 160, 120, 56, theme.btnFontColor);
	m_cheatLblItem[2] = _addLabel(theme, "CHEAT/ITEM_2", theme.lblFont, L"", 40, 220, 460, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cheatBtnItem[2] = _addButton(theme, "CHEAT/ITEM_2_BTN", theme.btnFont, L"", 500, 220, 120, 56, theme.btnFontColor);
	m_cheatLblItem[3] = _addLabel(theme, "CHEAT/ITEM_3", theme.lblFont, L"", 40, 280, 460, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cheatBtnItem[3] = _addButton(theme, "CHEAT/ITEM_3_BTN", theme.btnFont, L"", 500, 280, 120, 56, theme.btnFontColor);
	m_cheatLblItem[4] = _addLabel(theme, "CHEAT/ITEM_4", theme.lblFont, L"", 40, 340, 460, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cheatBtnItem[4] = _addButton(theme, "CHEAT/ITEM_4_BTN", theme.btnFont, L"", 500, 340, 120, 56, theme.btnFontColor);

	_setHideAnim(m_cheatBtnApply, "CHEAT/APPLY_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_cheatBtnBack, "CHEAT/BACK_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_cheatBtnDownload, "CHEAT/DOWNLOAD_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_cheatLblPage, "CHEAT/PAGE_BTN", 0, 200, 1.f, 0.f);
	_setHideAnim(m_cheatBtnPageM, "CHEAT/PAGE_MINUS", 0, 200, 1.f, 0.f);
	_setHideAnim(m_cheatBtnPageP, "CHEAT/PAGE_PLUS", 0, 200, 1.f, 0.f);
	
	for (int i=0;i<5;++i) {
		_setHideAnim(m_cheatBtnItem[i], sfmt("CHEAT/ITEM_%i_BTN", i).c_str(), 200, 0, 1.f, 0.f);
		_setHideAnim(m_cheatLblItem[i], sfmt("CHEAT/ITEM_%i_BTN", i).c_str(), -200, 0, 1.f, 0.f);
	}
	
	_hideCheatSettings(true);
	_textCheatSettings();
}

void CMenu::_textCheatSettings(void)
{
	m_btnMgr.setText(m_cheatBtnBack, _t("cheat1", L"Back"));
	m_btnMgr.setText(m_cheatBtnApply, _t("cheat2", L"Apply"));
	m_btnMgr.setText(m_cheatBtnDownload, _t("cfg4", L"Download"));
}
