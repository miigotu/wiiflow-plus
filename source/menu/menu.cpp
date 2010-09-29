#include "menu.hpp"
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "loader/alt_ios.h"
#include "loader/fs.h"
#include "oggplayer.h"

#include "network/gcard.h"

#include <fstream>
#include <map>
#include <sys/stat.h>
#include <fstream>
#include <dirent.h>
#include <mp3player.h>
#include <time.h>
#include <wchar.h>

#include "gecko.h"
#include "channels.h"

// Sounds
extern const u8 click_wav[];
extern const u32 click_wav_size;
extern const u8 hover_wav[];
extern const u32 hover_wav_size;
extern const u8 camera_wav[];
extern const u32 camera_wav_size;
// Pics
extern const u8 wait_png[];
extern const u8 btnplus_png[];
extern const u8 btnpluss_png[];
extern const u8 btnminus_png[];
extern const u8 btnminuss_png[];
extern const u8 background_png[];
extern const u8 butleft_png[];
extern const u8 butcenter_png[];
extern const u8 butright_png[];
extern const u8 butsleft_png[];
extern const u8 butscenter_png[];
extern const u8 butsright_png[];
extern const u8 pbarleft_png[];
extern const u8 pbarcenter_png[];
extern const u8 pbarright_png[];
extern const u8 pbarlefts_png[];
extern const u8 pbarcenters_png[];
extern const u8 pbarrights_png[];
// Fonts
extern const u8 butfont_ttf[];
extern const u32 butfont_ttf_size;
extern const u8 cffont_ttf[];
extern const u32 cffont_ttf_size;
const u8 *titlefont_ttf = butfont_ttf;
const u32 titlefont_ttf_size = butfont_ttf_size;
const u8 *lblfont_ttf = cffont_ttf;
const u32 lblfont_ttf_size = cffont_ttf_size;
const u8 *thxfont_ttf = cffont_ttf;
const u32 thxfont_ttf_size = cffont_ttf_size;

#define WIINNERTAG_URL "http://www.wiinnertag.com/wiinnertag_scripts/update_sign.php?key={KEY}&game_id={ID6}"

using namespace std;

CMenu::CMenu(CVideo &vid) :
	m_vid(vid)
{
	m_aa = 0;
	m_thrdWorking = false;
	m_thrdStop = false;
	m_thrdProgress = 0.f;
	m_thrdStep = 0.f;
	m_thrdStepLen = 0.f;
	m_locked = false;
	m_favorites = false;
	m_category = 0;
	m_networkInit = false;
	m_mutex = 0;
	m_showtimer = 0;
	m_gameSoundThread = 0;
	m_gameSndMutex = 0;
	m_numCFVersions = 0;
	m_bgCrossFade = 0;
	m_bnrSndVol = 0;
	m_gameSettingsPage = 0;
	m_directLaunch = false;
	m_exit = false;
	m_initialCoverStatusComplete = false;
}

extern "C" { int makedir(char *newdir); }

void CMenu::init()
{
	string themeName;
	const char *drive = "usb";
	const char *wfdrv = "usb";
	u8 defaultMenuLanguage;
	struct stat dummy;
	
	m_waitMessage.fromPNG(wait_png);
	// Data path
	if (FS_SDAvailable() && FS_USBAvailable())
	{
		if (stat(sfmt("sd:/%s/boot.dol", APPDATA_DIR2).c_str(), &dummy) == 0)
			wfdrv = "sd";
	}
	else
		wfdrv = FS_USBAvailable() ? "usb" : "sd";

	m_appDir = sfmt("%s:/%s", wfdrv, APPDATA_DIR2);
	gprintf("Wiiflow boot.dol Location: %s\n", m_appDir.c_str());
	m_cfg.load(sfmt("%s/" CFG_FILENAME, m_appDir.c_str()).c_str());

	drive = m_cfg.getBool("GENERAL", "data_on_usb", strcmp(wfdrv, "usb") == 0) ? FS_USBAvailable() ? "usb" : "sd" : FS_SDAvailable() ? "sd" : "usb";
	m_dataDir = sfmt("%s:/%s", drive, APPDATA_DIR);
	gprintf("Data Directory: %s\n", m_dataDir.c_str());
	
	m_dol = sfmt("%s/boot.dol", m_appDir.c_str());
	m_ver = sfmt("%s/versions", m_appDir.c_str());
	m_app_update_zip = sfmt("%s/update.zip", m_appDir.c_str());
	m_data_update_zip = sfmt("%s/update.zip", m_dataDir.c_str());
	//
	m_cacheDir = m_cfg.getString("GENERAL", "dir_cache", sfmt("%s/cache", m_dataDir.c_str()));
	m_settingsDir = m_cfg.getString("GENERAL", "dir_settings", sfmt("%s/settings", m_dataDir.c_str()));
	m_languagesDir = m_cfg.getString("GENERAL", "dir_languages", sfmt("%s/languages", m_dataDir.c_str()));
	m_boxPicDir = m_cfg.getString("GENERAL", "dir_box_covers", sfmt("%s/boxcovers", m_dataDir.c_str()));
	m_picDir = m_cfg.getString("GENERAL", "dir_flat_covers", sfmt("%s/covers", m_dataDir.c_str()));
	m_themeDir = m_cfg.getString("GENERAL", "dir_themes", sfmt("%s/themes", m_dataDir.c_str()));
	m_musicDir = m_cfg.getString("GENERAL", "dir_music", sfmt("%s/music", m_dataDir.c_str())); 
	m_videoDir = m_cfg.getString("GENERAL", "dir_trailers", sfmt("%s/trailers", m_dataDir.c_str()));
	m_fanartDir = m_cfg.getString("GENERAL", "dir_fanart", sfmt("%s/fanart", m_dataDir.c_str()));
	m_screenshotDir = m_cfg.getString("GENERAL", "dir_screenshot", sfmt("%s/screenshots", m_dataDir.c_str()));
	m_txtCheatDir = m_cfg.getString("GENERAL", "dir_txtcheat", sfmt("%s/codes", m_dataDir.c_str()));
	m_altDolDir = m_cfg.getString("GENERAL", "dir_altdol", sfmt("%s/alt_dols", m_txtCheatDir.c_str()));
	m_bcaDir = m_cfg.getString("GENERAL", "dir_bca", sfmt("%s/bca", m_txtCheatDir.c_str()));
	m_cheatDir = m_cfg.getString("GENERAL", "dir_cheat", sfmt("%s/gct", m_txtCheatDir.c_str()));
	m_wdmDir = m_cfg.getString("GENERAL", "dir_wdm", sfmt("%s/wdm", m_txtCheatDir.c_str()));
	m_wipDir = m_cfg.getString("GENERAL", "dir_wip", sfmt("%s/wip", m_txtCheatDir.c_str()));
	//
	m_cf.init();
	//Make important folders first.
	makedir((char *)m_cacheDir.c_str());
	makedir((char *)m_settingsDir.c_str());
	makedir((char *)m_languagesDir.c_str());
	makedir((char *)m_boxPicDir.c_str());
	makedir((char *)m_picDir.c_str());
	makedir((char *)m_themeDir.c_str());
	makedir((char *)m_musicDir.c_str());
	makedir((char *)m_videoDir.c_str());
	makedir((char *)m_fanartDir.c_str());
	makedir((char *)m_screenshotDir.c_str());
	makedir((char *)m_txtCheatDir.c_str());
	makedir((char *)m_altDolDir.c_str());
	makedir((char *)m_bcaDir.c_str());
	makedir((char *)m_cheatDir.c_str());
	makedir((char *)m_wdmDir.c_str());
	makedir((char *)m_wipDir.c_str());

	// INI files
	m_cat.load(sfmt("%s/" CAT_FILENAME, m_settingsDir.c_str()).c_str());
	themeName = m_cfg.getString("GENERAL", "theme", "DEFAULT");
	m_themeDataDir = sfmt("%s/%s", m_themeDir.c_str(), themeName.c_str());
	m_theme.load(sfmt("%s.ini", m_themeDataDir.c_str()).c_str());
	//
	defaultMenuLanguage = 7; //English
	switch (CONF_GetLanguage())
	{
		case CONF_LANG_JAPANESE:
			defaultMenuLanguage = 14; //Japanese
			break;
		case CONF_LANG_GERMAN:
			defaultMenuLanguage = 11; //German
			break;
		case CONF_LANG_FRENCH:
			defaultMenuLanguage = 9; //French
			break;
		case CONF_LANG_SPANISH:
			defaultMenuLanguage = 19; //Spanish
			break;
		case CONF_LANG_ITALIAN:
			defaultMenuLanguage = 13; //Italian
			break;
		case CONF_LANG_DUTCH:
			defaultMenuLanguage = 6; //Dutch
			break;
		case CONF_LANG_SIMP_CHINESE:
			defaultMenuLanguage = 3; //Chinese_S
			break;
		case CONF_LANG_TRAD_CHINESE:
			defaultMenuLanguage = 4; //Chinese_T
			break;
		case CONF_LANG_KOREAN:
			defaultMenuLanguage = 7; // No Korean translation has been done for wiiflow, so the menu will use english in this case.
			break;
	}
	if (CONF_GetArea() == CONF_AREA_BRA)
		defaultMenuLanguage = 2; //Brazilian

	int lang = m_cfg.getInt("GENERAL", "language", defaultMenuLanguage);
	m_curLanguage = CMenu::_translations[lang];
	if (!m_loc.load(sfmt("%s/%s.ini", m_languagesDir.c_str(), m_curLanguage.c_str()).c_str()))
	{
		m_cfg.setInt("GENERAL", "language", 0);
		m_curLanguage = CMenu::_translations[0];
		m_loc.load(sfmt("%s/%s.ini", m_languagesDir.c_str(), m_curLanguage.c_str()).c_str());
	}
	// 
	m_aa = 3;
	
	bool pWide;
	CColor pShadowColor;
	float pShadowX;
	float pShadowY;
	bool pShadowBlur;
	pWide = m_vid.wide();
	pShadowColor = m_theme.getColor("GENERAL", "pointer_shadow_color", CColor(0x3F000000));
	pShadowX = m_theme.getFloat("GENERAL", "pointer_shadow_x", 3.f);
	pShadowY = m_theme.getFloat("GENERAL", "pointer_shadow_y", 3.f);
	pShadowBlur = m_theme.getBool("GENERAL", "pointer_shadow_blur", false);

	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
		m_cursor[chan].init(sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString("GENERAL", sfmt("pointer%i", chan+1).c_str()).c_str()).c_str(),
			pWide, pShadowColor, pShadowX, pShadowY, pShadowBlur, chan);
		
	m_btnMgr.init(m_vid);

	_buildMenus();
	_loadCFCfg();
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
		WPAD_SetVRes(chan, m_vid.width() + m_cursor[chan].width(), m_vid.height() + m_cursor[chan].height());
	m_locked = m_cfg.getString("GENERAL", "parent_code", "").size() >= 4;
	m_btnMgr.setRumble(m_cfg.getBool("GENERAL", "rumble", true));
	m_vid.set2DViewport(m_cfg.getInt("GENERAL", "tv_width", 640), m_cfg.getInt("GENERAL", "tv_height", 480),
	m_cfg.getInt("GENERAL", "tv_x", 0), m_cfg.getInt("GENERAL", "tv_y", 0));
	Sys_ExitTo(m_cfg.getInt("GENERAL", "exit_to", 0));
	LWP_MutexInit(&m_mutex, 0);
	LWP_MutexInit(&m_gameSndMutex, 0);
	soundInit();
	m_cf.setSoundVolume(m_cfg.getInt("GENERAL", "sound_volume_coverflow", 255));
	m_btnMgr.setSoundVolume(m_cfg.getInt("GENERAL", "sound_volume_gui", 255));
	m_bnrSndVol = m_cfg.getInt("GENERAL", "sound_volume_bnr", 255);
	if (m_cfg.getBool("GENERAL", "favorites_on_startup", false))
		m_favorites = m_cfg.getBool("GENERAL", "favorites", false);
	m_category = m_cat.getInt("GENERAL", "category", 0);//currently selected category
	m_max_categories = m_cat.getInt("GENERAL", "numcategories", 12);//configured amount of categories to use
	//m_current_view = m_cfg.getInt("GENERAL", "currentview", COVERFLOW_USB);
	//if (m_current_view > COVERFLOW_MAX) m_current_view = COVERFLOW_USB;
	m_current_view = COVERFLOW_USB;
	m_loaded_ios_base = get_ios_base();
	m_show_cover_after_animation = m_cfg.getOptBool("FANART", "show_cover_after_animation", 2); // 0 is false, 1 is true, 2 is default
	m_hidecover = m_cfg.getOptBool("FANART", "hidecover", 2);
	m_allow_fanart_on_top = m_cfg.getBool("FANART", "allow_artwork_on_top", true);
	
	register_card_provider(m_cfg.getString("GAMERCARD", "wiinnertag_url", WIINNERTAG_URL).c_str(),
						   m_cfg.getString("GAMERCARD", "wiinnertag_key", "").c_str(),
						   m_cfg.getBool("GAMERCARD", "wiinnertag_enable", false) ? 1 : 0);
}

