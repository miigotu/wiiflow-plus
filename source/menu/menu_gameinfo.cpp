#include "menu.hpp"

#include <wiiuse/wpad.h>
#include "sys.h"
#include "alt_ios.h"
#include "../xml/xml.h"
#include "gecko.h"

extern const u8		wifi1_png[];
extern const u8		wifi2_png[];
extern const u8		wifi4_png[];
extern const u8		wifi8_png[];
extern const u8		wifi12_png[];
extern const u8		wifi16_png[];
extern const u8		wifi32_png[];

extern const u8		wiimote1_png[];
extern const u8		wiimote2_png[];
extern const u8		wiimote3_png[];
extern const u8		wiimote4_png[];
extern const u8		wiimote8_png[];

extern const u8		guitar_png[];
extern const u8		guitarR_png[];
extern const u8		microphone_png[];
extern const u8		gcncontroller_png[];
extern const u8		classiccontroller_png[];
extern const u8		nunchuk_png[];
extern const u8		nunchukR_png[];
extern const u8		dancepad_png[];
extern const u8		dancepadR_png[];
extern const u8		balanceboard_png[];
extern const u8		balanceboardR_png[];
extern const u8		drums_png[];
extern const u8		drumsR_png[];
extern const u8		motionplus_png[];
extern const u8		motionplusR_png[];
extern const u8		wheel_png[];
extern const u8		zapper_png[];
extern const u8		wiispeak_png[];

//Ratings
extern const u8		norating_png[];

extern const u8		esrb_ec_png[];
extern const u8		esrb_e_png[];
extern const u8		esrb_eten_png[];
extern const u8		esrb_t_png[];
extern const u8		esrb_m_png[];
extern const u8		esrb_ao_png[];

extern const u8		cero_a_png[];
extern const u8		cero_b_png[];
extern const u8		cero_c_png[];
extern const u8		cero_d_png[];
extern const u8		cero_z_png[];

extern const u8		pegi_3_png[];
extern const u8		pegi_7_png[];
extern const u8		pegi_12_png[];
extern const u8		pegi_16_png[];
extern const u8		pegi_18_png[];

extern struct gameXMLinfo gameinfo;

static bool titlecheck = false;
u8 cnt_controlsreq = 0, cnt_controls = 0;

