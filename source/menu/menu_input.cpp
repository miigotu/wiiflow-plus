#include "menu.hpp"
#include "gecko.h"

using namespace std;


static const u32 g_repeatDelay = 15;

void CMenu::SetupInput()
{
	m_wpadLeftDelay = 0;
	m_wpadDownDelay = 0;
	m_wpadRightDelay = 0;
	m_wpadUpDelay = 0;
	m_padLeftDelay = 0;
	m_padDownDelay = 0;
	m_padRightDelay = 0;
	m_padUpDelay = 0;
	repeatButton = 0;		
	buttonHeld = (u32)-1;
	
	for(int wmote=0;wmote<4;wmote++)
	{
		angle[wmote] = 0;
		mag[wmote] = 0;
	}
}

void CMenu::ScanInput()
{
	//use left sticks only for now
	int right = 0;
	
	m_show_zone_main = false;
	m_show_zone_main2 = false;
	m_show_zone_main3 = false;
	m_show_zone_prev = false;
	m_show_zone_next = false;
	
    WPAD_ScanPads();
    PAD_ScanPads();
	
	ButtonsPressed();
	ButtonsHeld();
	wii_repeat = wii_btnRepeat();
	gc_repeat = gc_btnRepeat();
	
	for(int wmote=0;wmote<4;wmote++)
	{
		wd[wmote] = WPAD_Data(wmote);
		angle[wmote] = 0;
		mag[wmote] = 0;		
		switch (wd[wmote]->exp.type)
		{
			case WPAD_EXP_NUNCHUK:
			case WPAD_EXP_GUITARHERO3:
				if (right == 0)
				{
					mag[wmote] = wd[wmote]->exp.nunchuk.js.mag;
					angle[wmote] = wd[wmote]->exp.nunchuk.js.ang;
				}
				break;
			case WPAD_EXP_CLASSIC:
				if (right == 0)
				{
					mag[wmote] = wd[wmote]->exp.classic.ljs.mag;
					angle[wmote] = wd[wmote]->exp.classic.ljs.ang;
				} 
				else
				{
					mag[wmote] = wd[wmote]->exp.classic.rjs.mag;
					angle[wmote] = wd[wmote]->exp.classic.rjs.ang;
				}
				break;

			default:
				break;
		}
	}
	if (WPadIR_Valid(0))
	{
		m_shown_pointer = 1;
		m_btnMgr.mouse(0, wd[0]->ir.x - m_cursor1.width() / 2, wd[0]->ir.y - m_cursor1.height() / 2);
		m_cursor1.draw(wd[0]->ir.x, wd[0]->ir.y, wd[0]->ir.angle);
	}
	else if (WPadIR_Valid(1))
	{
		m_shown_pointer = 2;
		m_btnMgr.mouse(1, wd[1]->ir.x - m_cursor2.width() / 2, wd[1]->ir.y - m_cursor2.height() / 2);
		m_cursor2.draw(wd[1]->ir.x, wd[1]->ir.y, wd[1]->ir.angle);
	}
	else if (WPadIR_Valid(2))
	{
		m_shown_pointer = 3;
		m_btnMgr.mouse(2, wd[2]->ir.x - m_cursor3.width() / 2, wd[2]->ir.y - m_cursor3.height() / 2);
		m_cursor3.draw(wd[2]->ir.x, wd[2]->ir.y, wd[2]->ir.angle);
	}
	else if (WPadIR_Valid(3))
	{
		m_shown_pointer = 4;
		m_btnMgr.mouse(3, wd[3]->ir.x - m_cursor4.width() / 2, wd[3]->ir.y - m_cursor4.height() / 2);
		m_cursor4.draw(wd[3]->ir.x, wd[3]->ir.y, wd[3]->ir.angle);
	}
	ShowMainZone();
	ShowMainZone2();
	ShowMainZone3();
	ShowPrevZone();
	ShowNextZone();
}

void CMenu::ButtonsPressed()
{
    int i;
	wii_btnsPressed = 0;
	gc_btnsPressed = 0;

    for (i=3; i >= 0; i--) {
        wii_btnsPressed |= WPAD_ButtonsDown(i);
        gc_btnsPressed |= PAD_ButtonsDown(i);
    }
}

void CMenu::ButtonsHeld()
{
    int i;
	wii_btnsHeld = 0;
	gc_btnsHeld = 0;

    for (i=3; i >= 0; i--) {
        wii_btnsHeld |= WPAD_ButtonsHeld(i);
        gc_btnsHeld |= PAD_ButtonsHeld(i);
    }
}

bool CMenu::WPadIR_Valid(int i)
{
	wd[i] = WPAD_Data(i);
	if (wd[i]->ir.valid)
		return true;
	return false;
}

bool CMenu::WPadIR_ANY()
{
	return (wd[0]->ir.valid || wd[1]->ir.valid || wd[2]->ir.valid || wd[3]->ir.valid);
}