void CMenu::cleanup(void)
{
	_stopSounds();
	soundDeinit();
	_waitForGameSoundExtract();
	LWP_MutexDestroy(m_mutex);
	m_mutex = 0;
	LWP_MutexDestroy(m_gameSndMutex);
	m_gameSndMutex = 0;
	_deinitNetwork();
}

void CMenu::_setAA(int aa)
{
	switch (aa)
	{
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 8:
			m_aa = aa;
			break;
		case 7:
			m_aa = 6;
			break;
		default:
			m_aa = 0;
	}
}

void CMenu::_loadCFCfg()
{
	string texLoading;
	string texNoCover;
	string texLoadingFlat;
	string texNoCoverFlat;
	SFont font;
	string filename;
	const char *domain = "_COVERFLOW";
	SSoundEffect cfFlipSnd;
	SSoundEffect cfHoverSnd;
	SSoundEffect cfSelectSnd;
	SSoundEffect cfCancelSnd;

	m_cf.setCachePath(m_cacheDir.c_str(), !m_cfg.getBool("GENERAL", "keep_png", true), m_cfg.getBool("GENERAL", "compress_cache", false));
	m_cf.setBufferSize(m_cfg.getInt("GENERAL", "cover_buffer", 120));
	filename = m_theme.getString(domain, "sound_flip");
	cfFlipSnd.fromWAVFile(sfmt("%s/%s", m_themeDataDir.c_str(), filename.c_str()).c_str());
	filename = m_theme.getString(domain, "sound_hover");
	cfHoverSnd.fromWAVFile(sfmt("%s/%s", m_themeDataDir.c_str(), filename.c_str()).c_str());
	filename = m_theme.getString(domain, "sound_select");
	cfSelectSnd.fromWAVFile(sfmt("%s/%s", m_themeDataDir.c_str(), filename.c_str()).c_str());
	filename = m_theme.getString(domain, "sound_cancel");
	cfCancelSnd.fromWAVFile(sfmt("%s/%s", m_themeDataDir.c_str(), filename.c_str()).c_str());
	m_cf.setSounds(cfFlipSnd, cfHoverSnd, cfSelectSnd, cfCancelSnd);
	// Textures
	texLoading = sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "loading_cover_box").c_str());
	texNoCover = sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "missing_cover_box").c_str());
	texLoadingFlat = sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "loading_cover_flat").c_str());
	texNoCoverFlat = sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "missing_cover_flat").c_str());
	m_cf.setTextures(texLoading, texLoadingFlat, texNoCover, texNoCoverFlat);
	// Font
	filename = m_theme.getString(domain, "font");
	if (!filename.empty())
		font.fromFile(sfmt("%s/%s", m_themeDataDir.c_str(), filename.c_str()).c_str(),
			m_theme.getInt(domain, "font_size", 32),
			m_theme.getInt(domain, "font_line_height", 32));
	m_cf.setFont(font, m_theme.getColor(domain, "font_color", CColor(0xFFFFFFFF)));
	// 
	m_numCFVersions = min(max(2, m_theme.getInt("_COVERFLOW", "number_of_modes", 2)), 8);
	for (u32 i = 1; i <= m_numCFVersions; ++i)
		_loadCFLayout(i);
	_loadCFLayout(min(max(1, m_cfg.getInt("GENERAL", "last_cf_mode", 1)), (int)m_numCFVersions));
}

Vector3D CMenu::_getCFV3D(const string &domain, const string &key, const Vector3D &def, bool otherScrnFmt)
{
	string key169(key);
	string key43(key);

	key43 += "_4_3";
	if (m_vid.wide() == otherScrnFmt)
		swap(key169, key43);
	if (m_theme.has(domain, key169))
	{
		Vector3D v(m_theme.getVector3D(domain, key169));
		m_theme.getVector3D(domain, key43, v);
		return v;
	}
	return m_theme.getVector3D(domain, key169, m_theme.getVector3D(domain, key43, def));
}

