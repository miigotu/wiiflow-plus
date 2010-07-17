#include "menu.hpp"
#include "gecko.h"

using namespace std;


static const u32 g_repeatDelay = 15;

void CMenu::SetupInput()
{
	WPAD_Rumble(WPAD_CHAN_ALL, 0);
	
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
	
    WPAD_ScanPads();
    PAD_ScanPads();
	
	ButtonsPressed();
	ButtonsHeld();
	btn = _btnRepeat();

	for(int wmote=0;wmote<4;wmote++)
	{
		wd[wmote] = WPAD_Data(wmote);
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
		if (WPadIR_Valid(wmote))
		{
			m_btnMgr.mouse(wmote, wd[wmote]->ir.x - m_cur.width() / 2, wd[wmote]->ir.y - m_cur.height() / 2);
			m_cur.draw(wd[wmote]->ir.x, wd[wmote]->ir.y, wd[wmote]->ir.angle);
		}
	}
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

u32 CMenu::_btnRepeat()
{
	u32 b = 0;
	btn = wii_btnsHeld;
	//WPAD
	if (BTN_LEFT_REPEAT)
	{
		if (m_padLeftDelay == 0 || m_padLeftDelay > g_repeatDelay)
			b |= WBTN_LEFT;
		++m_padLeftDelay;
	}
	else
		m_padLeftDelay = 0;
	if (BTN_DOWN_REPEAT)
	{
		if (m_padDownDelay == 0 || m_padDownDelay > g_repeatDelay)
			b |= WBTN_DOWN;
		++m_padDownDelay;
	}
	else
		m_padDownDelay = 0;
	if (BTN_RIGHT_REPEAT)
	{
		if (m_padRightDelay == 0 || m_padRightDelay > g_repeatDelay)
			b |= WBTN_RIGHT;
		++m_padRightDelay;
	}
	else
		m_padRightDelay = 0;
	if (BTN_UP_REPEAT)
	{
		if (m_padUpDelay == 0 || m_padUpDelay > g_repeatDelay)
			b |= WBTN_UP;
		++m_padUpDelay;
	}
	else
		m_padUpDelay = 0;
	//GC Pad
	if ((btn & BTN_LEFT) != 0)
	{
		if (m_padLeftDelay == 0 || m_padLeftDelay > g_repeatDelay)
			b |= BTN_LEFT;
		++m_padLeftDelay;
	}
	else
		m_padLeftDelay = 0;
	if ((btn & BTN_DOWN) != 0)
	{
		if (m_padDownDelay == 0 || m_padDownDelay > g_repeatDelay)
			b |= BTN_DOWN;
		++m_padDownDelay;
	}
	else
		m_padDownDelay = 0;
	if ((btn & BTN_RIGHT) != 0)
	{
		if (m_padRightDelay == 0 || m_padRightDelay > g_repeatDelay)
			b |= BTN_RIGHT;
		++m_padRightDelay;
	}
	else
		m_padRightDelay = 0;
	if ((btn & BTN_UP) != 0)
	{
		if (m_padUpDelay == 0 || m_padUpDelay > g_repeatDelay)
			b |= BTN_UP;
		++m_padUpDelay;
	}
	else
		m_padUpDelay = 0;
	return b;
}