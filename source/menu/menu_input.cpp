#include "menu.hpp"
#include "gecko.h"

//#DEFINE wd(WPAD_Data(0)|WPAD_Data(1)|WPAD_Data(2)|WPAD_Data(3))
using namespace std;

static const u32 g_repeatDelay = 15;
int m_paddata = 0;
class LockMutex
{
	mutex_t &m_mutex;
public:
	LockMutex(mutex_t &m) : m_mutex(m) { LWP_MutexLock(m_mutex); }
	~LockMutex(void) { LWP_MutexUnlock(m_mutex); }
};

u32 CMenu::PadState()
{
	int ret;
	for (int i=0;i<4;i++)
	{
		ret = WPAD_ButtonsDown(i);
		if (ret)
			return ret;
	}
	return false;
}

u32 CMenu::PadHeld()
{
	int ret;
	for (int i=0;i<4;i++)
	{
		ret = WPAD_ButtonsHeld(i);
		if (ret)
			return ret;
	}
	return false;
}

/*
WPADData* CMenu::PadData()
{
	for (int i=0;i<4;i++)
	{
		wd = WPAD_Data(i);
		if (wd)
			return wd;
	}
}
*/
void CMenu::SetupInput()
{
		WPAD_Rumble(WPAD_CHAN_ALL, 0);

		m_padLeftDelay = 0;
		m_padDownDelay = 0;
		m_padRightDelay = 0;
		m_padUpDelay = 0;
		angle = 0;
		mag = 0;
		repeatButton = 0;
		buttonHeld = (u32)-1;
}

void CMenu::ScanInput()
{
	WPAD_ScanPads();
	padsState = PadState();
	//wd = WPAD_Data(m_paddata);
	wd = WPAD_Data(0);
	//if (m_paddata < 3)
		//m_paddata++;
	//else
		//m_paddata = 0;	
	btn = _btnRepeat();
	//Get Nunchuk values
	angle = wd->exp.nunchuk.js.ang;
	mag = wd->exp.nunchuk.js.mag;
}

u32 CMenu::_btnRepeat()
{
	u32 b = 0;
	btn = PadHeld();

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