int CMenu::_getCFInt(const string &domain, const string &key, int def, bool otherScrnFmt)
{
	string key169(key);
	string key43(key);

	key43 += "_4_3";
	if (m_vid.wide() == otherScrnFmt)
		swap(key169, key43);
	if (m_theme.has(domain, key169))
	{
		int v = m_theme.getInt(domain, key169);
		m_theme.getInt(domain, key43, v);
		return v;
	}
	return m_theme.getInt(domain, key169, m_theme.getInt(domain, key43, def));
}

float CMenu::_getCFFloat(const string &domain, const string &key, float def, bool otherScrnFmt)
{
	string key169(key);
	string key43(key);

	key43 += "_4_3";
	if (m_vid.wide() == otherScrnFmt)
		swap(key169, key43);
	if (m_theme.has(domain, key169))
	{
		float v = m_theme.getFloat(domain, key169);
		m_theme.getFloat(domain, key43, v);
		return v;
	}
	return m_theme.getFloat(domain, key169, m_theme.getFloat(domain, key43, def));
}

void CMenu::_loadCFLayout(int version, bool forceAA, bool otherScrnFmt)
{
	string domain(sfmt("_COVERFLOW_%i", version).c_str());
	string domainSel(sfmt("_COVERFLOW_%i_S", version).c_str());
	bool sf = otherScrnFmt;

	if (forceAA)
		_setAA(m_theme.getInt(domain, "max_fsaa", 3));
	else
		_setAA(min(m_theme.getInt(domain, "max_fsaa", 3), m_cfg.getInt("GENERAL", "max_fsaa", 5)));
	m_cf.setTextureQuality(
		m_theme.getFloat(domain, "tex_lod_bias", -3.f),
		m_theme.getInt(domain, "tex_aniso", 2),
		m_theme.getBool(domain, "tex_edge_lod", true));
	m_cf.setRange(_getCFInt(domain, "rows", 1, sf), _getCFInt(domain, "columns", 9, sf));
	m_cf.setCameraPos(false,
		_getCFV3D(domain, "camera_pos", Vector3D(0.f, 1.5f, 5.f), sf),
		_getCFV3D(domain, "camera_aim", Vector3D(0.f, 0.f, -1.f), sf));
	m_cf.setCameraPos(true,
		_getCFV3D(domainSel, "camera_pos", Vector3D(0.f, 1.5f, 5.f), sf),
		_getCFV3D(domainSel, "camera_aim", Vector3D(0.f, 0.f, -1.f), sf));
	m_cf.setCameraOsc(false,
		_getCFV3D(domain, "camera_osc_speed", Vector3D(2.f, 1.1f, 1.3f), sf),
		_getCFV3D(domain, "camera_osc_amp", Vector3D(0.1f, 0.2f, 0.1f), sf));
	m_cf.setCameraOsc(true,
		_getCFV3D(domainSel, "camera_osc_speed", Vector3D(0.f, 0.f, 0.f), sf),
		_getCFV3D(domainSel, "camera_osc_amp", Vector3D(0.f, 0.f, 0.f), sf));
	m_cf.setCoverPos(false,
		_getCFV3D(domain, "left_pos", Vector3D(-1.6f, 0.f, 0.f), sf),
		_getCFV3D(domain, "right_pos", Vector3D(1.6f, 0.f, 0.f), sf),
		_getCFV3D(domain, "center_pos", Vector3D(0.f, 0.f, 1.f), sf),
		_getCFV3D(domain, "row_center_pos", Vector3D(0.f, 0.f, 0.f), sf));
	m_cf.setCoverPos(true,
		_getCFV3D(domainSel, "left_pos", Vector3D(-4.6f, 2.f, 0.f), sf),
		_getCFV3D(domainSel, "right_pos", Vector3D(4.6f, 2.f, 0.f), sf),
		_getCFV3D(domainSel, "center_pos", Vector3D(-0.6f, 0.f, 2.6f), sf),
		_getCFV3D(domainSel, "row_center_pos", Vector3D(0.f, 2.f, 0.f), sf));
	m_cf.setCoverAngleOsc(false,
		m_theme.getVector3D(domain, "cover_osc_speed", Vector3D(2.f, 2.f, 0.f)),
		m_theme.getVector3D(domain, "cover_osc_amp", Vector3D(5.f, 10.f, 0.f)));
	m_cf.setCoverAngleOsc(true,
		m_theme.getVector3D(domainSel, "cover_osc_speed", Vector3D(2.1f, 2.1f, 0.f)),
		m_theme.getVector3D(domainSel, "cover_osc_amp", Vector3D(2.f, 5.f, 0.f)));
	m_cf.setCoverPosOsc(false,
		m_theme.getVector3D(domain, "cover_pos_osc_speed", Vector3D(0.f, 0.f, 0.f)),
		m_theme.getVector3D(domain, "cover_pos_osc_amp", Vector3D(0.f, 0.f, 0.f)));
	m_cf.setCoverPosOsc(true,
		m_theme.getVector3D(domainSel, "cover_pos_osc_speed", Vector3D(0.f, 0.f, 0.f)),
		m_theme.getVector3D(domainSel, "cover_pos_osc_amp", Vector3D(0.f, 0.f, 0.f)));
	m_cf.setSpacers(false,
		m_theme.getVector3D(domain, "left_spacer", Vector3D(-0.35f, 0.f, 0.f)),
		m_theme.getVector3D(domain, "right_spacer", Vector3D(0.35f, 0.f, 0.f)));
	m_cf.setSpacers(true,
		m_theme.getVector3D(domainSel, "left_spacer", Vector3D(-0.35f, 0.f, 0.f)),
		m_theme.getVector3D(domainSel, "right_spacer", Vector3D(0.35f, 0.f, 0.f)));
	m_cf.setDeltaAngles(false,
		m_theme.getVector3D(domain, "left_delta_angle", Vector3D(0.f, 0.f, 0.f)),
		m_theme.getVector3D(domain, "right_delta_angle", Vector3D(0.f, 0.f, 0.f)));
	m_cf.setDeltaAngles(true,
		m_theme.getVector3D(domainSel, "left_delta_angle", Vector3D(0.f, 0.f, 0.f)),
		m_theme.getVector3D(domainSel, "right_delta_angle", Vector3D(0.f, 0.f, 0.f)));
	m_cf.setAngles(false,
		m_theme.getVector3D(domain, "left_angle", Vector3D(0.f, 70.f, 0.f)),
		m_theme.getVector3D(domain, "right_angle", Vector3D(0.f, -70.f, 0.f)),
		m_theme.getVector3D(domain, "center_angle", Vector3D(0.f, 0.f, 0.f)),
		m_theme.getVector3D(domain, "row_center_angle", Vector3D(0.f, 0.f, 0.f)));
	m_cf.setAngles(true,
		m_theme.getVector3D(domainSel, "left_angle", Vector3D(-45.f, 90.f, 0.f)),
		m_theme.getVector3D(domainSel, "right_angle", Vector3D(-45.f, -90.f, 0.f)),
		m_theme.getVector3D(domainSel, "center_angle", Vector3D(0.f, 380.f, 0.f)),
		m_theme.getVector3D(domainSel, "row_center_angle", Vector3D(0.f, 0.f, 0.f)));
	m_cf.setTitleAngles(false,
		_getCFFloat(domain, "text_left_angle", -55.f, sf),
		_getCFFloat(domain, "text_right_angle", 55.f, sf),
		_getCFFloat(domain, "text_center_angle", 0.f, sf));
	m_cf.setTitleAngles(true,
		_getCFFloat(domainSel, "text_left_angle", -55.f, sf),
		_getCFFloat(domainSel, "text_right_angle", 55.f, sf),
		_getCFFloat(domainSel, "text_center_angle", 0.f, sf));
	m_cf.setTitlePos(false,
		_getCFV3D(domain, "text_left_pos", Vector3D(-4.f, 0.f, 1.3f), sf),
		_getCFV3D(domain, "text_right_pos", Vector3D(4.f, 0.f, 1.3f), sf),
		_getCFV3D(domain, "text_center_pos", Vector3D(0.f, 0.f, 2.6f), sf));
	m_cf.setTitlePos(true,
		_getCFV3D(domainSel, "text_left_pos", Vector3D(-4.f, 0.f, 1.3f), sf),
		_getCFV3D(domainSel, "text_right_pos", Vector3D(4.f, 0.f, 1.3f), sf),
		_getCFV3D(domainSel, "text_center_pos", Vector3D(1.7f, 1.8f, 1.6f), sf));
	m_cf.setTitleWidth(false,
		_getCFFloat(domain, "text_side_wrap_width", 500.f, sf),
		_getCFFloat(domain, "text_center_wrap_width", 500.f, sf));
	m_cf.setTitleWidth(true,
		_getCFFloat(domainSel, "text_side_wrap_width", 500.f, sf),
		_getCFFloat(domainSel, "text_center_wrap_width", 310.f, sf));
	m_cf.setTitleStyle(false,
		_textStyle(domain.c_str(), "text_side_style", FTGX_ALIGN_BOTTOM | FTGX_JUSTIFY_CENTER),
		_textStyle(domain.c_str(), "text_center_style", FTGX_ALIGN_BOTTOM | FTGX_JUSTIFY_CENTER));
	m_cf.setTitleStyle(true,
		_textStyle(domainSel.c_str(), "text_side_style", FTGX_ALIGN_BOTTOM | FTGX_JUSTIFY_CENTER),
		_textStyle(domainSel.c_str(), "text_center_style", FTGX_ALIGN_TOP | FTGX_JUSTIFY_RIGHT));
	m_cf.setColors(false,
		m_theme.getColor(domain, "color_beg", 0xCFFFFFFF),
		m_theme.getColor(domain, "color_end", 0x3FFFFFFF),
		m_theme.getColor(domain, "color_off", 0x7FFFFFFF));
	m_cf.setColors(true,
		m_theme.getColor(domainSel, "color_beg", 0x7FFFFFFF),
		m_theme.getColor(domainSel, "color_end", 0x1FFFFFFF),
		m_theme.getColor(domain, "color_off", 0x7FFFFFFF));	// Mouse not used once a selection has been made
	m_cf.setMirrorAlpha(m_theme.getFloat(domain, "mirror_alpha", 0.25f), m_theme.getFloat(domain, "title_mirror_alpha", 0.2f));	// Doesn't depend on selection
	m_cf.setMirrorBlur(m_theme.getBool(domain, "mirror_blur", true));	// Doesn't depend on selection
	m_cf.setShadowColors(false,
		m_theme.getColor(domain, "color_shadow_center", 0x00000000),
		m_theme.getColor(domain, "color_shadow_beg", 0x00000000),
		m_theme.getColor(domain, "color_shadow_end", 0x00000000),
		m_theme.getColor(domain, "color_shadow_off", 0x00000000));
	m_cf.setShadowColors(true,
		m_theme.getColor(domainSel, "color_shadow_center", 0x0000007F),
		m_theme.getColor(domainSel, "color_shadow_beg", 0x0000007F),
		m_theme.getColor(domainSel, "color_shadow_end", 0x0000007F),
		m_theme.getColor(domainSel, "color_shadow_off", 0x0000007F));
	m_cf.setShadowPos(m_theme.getFloat(domain, "shadow_scale", 1.1f),
		m_theme.getFloat(domain, "shadow_x", 0.f),
		m_theme.getFloat(domain, "shadow_y", 0.f));
	m_cf.setRowSpacers(false,
		m_theme.getVector3D(domain, "top_spacer", Vector3D(0.f, 2.f, 0.f)),
		m_theme.getVector3D(domain, "bottom_spacer", Vector3D(0.f, -2.f, 0.f)));
	m_cf.setRowSpacers(true,
		m_theme.getVector3D(domainSel, "top_spacer", Vector3D(0.f, 2.f, 0.f)),
		m_theme.getVector3D(domainSel, "bottom_spacer", Vector3D(0.f, -2.f, 0.f)));
	m_cf.setRowDeltaAngles(false,
		m_theme.getVector3D(domain, "top_delta_angle", Vector3D(0.f, 0.f, 0.f)),
		m_theme.getVector3D(domain, "bottom_delta_angle", Vector3D(0.f, 0.f, 0.f)));
	m_cf.setRowDeltaAngles(true,
		m_theme.getVector3D(domainSel, "top_delta_angle", Vector3D(0.f, 0.f, 0.f)),
		m_theme.getVector3D(domainSel, "bottom_delta_angle", Vector3D(0.f, 0.f, 0.f)));
	m_cf.setRowAngles(false,
		m_theme.getVector3D(domain, "top_angle", Vector3D(0.f, 0.f, 0.f)),
		m_theme.getVector3D(domain, "bottom_angle", Vector3D(0.f, 0.f, 0.f)));
	m_cf.setRowAngles(true,
		m_theme.getVector3D(domainSel, "top_angle", Vector3D(0.f, 0.f, 0.f)),
		m_theme.getVector3D(domainSel, "bottom_angle", Vector3D(0.f, 0.f, 0.f)));
	m_cf.setCoverScale(false,
		m_theme.getVector3D(domain, "left_scale", Vector3D(1.f, 1.f, 1.f)),
		m_theme.getVector3D(domain, "right_scale", Vector3D(1.f, 1.f, 1.f)),
		m_theme.getVector3D(domain, "center_scale", Vector3D(1.f, 1.f, 1.f)),
		m_theme.getVector3D(domain, "row_center_scale", Vector3D(1.f, 1.f, 1.f)));
	m_cf.setCoverScale(true,
		m_theme.getVector3D(domainSel, "left_scale", Vector3D(1.f, 1.f, 1.f)),
		m_theme.getVector3D(domainSel, "right_scale", Vector3D(1.f, 1.f, 1.f)),
		m_theme.getVector3D(domainSel, "center_scale", Vector3D(1.f, 1.f, 1.f)),
		m_theme.getVector3D(domainSel, "row_center_scale", Vector3D(1.f, 1.f, 1.f)));
	m_cf.setCoverFlipping(
		_getCFV3D(domainSel, "flip_pos", Vector3D(0.f, 0.f, 0.f), sf),
		_getCFV3D(domainSel, "flip_angle", Vector3D(0.f, 180.f, 0.f), sf),
		_getCFV3D(domainSel, "flip_scale", Vector3D(1.f, 1.f, 1.f), sf));
	m_cf.setBlur(
		m_theme.getInt(domain, "blur_resolution", 1),
		m_theme.getInt(domain, "blur_radius", 2),
		m_theme.getFloat(domain, "blur_factor", 1.f));
}