void CMenu::_gameinfo(void)
{ 
	SetupInput();
	_showGameInfo();

	u8 page = 0;
	
	int pixels_to_skip = 10;
	int amount_of_skips = 0;
	
	int x = 0, y = 0, synopsis_x = 0, synopsis_y = 0;
	u32 synopsis_w = 0, synopsis_h = 0;
	
	do
	{
		_mainLoopCommon();

		if ((BTN_DOWN_PRESSED || BTN_DOWN_HELD) && !(m_thrdWorking && m_thrdStop) && page == 1)
		{
			if (amount_of_skips == 0)
			{
				m_btnMgr.getDimensions(m_gameinfoLblSynopsis, synopsis_x, synopsis_y, synopsis_w, synopsis_h); // Get original dimensions
			}
			
			m_btnMgr.getDimensions(m_gameinfoLblSynopsis, x, y, synopsis_w, synopsis_h); // Get current dimensions
			
			gprintf("Variables: synopsis_y: %d, synopsis_h: %d, m_vid.height: %d\n", synopsis_y, synopsis_h, m_vid.height());
			gprintf("Min Y for scrolling is: %d\n", (int) (synopsis_y - (synopsis_h - (m_vid.height() - 20))));
			gprintf("Current Y is: %d\n", y);
			if (y > (int) (synopsis_y - (synopsis_h - (m_vid.height() - 20)))) // 20 pixels from the bottom
			{
				// Move the control instead
				// Height of a single line is...what? 10 pixels?
				m_btnMgr.moveBy(m_gameinfoLblSynopsis, 0, -pixels_to_skip);
				amount_of_skips++;
			}
		}
		else if ((BTN_UP_PRESSED || BTN_UP_HELD) && !(m_thrdWorking && m_thrdStop) && page == 1)
		{
			if (amount_of_skips)
			{
				// Move the control instead
				// Height of a single line is...what? 10 pixels?
				m_btnMgr.moveBy(m_gameinfoLblSynopsis, 0, pixels_to_skip);
				amount_of_skips--;
			}
		}
		else if (BTN_RIGHT_PRESSED && !(m_thrdWorking && m_thrdStop) && page == 0)
		{
			page = 1;
						
			m_btnMgr.reset(m_gameinfoLblSynopsis);
			m_btnMgr.setText(m_gameinfoLblSynopsis, wfmt(L"%s", gameinfo.synopsis)); //, line, false);

			m_btnMgr.hide(m_gameinfoLblID, true);
			m_btnMgr.hide(m_gameinfoLblDev, true);
			m_btnMgr.hide(m_gameinfoLblRegion, true);
			m_btnMgr.hide(m_gameinfoLblPublisher, true);
			m_btnMgr.hide(m_gameinfoLblRlsdate, true);
			m_btnMgr.hide(m_gameinfoLblGenre, true);
			m_btnMgr.hide(m_gameinfoLblRating, true);
			m_btnMgr.hide(m_gameinfoLblWifiplayers, true);

			for (u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControlsReq); ++i)
				if (m_gameinfoLblControlsReq[i] != -1u)
					m_btnMgr.hide(m_gameinfoLblControlsReq[i], true);
			
			for (u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControls); ++i)
				if (m_gameinfoLblControls[i] != -1u)
					m_btnMgr.hide(m_gameinfoLblControls[i], true);
			
			// When showing synopsis, only show user labels 2 and 3
			for (u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblUser); ++i)
				if (i < ARRAY_SIZE(m_gameinfoLblUser) / 2)
					m_btnMgr.hide(m_gameinfoLblUser[i], true);
				else
					m_btnMgr.show(m_gameinfoLblUser[i]);
			
			m_btnMgr.show(m_gameinfoLblSynopsis);
		}
		else if (BTN_LEFT_PRESSED && !(m_thrdWorking && m_thrdStop))
		{
			page = 0;
			m_btnMgr.show(m_gameinfoLblID);
			m_btnMgr.show(m_gameinfoLblDev);
			m_btnMgr.show(m_gameinfoLblRegion);	
			m_btnMgr.show(m_gameinfoLblPublisher);
			m_btnMgr.show(m_gameinfoLblRlsdate);
			m_btnMgr.show(m_gameinfoLblGenre);
			m_btnMgr.show(m_gameinfoLblRating);
			m_btnMgr.show(m_gameinfoLblWifiplayers);

			for (u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControlsReq); ++i)
				if (m_gameinfoLblControlsReq[i] != -1u && i < cnt_controlsreq)
					m_btnMgr.show(m_gameinfoLblControlsReq[i]);
				
			for (u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControls); ++i)
				if (m_gameinfoLblControls[i] != -1u && i < cnt_controls)
					m_btnMgr.show(m_gameinfoLblControls[i]);
				
			// When showing synopsis, only show user labels 2 and 3
			for (u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblUser); ++i)
				if (i < ARRAY_SIZE(m_gameinfoLblUser) / 2)
					m_btnMgr.show(m_gameinfoLblUser[i]);
				else
					m_btnMgr.hide(m_gameinfoLblUser[i], true);
				
			m_btnMgr.hide(m_gameinfoLblSynopsis,true);
		}

	} while (!BTN_HOME_PRESSED && !BTN_B_PRESSED);

	_hideGameInfo(false);
}

