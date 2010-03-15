
#include "menu.hpp"
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "loader/fs.h"
#include "oggplayer.h"

#include <wiiuse/wpad.h>
#include <fstream>
#include <map>
#include <sys/stat.h>
#include <fstream>
#include <dirent.h>

#include "gecko.h"

// Sounds
extern const u8 click_wav[];
extern const u32 click_wav_size;
extern const u8 hover_wav[];
extern const u32 hover_wav_size;
// Pics
extern const u8 wait_png[];
extern const u8 fbi_png[];
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

using namespace std;

static const u32 g_repeatDelay = 15;

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
	m_networkInit = false;
	m_mutex = 0;
	m_letter = 0;
	m_noHBC = false;
	m_gameSoundThread = 0;
	m_gameSndMutex = 0;
	m_numCFVersions = 0;
	m_bgCrossFade = 0;
	m_bnrSndVol = 0;
	m_padLeftDelay = 0;
	m_padRightDelay = 0;
	m_gameSettingsPage = 0;
}

void CMenu::init(bool fromHBC)
{
	string themeName;
	const char *drive = "sd";
	const char *defaultLanguage;
	string appdir = APPDATA_DIR;

	m_noHBC = !fromHBC;
	m_waitMessage.fromPNG(wait_png);
	// Data path
	if (Fat_SDAvailable() && Fat_USBAvailable())
	{		
		//if (!m_cfg.load(sfmt("sd:/" APPDATA_DIR "/" CFG_FILENAME).c_str()))
		if (!m_cfg.load(sfmt("sd:/%s/%s", appdir.c_str(),CFG_FILENAME).c_str()))
		{
				appdir = APPDATA_DIR2;
				m_cfg.load(sfmt("sd:/%s/%s", appdir.c_str(),CFG_FILENAME).c_str());
		}
				
		bool dataOnUSB = m_cfg.getBool(" GENERAL", "data_on_usb", true);
		drive = dataOnUSB ? "usb" : "sd";
//		if (!m_cfg.loaded() && dataOnUSB)
//			m_cfg.save();
	}
	else
		drive = Fat_USBAvailable() ? "usb" : "sd";
	// 
	m_dataDir = sfmt("%s:/%s", drive,appdir.c_str());
	if(!m_cfg.load(sfmt("%s/" CFG_FILENAME, m_dataDir.c_str()).c_str())) 
	{
			appdir = APPDATA_DIR2;
			m_dataDir = sfmt("%s:/%s", drive,appdir.c_str());
			m_cfg.load(sfmt("%s/" CFG_FILENAME, m_dataDir.c_str()).c_str());
	}
	//
	m_picDir = m_cfg.getString(" GENERAL", "dir_flat_covers", sfmt("%s:/%s/covers", drive, appdir.c_str()));
	m_boxPicDir = m_cfg.getString(" GENERAL", "dir_box_covers", sfmt("%s:/%s/boxcovers", drive, appdir.c_str()));
	m_cacheDir = m_cfg.getString(" GENERAL", "dir_cache", sfmt("%s:/%s/cache", drive, appdir.c_str()));
	m_themeDir = m_cfg.getString(" GENERAL", "dir_themes", sfmt("%s:/%s/themes", drive, appdir.c_str()));
	m_musicDir = m_cfg.getString(" GENERAL", "dir_music", sfmt("%s:/%s/music", drive, appdir.c_str())); 
	/*m_picDir = m_cfg.getString(" GENERAL", "dir_flat_covers", sfmt("%s:/" APPDATA_DIR "/covers", drive));
	m_boxPicDir = m_cfg.getString(" GENERAL", "dir_box_covers", sfmt("%s:/" APPDATA_DIR "/boxcovers", drive));
	m_cacheDir = m_cfg.getString(" GENERAL", "dir_cache", sfmt("%s:/" APPDATA_DIR "/cache", drive));
	m_themeDir = m_cfg.getString(" GENERAL", "dir_themes", sfmt("%s:/" APPDATA_DIR "/themes", drive));
	m_musicDir = m_cfg.getString(" GENERAL", "dir_music", sfmt("%s:/" APPDATA_DIR "/music", drive));*/
	// 
	m_cf.init();
	// 
	struct stat dummy;
	if (stat(sfmt("%s:/", drive).c_str(), &dummy) == 0)
	{
		mkdir(m_dataDir.c_str(), 0777);
		mkdir(m_picDir.c_str(), 0777);
		mkdir(m_boxPicDir.c_str(), 0777);
		mkdir(m_cacheDir.c_str(), 0777);
		mkdir(m_themeDir.c_str(), 0777);
	}
	// INI files
	m_loc.load(sfmt("%s/" LANG_FILENAME, m_dataDir.c_str()).c_str());
	themeName = m_cfg.getString(" GENERAL", "theme", "DEFAULT");
	m_themeDataDir = sfmt("%s/%s", m_themeDir.c_str(), themeName.c_str());
	m_theme.load(sfmt("%s/%s.INI", m_themeDir.c_str(), themeName.c_str()).c_str());
	// 
	defaultLanguage = "ENGLISH";
	switch (CONF_GetLanguage())
	{
		case CONF_LANG_JAPANESE:
			defaultLanguage = "JAPANESE";
			break;
		case CONF_LANG_GERMAN:
			defaultLanguage = "GERMAN";
			break;
		case CONF_LANG_FRENCH:
			defaultLanguage = "FRENCH";
			break;
		case CONF_LANG_SPANISH:
			defaultLanguage = "SPANISH";
			break;
		case CONF_LANG_ITALIAN:
			defaultLanguage = "ITALIAN";
			break;
		case CONF_LANG_DUTCH:
			defaultLanguage = "DUTCH";
			break;
		case CONF_LANG_SIMP_CHINESE:
			defaultLanguage = "CHINESE S";
			break;
		case CONF_LANG_TRAD_CHINESE:
			defaultLanguage = "CHINESE T";
			break;
		case CONF_LANG_KOREAN:
			defaultLanguage = "KOREAN";
			break;
	}
	if (CONF_GetArea() == CONF_AREA_BRA)
		defaultLanguage = "BRAZILIAN";
	m_curLanguage = m_cfg.getString(" GENERAL", "language", defaultLanguage);
	// 
	m_aa = 3;
	m_cur.init(sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(" GENERAL", "pointer").c_str()).c_str(),
		m_vid.wide(),
		m_theme.getColor(" GENERAL", "pointer_shadow_color", CColor(0x3F000000)),
		m_theme.getFloat(" GENERAL", "pointer_shadow_x", 3.f),
		m_theme.getFloat(" GENERAL", "pointer_shadow_y", 3.f),
		m_theme.getBool(" GENERAL", "pointer_shadow_blur", false));
	m_btnMgr.init();
	_buildMenus();
	_loadCFCfg();
	WPAD_SetVRes(0, m_vid.width() + m_cur.width(), m_vid.height() + m_cur.height());
	m_locked = m_cfg.getString(" GENERAL", "parent_code", "").size() >= 4;
	m_btnMgr.setRumble(m_cfg.getBool(" GENERAL", "rumble", true));
	m_vid.set2DViewport(m_cfg.getInt(" GENERAL", "tv_width", 640), m_cfg.getInt(" GENERAL", "tv_height", 480),
		m_cfg.getInt(" GENERAL", "tv_x", 0), m_cfg.getInt(" GENERAL", "tv_y", 0));
	Sys_ExitToWiiMenu(m_noHBC || m_cfg.getBool(" GENERAL", "exit_to_wii_menu", false));
	LWP_MutexInit(&m_mutex, 0);
	LWP_MutexInit(&m_gameSndMutex, 0);
	soundInit();
	m_cf.setSoundVolume(m_cfg.getInt(" GENERAL", "sound_volume_coverflow", 255));
	m_btnMgr.setSoundVolume(m_cfg.getInt(" GENERAL", "sound_volume_gui", 255));
	m_bnrSndVol = m_cfg.getInt(" GENERAL", "sound_volume_bnr", 255);
	m_alphaSearch = m_cfg.getBool(" GENERAL", "alphabetic_search_on_plus_minus", false);
	if (m_cfg.getBool(" GENERAL", "favorites_on_startup", false))
		m_favorites = m_cfg.getBool(" GENERAL", "favorites", false);
	if (m_cfg.getBool(" GENERAL", "fbi", false))
		m_waitMessage.fromPNG(fbi_png);
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

	m_cf.setCachePath(m_cacheDir.c_str(), !m_cfg.getBool(" GENERAL", "keep_png", true), m_cfg.getBool(" GENERAL", "compress_cache", false));
	m_cf.setBufferSize(m_cfg.getInt(" GENERAL", "cover_buffer", 120));
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
	_loadCFLayout(min(max(1, m_cfg.getInt(" GENERAL", "last_cf_mode", 1)), (int)m_numCFVersions));
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
		_setAA(min(m_theme.getInt(domain, "max_fsaa", 3), m_cfg.getInt(" GENERAL", "max_fsaa", 5)));
	m_cf.setTextureQuality(
		m_theme.getFloat(domain, "tex_lod_bias", -0.3f),
		m_theme.getInt(domain, "tex_aniso", 0),
		m_theme.getBool(domain, "tex_edge_lod", false));
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
	theme.btnFont = _font(theme.fontSet, " GENERAL", "button_font", theme.btnFont);
	theme.btnFontColor = m_theme.getColor(" GENERAL", "button_font_color", 0xD0BFDFFF);
	theme.lblFont.fromBuffer(lblfont_ttf, lblfont_ttf_size, 24, 24);
	theme.lblFont = _font(theme.fontSet, " GENERAL", "label_font", theme.lblFont);
	theme.lblFontColor = m_theme.getColor(" GENERAL", "label_font_color", 0xD0BFDFFF);
	theme.txtFontColor = m_theme.getColor(" GENERAL", "text_font_color", 0xFFFFFFFF);
	theme.titleFont.fromBuffer(titlefont_ttf, titlefont_ttf_size, 48, 48);
	theme.titleFont = _font(theme.fontSet, " GENERAL", "title_font", theme.titleFont);
	theme.titleFontColor = m_theme.getColor(" GENERAL", "title_font_color", 0xD0BFDFFF);
	theme.clickSound.fromWAV(click_wav, click_wav_size);
	theme.clickSound = _sound(theme.soundSet, " GENERAL", "click_sound", theme.clickSound);
	theme.hoverSound.fromWAV(hover_wav, hover_wav_size);
	theme.hoverSound = _sound(theme.soundSet, " GENERAL", "hover_sound", theme.hoverSound);
	// Default textures
	theme.btnTexL.fromPNG(butleft_png);
	theme.btnTexL = _texture(theme.texSet, " GENERAL", "button_texture_left", theme.btnTexL); 
	theme.btnTexR.fromPNG(butright_png);
	theme.btnTexR = _texture(theme.texSet, " GENERAL", "button_texture_right", theme.btnTexR); 
	theme.btnTexC.fromPNG(butcenter_png);
	theme.btnTexC = _texture(theme.texSet, " GENERAL", "button_texture_center", theme.btnTexC); 
	theme.btnTexLS.fromPNG(butsleft_png);
	theme.btnTexLS = _texture(theme.texSet, " GENERAL", "button_texture_left_selected", theme.btnTexLS); 
	theme.btnTexRS.fromPNG(butsright_png);
	theme.btnTexRS = _texture(theme.texSet, " GENERAL", "button_texture_right_selected", theme.btnTexRS); 
	theme.btnTexCS.fromPNG(butscenter_png);
	theme.btnTexCS = _texture(theme.texSet, " GENERAL", "button_texture_center_selected", theme.btnTexCS); 
	theme.pbarTexL.fromPNG(pbarleft_png);
	theme.pbarTexL = _texture(theme.texSet, " GENERAL", "progressbar_texture_left", theme.pbarTexL); 
	theme.pbarTexR.fromPNG(pbarright_png);
	theme.pbarTexR = _texture(theme.texSet, " GENERAL", "progressbar_texture_right", theme.pbarTexR); 
	theme.pbarTexC.fromPNG(pbarcenter_png);
	theme.pbarTexC = _texture(theme.texSet, " GENERAL", "progressbar_texture_center", theme.pbarTexC); 
	theme.pbarTexLS.fromPNG(pbarlefts_png);
	theme.pbarTexLS = _texture(theme.texSet, " GENERAL", "progressbar_texture_left_selected", theme.pbarTexLS); 
	theme.pbarTexRS.fromPNG(pbarrights_png);
	theme.pbarTexRS = _texture(theme.texSet, " GENERAL", "progressbar_texture_right_selected", theme.pbarTexRS); 
	theme.pbarTexCS.fromPNG(pbarcenters_png);
	theme.pbarTexCS = _texture(theme.texSet, " GENERAL", "progressbar_texture_center_selected", theme.pbarTexCS); 
	theme.btnTexPlus.fromPNG(btnplus_png);
	theme.btnTexPlus = _texture(theme.texSet, " GENERAL", "plus_button_texture", theme.btnTexPlus); 
	theme.btnTexPlusS.fromPNG(btnpluss_png);
	theme.btnTexPlusS = _texture(theme.texSet, " GENERAL", "plus_button_texture_selected", theme.btnTexPlusS); 
	theme.btnTexMinus.fromPNG(btnminus_png);
	theme.btnTexMinus = _texture(theme.texSet, " GENERAL", "minus_button_texture", theme.btnTexMinus); 
	theme.btnTexMinusS.fromPNG(btnminuss_png);
	theme.btnTexMinusS = _texture(theme.texSet, " GENERAL", "minus_button_texture_selected", theme.btnTexMinusS); 
	// Default background
	theme.bg.fromPNG(background_png, GX_TF_RGBA8, ALLOC_MEM2);
	m_mainBgLQ.fromPNG(background_png, GX_TF_CMPR, ALLOC_MEM2, 64, 64);
	m_gameBgLQ = m_mainBgLQ;
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
	STexture emptyTex;

	for (u32 i = 0; i < size; ++i)
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