void CMenu::_buildMenus(void)
{
	SThemeData theme;

	// Default fonts
	theme.btnFont.fromBuffer(butfont_ttf, butfont_ttf_size, 24, 24);
	theme.btnFont = _font(theme.fontSet, "GENERAL", "button_font", theme.btnFont);
	theme.btnFontColor = m_theme.getColor("GENERAL", "button_font_color", 0xD0BFDFFF);
	theme.lblFont.fromBuffer(lblfont_ttf, lblfont_ttf_size, 24, 24);
	theme.lblFont = _font(theme.fontSet, "GENERAL", "label_font", theme.lblFont);
	theme.lblFontColor = m_theme.getColor("GENERAL", "label_font_color", 0xD0BFDFFF);
	theme.txtFontColor = m_theme.getColor("GENERAL", "text_font_color", 0xFFFFFFFF);
	theme.titleFont.fromBuffer(titlefont_ttf, titlefont_ttf_size, 36, 36);
	theme.titleFont = _font(theme.fontSet, "GENERAL", "title_font", theme.titleFont);
	theme.titleFontColor = m_theme.getColor("GENERAL", "title_font_color", 0xD0BFDFFF);
	theme.thxFont.fromBuffer(thxfont_ttf, thxfont_ttf_size, 18, 18);
	theme.thxFont = _font(theme.fontSet, "GENERAL", "thxfont", theme.thxFont);
	// Default Sounds
	theme.clickSound.fromWAV(click_wav, click_wav_size);
	theme.clickSound = _sound(theme.soundSet, "GENERAL", "click_sound", theme.clickSound);
	theme.hoverSound.fromWAV(hover_wav, hover_wav_size);
	theme.hoverSound = _sound(theme.soundSet, "GENERAL", "hover_sound", theme.hoverSound);
	theme.cameraSound.fromWAV(camera_wav, camera_wav_size);
	theme.cameraSound = _sound(theme.soundSet, "GENERAL", "camera_sound", theme.cameraSound);
	m_cameraSound = theme.cameraSound;
	// Default textures
	theme.btnTexL.fromPNG(butleft_png);
	theme.btnTexL = _texture(theme.texSet, "GENERAL", "button_texture_left", theme.btnTexL); 
	theme.btnTexR.fromPNG(butright_png);
	theme.btnTexR = _texture(theme.texSet, "GENERAL", "button_texture_right", theme.btnTexR); 
	theme.btnTexC.fromPNG(butcenter_png);
	theme.btnTexC = _texture(theme.texSet, "GENERAL", "button_texture_center", theme.btnTexC); 
	theme.btnTexLS.fromPNG(butsleft_png);
	theme.btnTexLS = _texture(theme.texSet, "GENERAL", "button_texture_left_selected", theme.btnTexLS); 
	theme.btnTexRS.fromPNG(butsright_png);
	theme.btnTexRS = _texture(theme.texSet, "GENERAL", "button_texture_right_selected", theme.btnTexRS); 
	theme.btnTexCS.fromPNG(butscenter_png);
	theme.btnTexCS = _texture(theme.texSet, "GENERAL", "button_texture_center_selected", theme.btnTexCS); 
	theme.pbarTexL.fromPNG(pbarleft_png);
	theme.pbarTexL = _texture(theme.texSet, "GENERAL", "progressbar_texture_left", theme.pbarTexL); 
	theme.pbarTexR.fromPNG(pbarright_png);
	theme.pbarTexR = _texture(theme.texSet, "GENERAL", "progressbar_texture_right", theme.pbarTexR); 
	theme.pbarTexC.fromPNG(pbarcenter_png);
	theme.pbarTexC = _texture(theme.texSet, "GENERAL", "progressbar_texture_center", theme.pbarTexC); 
	theme.pbarTexLS.fromPNG(pbarlefts_png);
	theme.pbarTexLS = _texture(theme.texSet, "GENERAL", "progressbar_texture_left_selected", theme.pbarTexLS); 
	theme.pbarTexRS.fromPNG(pbarrights_png);
	theme.pbarTexRS = _texture(theme.texSet, "GENERAL", "progressbar_texture_right_selected", theme.pbarTexRS); 
	theme.pbarTexCS.fromPNG(pbarcenters_png);
	theme.pbarTexCS = _texture(theme.texSet, "GENERAL", "progressbar_texture_center_selected", theme.pbarTexCS); 
	theme.btnTexPlus.fromPNG(btnplus_png);
	theme.btnTexPlus = _texture(theme.texSet, "GENERAL", "plus_button_texture", theme.btnTexPlus); 
	theme.btnTexPlusS.fromPNG(btnpluss_png);
	theme.btnTexPlusS = _texture(theme.texSet, "GENERAL", "plus_button_texture_selected", theme.btnTexPlusS); 
	theme.btnTexMinus.fromPNG(btnminus_png);
	theme.btnTexMinus = _texture(theme.texSet, "GENERAL", "minus_button_texture", theme.btnTexMinus); 
	theme.btnTexMinusS.fromPNG(btnminuss_png);
	theme.btnTexMinusS = _texture(theme.texSet, "GENERAL", "minus_button_texture_selected", theme.btnTexMinusS); 
	// Default background
	theme.bg.fromPNG(background_png, GX_TF_RGBA8, ALLOC_MEM2);
	m_mainBgLQ.fromPNG(background_png, GX_TF_CMPR, ALLOC_MEM2, 64, 64);
	m_gameBgLQ = m_mainBgLQ;
	
	m_waitMessage = _texture(theme.texSet, "GENERAL", "waitmessage", m_waitMessage); 

	// Build menus
	_initMainMenu(theme);
	_initErrorMenu(theme);
	_initConfigAdvMenu(theme);
	_initConfigSndMenu(theme);
	_initConfig5Menu(theme);
	_initConfig4Menu(theme);
	_initConfig3Menu(theme);
	_initConfig2Menu(theme);
	_initConfigMenu(theme);
	_initGameMenu(theme);
	_initDownloadMenu(theme);
	_initCodeMenu(theme);
	_initAboutMenu(theme);
	_initWBFSMenu(theme);
	_initCFThemeMenu(theme);
	_initGameSettingsMenu(theme);
	_initCheatSettingsMenu(theme); 
	_initCategorySettingsMenu(theme);
	_initSystemMenu(theme);
	_initGameInfoMenu(theme);
}