void CMenu::_hideGameInfo(bool instant)
{
	m_btnMgr.hide(m_gameinfoLblID, instant);
	m_btnMgr.hide(m_gameinfoLblTitle, instant);
	m_btnMgr.hide(m_gameinfoLblSynopsis, instant);
	m_btnMgr.hide(m_gameinfoLblDev, instant);
	m_btnMgr.hide(m_gameinfoLblRegion, instant);
	m_btnMgr.hide(m_gameinfoLblPublisher, instant);
	m_btnMgr.hide(m_gameinfoLblRlsdate, instant);
	m_btnMgr.hide(m_gameinfoLblGenre, instant);
	m_btnMgr.hide(m_gameinfoLblRating, instant);
	m_btnMgr.hide(m_gameinfoLblWifiplayers, instant);
	
	for (u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControlsReq); ++i)
		if (m_gameinfoLblControlsReq[i] != -1u)
			m_btnMgr.hide(m_gameinfoLblControlsReq[i], instant);

	for (u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblUser); ++i)
		m_btnMgr.hide(m_gameinfoLblUser[i], instant);

	for (u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControls); ++i)
		if (m_gameinfoLblControls[i] != -1u)
			m_btnMgr.hide(m_gameinfoLblControls[i], instant);
}

void CMenu::_showGameInfo(void)
{
	_setBg(m_gameinfoBg, m_gameinfoBg);
	_textGameInfo();
	
	if(titlecheck) {
	m_btnMgr.show(m_gameinfoLblID);
	m_btnMgr.show(m_gameinfoLblTitle);
	m_btnMgr.show(m_gameinfoLblRating);
	m_btnMgr.show(m_gameinfoLblRegion);	
	m_btnMgr.show(m_gameinfoLblDev);
	m_btnMgr.show(m_gameinfoLblPublisher);
	m_btnMgr.show(m_gameinfoLblRlsdate);
	m_btnMgr.show(m_gameinfoLblGenre);
	m_btnMgr.show(m_gameinfoLblWifiplayers);

	for (u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblUser); ++i)
		if (i < ARRAY_SIZE(m_gameinfoLblUser) / 2)
			m_btnMgr.show(m_gameinfoLblUser[i]);
	
	for (u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControlsReq); ++i)
		if (m_gameinfoLblControlsReq[i] != -1u && i < cnt_controlsreq)
			m_btnMgr.show(m_gameinfoLblControlsReq[i]);
		
	for (u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControls); ++i)
		if (m_gameinfoLblControls[i] != -1u && i < cnt_controls)
			m_btnMgr.show(m_gameinfoLblControls[i]);
	}
}