u32 CMenu::_btnRepeat(u32 btn)
{
	u32 b = 0;

	if ((btn & WPAD_BUTTON_LEFT) != 0)
	{
		if (m_padLeftDelay == 0 || m_padLeftDelay > g_repeatDelay)
			b |= WPAD_BUTTON_LEFT;
		++m_padLeftDelay;
	}
	else
		m_padLeftDelay = 0;
	if ((btn & WPAD_BUTTON_RIGHT) != 0)
	{
		if (m_padRightDelay == 0 || m_padRightDelay > g_repeatDelay)
			b |= WPAD_BUTTON_RIGHT;
		++m_padRightDelay;
	}
	else
		m_padRightDelay = 0;
	return b;
}

void CMenu::_initCF(void)
{
	string id;
	bool cmpr;
	Config titles;

	titles.load(sfmt("%s/titles.ini", m_dataDir.c_str()).c_str());	
	m_cf.clear();
	m_cf.reserve(m_gameList.size());
	for (u32 i = 0; i < m_gameList.size(); ++i)
	{
		id = string((const char *)m_gameList[i].id, sizeof m_gameList[0].id);
		if ((!m_favorites || m_cfg.getBool(id, "favorite", false)) && (!m_locked || !m_cfg.getBool(id, "adult_only", false)))
		{
			wstringEx w(titles.getWString("TITLES", id));
			if (w.empty())
				w = titles.getWString("TITLES", id.substr(0, 4), string(m_gameList[i].title, sizeof m_gameList[0].title));
			m_cf.addItem(id.c_str(), w.c_str(), sfmt("%s/%s.png", m_picDir.c_str(), id.c_str()).c_str(), sfmt("%s/%s.png", m_boxPicDir.c_str(), id.c_str()).c_str());
		}
	}
	m_cf.setBoxMode(m_cfg.getBool(" GENERAL", "box_mode", true));
	cmpr = m_cfg.getBool(" GENERAL", "allow_texture_compression", true);
	m_cf.setCompression(cmpr);
	m_cf.setBufferSize(m_cfg.getInt(" GENERAL", "cover_buffer", 120));
	if (m_cf.start())
		if (m_curGameId.empty() || !m_cf.findId(m_curGameId.c_str(), true))
			m_cf.findId(m_cfg.getString(" GENERAL", "current_game").c_str(), true);
}