SFont CMenu::_font(CMenu::FontSet &fontSet, const char *domain, const char *key, SFont def)
{
	string filename;
	string fontSizeKey;
	string lineSpacingKey;
	SFont retFont;
	u32 fontSize = 0;
	u32 lineSpacing = 0;
	CMenu::FontSet::iterator i;
	CMenu::FontSet::iterator j;

	if (m_theme.loaded())
	{
		filename = m_theme.getString(domain, key);
		for (u32 c = 0; c < filename.size(); ++c)
			if (filename[c] >= 'A' && filename[c] <= 'Z')
				filename[c] |= 0x20;
		fontSizeKey = key;
		fontSizeKey += "_size";
		fontSize = min(max(6u, (u32)m_theme.getInt(domain, fontSizeKey)), 300u);
		lineSpacingKey = key;
		lineSpacingKey += "_line_height";
		lineSpacing = min(max(6u, (u32)m_theme.getInt(domain, lineSpacingKey)), 300u);
		if (!filename.empty())
		{
			// Try to find the same font with the same size
			i = fontSet.find(CMenu::FontDesc(filename, fontSize));
			if (i != fontSet.end())
			{
				retFont = i->second;
				if (!!retFont.data)
					retFont.lineSpacing = lineSpacing;
				return retFont;
			}
			// Then try to find the same font with a different size
			i = fontSet.lower_bound(CMenu::FontDesc(filename, 0));
			if (i != fontSet.end() && i->first.first == filename)
			{
				retFont = i->second;
				retFont.newSize(fontSize, lineSpacing);
				fontSet[CMenu::FontDesc(filename, fontSize)] = retFont;
				return retFont;
			}
			// TTF not found in memory, load it to create a new font
			if (retFont.fromFile(sfmt("%s/%s", m_themeDataDir.c_str(), filename.c_str()).c_str(), fontSize, lineSpacing))
			{
				fontSet[CMenu::FontDesc(filename, fontSize)] = retFont;
				return retFont;
			}
		}
	}
	// Default font
	fontSet[CMenu::FontDesc(filename, fontSize)] = def;
	return def;
}

STexture CMenu::_texture(CMenu::TexSet &texSet, const char *domain, const char *key, STexture def)
{
	string filename;
	SmartBuf ptrPNG;
	STexture tex;
	CMenu::TexSet::iterator i;

	if (m_theme.loaded())
	{
		filename = m_theme.getString(domain, key);
		if (!filename.empty())
		{
			i = texSet.find(filename);
			if (i != texSet.end())
				return i->second;
			if (STexture::TE_OK == tex.fromPNGFile(sfmt("%s/%s", m_themeDataDir.c_str(), filename.c_str()).c_str(), GX_TF_RGBA8, ALLOC_MEM2))
			{
				texSet[filename] = tex;
				return tex;
			}
		}
	}
	texSet[filename] = def;
	return def;
}

SSoundEffect CMenu::_sound(CMenu::SoundSet &soundSet, const char *domain, const char *key, SSoundEffect def)
{
	CMenu::SoundSet::iterator i;
	string filename;
	SSoundEffect sound;

	filename = m_theme.getString(domain, key);
	if (filename.empty())
		return def;
	for (u32 c = 0; c < filename.size(); ++c)
		if (filename[c] >= 'A' && filename[c] <= 'Z')
			filename[c] |= 0x20;
	i = soundSet.find(filename);
	if (i == soundSet.end())
	{
		sound.fromWAVFile(sfmt("%s/%s", m_themeDataDir.c_str(), filename.c_str()).c_str());
		soundSet[filename] = sound;
		return sound;
	}
	return i->second;
}

u16 CMenu::_textStyle(const char *domain, const char *key, u16 def)
{
	u16 textStyle = 0;
	string style(m_theme.getString(domain, key));
	if (style.empty())
		return def;
	if (style.find_first_of("Cc") != string::npos)
		textStyle |= FTGX_JUSTIFY_CENTER;
	else if (style.find_first_of("Rr") != string::npos)
		textStyle |= FTGX_JUSTIFY_RIGHT;
	else
		textStyle |= FTGX_JUSTIFY_LEFT;
	if (style.find_first_of("Mm") != string::npos)
		textStyle |= FTGX_ALIGN_MIDDLE;
	else if (style.find_first_of("Bb") != string::npos)
		textStyle |= FTGX_ALIGN_BOTTOM;
	else
		textStyle |= FTGX_ALIGN_TOP;
	return textStyle;
}