u32 CMenu::wii_btnRepeat()
{
	u32 b = 0;
	wii_repeat = wii_btnsHeld;

	if ((wii_repeat & WBTN_LEFT) != 0)
	{
		if (m_wpadLeftDelay == 0 || m_wpadLeftDelay > g_repeatDelay)
			b |= WPAD_BUTTON_LEFT;
		++m_wpadLeftDelay;
	}
	else
		m_wpadLeftDelay = 0;
	if ((wii_repeat & WBTN_DOWN) != 0)
	{
		if (m_wpadDownDelay == 0 || m_wpadDownDelay > g_repeatDelay)
			b |= WPAD_BUTTON_DOWN;
		++m_wpadDownDelay;
	}
	else
		m_wpadDownDelay = 0;
	if ((wii_repeat & WBTN_RIGHT) != 0)
	{
		if (m_wpadRightDelay == 0 || m_wpadRightDelay > g_repeatDelay)
			b |= WPAD_BUTTON_RIGHT;
		++m_wpadRightDelay;
	}
	else
		m_wpadRightDelay = 0;
	if ((wii_repeat & WBTN_UP) != 0)
	{
		if (m_wpadUpDelay == 0 || m_padUpDelay > g_repeatDelay)
			b |= WPAD_BUTTON_UP;
		++m_wpadUpDelay;
	}
	else
		m_wpadUpDelay = 0;
	return b;
}

u32 CMenu::gc_btnRepeat()
{
	u32 b = 0;
	gc_repeat = gc_btnsHeld;

	if ((gc_repeat & BTN_LEFT) != 0)
	{
		if (m_padLeftDelay == 0 || m_padLeftDelay > g_repeatDelay)
			b |= BTN_LEFT;
		++m_padLeftDelay;
	}
	else
		m_padLeftDelay = 0;
	if (gc_repeat & BTN_DOWN)
	{
		if (m_padDownDelay == 0 || m_padDownDelay > g_repeatDelay)
			b |= BTN_DOWN;
		++m_padDownDelay;
	}
	else
		m_padDownDelay = 0;
	if (gc_repeat & BTN_RIGHT)
	{
		if (m_padRightDelay == 0 || m_padRightDelay > g_repeatDelay)
			b |= BTN_RIGHT;
		++m_padRightDelay;
	}
	else
		m_padRightDelay = 0;
	if (gc_repeat & BTN_UP)
	{
		if (m_padUpDelay == 0 || m_padUpDelay > g_repeatDelay)
			b |= BTN_UP;
		++m_padUpDelay;
	}
	else
		m_padUpDelay = 0;
	return b;
}

void CMenu::ShowMainZone()
{
	m_show_zone_main = false;
	if ((WPadIR_Valid(0) && m_cursor1.x() >= m_mainButtonsZone.x && m_cursor1.y() >= m_mainButtonsZone.y
		&& m_cursor1.x() < m_mainButtonsZone.x + m_mainButtonsZone.w && m_cursor1.y() < m_mainButtonsZone.y + m_mainButtonsZone.h) 
		|| (WPadIR_Valid(1) && m_cursor2.x() >= m_mainButtonsZone.x && m_cursor2.y() >= m_mainButtonsZone.y
		&& m_cursor2.x() < m_mainButtonsZone.x + m_mainButtonsZone.w && m_cursor2.y() < m_mainButtonsZone.y + m_mainButtonsZone.h)
		|| (WPadIR_Valid(2) && m_cursor3.x() >= m_mainButtonsZone.x && m_cursor3.y() >= m_mainButtonsZone.y
		&& m_cursor3.x() < m_mainButtonsZone.x + m_mainButtonsZone.w && m_cursor3.y() < m_mainButtonsZone.y + m_mainButtonsZone.h)
		|| (WPadIR_Valid(3) && m_cursor4.x() >= m_mainButtonsZone.x && m_cursor4.y() >= m_mainButtonsZone.y
		&& m_cursor4.x() < m_mainButtonsZone.x + m_mainButtonsZone.w && m_cursor4.y() < m_mainButtonsZone.y + m_mainButtonsZone.h))
		m_show_zone_main = true;
}

void CMenu::ShowMainZone2()
{
	m_show_zone_main2 = false;
	if ((WPadIR_Valid(0) && m_cursor1.x() >= m_mainButtonsZone2.x && m_cursor1.y() >= m_mainButtonsZone2.y
		&& m_cursor1.x() < m_mainButtonsZone2.x + m_mainButtonsZone2.w && m_cursor1.y() < m_mainButtonsZone2.y + m_mainButtonsZone2.h)
		|| (WPadIR_Valid(1) && m_cursor2.x() >= m_mainButtonsZone2.x && m_cursor2.y() >= m_mainButtonsZone2.y
		&& m_cursor2.x() < m_mainButtonsZone2.x + m_mainButtonsZone2.w && m_cursor2.y() < m_mainButtonsZone2.y + m_mainButtonsZone2.h)
		|| (WPadIR_Valid(2) && m_cursor3.x() >= m_mainButtonsZone2.x && m_cursor3.y() >= m_mainButtonsZone2.y
		&& m_cursor3.x() < m_mainButtonsZone2.x + m_mainButtonsZone2.w && m_cursor3.y() < m_mainButtonsZone2.y + m_mainButtonsZone2.h)
		|| (WPadIR_Valid(3) && m_cursor4.x() >= m_mainButtonsZone2.x && m_cursor4.y() >= m_mainButtonsZone2.y
		&& m_cursor4.x() < m_mainButtonsZone2.x + m_mainButtonsZone2.w && m_cursor4.y() < m_mainButtonsZone2.y + m_mainButtonsZone2.h))
		m_show_zone_main2 = true;
}