void CMenu::_mainLoopCommon(const WPADData *wd, bool withCF, bool blockReboot, bool adjusting)
{
	if (withCF)
		m_cf.tick();
	m_btnMgr.tick();
	_updateBg();
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
		if (withCF)
		{
			m_cf.draw();
			m_vid.setup2DProjection();
			m_cf.drawEffect();
			m_cf.drawText(adjusting);
		}
	}
	m_vid.setup2DProjection();
	m_btnMgr.draw();
	if (wd->ir.valid)
		m_cur.draw(wd->ir.x, wd->ir.y, wd->ir.angle);
	m_vid.render();
	if (!blockReboot)
	{
		if (withCF && Sys_Exiting())
			m_cf.clear();
		if (Sys_Exiting())
			m_cfg.save();
		Sys_Test();
	}
	LWP_MutexLock(m_gameSndMutex);
	if (!!m_gameSoundTmp.data)
	{
		m_gameSound.stop();
		m_gameSound = m_gameSoundTmp;
		m_gameSoundTmp.data.release();
		m_gameSound.play(m_bnrSndVol);
	}
	LWP_MutexUnlock(m_gameSndMutex);
	if (withCF && m_gameSoundThread == 0)
		m_cf.startPicLoader();
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
}

const wstringEx CMenu::_fmt(const char *key, const wchar_t *def)
{
	wstringEx ws = m_loc.getWString(m_curLanguage, key, def);
	if (checkFmt(def, ws))
		return ws;
	return def;
}