void CMenu::_initGameInfoMenu(CMenu::SThemeData &theme)
{
	gprintf("_initGameInfoMenu\n");
	
	STexture emptyTex;
	_addUserLabels(theme, m_gameinfoLblUser, 0, 1, "GAMEINFO");
	_addUserLabels(theme, m_gameinfoLblUser, 2, 1, "GAMEINFO");
	
	m_gameinfoBg = _texture(theme.texSet, "GAMEINFO/BG", "texture", theme.bg);
	m_gameinfoLblID = _addLabel(theme, "GAMEINFO/GAMEID", theme.btnFont, L"", 125, 10, 420, 75, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_gameinfoLblGenre = _addLabel(theme, "GAMEINFO/GENRE", theme.thxFont, L"", 40, 140, 460, 56, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_gameinfoLblDev = _addLabel(theme, "GAMEINFO/DEVELOPER", theme.thxFont, L"", 40, 170, 460, 56, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_gameinfoLblPublisher = _addLabel(theme, "GAMEINFO/PUBLISHER", theme.thxFont, L"", 40, 200, 460, 56, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_gameinfoLblRlsdate = _addLabel(theme, "GAMEINFO/RLSDATE", theme.thxFont, L"", 40, 230, 460, 56, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_gameinfoLblRegion = _addLabel(theme, "GAMEINFO/REGION", theme.thxFont, L"", 40, 260, 460, 56, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_gameinfoLblRating = _addLabel(theme, "GAMEINFO/RATING", theme.titleFont, L"", 550, 380, 48, 60, theme.titleFontColor, 0, m_rating);
	m_gameinfoLblSynopsis = _addLabel(theme, "GAMEINFO/SYNOPSIS", theme.thxFont, L"", 40, 110, 575, 40, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_gameinfoLblWifiplayers = _addLabel(theme, "GAMEINFO/WIFIPLAYERS", theme.thxFont, L"", 550, 110, 68, 60, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP,m_wifi);

	_addUserLabels(theme, m_gameinfoLblUser, 1, 1, "GAMEINFO");
	_addUserLabels(theme, m_gameinfoLblUser, 3, 2, "GAMEINFO");

	m_gameinfoLblTitle = _addLabel(theme, "GAMEINFO/TITLE", theme.btnFont, L"", 125, 37, 440, 75, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);

	for (u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControlsReq); ++i) {
		string dom(sfmt("GAMEINFO/CONTROLSREQ%i", i + 1));
		m_gameinfoLblControlsReq[i] = _addLabel(theme, dom.c_str(), theme.thxFont, L"", 40 + (i*60), 310, 60, 40, theme.txtFontColor, 0, emptyTex);
		_setHideAnim(m_gameinfoLblControlsReq[i], dom.c_str(), 0, -100, 0.f, 0.f);
	}

	for (u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControls); ++i) {
		string dom(sfmt("GAMEINFO/CONTROLS%i", i + 1));
		m_gameinfoLblControls[i] = _addLabel(theme, dom.c_str(), theme.thxFont, L"", 40 + (i*60), 380, 60, 40, theme.txtFontColor, 0, emptyTex);
		_setHideAnim(m_gameinfoLblControls[i], dom.c_str(), 0, -100, 0.f, 0.f);
	}
	// 
	_setHideAnim(m_gameinfoLblID, "GAMEINFO/GAMEID",0, 100, 0.f, 0.f);
	_setHideAnim(m_gameinfoLblTitle, "GAMEINFO/TITLE", 0, 100, 0.f, 0.f);
	_setHideAnim(m_gameinfoLblRating, "GAMEINFO/RATING", 100, 0, 0.f, 0.f);
	_setHideAnim(m_gameinfoLblSynopsis, "GAMEINFO/SYNOPSIS", 0, 700, 1.f, 1.f);
	_setHideAnim(m_gameinfoLblRegion, "GAMEINFO/REGION", 0, -100, 0.f, 0.f);
	_setHideAnim(m_gameinfoLblDev, "GAMEINFO/DEVELOPER", 0, -100, 0.f, 0.f);
	_setHideAnim(m_gameinfoLblPublisher, "GAMEINFO/PUBLISHER", 0, -100, 0.f, 0.f);
	_setHideAnim(m_gameinfoLblRlsdate, "GAMEINFO/RLSDATE", 0, -100, 0.f, 0.f);
	_setHideAnim(m_gameinfoLblGenre, "GAMEINFO/GENRE", 0, -100, 0.f, 0.f);
	_setHideAnim(m_gameinfoLblWifiplayers, "GAMEINFO/WIFIPLAYERS", 0, -100, 0.f, 0.f);
	// 
	_hideGameInfo(true);
}

void CMenu::_textGameInfo(void)
{
	cnt_controlsreq = 0;
	cnt_controls = 0;

	titlecheck = LoadGameInfoFromXML((char*)m_cf.getId().c_str());
		
	if(titlecheck)
	{
		gprintf("ID: %s\nTitle: %s\n", gameinfo.id, gameinfo.title);
		m_btnMgr.setText(m_gameinfoLblID, wfmt(L"%s", gameinfo.id), true);
		m_btnMgr.setText(m_gameinfoLblTitle, wfmt(L"%s", gameinfo.title), true);
		m_btnMgr.setText(m_gameinfoLblSynopsis, wfmt(L"%s", "" ), false);
		m_btnMgr.setText(m_gameinfoLblDev, wfmt(_fmt("gameinfo1",L"Developer: %s"), gameinfo.developer), true);
		m_btnMgr.setText(m_gameinfoLblPublisher, wfmt(_fmt("gameinfo2",L"Publisher: %s"), gameinfo.publisher), true);
		m_btnMgr.setText(m_gameinfoLblRegion, wfmt(_fmt("gameinfo3",L"Region: %s"), gameinfo.region), true);
		m_btnMgr.setText(m_gameinfoLblGenre, wfmt(_fmt("gameinfo5",L"Genre: %s"), gameinfo.genre), true);
		
		switch(CONF_GetRegion())
		{
		case 0:
		case 4:
		case 5:
			m_btnMgr.setText(m_gameinfoLblRlsdate, wfmt(_fmt("gameinfo4",L"Release Date: %i-%i-%i"), gameinfo.year,gameinfo.month,gameinfo.day), true);
			break;
		case 1:
			m_btnMgr.setText(m_gameinfoLblRlsdate, wfmt(_fmt("gameinfo4",L"Release Date: %i-%i-%i"), gameinfo.month,gameinfo.day,gameinfo.year), true);
			break;
		case 2:
			m_btnMgr.setText(m_gameinfoLblRlsdate, wfmt(_fmt("gameinfo4",L"Release Date: %i-%i-%i"), gameinfo.day,gameinfo.month,gameinfo.year), true);
			break;
		}
		
		//Ratings
		switch(ConvertRatingToIndex(gameinfo.ratingtype)) {
			case 0:
				//CERO
				if(!strcmp(gameinfo.ratingvalue,"A"))
					m_rating.fromPNG(cero_a_png);
				else if(!strcmp(gameinfo.ratingvalue,"B"))
					m_rating.fromPNG(cero_b_png);
				else if(!strcmp(gameinfo.ratingvalue,"D"))
					m_rating.fromPNG(cero_d_png);
				else if(!strcmp(gameinfo.ratingvalue,"C"))
					m_rating.fromPNG(cero_c_png);
				else if(!strcmp(gameinfo.ratingvalue,"Z"))
					m_rating.fromPNG(cero_z_png);
				else
					m_rating.fromPNG(norating_png);		
				break;
			case 1:
				//ESRB
				if(!strcmp(gameinfo.ratingvalue,"AO"))
					m_rating.fromPNG(esrb_ao_png);
				else if(!strcmp(gameinfo.ratingvalue,"E"))
					m_rating.fromPNG(esrb_e_png);
				else if(!strcmp(gameinfo.ratingvalue,"EC"))
					m_rating.fromPNG(esrb_ec_png);
				else if(!strcmp(gameinfo.ratingvalue,"E10+"))
					m_rating.fromPNG(esrb_eten_png);
				else if(!strcmp(gameinfo.ratingvalue,"T"))
					m_rating.fromPNG(esrb_t_png);
				else if(!strcmp(gameinfo.ratingvalue,"M"))
					m_rating.fromPNG(esrb_m_png);
				else
					m_rating.fromPNG(norating_png);
				break;
			case 2:
				//PEGI
				if(!strcmp(gameinfo.ratingvalue,"3"))
					m_rating.fromPNG(pegi_3_png);
				else if(!strcmp(gameinfo.ratingvalue,"7"))
					m_rating.fromPNG(pegi_7_png);
				else if(!strcmp(gameinfo.ratingvalue,"12"))
					m_rating.fromPNG(pegi_12_png);
				else if(!strcmp(gameinfo.ratingvalue,"16"))
					m_rating.fromPNG(pegi_16_png);
				else if(!strcmp(gameinfo.ratingvalue,"18"))
					m_rating.fromPNG(pegi_18_png);
				else
					m_rating.fromPNG(norating_png);
				break;
			default:
				break;
		}	

		m_btnMgr.setTexture(m_gameinfoLblRating ,m_rating);
		
		//Wifi players
		STexture emptyTex;
		if(gameinfo.wifiplayers == 1)
			m_wifi.fromPNG(wifi1_png);
		else if(gameinfo.wifiplayers == 2)
			m_wifi.fromPNG(wifi2_png);
		else if(gameinfo.wifiplayers == 4)
			m_wifi.fromPNG(wifi4_png);
		else if(gameinfo.wifiplayers == 8)
			m_wifi.fromPNG(wifi8_png);
		else if(gameinfo.wifiplayers == 12)
			m_wifi.fromPNG(wifi12_png);
		else if(gameinfo.wifiplayers == 16)
			m_wifi.fromPNG(wifi16_png);
		else if(gameinfo.wifiplayers == 32)
			m_wifi.fromPNG(wifi32_png);

		if(gameinfo.wifiplayers > 0)
			m_btnMgr.setTexture(m_gameinfoLblWifiplayers ,m_wifi);
		else 
			m_btnMgr.setTexture(m_gameinfoLblWifiplayers ,emptyTex);

		u8 wiimote=0,
			nunchuk=0,
            classiccontroller=0,
            balanceboard=0,
            dancepad=0,
            guitar=0,
            gamecube=0,
            motionplus=0,
            drums=0,
			microphone=0,
			wheel=0;
		
		//check required controlls
		for (u8 i=0;i<16;i++) {
			if (strcmp(gameinfo.accessoriesReq[i],"wiimote")==0)
                wiimote=1;
            if (strcmp(gameinfo.accessoriesReq[i],"nunchuk")==0)
                nunchuk=1;
            if (strcmp(gameinfo.accessoriesReq[i],"guitar")==0)
                guitar=1;
            if (strcmp(gameinfo.accessoriesReq[i],"drums")==0)
                drums=1;
            if (strcmp(gameinfo.accessoriesReq[i],"dancepad")==0)
                dancepad=1;
            if (strcmp(gameinfo.accessoriesReq[i],"motionplus")==0)
                motionplus=1;
            if (strcmp(gameinfo.accessoriesReq[i],"balanceboard")==0)
                balanceboard=1;
        }

		u8 x = 0;
		u8 max_controlsReq = ARRAY_SIZE(m_gameinfoLblControlsReq);
		
		if(wiimote && x < max_controlsReq)
		{
			u8 players = gameinfo.players;
			if ( gameinfo.players >= 10)
				players = players/10;

			if(players == 1)
				m_controlsreq[x].fromPNG(wiimote1_png);
			else if(players == 2)
				m_controlsreq[x].fromPNG(wiimote2_png);
			else if(players == 3)
				m_controlsreq[x].fromPNG(wiimote3_png);
			else if(players == 4)
				m_controlsreq[x].fromPNG(wiimote4_png);
			else if(players == 8)
				m_controlsreq[x].fromPNG(wiimote8_png);

			m_btnMgr.setTexture(m_gameinfoLblControlsReq[x] ,m_controlsreq[x], 20, 60);
			x++;
		}
		if(nunchuk && x < max_controlsReq)
		{
			m_controlsreq[x].fromPNG(nunchukR_png);
			m_btnMgr.setTexture(m_gameinfoLblControlsReq[x] ,m_controlsreq[x], 52, 60);
			x++;
		}
		if(guitar && x < max_controlsReq)
		{
			m_controlsreq[x].fromPNG(guitarR_png);
			m_btnMgr.setTexture(m_gameinfoLblControlsReq[x] ,m_controlsreq[x], 60, 60);
			x++;
		}
		if(drums && x < max_controlsReq)
		{
			m_controlsreq[x].fromPNG(drumsR_png);
			m_btnMgr.setTexture(m_gameinfoLblControlsReq[x] ,m_controlsreq[x], 60, 60);
			x++;
		}
		if(motionplus && x < max_controlsReq)
		{
			m_controlsreq[x].fromPNG(motionplusR_png);
			m_btnMgr.setTexture(m_gameinfoLblControlsReq[x] ,m_controlsreq[x], 20, 60);
			x++;
		}
		if(dancepad && x < max_controlsReq)
		{
			m_controlsreq[x].fromPNG(dancepadR_png);
			m_btnMgr.setTexture(m_gameinfoLblControlsReq[x] ,m_controlsreq[x], 60, 60);
			x++;
		}
		if(balanceboard && x < max_controlsReq)
		{
			m_controlsreq[x].fromPNG(balanceboardR_png);
			m_btnMgr.setTexture(m_gameinfoLblControlsReq[x] ,m_controlsreq[x], 60, 60);
			x++;
		}

		cnt_controlsreq = x;

		/*for(unsigned int i = 0;i<ARRAY_SIZE(m_gameinfoLblControlsReq);i++)
			m_btnMgr.setTexture(m_gameinfoLblControlsReq[i] ,m_controlsreq[i]);*/

		//check optional controlls
		wiimote=0,
		nunchuk=0,
        classiccontroller=0,
        balanceboard=0,
        dancepad=0,
        guitar=0,
        gamecube=0,
        motionplus=0,
        drums=0,
		microphone=0,
		wheel=0;

		for (u8 i=0;i<16;i++) {
            if (strcmp(gameinfo.accessories[i],"classiccontroller")==0)
                classiccontroller=1;
            if (strcmp(gameinfo.accessories[i],"nunchuk")==0)
                nunchuk=1;
            if (strcmp(gameinfo.accessories[i],"guitar")==0)
                guitar=1;
            if (strcmp(gameinfo.accessories[i],"drums")==0)
                drums=1;
            if (strcmp(gameinfo.accessories[i],"dancepad")==0)
                dancepad=1;
            if (strcmp(gameinfo.accessories[i],"motionplus")==0)
                motionplus=1;
            if (strcmp(gameinfo.accessories[i],"balanceboard")==0)
                balanceboard=1;
            if (strcmp(gameinfo.accessories[i],"microphone")==0)
                microphone=1;
            if (strcmp(gameinfo.accessories[i],"gamecube")==0)
                gamecube=1;
			if (strcmp(gameinfo.accessories[i],"wheel")==0)
                wheel=1;
        }
		
		x = 0;
		u8 max_controls = ARRAY_SIZE(m_gameinfoLblControls);
		
		if(classiccontroller && x < max_controls)
		{
			m_controls[x].fromPNG(classiccontroller_png);
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 60, 60);
			x++;
		}
		if(nunchuk && x < max_controls)
		{
			m_controls[x].fromPNG(nunchuk_png);
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 52, 60);
			x++;
		}
		if(guitar && x < max_controls)
		{
			m_controls[x].fromPNG(guitarR_png);
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 60, 60);
			x++;
		}
		if(drums && x < max_controls)
		{
			m_controls[x].fromPNG(drums_png);
			x++;
		}
		if(dancepad && x < max_controls)
		{
			m_controls[x].fromPNG(dancepad_png);
			x++;
		}
		if(motionplus && x < max_controls)
		{
			m_controls[x].fromPNG(motionplus_png);
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 20, 60);
			x++;
		}
		if(balanceboard && x < max_controls)
		{
			m_controls[x].fromPNG(balanceboard_png);
			x++;
		}
		if(microphone && x < max_controls)
		{
			m_controls[x].fromPNG(microphone_png);
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 48, 60);
			x++;
		}
		if(gamecube && x < max_controls)
		{
			m_controls[x].fromPNG(gcncontroller_png);
			x++;
		}
		if(wheel && x < max_controls)
		{
			m_controls[x].fromPNG(wheel_png);
			x++;
		}

		cnt_controls = x;
		for(unsigned int i = 0;i<ARRAY_SIZE(m_gameinfoLblControls);i++)
			m_btnMgr.setTexture(m_gameinfoLblControls[i] ,m_controls[i]);
	}
	else
		m_btnMgr.setText(m_gameinfoLblTitle, wfmt(L"%s", "No Gameinfo"), true);
}