u32 CMenu::_addButton(CMenu::SThemeData &theme, const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color)
{
	SButtonTextureSet btnTexSet;
	CColor c(color);
	SSoundEffect clickSound;
	SSoundEffect hoverSound;

	c = m_theme.getColor(domain, "color", c);
	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	btnTexSet.left = _texture(theme.texSet, domain, "texture_left", theme.btnTexL);
	btnTexSet.right = _texture(theme.texSet, domain, "texture_right", theme.btnTexR);
	btnTexSet.center = _texture(theme.texSet, domain, "texture_center", theme.btnTexC);
	btnTexSet.leftSel = _texture(theme.texSet, domain, "texture_left_selected", theme.btnTexLS);
	btnTexSet.rightSel = _texture(theme.texSet, domain, "texture_right_selected", theme.btnTexRS);
	btnTexSet.centerSel = _texture(theme.texSet, domain, "texture_center_selected", theme.btnTexCS);
	font = _font(theme.fontSet, domain, "font", font);
	clickSound = _sound(theme.soundSet, domain, "click_sound", theme.clickSound);
	hoverSound = _sound(theme.soundSet, domain, "hover_sound", theme.hoverSound);
	return m_btnMgr.addButton(font, text, x, y, width, height, c, btnTexSet, clickSound, hoverSound);
}

u32 CMenu::_addPicButton(CMenu::SThemeData &theme, const char *domain, STexture &texNormal, STexture &texSelected, int x, int y, u32 width, u32 height)
{
	STexture tex1;
	STexture tex2;
	SSoundEffect clickSound;
	SSoundEffect hoverSound;

	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	tex1 = _texture(theme.texSet, domain, "texture_normal", texNormal);
	tex2 = _texture(theme.texSet, domain, "texture_selected", texSelected);
	clickSound = _sound(theme.soundSet, domain, "click_sound", theme.clickSound);
	hoverSound = _sound(theme.soundSet, domain, "hover_sound", theme.hoverSound);
	return m_btnMgr.addPicButton(tex1, tex2, x, y, width, height, clickSound, hoverSound);
}

u32 CMenu::_addLabel(CMenu::SThemeData &theme, const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, u16 style)
{
	CColor c(color);

	c = m_theme.getColor(domain, "color", c);
	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	font = _font(theme.fontSet, domain, "font", font);
	style = _textStyle(domain, "style", style);
	return m_btnMgr.addLabel(font, text, x, y, width, height, c, style);
}

u32 CMenu::_addLabel(CMenu::SThemeData &theme, const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, u16 style, STexture &bg)
{
	STexture texBg;
	SButtonTextureSet btnTexSet;
	CColor c(color);

	c = m_theme.getColor(domain, "color", c);
	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	font = _font(theme.fontSet, domain, "font", font);
	texBg = _texture(theme.texSet, domain, "background_texture", bg);
	style = _textStyle(domain, "style", style);
	return m_btnMgr.addLabel(font, text, x, y, width, height, c, style, texBg);
}

u32 CMenu::_addProgressBar(CMenu::SThemeData &theme, const char *domain, int x, int y, u32 width, u32 height)
{
	SButtonTextureSet btnTexSet;

	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	btnTexSet.left = _texture(theme.texSet, domain, "texture_left", theme.pbarTexL);
	btnTexSet.right = _texture(theme.texSet, domain, "texture_right", theme.pbarTexR);
	btnTexSet.center = _texture(theme.texSet, domain, "texture_center", theme.pbarTexC);
	btnTexSet.leftSel = _texture(theme.texSet, domain, "texture_left_selected", theme.pbarTexLS);
	btnTexSet.rightSel = _texture(theme.texSet, domain, "texture_right_selected", theme.pbarTexRS);
	btnTexSet.centerSel = _texture(theme.texSet, domain, "texture_center_selected", theme.pbarTexCS);
	return m_btnMgr.addProgressBar(x, y, width, height, btnTexSet);
}

void CMenu::_setHideAnim(u32 id, const char *domain, int dx, int dy, float scaleX, float scaleY)
{
	dx = m_theme.getInt(domain, "effect_x", dx);
	dy = m_theme.getInt(domain, "effect_y", dy);
	scaleX = m_theme.getFloat(domain, "effect_scale_x", scaleX);
	scaleY = m_theme.getFloat(domain, "effect_scale_y", scaleY);
	m_btnMgr.hide(id, dx, dy, scaleX, scaleY, true);
}

void CMenu::_addUserLabels(CMenu::SThemeData &theme, u32 *ids, u32 size, const char *domain)
{
	_addUserLabels(theme, ids, 0, size, domain);
}

void CMenu::_addUserLabels(CMenu::SThemeData &theme, u32 *ids, u32 start, u32 size, const char *domain)
{
	STexture emptyTex;

	for (u32 i = start; i < start + size; ++i)
	{
		string dom(sfmt("%s/USER%i", domain, i + 1));
		if (m_theme.hasDomain(dom))
		{
			ids[i] = _addLabel(theme, dom.c_str(), theme.lblFont, L"", 40, 200, 64, 64, CColor(0xFFFFFFFF), 0, emptyTex);
			_setHideAnim(ids[i], dom.c_str(), -50, 0, 0.f, 0.f);
		}
		else
			ids[i] = -1u;
	}
}

void CMenu::_initCF(void)
{
	string id, test_id;
	bool cmpr;
	Config titles;
	u64 chantitle;
	m_titles_loaded = false;
	if (m_current_view == COVERFLOW_USB) m_gamelistdump = m_cfg.getBool("GENERAL", "dump_gamelist", true);
	else if (m_current_view == COVERFLOW_CHANNEL) m_gamelistdump = m_cfg.getBool("GENERAL", "dump_chanlist", true);
	if (m_gamelistdump)
		m_dump.load(sfmt("%s/titlesdump.ini", m_settingsDir.c_str()).c_str());

	if (titles.load(sfmt("%s/titles.ini", m_settingsDir.c_str()).c_str()))
		m_titles_loaded = true;
	m_cf.clear();
	m_cf.reserve(m_gameList.size());
	m_gcfg1.load(sfmt("%s/gameconfig1.ini", m_settingsDir.c_str()).c_str());
	for (u32 i = 0; i < m_gameList.size(); ++i)
	{
		id = string((const char *)m_gameList[i].hdr.id, m_gameList[i].hdr.id[5] == 0 ? strlen((const char *) m_gameList[i].hdr.id) : sizeof m_gameList[0].hdr.id);
		chantitle = m_gameList[i].hdr.chantitle;
		test_id = (m_current_view == COVERFLOW_CHANNEL && chantitle == 281482209522966ULL) ? "JODI" : id;
		
		if ((!m_favorites || m_gcfg1.getBool("FAVORITES", test_id, false)) && (!m_locked || !m_gcfg1.getBool("ADULTONLY", test_id, false)) && !m_gcfg1.getBool("HIDDEN", test_id, false))
		{
			if (m_category != 0)
			{
				const char *categories = m_cat.getString("CATEGORIES", test_id, "").c_str();
				if (strlen(categories) != 12 || categories[m_category] == '0') { // 12 categories max!
					continue;
				}
			}
			wstringEx w(titles.getWString("TITLES", test_id));
			if (w.empty())
			{
				if (m_current_view == COVERFLOW_CHANNEL) // Required, since Channel titles are already in wchar_t
				{
					wchar_t channelname[IMET_MAX_NAME_LEN];
					mbstowcs(channelname, m_gameList[i].hdr.title, IMET_MAX_NAME_LEN);
					w = titles.getWString("TITLES", id.substr(0, 4), channelname);
				}
				else
					w = titles.getWString("TITLES", id.substr(0, 4), string(m_gameList[i].hdr.title, sizeof m_gameList[0].hdr.title));
			}
			int playcount = m_gcfg1.getInt("PLAYCOUNT", test_id, 0);
			unsigned int lastPlayed = m_gcfg1.getUInt("LASTPLAYED", test_id, 0);

			if (m_current_view == COVERFLOW_CHANNEL && m_gamelistdump)
				m_dump.setWString("CHANNELS", id.substr(0, 4), w);
			else if (m_current_view == COVERFLOW_USB && m_gamelistdump)
				m_dump.setWString("GAMES", id, w);

			m_cf.addItem(&m_gameList[i], w.c_str(), chantitle, sfmt("%s/%s.png", m_picDir.c_str(), test_id.c_str()).c_str(), sfmt("%s/%s.png", m_boxPicDir.c_str(), test_id.c_str()).c_str(), playcount, lastPlayed);
		}
	}
	m_gcfg1.unload();
	if (m_gamelistdump)
	{
		m_dump.save();
		m_dump.unload();
		if (m_current_view == COVERFLOW_USB) m_cfg.setBool("GENERAL", "dump_gamelist", false);
		else if (m_current_view == COVERFLOW_CHANNEL) m_cfg.setBool("GENERAL", "dump_chanlist", false);
	}
	m_cf.setBoxMode(m_cfg.getBool("GENERAL", "box_mode", true));
	cmpr = m_cfg.getBool("GENERAL", "allow_texture_compression", true);
	m_cf.setCompression(cmpr);
	m_cf.setBufferSize(m_cfg.getInt("GENERAL", "cover_buffer", 120));
	if (m_cf.setSorting((Sorting)m_cfg.getInt("GENERAL", "sort", 0)))
		if (m_curGameId.empty() || !m_cf.findId(m_curGameId.c_str(), true))
			m_cf.findId(m_cfg.getString("GENERAL", m_current_view == COVERFLOW_CHANNEL ? "current_channel" : "current_game").c_str(), true);
}

