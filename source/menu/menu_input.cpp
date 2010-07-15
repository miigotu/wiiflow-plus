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
	WPAD_ScanPads();
	wpadsState = WPadState();
	wpadsHeld = WPadHeld();
	for(int wmote=0;wmote<4;wmote++)
	{
		wd[wmote] = WPAD_Data(wmote);
		angle[wmote] = wd[wmote]->exp.nunchuk.js.ang;
		mag[wmote] = wd[wmote]->exp.nunchuk.js.mag;
	}
	btn = _btnRepeat();
}

u32 CMenu::WPadState()
{
	u32 ret;
	for(int wmote=0;wmote<4;wmote++)
	{
		ret = WPAD_ButtonsDown(wmote);
		if (ret != 0)
			return ret;
	}
	return 0;
}

u32 CMenu::WPadHeld()
{
	u32 ret;
	for(int wmote=0;wmote<4;wmote++)
	{
		ret = WPAD_ButtonsHeld(wmote);
		if (ret != 0)
			return ret;
	}
	return 0;
}

int CMenu::WPadIR_Valid(int i)
{
	wd[i] = WPAD_Data(i);
	if (wd[i]->ir.valid)
		return true;
	return false;
}

bool CMenu::WPadIR_ANY()
{
	bool ret = false;
	for(int wmote=0;wmote<4;wmote++)
	{
		wd[wmote] = WPAD_Data(wmote);
		if (wd[wmote]->ir.valid)
			ret = true;
	}
	return ret;
}

u32 CMenu::_btnRepeat()
{
	u32 b = 0;
	btn = wpadsHeld;

	if ((btn & WPAD_BUTTON_LEFT) != 0)
	{
		if (m_padLeftDelay == 0 || m_padLeftDelay > g_repeatDelay)
			b |= WPAD_BUTTON_LEFT;
		++m_padLeftDelay;
	}
	else
		m_padLeftDelay = 0;
	if ((btn & WPAD_BUTTON_DOWN) != 0)
	{
		if (m_padDownDelay == 0 || m_padDownDelay > g_repeatDelay)
			b |= WPAD_BUTTON_DOWN;
		++m_padDownDelay;
	}
	else
		m_padDownDelay = 0;
	if ((btn & WPAD_BUTTON_RIGHT) != 0)
	{
		if (m_padRightDelay == 0 || m_padRightDelay > g_repeatDelay)
			b |= WPAD_BUTTON_RIGHT;
		++m_padRightDelay;
	}
	else
		m_padRightDelay = 0;
	if ((btn & WPAD_BUTTON_UP) != 0)
	{
		if (m_padUpDelay == 0 || m_padUpDelay > g_repeatDelay)
			b |= WPAD_BUTTON_UP;
		++m_padUpDelay;
	}
	else
		m_padUpDelay = 0;
	return b;
}