void CMenu::ShowMainZone3()
{
	m_show_zone_main3 = false;
	if ((WPadIR_Valid(0) && m_cursor1.x() >= m_mainButtonsZone3.x && m_cursor1.y() >= m_mainButtonsZone3.y
		&& m_cursor1.x() < m_mainButtonsZone3.x + m_mainButtonsZone3.w && m_cursor1.y() < m_mainButtonsZone3.y + m_mainButtonsZone3.h)
		|| (WPadIR_Valid(1) && m_cursor2.x() >= m_mainButtonsZone3.x && m_cursor2.y() >= m_mainButtonsZone3.y
		&& m_cursor2.x() < m_mainButtonsZone3.x + m_mainButtonsZone3.w && m_cursor2.y() < m_mainButtonsZone3.y + m_mainButtonsZone3.h)
		|| (WPadIR_Valid(2) && m_cursor3.x() >= m_mainButtonsZone3.x && m_cursor3.y() >= m_mainButtonsZone3.y
		&& m_cursor3.x() < m_mainButtonsZone3.x + m_mainButtonsZone3.w && m_cursor3.y() < m_mainButtonsZone3.y + m_mainButtonsZone3.h)
		|| (WPadIR_Valid(3) && m_cursor4.x() >= m_mainButtonsZone3.x && m_cursor4.y() >= m_mainButtonsZone3.y
		&& m_cursor4.x() < m_mainButtonsZone3.x + m_mainButtonsZone3.w && m_cursor4.y() < m_mainButtonsZone3.y + m_mainButtonsZone3.h))
		m_show_zone_main3 = true;
}

void CMenu::ShowPrevZone()
{
	m_show_zone_prev = false;
	if ((WPadIR_Valid(0) && m_cursor1.x() >= m_mainPrevZone.x && m_cursor1.y() >= m_mainPrevZone.y
		&& m_cursor1.x() < m_mainPrevZone.x + m_mainPrevZone.w && m_cursor1.y() < m_mainPrevZone.y + m_mainPrevZone.h)
		|| (WPadIR_Valid(1) && m_cursor2.x() >= m_mainPrevZone.x && m_cursor2.y() >= m_mainPrevZone.y
		&& m_cursor2.x() < m_mainPrevZone.x + m_mainPrevZone.w && m_cursor2.y() < m_mainPrevZone.y + m_mainPrevZone.h)
		|| (WPadIR_Valid(2) && m_cursor3.x() >= m_mainPrevZone.x && m_cursor3.y() >= m_mainPrevZone.y
		&& m_cursor3.x() < m_mainPrevZone.x + m_mainPrevZone.w && m_cursor3.y() < m_mainPrevZone.y + m_mainPrevZone.h)
		|| (WPadIR_Valid(3) && m_cursor4.x() >= m_mainPrevZone.x && m_cursor4.y() >= m_mainPrevZone.y
		&& m_cursor4.x() < m_mainPrevZone.x + m_mainPrevZone.w && m_cursor4.y() < m_mainPrevZone.y + m_mainPrevZone.h))
		m_show_zone_prev = true;
}

void CMenu::ShowNextZone()
{
	m_show_zone_next = false;
	if ((WPadIR_Valid(0) && m_cursor1.x() >= m_mainNextZone.x && m_cursor1.y() >= m_mainNextZone.y
		&& m_cursor1.x() < m_mainNextZone.x + m_mainNextZone.w && m_cursor1.y() < m_mainNextZone.y + m_mainNextZone.h)
		|| (WPadIR_Valid(1) && m_cursor2.x() >= m_mainNextZone.x && m_cursor2.y() >= m_mainNextZone.y
		&& m_cursor2.x() < m_mainNextZone.x + m_mainNextZone.w && m_cursor2.y() < m_mainNextZone.y + m_mainNextZone.h)
		|| (WPadIR_Valid(2) && m_cursor3.x() >= m_mainNextZone.x && m_cursor3.y() >= m_mainNextZone.y
		&& m_cursor3.x() < m_mainNextZone.x + m_mainNextZone.w && m_cursor3.y() < m_mainNextZone.y + m_mainNextZone.h)
		|| (WPadIR_Valid(3) && m_cursor4.x() >= m_mainNextZone.x && m_cursor4.y() >= m_mainNextZone.y
		&& m_cursor4.x() < m_mainNextZone.x + m_mainNextZone.w && m_cursor4.y() < m_mainNextZone.y + m_mainNextZone.h))
		m_show_zone_next = true;
}