void CMenu::_mainLoopCommon(bool withCF, bool blockReboot, bool adjusting)
{
	if (withCF)
		m_cf.tick();
	m_btnMgr.tick();
	m_fa.tick();
	m_cf.setFanartPlaying(m_fa.isLoaded());
	m_cf.setFanartTextColor(m_fa.getTextColor(m_theme.getColor("_COVERFLOW", "font_color", CColor(0xFFFFFFFF))));

	_updateBg();
	
	if (m_fa.hideCover(m_hidecover, m_show_cover_after_animation))
		m_cf.hideCover();
	else
		m_cf.showCover();

	if (withCF)
		m_cf.makeEffectTexture(m_vid, m_lqBg);
	if (withCF && m_aa > 0)
	{
		m_vid.setAA(m_aa, true);
		for (int i = 0; i < m_aa; ++i)
		{
			m_vid.prepareAAPass(i);
			m_vid.setup2DProjection(false, true);
			_drawBg();
			m_fa.draw(m_allow_fanart_on_top, false);
			m_cf.draw();
			m_vid.setup2DProjection(false, true);
			m_cf.drawEffect();
			m_cf.drawText(adjusting);
			m_vid.renderAAPass(i);
		}
		m_vid.setup2DProjection();
		m_vid.drawAAScene();
	}
	else
	{
		m_vid.prepare();
		m_vid.setup2DProjection();
		_drawBg();
		m_fa.draw(m_allow_fanart_on_top, false);
		if (withCF)
		{
			m_cf.draw();
			m_vid.setup2DProjection();
			m_cf.drawEffect();
			m_cf.drawText(adjusting);
		}
	}
	
	m_fa.draw(m_allow_fanart_on_top);
	
	m_btnMgr.draw();
	ScanInput();
	m_vid.render();
	if (!blockReboot)
	{
		if (withCF && Sys_Exiting())
			m_cf.clear();
		if (Sys_Exiting())
		{
			m_cat.save();
			m_cfg.save();
		}
		Sys_Test();
	}
	LWP_MutexLock(m_gameSndMutex);
	if (withCF && m_gameSelected && !!m_gameSoundTmp.data)
	{
		m_gameSound.stop();
		m_gameSound = m_gameSoundTmp;
		m_gameSoundTmp.data.release();
		m_gameSound.play(m_bnrSndVol);
	}
	else if (!withCF || !m_gameSelected)
		m_gameSound.stop();

	LWP_MutexUnlock(m_gameSndMutex);
	if (withCF && m_gameSoundThread == 0)
		m_cf.startPicLoader();
	if (!m_video_playing)
		_loopMusic();
	//Take Screenshot
	if ((gc_btnsPressed & PAD_TRIGGER_Z) != 0)
	{
		time_t rawtime;
		struct tm * timeinfo;
		char buffer[80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(buffer,80,"%b-%d-20%y-%Hh%Mm%Ss.png",timeinfo);
		gprintf("Screenshot taken and saved to: %s/%s\n", m_screenshotDir.c_str(), buffer);
		m_vid.TakeScreenshot(sfmt("%s/%s", m_screenshotDir.c_str(), buffer).c_str());
		m_cameraSound.play(255);
	}
}

void CMenu::_setBg(const STexture &tex, const STexture &lqTex)
{
	m_lqBg = lqTex;
	if (tex.data.get() == m_nextBg.data.get())
		return;
	m_prevBg = m_curBg;
	m_nextBg = tex;
	m_bgCrossFade = 0xFF;
}

void CMenu::_updateBg(void)
{
	Mtx modelViewMtx;
	GXTexObj texObj;
	GXTexObj texObj2;
	Mtx44 projMtx;

	if (m_bgCrossFade == 0)
		return;
	m_bgCrossFade = max(0, (int)m_bgCrossFade - 14);
	if (m_bgCrossFade == 0)
	{
		m_curBg = m_nextBg;
		return;
	}
	if (m_curBg.data.get() == m_prevBg.data.get())
		m_curBg.data.release();
	m_vid.prepare();
	GX_SetViewport(0.f, 0.f, 640.f, 480.f, 0.f, 1.f);
	guOrtho(projMtx, 0.f, 480.f, 0.f, 640.f, 0.f, 1000.0f);
	GX_LoadProjectionMtx(projMtx, GX_ORTHOGRAPHIC);
	GX_ClearVtxDesc();
	GX_SetNumTevStages(!m_prevBg.data ? 1 : 2);
	GX_SetNumChans(0);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetNumTexGens(!m_prevBg.data ? 1 : 2);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetTevKColor(GX_KCOLOR0, CColor(m_bgCrossFade, 0xFF - m_bgCrossFade, 0, 0));
	GX_SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0_R);
	GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_KONST, GX_CC_ZERO);
	GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
	GX_SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K0_G);
	GX_SetTevColorIn(GX_TEVSTAGE1, GX_CC_TEXC, GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV);
	GX_SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GX_SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLORNULL);
	GX_SetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_FALSE);
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_FALSE);
	guMtxIdentity(modelViewMtx);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	GX_InitTexObj(&texObj, m_nextBg.data.get(), m_nextBg.width, m_nextBg.height, m_nextBg.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);
	if (!!m_prevBg.data)
	{
		GX_InitTexObj(&texObj2, m_prevBg.data.get(), m_prevBg.width, m_prevBg.height, m_prevBg.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_LoadTexObj(&texObj2, GX_TEXMAP1);
	}
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(0.f, 0.f, 0.f);
	GX_TexCoord2f32(0.f, 0.f);
	GX_Position3f32(640.f, 0.f, 0.f);
	GX_TexCoord2f32(1.f, 0.f);
	GX_Position3f32(640.f, 480.f, 0.f);
	GX_TexCoord2f32(1.f, 1.f);
	GX_Position3f32(0.f, 480.f, 0.f);
	GX_TexCoord2f32(0.f, 1.f);
	GX_End();
	GX_SetNumTevStages(1);
	m_curBg.width = 640;
	m_curBg.height = 480;
	m_curBg.format = GX_TF_RGBA8;
	m_curBg.maxLOD = 0;
	m_vid.renderToTexture(m_curBg, true);
	if (!m_curBg.data)
	{
		m_curBg = m_nextBg;
		m_bgCrossFade = 0;
	}
}

void CMenu::_drawBg(void)
{
	Mtx modelViewMtx;
	GXTexObj texObj;

	GX_ClearVtxDesc();
	GX_SetNumTevStages(1);
	GX_SetNumChans(0);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetNumTexGens(1);
	GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
	GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
	GX_SetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_FALSE);
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_FALSE);
	guMtxIdentity(modelViewMtx);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	GX_InitTexObj(&texObj, m_curBg.data.get(), m_curBg.width, m_curBg.height, m_curBg.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(0.f, 0.f, 0.f);
	GX_TexCoord2f32(0.f, 0.f);
	GX_Position3f32(640.f, 0.f, 0.f);
	GX_TexCoord2f32(1.f, 0.f);
	GX_Position3f32(640.f, 480.f, 0.f);
	GX_TexCoord2f32(1.f, 1.f);
	GX_Position3f32(0.f, 480.f, 0.f);
	GX_TexCoord2f32(0.f, 1.f);
	GX_End();
}