bool CMenu::_loadGameList(void)
{
	s32 ret;
	u32 len;
	SmartBuf buffer;
	u32 count;

	ret = WBFS_OpenNamed((char *) m_cfg.getString(" GENERAL", "partition", "WBFS1").c_str());
	if (ret < 0)
	{
		error(wfmt(_fmt("wbfs2", L"WBFS_Open failed : %i"), ret));
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

	file = fopen("sd:/wiiflow/hdrdump", "rb");
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
	ret = WBFS_GetHeaders((discHdr *)buffer.get(), count, sizeof (struct discHdr));
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
	discHdr *b = (discHdr *)buffer.get();
	for (u32 i = 0; i < count; ++i)
		if (memcmp(b[i].id, "__CFG_", sizeof b[i].id) != 0)	// Because of uLoader
			m_gameList.push_back(b[i]);
	return true;
}

static void listOGG(const char *path, vector<string> &oggFiles)
{
	DIR *d;
	struct dirent *dir;
	string fileName;

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
			if (fileName.size() > 4 && fileName.substr(fileName.size() - 4, 4) == ".OGG")
				oggFiles.push_back(fileName);
			dir = readdir(d);
		}
		closedir(d);
	}
}

void CMenu::_startMusic(void)
{
	vector<string> v;
	u32 fileSize;
	SmartBuf buffer;

	_stopMusic();
	listOGG(m_musicDir.c_str(), v);
	if (v.empty())
		return;
	srand(time(NULL));
	ifstream file(sfmt("%s/%s", m_musicDir.c_str(), v[rand() % v.size()].c_str()).c_str(), ios::in | ios::binary);
	if (!file.is_open())
		return;
	file.seekg(0, ios::end);
	fileSize = file.tellg();
	file.seekg(0, ios::beg);
	buffer = smartMem2Alloc(fileSize);
	if (!buffer)
		return;
	file.read((char *)buffer.get(), fileSize);
	if (file.fail())
		return;
	file.close();
	m_music = buffer;
	PlayOgg(mem_open((char *)m_music.get(), fileSize), 0, OGG_INFINITE_TIME);
	SetVolumeOgg(m_cfg.getInt(" GENERAL", "sound_volume_music", 255));
}

void CMenu::_stopMusic(void)
{
	StopOgg();
	m_music.release();
}

void CMenu::_pauseMusic(void)
{
	PauseOgg(1);
}

void CMenu::_resumeMusic(void)
{
	PauseOgg(0);
}

void CMenu::_stopSounds(void)
{
	_stopMusic();
	m_btnMgr.stopSounds();
	m_cf.stopSound();
}