void CMenu::_updateText(void)
{
	_textMain();
	_textError();
	_textConfig();
	_textConfig2();
	_textConfig3();
	_textConfig4();
	_textConfig5();
	_textConfigSnd();
	_textConfigAdv();
	_textDownload();
	_textGame();
	_textCode();
	_textWBFS();
	_textGameSettings();
	_textCategorySettings();
	_textSystem();
}

const wstringEx CMenu::_fmt(const char *key, const wchar_t *def)
{
	wstringEx ws = m_loc.getWString(m_curLanguage, key, def);
	if (checkFmt(def, ws))
		return ws;
	return def;
}

bool CMenu::_loadChannelList(void)
{
	string langCode = m_loc.getString(m_curLanguage, "wiitdb_code", "EN");
	
	m_channels.Init(0, langCode);
	SmartBuf buffer;
	u32 count = m_channels.Count();
	u32 len = count * sizeof m_gameList[0];
	buffer = smartAnyAlloc(len);
	if (!buffer)
		return false;
	memset(buffer.get(), 0, len);

	m_gameList.clear();
	m_gameList.reserve(count);
	dir_discHdr *b = (dir_discHdr *)buffer.get();
	for (u32 i = 0; i < count; ++i) {
		Channel *chan = m_channels.GetChannel(i);
		
		if (chan->id == NULL) continue; // Skip invalid channels

		memcpy(&b[i].hdr.id, chan->id, 4);
		wcstombs(b[i].hdr.title, chan->name, sizeof(b->hdr.title));
//		memcpy(&b[i].hdr.title, chan->name, sizeof(b->hdr.title)); // IMET header specifies max name length 42 (wchar, so * 4!), so we copy only the first 64 bytes here...
		b[i].hdr.chantitle = chan->title;
	
		m_gameList.push_back(b[i]);
	}
	return true;
}

bool CMenu::_loadList(void)
{
	if (m_current_view == COVERFLOW_CHANNEL)
		return _loadChannelList();
	else if (m_current_view == COVERFLOW_USB)
		return _loadGameList();
	return false;
}

bool CMenu::_loadGameList(void)
{
	s32 ret;
	u32 len;
	SmartBuf buffer;
	u32 count;

	char part[6];
	WBFS_GetPartitionName(0, (char *) &part);
	
	ret = WBFS_OpenNamed((char *) m_cfg.getString("GENERAL", "partition", part).c_str());
 	if (ret < 0)
	{
		gprintf("Open partition failed : %i\n", ret);
		return false;
	}
	ret = WBFS_GetCount(&count);
	if (ret < 0)
	{
		error(wfmt(_fmt("wbfs3", L"WBFS_GetCount failed : %i"), ret));
		return false;
	}
	len = count * sizeof m_gameList[0];
#if 0
	FILE *file = 0;
	long fileSize = 0;

	file = fopen(sfmt("%s/%s", m_dataDir.c_str(), "hdrdump").c_str(), "rb");
	if (file == 0)
		return false;
	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	len = fileSize;
	count = len / sizeof m_gameList[0];
	fseek(file, 0, SEEK_SET);
	if (fileSize > 0)
	{
		buffer = smartMemAlign32(fileSize);
		fread(buffer.get(), 1, fileSize, file);
	}
	fclose(file);
	file = 0;
#else
	buffer = smartAnyAlloc(len);
	if (!buffer)
		return false;
	memset(buffer.get(), 0, len);
	ret = WBFS_GetHeaders((dir_discHdr *)buffer.get(), count, sizeof (struct dir_discHdr));
#endif
	if (ret < 0)
	{
		error(wfmt(_fmt("wbfs4", L"WBFS_GetHeaders failed : %i"), ret));
		return false;
	}
#if 0
	FILE *file = fopen(sfmt("%s/%s", m_dataDir.c_str(), "hdrdump").c_str(), "wb");
	fwrite(buffer.get(), 1, len, file);
	fclose(file);
#endif

	m_gameList.clear();
	m_gameList.reserve(count);
	dir_discHdr *b = (dir_discHdr *)buffer.get();
	for (u32 i = 0; i < count; ++i)
		if (memcmp(b[i].hdr.id, "__CFG_", sizeof b[i].hdr.id) != 0)	// Because of uLoader
			m_gameList.push_back(b[i]);
	return true;
}

static void listOGGMP3(const char *path, vector<string> &oggFiles)
{
	DIR *d;
	struct dirent *dir;
	string fileName;
	string fileExt;

	oggFiles.clear();
	d = opendir(path);
	if (d != 0)
	{
		dir = readdir(d);
		while (dir != 0)
		{
			fileName = dir->d_name;
			for (u32 i = 0; i < fileName.size(); ++i)
				if (fileName[i] >= 'a' && fileName[i] <= 'z')
					fileName[i] &= 0xDF;
			if (fileName.size() > 4)
			{
				fileExt = fileName.substr(fileName.size() - 4, 4);
				if (fileExt == ".OGG" || fileExt == ".MP3")
					oggFiles.push_back(fileName);
			}
			dir = readdir(d);
		}
		closedir(d);
	}
}

void CMenu::_searchMusic(void)
{
	listOGGMP3(m_musicDir.c_str(), music_files);
	if (music_files.empty())
		return;
		
	if (m_cfg.getBool("GENERAL", "randomize_music", true))
		random_shuffle(music_files.begin(), music_files.end());
		
	current_music = music_files.begin();
}

void CMenu::_startMusic(void)
{
	SmartBuf buffer;

	if (music_files.empty())
		return;
		
	if (current_music == music_files.end())
		current_music = music_files.begin();

	_stopMusic();

	ifstream file(sfmt("%s/%s", m_musicDir.c_str(), (*current_music).c_str()).c_str(), ios::in | ios::binary);
	m_music_ismp3 = (*current_music).substr((*current_music).size() - 4, 4) == ".MP3";
	if (!file.is_open())
		return;
	file.seekg(0, ios::end);
	m_music_fileSize = file.tellg();
	file.seekg(0, ios::beg);
	buffer = smartMem2Alloc(m_music_fileSize);
	if (!buffer)
		return;
	file.read((char *)buffer.get(), m_music_fileSize);
	if (file.fail())
		return;
	file.close();
	m_music = buffer;
	if(m_music_ismp3)
	{
		MP3Player_PlayBuffer((char *)m_music.get(), m_music_fileSize, NULL);
	}
	else
	{
		PlayOgg(mem_open((char *)m_music.get(), m_music_fileSize), 0, OGG_INFINITE_TIME);
	}
	SetVolumeOgg(m_cfg.getInt("GENERAL", "sound_volume_music", 255));
	MP3Player_Volume(m_cfg.getInt("GENERAL", "sound_volume_music", 255));
	
	current_music++;
}

void CMenu::_stopMusic(void)
{
	MP3Player_Stop();
	StopOgg();
	m_music.release();
}

void CMenu::_pauseMusic(void)
{
	PauseOgg(1);
	MP3Player_Stop();
}

void CMenu::_resumeMusic(void)
{
	PauseOgg(0);
	if(m_music_ismp3)
	{
		MP3Player_PlayBuffer((char *)m_music.get(), m_music_fileSize, NULL);
	}	
}

void CMenu::_loopMusic(void)
{		
	if((m_music_ismp3 && !MP3Player_IsPlaying()) || StatusOgg() == OGG_STATUS_EOF)
	{
		_startMusic();
	}
	return;
}

void CMenu::_stopSounds(void)
{
	_stopMusic();
	m_btnMgr.stopSounds();
	m_cf.stopSound();
	m_gameSound.stop();
}

bool CMenu::_loadFile(SmartBuf &buffer, u32 &size, const char *path, const char *file)
{
	FILE *fp = 0;
	u32 fileSize;
	SmartBuf fileBuf;

	buffer.release();
	size = 0;
	fp = fopen(file == NULL ? path : fmt("%s/%s", path, file), "rb");
		
	if (fp == 0)
		return false;

	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fileBuf = smartCoverAlloc(fileSize);
	if (!fileBuf)
	{
		fclose(fp);
		return false;
	}
	if (fread(fileBuf.get(), 1, fileSize, fp) != fileSize)
	{
		fclose(fp);
		return false;
	}
	fclose(fp);
	buffer = fileBuf;
	size = fileSize;
	
	return true;
}