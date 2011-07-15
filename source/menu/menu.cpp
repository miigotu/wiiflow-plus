#include "menu.hpp"
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "loader/alt_ios.h"

#include "network/gcard.h"

#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <wchar.h>

#include "gecko.h"
#include "defines.h"
#include "fonts.h"
#include "music/SoundHandler.hpp"
#include "fs.h"
#include "U8Archive.h"

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
SmartBuf base_font;
u32 base_font_size = 0;

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
	m_thrdNetwork = false;
	m_mutex = 0;
	m_showtimer = 0;
	m_gameSoundThread = LWP_THREAD_NULL;
	m_gameSoundHdr = NULL;
	m_numCFVersions = 0;
	m_bgCrossFade = 0;
	m_bnrSndVol = 0;
	m_gameSettingsPage = 0;
	m_directLaunch = false;
	m_exit = false;
	m_initialCoverStatusComplete = false;
	m_reload = false;
	bootHB = false;
	m_gamesound_changed = false;
}

extern "C" { int makedir(char *newdir); }

void CMenu::init()
{
	const char *drive = "empty";
	const char *check = "empty";
	struct stat dummy;

	for(int i = SD; i <= USB8; i++) //Find the first partition with a wiiflow.ini
		if (DeviceHandler::Instance()->IsInserted(i) && DeviceHandler::Instance()->GetFSType(i) != PART_FS_WBFS && stat(sfmt("%s:/%s/" CFG_FILENAME, DeviceName[i], APPDATA_DIR2).c_str(), &dummy) == 0)
		{
			drive = DeviceName[i];
			break;
		}

	if(drive == check) //No wiiflow.ini found
		for(int i = SD; i <= USB8; i++) //Find the first partition with a boot.dol
			if (DeviceHandler::Instance()->IsInserted(i) && DeviceHandler::Instance()->GetFSType(i) != PART_FS_WBFS && stat(sfmt("%s:/%s/boot.dol", DeviceName[i], APPDATA_DIR2).c_str(), &dummy) == 0)
			{
				drive = DeviceName[i];
				break;
			}
			
	if(drive == check) //No boot.dol found
		for(int i = SD; i <= USB8; i++) //Find the first partition with apps/wiiflow folder
			if (DeviceHandler::Instance()->IsInserted(i) && DeviceHandler::Instance()->GetFSType(i) != PART_FS_WBFS && stat(sfmt("%s:/%s", DeviceName[i], APPDATA_DIR2).c_str(), &dummy) == 0)
			{
				drive = DeviceName[i];
				break;
			}

	if(drive == check) //No apps/wiiflow folder found
		for(int i = SD; i <= USB8; i++) // Find the first writable partition
			if (DeviceHandler::Instance()->IsInserted(i) && DeviceHandler::Instance()->GetFSType(i) != PART_FS_WBFS)
			{
				drive = DeviceName[i];
				makedir((char *)sfmt("%s:/%s", DeviceName[i], APPDATA_DIR2).c_str()); //Make the apps dir, so saving wiiflow.ini does not fail.
				break;
			}

	_loadDefaultFont(CONF_GetLanguage() == CONF_LANG_KOREAN);

	if(drive == check) // Should not happen
	{
		_buildMenus();
		error(L"No available partitions found!");
		m_exit = true;
		return;
	}

	m_appDir = sfmt("%s:/%s", drive, APPDATA_DIR2);
	gprintf("Wiiflow boot.dol Location: %s\n", m_appDir.c_str());

	m_cfg.load(sfmt("%s/" CFG_FILENAME, m_appDir.c_str()).c_str());

	bool onUSB = m_cfg.getBool("GENERAL", "data_on_usb", strncmp(drive, "usb", 3) == 0);
	
	drive = check; //reset the drive variable for the check

	if (onUSB)
	{
		for(int i = USB1; i <= USB8; i++) //Look for first partition with a wiiflow folder in root
			if (DeviceHandler::Instance()->IsInserted(i) && DeviceHandler::Instance()->GetFSType(i) != PART_FS_WBFS && stat(sfmt("%s:/%s", DeviceName[i], APPDATA_DIR).c_str(), &dummy) == 0)
			{
				drive = DeviceName[i];
				break;
			}
	}
	else if(DeviceHandler::Instance()->IsInserted(SD)) drive = DeviceName[SD];

	if(drive == check && onUSB) //No wiiflow folder found in root of any usb partition, and data_on_usb=yes
		for(int i = USB1; i <= USB8; i++) // Try first USB partition with wbfs folder.
			if (DeviceHandler::Instance()->IsInserted(i) && DeviceHandler::Instance()->GetFSType(i) != PART_FS_WBFS && stat(sfmt(GAMES_DIR, DeviceName[i]).c_str(), &dummy) == 0)
			{
				drive = DeviceName[i];
				break;
			}

	if(drive == check && onUSB) // No wbfs folder found and data_on_usb=yes
		for(int i = USB1; i <= USB8; i++) // Try first available USB partition.
			if (DeviceHandler::Instance()->IsInserted(i) && DeviceHandler::Instance()->GetFSType(i) != PART_FS_WBFS)
			{
				drive = DeviceName[i];
				break;
			}

	if(drive == check)
	{	
		_buildMenus();
		if(DeviceHandler::Instance()->IsInserted(SD))
		{
			error(L"data_on_usb=yes and No available usb partitions for data!\nUsing SD.");
			drive = DeviceName[SD];
		}
		else
		{
			error(L"No available usb partitions for data and no SD inserted!\nExitting.");
			m_exit = true;
			return;
		}
	}

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
	m_cheatDir = m_cfg.getString("GENERAL", "dir_cheat", sfmt("%s/gct", m_txtCheatDir.c_str()));
	m_wipDir = m_cfg.getString("GENERAL", "dir_wip", sfmt("%s/wip", m_txtCheatDir.c_str()));
	m_listCacheDir = m_cfg.getString("GENERAL", "dir_list_cache", sfmt("%s/lists", m_cacheDir.c_str()));
	//

	DeviceHandler::SetWatchdog(m_cfg.getUInt("GENERAL", "watchdog_timeout", 10));

	u8 partition = m_cfg.getInt("GENERAL", "partition");  //Auto find a valid partition and fix old ini partition settings.
	if(partition < USB1 || partition > USB8 || partition > DeviceHandler::Instance()->GetMountedCount(partition))
	{
		m_cfg.remove("GENERAL", "partition");
		for(int i = USB1; i <= USB8; i++) // Find a usb partition with the wbfs folder or wbfs file system, else leave it blank (defaults to 1 later)
		if (DeviceHandler::Instance()->IsInserted(i) && (DeviceHandler::Instance()->GetFSType(i) == PART_FS_WBFS || stat(sfmt(GAMES_DIR, DeviceName[i]).c_str(), &dummy) == 0))
		{
			m_cfg.setInt("GENERAL", "partition", i);
			break;
		}
	}

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
	makedir((char *)m_cheatDir.c_str());
	makedir((char *)m_wipDir.c_str());

	makedir((char *)m_listCacheDir.c_str());
	m_gameList.Init(m_listCacheDir, m_settingsDir, m_curLanguage);

	// INI files
	m_cat.load(sfmt("%s/" CAT_FILENAME, m_settingsDir.c_str()).c_str());
	string themeName = m_cfg.getString("GENERAL", "theme", "DEFAULT");
	m_themeDataDir = sfmt("%s/%s", m_themeDir.c_str(), themeName.c_str());
	m_theme.load(sfmt("%s.ini", m_themeDataDir.c_str()).c_str());

	u8 defaultMenuLanguage = 7; //English
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

	m_curLanguage = CMenu::_translations[m_cfg.getInt("GENERAL", "language", defaultMenuLanguage)];
	if (!m_loc.load(sfmt("%s/%s.ini", m_languagesDir.c_str(), m_curLanguage.c_str()).c_str()))
	{
		m_cfg.setInt("GENERAL", "language", 0);
		m_curLanguage = CMenu::_translations[0];
		m_loc.load(sfmt("%s/%s.ini", m_languagesDir.c_str(), m_curLanguage.c_str()).c_str());
	}

	m_aa = 3;

	CColor pShadowColor = m_theme.getColor("GENERAL", "pointer_shadow_color", CColor(0x3F000000));
	float pShadowX = m_theme.getFloat("GENERAL", "pointer_shadow_x", 3.f);
	float pShadowY = m_theme.getFloat("GENERAL", "pointer_shadow_y", 3.f);
	bool pShadowBlur = m_theme.getBool("GENERAL", "pointer_shadow_blur", false);

	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		m_cursor[chan].init(sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString("GENERAL", sfmt("pointer%i", chan+1).c_str()).c_str()).c_str(),
			m_vid.wide(), pShadowColor, pShadowX, pShadowY, pShadowBlur, chan);
		WPAD_SetVRes(chan, m_vid.width() + m_cursor[chan].width(), m_vid.height() + m_cursor[chan].height());
	}
		
	m_btnMgr.init(m_vid);
	MusicPlayer::Instance()->Init(m_cfg, m_musicDir, sfmt("%s/music", m_themeDataDir.c_str()));

	_load_installed_cioses();	

	_buildMenus();
	_loadCFCfg();

	m_locked = m_cfg.getString("GENERAL", "parent_code", "").size() >= 4;
	m_btnMgr.setRumble(m_cfg.getBool("GENERAL", "rumble", true));

	int exit_to = m_cfg.getInt("GENERAL", "exit_to", 0);
	m_disable_exit = exit_to == EXIT_TO_DISABLE;

	if(exit_to == EXIT_TO_BOOTMII && (!DeviceHandler::Instance()->IsInserted(SD) || 
	stat(sfmt("%s:/bootmii/armboot.bin",DeviceName[SD]).c_str(), &dummy) != 0 || 
	stat(sfmt("%s:/bootmii/ppcboot.elf", DeviceName[SD]).c_str(), &dummy) != 0))
		exit_to = EXIT_TO_HBC;
	Sys_ExitTo(exit_to);

	LWP_MutexInit(&m_mutex, 0);

	m_cf.setSoundVolume(m_cfg.getInt("GENERAL", "sound_volume_coverflow", 255));
	m_btnMgr.setSoundVolume(m_cfg.getInt("GENERAL", "sound_volume_gui", 255));
	m_bnrSndVol = m_cfg.getInt("GENERAL", "sound_volume_bnr", 255);
	
	if (m_cfg.getBool("GENERAL", "favorites_on_startup", false))
		m_favorites = m_cfg.getBool("GENERAL", "favorites", false);
	m_category = m_cat.getInt("GENERAL", "category", 0);
	m_max_categories = m_cat.getInt("GENERAL", "numcategories", 12);

	m_current_view = COVERFLOW_USB;

	safe_vector<string> gamercards = stringToVector(m_cfg.getString("GAMERCARD", "gamercards"), '|');
	if (gamercards.size() == 0 && m_cfg.getBool("GAMERCARD", "wiinnertag_enable", false))
	{
		gamercards.push_back("wiinnertag");
		m_cfg.setString("GAMERCARD", "gamercards", "wiinnertag");
		m_cfg.remove("GAMERCARD", "wiinnertag_enable");
	}

	for (safe_vector<string>::iterator itr = gamercards.begin(); itr != gamercards.end(); itr++)
	{
		register_card_provider(
			m_cfg.getString("GAMERCARD", sfmt("%s_url", (*itr).c_str())).c_str(),
			m_cfg.getString("GAMERCARD", sfmt("%s_key", (*itr).c_str())).c_str()
		);
	}
}

void CMenu::cleanup(bool ios_reload)
{
	m_cf.stopCoverLoader();

	CheckGameSoundThread(true);

	_stopSounds();
	
	if (!ios_reload)
	{
		SMART_FREE(m_cameraSound);
	}
	
	MusicPlayer::DestroyInstance();
	SoundHandler::DestroyInstance();
	soundDeinit();
	if (!ios_reload)
	{
		LWP_MutexDestroy(m_mutex);
		m_mutex = 0;
	}

	DeviceHandler::DestroyInstance(); // Destruction must be done manually, also unmounts all devices.

	if (!ios_reload)
	{
		_cleanupDefaultFont();
	}
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
	const char *domain = "_COVERFLOW";

	gprintf("Preparing to load sound from %s\n", sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "sound_flip").c_str()).c_str());
	m_cf.setCachePath(m_cacheDir.c_str(), !m_cfg.getBool("GENERAL", "keep_png", true), m_cfg.getBool("GENERAL", "compress_cache", false));
	m_cf.setBufferSize(m_cfg.getInt("GENERAL", "cover_buffer", 120));
	// Coverflow Sounds
	m_cf.setSounds(
		SmartPtr<GuiSound>(new GuiSound(sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "sound_flip").c_str()))),
		SmartPtr<GuiSound>(new GuiSound(sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "sound_hover").c_str()))),
		SmartPtr<GuiSound>(new GuiSound(sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "sound_select").c_str()))),
		SmartPtr<GuiSound>(new GuiSound(sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "sound_cancel").c_str())))
	);
	// Textures
	string texLoading = sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "loading_cover_box").c_str());
	string texNoCover = sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "missing_cover_box").c_str());
	string texLoadingFlat = sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "loading_cover_flat").c_str());
	string texNoCoverFlat = sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "missing_cover_flat").c_str());
	m_cf.setTextures(texLoading, texLoadingFlat, texNoCover, texNoCoverFlat);
	// Font
	FontSet dummy;
	SFont font = _font(dummy, domain, "font",
		(u32)m_theme.getInt(domain, "font_size", TITLEFONT_PT_SZ),
		(u32)m_theme.getInt(domain, "font_line_height", TITLEFONT_PT_SZ),
		FONT_BOLD,
		1
	);
	m_cf.setFont(font, m_theme.getColor(domain, "font_color", CColor(0xFFFFFFFF)));
	// 
	m_numCFVersions = min(max(2, m_theme.getInt("_COVERFLOW", "number_of_modes", 2)), 8);
	for (u32 i = 1; i <= m_numCFVersions; ++i)
		_loadCFLayout(i);

	int mode;
	switch(m_current_view)
	{
		case COVERFLOW_HOMEBREW:
			mode = m_cfg.getInt("GENERAL", "last_hb_cf_mode" , 1);
			break;
		case COVERFLOW_CHANNEL:
			mode = m_cfg.getInt("GENERAL", "last_chan_cf_mode" , 1);
			break;
		case COVERFLOW_USB:
		default:
			mode = m_cfg.getInt("GENERAL", "last_cf_mode", 1);
			break;
	}
	_loadCFLayout(min(max(1, mode), (int)m_numCFVersions));
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
		_setAA(min(m_theme.getInt(domain, "max_fsaa", 3), m_cfg.getInt("GENERAL", "max_fsaa", 3)));

	m_cf.setTextureQuality(m_theme.getFloat(domain, "tex_lod_bias", -3.f),
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

	if(!base_font.get()) _loadDefaultFont(CONF_GetLanguage() == CONF_LANG_KOREAN);
	
	// Default fonts
	theme.btnFont = _font(theme.fontSet, "GENERAL", "button_font", BUTTONFONT);
	theme.btnFontColor = m_theme.getColor("GENERAL", "button_font_color", 0xD0BFDFFF);

	theme.lblFont = _font(theme.fontSet, "GENERAL", "label_font", LABELFONT);
	theme.lblFontColor = m_theme.getColor("GENERAL", "label_font_color", 0xD0BFDFFF);

	theme.titleFont = _font(theme.fontSet, "GENERAL", "title_font", TITLEFONT);
	theme.titleFontColor = m_theme.getColor("GENERAL", "title_font_color", 0xD0BFDFFF);

	theme.thxFont = _font(theme.fontSet, "GENERAL", "thxfont", THANKSFONT);
	theme.txtFontColor = m_theme.getColor("GENERAL", "text_font_color", 0xFFFFFFFF);
	
	// Default Sounds
	theme.clickSound = SmartPtr<GuiSound>(new GuiSound(click_wav, click_wav_size, false));
	theme.clickSound = _sound(theme.soundSet, "GENERAL", "click_sound", theme.clickSound);
	theme.hoverSound = SmartPtr<GuiSound>(new GuiSound(hover_wav, hover_wav_size, false));
	theme.hoverSound = _sound(theme.soundSet, "GENERAL", "hover_sound", theme.hoverSound);
	theme.cameraSound = SmartPtr<GuiSound>(new GuiSound(camera_wav, camera_wav_size, false));
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
	
	// Build menus
	_initMainMenu(theme);
	_initErrorMenu(theme);
	_initConfigAdvMenu(theme);
	_initConfigSndMenu(theme);
	_initConfig7Menu(theme);
	_initConfig4Menu(theme);
	_initConfigScreenMenu(theme);
	_initConfig3Menu(theme);
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

SFont CMenu::_font(CMenu::FontSet &fontSet, const char *domain, const char *key, u32 fontSize, u32 lineSpacing, u32 weight, u32 index)
{
	SFont retFont;

	if(!base_font.get()) _loadDefaultFont(CONF_GetLanguage() == CONF_LANG_KOREAN);

	bool useDefault = false;
	string filename = m_theme.getString(domain, key);
	if(filename.empty())
	{
		useDefault = true;
		filename = key;
	}

	for (u32 c = 0; c < filename.size(); ++c)
		if (filename[c] >= 'A' && filename[c] <= 'Z')
			filename[c] |= 0x20;

	string fontSizeKey = key;
	fontSizeKey += "_size";

	u32 def_fontSize = fontSize;
	fontSize = (u32)m_theme.getInt(domain, fontSizeKey);
	fontSize = fontSize <= 0 ? def_fontSize : fontSize;
	fontSize = min(max(6u, fontSize), 300u);

	string lineSpacingKey = key;
	lineSpacingKey += "_line_height";

	u32 def_lineSpacing = lineSpacing;
	lineSpacing = (u32)m_theme.getInt(domain, lineSpacingKey);
	lineSpacing = lineSpacing <= 0 ? def_lineSpacing : lineSpacing;
	lineSpacing = min(max(6u, lineSpacing), 300u);

	string weightKey = key;
	weightKey += "_weight";

	u32 def_weight = weight;
	weight = (u32)m_theme.getInt(domain, weightKey);
	weight = weight <= 0 ? def_weight : weight;
	weight = max(1u,min(weight, 32u));

	// Try to find the same font with the same size
	CMenu::FontSet::iterator i = fontSet.find(CMenu::FontDesc(filename, fontSize));
	if (i != fontSet.end())
	{
		retFont = i->second;
		retFont.lineSpacing = lineSpacing;
		retFont.setWeight(weight);
		return retFont;
	}
	// Then try to find the same font with a different size
	i = fontSet.lower_bound(CMenu::FontDesc(filename, 0));
	if (i != fontSet.end() && i->first.first == filename)
	{
		retFont = i->second;
		retFont.newSize(fontSize, lineSpacing, weight);
		fontSet[CMenu::FontDesc(filename, fontSize)] = retFont;
		return retFont;
	}
	// TTF not found in memory, load it to create a new font
	if (!useDefault && retFont.fromFile(sfmt("%s/%s", m_themeDataDir.c_str(), filename.c_str()).c_str(), fontSize, lineSpacing, weight, index))
	{
		// Theme Font
		fontSet[CMenu::FontDesc(filename, fontSize)] = retFont;
		return retFont;
	}
	if(retFont.fromBuffer(base_font.get(), base_font_size, fontSize, lineSpacing, weight, index))
	{
		// Default font
		fontSet[CMenu::FontDesc(filename, fontSize)] = retFont;
		return retFont;
	}
	return retFont;
}

safe_vector<STexture> CMenu::_textures(TexSet &texSet, const char *domain, const char *key)
{
	safe_vector<STexture> textures;

	if (m_theme.loaded())
	{
		safe_vector<string> filenames = m_theme.getStrings(domain, key);
		if (filenames.size() > 0)
		{
			for (safe_vector<string>::iterator itr = filenames.begin(); itr != filenames.end(); itr++)
			{
				string filename = *itr;

				CMenu::TexSet::iterator i = texSet.find(filename);
				if (i != texSet.end())
				{
					textures.push_back(i->second);
				}
				STexture tex;
				if (STexture::TE_OK == tex.fromPNGFile(sfmt("%s/%s", m_themeDataDir.c_str(), filename.c_str()).c_str(), GX_TF_RGBA8, ALLOC_MEM2))
				{
					texSet[filename] = tex;
					textures.push_back(tex);
				}
			}
		}
	}
	return textures;
}

STexture CMenu::_texture(CMenu::TexSet &texSet, const char *domain, const char *key, STexture def)
{
	string filename;

	if (m_theme.loaded())
	{
		filename = m_theme.getString(domain, key);
		if (!filename.empty())
		{
			CMenu::TexSet::iterator i = texSet.find(filename);
			if (i != texSet.end())
				return i->second;

			STexture tex;
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

SmartPtr<GuiSound> CMenu::_sound(CMenu::SoundSet &soundSet, const char *domain, const char *key, SmartPtr<GuiSound> def)
{
	string filename = m_theme.getString(domain, key);
	if (filename.empty()) return def;

	for (u32 c = 0; c < filename.size(); ++c)
		if (filename[c] >= 'A' && filename[c] <= 'Z')
			filename[c] |= 0x20;

	CMenu::SoundSet::iterator i = soundSet.find(filename);
	if (i == soundSet.end())
	{
		SmartPtr<GuiSound> sound = SmartPtr<GuiSound>(new GuiSound(sfmt("%s/%s", m_themeDataDir.c_str(), filename.c_str()).c_str()));
		soundSet[filename] = sound;
		return sound;
	}
	return i->second;
}

u16 CMenu::_textStyle(const char *domain, const char *key, u16 def)
{
	u16 textStyle = 0;
	string style(m_theme.getString(domain, key));
	if (style.empty()) return def;

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

	font = _font(theme.fontSet, domain, "font", BUTTONFONT);

	SmartPtr<GuiSound> clickSound = _sound(theme.soundSet, domain, "click_sound", theme.clickSound);
	SmartPtr<GuiSound> hoverSound = _sound(theme.soundSet, domain, "hover_sound", theme.hoverSound);
	
	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
		x = m_vid.width() - x - width;
	if (btnPos & FTGX_ALIGN_BOTTOM)
		y = m_vid.height() - y - height;
	
	return m_btnMgr.addButton(font, text, x, y, width, height, c, btnTexSet, clickSound, hoverSound);
}

u32 CMenu::_addPicButton(CMenu::SThemeData &theme, const char *domain, STexture &texNormal, STexture &texSelected, int x, int y, u32 width, u32 height)
{
	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	STexture tex1 = _texture(theme.texSet, domain, "texture_normal", texNormal);
	STexture tex2 = _texture(theme.texSet, domain, "texture_selected", texSelected);
	SmartPtr<GuiSound> clickSound = _sound(theme.soundSet, domain, "click_sound", theme.clickSound);
	SmartPtr<GuiSound> hoverSound = _sound(theme.soundSet, domain, "hover_sound", theme.hoverSound);

	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
		x = m_vid.width() - x - width;
	if (btnPos & FTGX_ALIGN_BOTTOM)
		y = m_vid.height() - y - height;

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
	font = _font(theme.fontSet, domain, "font", LABELFONT);
	style = _textStyle(domain, "style", style);

	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
		x = m_vid.width() - x - width;
	if (btnPos & FTGX_ALIGN_BOTTOM)
		y = m_vid.height() - y - height;

	return m_btnMgr.addLabel(font, text, x, y, width, height, c, style);
}

u32 CMenu::_addLabel(CMenu::SThemeData &theme, const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, u16 style, STexture &bg)
{
	CColor c(color);

	c = m_theme.getColor(domain, "color", c);
	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	font = _font(theme.fontSet, domain, "font", LABELFONT);
	STexture texBg = _texture(theme.texSet, domain, "background_texture", bg);
	style = _textStyle(domain, "style", style);

	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
		x = m_vid.width() - x - width;
	if (btnPos & FTGX_ALIGN_BOTTOM)
		y = m_vid.height() - y - height;

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

	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
		x = m_vid.width() - x - width;
	if (btnPos & FTGX_ALIGN_BOTTOM)
		y = m_vid.height() - y - height;

	return m_btnMgr.addProgressBar(x, y, width, height, btnTexSet);
}

void CMenu::_setHideAnim(u32 id, const char *domain, int dx, int dy, float scaleX, float scaleY)
{
	dx = m_theme.getInt(domain, "effect_x", dx);
	dy = m_theme.getInt(domain, "effect_y", dy);
	scaleX = m_theme.getFloat(domain, "effect_scale_x", scaleX);
	scaleY = m_theme.getFloat(domain, "effect_scale_y", scaleY);

	int x, y;
	u32 width, height;
	m_btnMgr.getDimensions(id, x, y, width, height);

	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
	{
		dx = m_vid.width() - dx - width;
		scaleX = m_vid.width() - scaleX - width;
	}
	if (btnPos & FTGX_ALIGN_BOTTOM)
	{
		dy = m_vid.height() - dy - height;
		scaleY = m_vid.height() - scaleY - height;
	}

	m_btnMgr.hide(id, dx, dy, scaleX, scaleY, true);
}

void CMenu::_addUserLabels(CMenu::SThemeData &theme, u32 *ids, u32 size, const char *domain)
{
	_addUserLabels(theme, ids, 0, size, domain);
}

void CMenu::_addUserLabels(CMenu::SThemeData &theme, u32 *ids, u32 start, u32 size, const char *domain)
{

	for (u32 i = start; i < start + size; ++i)
	{
		string dom(sfmt("%s/USER%i", domain, i + 1));
		if (m_theme.hasDomain(dom))
		{
			STexture emptyTex;
			ids[i] = _addLabel(theme, dom.c_str(), theme.lblFont, L"", 40, 200, 64, 64, CColor(0xFFFFFFFF), 0, emptyTex);
			_setHideAnim(ids[i], dom.c_str(), -50, 0, 0.f, 0.f);
		}
		else
			ids[i] = -1u;
	}
}

void CMenu::_initCF(void)
{
	Config m_dump;

	m_cf.clear();
	m_cf.reserve(m_gameList.size());

 	m_gamelistdump = m_cfg.getBool("GENERAL", m_current_view == COVERFLOW_USB ? "dump_gamelist" : "dump_chanlist", true);
	if(m_gamelistdump) m_dump.load(sfmt("%s/titlesdump.ini", m_settingsDir.c_str()).c_str());

	m_gcfg1.load(sfmt("%s/gameconfig1.ini", m_settingsDir.c_str()).c_str());
	for (u32 i = 0; i < m_gameList.size(); ++i)
	{
		u64 chantitle = m_gameList[i].hdr.chantitle;
		if (m_current_view == COVERFLOW_CHANNEL && chantitle == HBC_108)
			strncpy((char *) m_gameList[i].hdr.id, "JODI", 6);

		string id = string((const char *)m_gameList[i].hdr.id, m_gameList[i].hdr.id[5] == 0 ? strlen((const char *) m_gameList[i].hdr.id) : sizeof m_gameList[0].hdr.id);
		
		if ((!m_favorites || m_gcfg1.getBool("FAVORITES", id, false)) && (!m_locked || !m_gcfg1.getBool("ADULTONLY", id, false)) && !m_gcfg1.getBool("HIDDEN", id, false))
		{
			if (m_category != 0)
			{
				const char *categories = m_cat.getString("CATEGORIES", id, "").c_str();
				if (strlen(categories) != 12 || categories[m_category] == '0') 
				{ // 12 categories max!
					continue;
				}
			}

			wstringEx w = wstringEx(m_gameList[i].hdr.title);

			int playcount = m_gcfg1.getInt("PLAYCOUNT", id, 0);
			unsigned int lastPlayed = m_gcfg1.getUInt("LASTPLAYED", id, 0);

			if(m_gamelistdump)
				m_dump.setWString(m_current_view == COVERFLOW_CHANNEL ? "CHANNELS" : "GAMES", m_current_view == COVERFLOW_CHANNEL ? id.substr(0, 4) : id, w);

			m_cf.addItem(&m_gameList[i], w.c_str(), chantitle, sfmt("%s/%s.png", m_picDir.c_str(), id.c_str()).c_str(), sfmt("%s/%s.png", m_boxPicDir.c_str(), id.c_str()).c_str(), m_gameList[i].hdr.casecolor, playcount, lastPlayed);
		}
	}
	m_gcfg1.unload();
 	if (m_gamelistdump)
	{
		m_dump.save();
		m_dump.unload();
		m_cfg.setBool("GENERAL", m_current_view == COVERFLOW_CHANNEL ? "dump_chanlist" : "dump_gamelist", false);
	}
 	m_cf.setBoxMode(m_cfg.getBool("GENERAL", "box_mode", true));
	m_cf.setCompression(m_cfg.getBool("GENERAL", "allow_texture_compression", true));
	m_cf.setBufferSize(m_cfg.getInt("GENERAL", "cover_buffer", 120));
	m_cf.setSorting((Sorting)m_cfg.getInt("GENERAL", "sort", 0));
	if (m_curGameId.empty() || !m_cf.findId(m_curGameId.c_str(), true))
		m_cf.findId(m_cfg.getString("GENERAL", m_current_view == COVERFLOW_CHANNEL ? "current_channel" : "current_game").c_str(), true);
	m_cf.startCoverLoader();
}

void CMenu::_mainLoopCommon(bool withCF, bool blockReboot, bool adjusting)
{
	if (withCF) m_cf.tick();
	m_btnMgr.tick();
	m_fa.tick();
	m_cf.setFanartPlaying(m_fa.isLoaded());
	m_cf.setFanartTextColor(m_fa.getTextColor(m_theme.getColor("_COVERFLOW", "font_color", CColor(0xFFFFFFFF))));

	_updateBg();

	m_fa.hideCover() ? 	m_cf.hideCover() : m_cf.showCover();

	if (withCF) m_cf.makeEffectTexture(m_vid, m_lqBg);
	if (withCF && m_aa > 0)
	{
		m_vid.setAA(m_aa, true);
		for (int i = 0; i < m_aa; ++i)
		{
			m_vid.prepareAAPass(i);
			m_vid.setup2DProjection(false, true);
			_drawBg();
			m_fa.draw(false);
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
		m_fa.draw(false);
		if (withCF)
		{
			m_cf.draw();
			m_vid.setup2DProjection();
			m_cf.drawEffect();
			m_cf.drawText(adjusting);
		}
	}

	m_fa.draw();
	
	m_btnMgr.draw();
	ScanInput();

	m_vid.setup2DProjection();
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

	if (withCF && m_gameSelected && m_gamesound_changed && (m_gameSoundHdr == NULL) && !m_gameSound.IsPlaying() && MusicPlayer::Instance()->GetVolume() == 0)
	{
		m_gameSound.Play(m_bnrSndVol);
		m_gamesound_changed = false;
	}
	else if (!m_gameSelected)
		m_gameSound.Stop();

	CheckThreads();

	if (withCF && m_gameSoundThread == LWP_THREAD_NULL)
		m_cf.startCoverLoader();

	MusicPlayer::Instance()->Tick(m_video_playing || (m_gameSelected && 
		m_gameSound.IsLoaded()) ||  m_gameSound.IsPlaying());
	
	//Take Screenshot
	if (gc_btnsPressed & PAD_TRIGGER_Z)
	{
		time_t rawtime;
		struct tm * timeinfo;
		char buffer[80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(buffer,80,"%b-%d-20%y-%Hh%Mm%Ss.png",timeinfo);
		gprintf("Screenshot taken and saved to: %s/%s\n", m_screenshotDir.c_str(), buffer);
		m_vid.TakeScreenshot(sfmt("%s/%s", m_screenshotDir.c_str(), buffer).c_str());
		if (!!m_cameraSound)
			m_cameraSound->Play(255);
	}
	#ifdef SHOWMEM
	m_btnMgr.setText(m_mem2FreeSize, wfmt(L"Mem2 Free: %u, Mem1 Free: %u", MEM2_freesize(), SYS_GetArena1Size()), true);
	#endif
}

void CMenu::_setBg(const STexture &tex, const STexture &lqTex)
{
	m_lqBg = lqTex;
	if (tex.data.get() == m_nextBg.data.get()) return;
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

	if (m_bgCrossFade == 0) return;
	m_bgCrossFade = max(0, (int)m_bgCrossFade - 14);
	if (m_bgCrossFade == 0)
	{
		m_curBg = m_nextBg;
		return;
	}
	if (m_curBg.data.get() == m_prevBg.data.get())
		SMART_FREE(m_curBg.data);
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
	_textConfig3();
	_textConfigScreen();
	_textConfig4();
	_textConfig7();
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
	if (checkFmt(def, ws)) return ws;
	return def;
}

bool CMenu::_loadChannelList(void)
{
	string langCode = m_loc.getString(m_curLanguage, "wiitdb_code", "EN");
	
	m_channels.Init(0, langCode);
	u32 count = m_channels.Count();
	u32 len = count * sizeof m_gameList[0];

	SmartBuf buffer = smartAnyAlloc(len);
	if (!buffer) return false;

	memset(buffer.get(), 0, len);

	m_gameList.reserve(count);

	dir_discHdr *b = (dir_discHdr *)buffer.get();
	for (u32 i = 0; i < count; ++i)
	{
		Channel *chan = m_channels.GetChannel(i);
		
		if (chan->id == NULL) continue; // Skip invalid channels

		memcpy(&b[i].hdr.id, chan->id, 4);
		wcstombs(b[i].hdr.title, chan->name, sizeof(b->hdr.title));
		b[i].hdr.chantitle = chan->title;
	
		m_gameList.push_back(b[i]);
	}
	return true;
}

bool CMenu::_loadList(void)
{
	m_cf.clear();
	m_gameList.clear();

	gprintf("Loading items of view %d\n", m_current_view);

	bool retval;
	switch(m_current_view)
	{
		case COVERFLOW_CHANNEL:
			gprintf("View = channel\n");
			retval = _loadChannelList();
			break;
		case COVERFLOW_HOMEBREW:
			gprintf("View = homebrew\n");
			retval = _loadHomebrewList();
			break;
		case COVERFLOW_USB:
		default:
			gprintf("View = usb\n");
			retval = _loadGameList();
			break;
	}
	
	return retval;
}

bool CMenu::_loadGameList(void)
{
	currentPartition = m_cfg.getInt("GENERAL", "partition", 1);
	gprintf("Opening partition %d\n", currentPartition);
	DeviceHandler::Instance()->Open_WBFS(currentPartition);
	m_gameList.Load(sfmt(GAMES_DIR, DeviceName[currentPartition]), ".wbfs|.iso");
	return m_gameList.size() > 0 ? true : false;
}

bool CMenu::_loadHomebrewList()
{
	currentPartition = m_cfg.getInt("GENERAL", "homebrew_partition", DeviceHandler::Instance()->PathToDriveType(m_appDir.c_str()));
	DeviceHandler::Instance()->Open_WBFS(currentPartition);
	m_gameList.Load(sfmt(HOMEBREW_DIR, DeviceName[currentPartition]), ".dol");
	return m_gameList.size() > 0 ? true : false;
}

void CMenu::_stopSounds(void)
{
	// Fade out sounds
	int fade_rate = m_cfg.getInt("GENERAL", "music_fade_rate", 8);

	if (!MusicPlayer::Instance()->IsStopped())
	{
		while (MusicPlayer::Instance()->GetVolume() > 0 || m_gameSound.GetVolume() > 0)
		{
			MusicPlayer::Instance()->Tick(true);
			
			if (m_gameSound.GetVolume() > 0)
				m_gameSound.SetVolume(m_gameSound.GetVolume() < fade_rate ? 0 : m_gameSound.GetVolume() - fade_rate);

			VIDEO_WaitVSync();		
		}
	}
	
	m_btnMgr.stopSounds();
	m_cf.stopSound();

	MusicPlayer::Instance()->Stop();
	m_gameSound.Stop();
}

bool CMenu::_loadFile(SmartBuf &buffer, u32 &size, const char *path, const char *file)
{
	SMART_FREE(buffer);
	size = 0;
	FILE *fp = fopen(file == NULL ? path : fmt("%s/%s", path, file), "rb");
		
	if (fp == 0) return false;

	fseek(fp, 0, SEEK_END);
	u32 fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	SmartBuf fileBuf = smartAnyAlloc(fileSize);
	if (!fileBuf)
	{
		SAFE_CLOSE(fp);
		return false;
	}
	if (fread(fileBuf.get(), 1, fileSize, fp) != fileSize)
	{
		SAFE_CLOSE(fp);
		return false;
	}
	SAFE_CLOSE(fp);
	buffer = fileBuf;
	size = fileSize;
	return true;
}

void CMenu::_load_installed_cioses()
{
	if (_installed_cios.size() > 0) return;
	
	// Do sjizzle
	u32 count;
	u32 ret = ES_GetNumTitles(&count);
	if (ret || !count)
	{
		gprintf("Cannot count...aaaah\n");
		return;
	}

	static u64 title_list[256] ATTRIBUTE_ALIGN(32);
	if (ES_GetTitles(title_list, count) > 0)
	{
		gprintf("Cannot get titles...\n");
		return;
	}
	
	for (u32 i = 0; i < count; i++)
	{
		u32 tmd_size;
		if (ES_GetStoredTMDSize(title_list[i], &tmd_size) > 0) continue;
		static u8 tmd_buf[MAX_SIGNED_TMD_SIZE] ATTRIBUTE_ALIGN(32);
		signed_blob *s_tmd = (signed_blob *) tmd_buf;
		if (ES_GetStoredTMD(title_list[i], s_tmd, tmd_size) > 0) continue;

		const tmd *t = (const tmd *) SIGNATURE_PAYLOAD(s_tmd);
		
		u32 kind = t->title_id >> 32;

		if (kind == 1)
		{
			u32 title_l = t->title_id & 0xFFFFFFFF;
			if (title_l == 0 || title_l == 2 || title_l == 0x100 || title_l == 0x101) continue;
			
			// We have an ios
			u32 version = t->title_version;
			if (tmd_buf[4] == 0 && (version < 100 || version == 0xFFFF || (version > D2X_MIN_VERSION && version < D2X_MAX_VERSION))) // Signature is empty
			{
				// Probably an cios
				_installed_cios.push_back(title_l);
			}
		}
	}
	
	sort(_installed_cios.begin(), _installed_cios.end());
}

void CMenu::_hideWaitMessage()
{
	m_vid.hideWaitMessage();
}

void CMenu::_showWaitMessage()
{
	TexSet texSet;
	m_vid.waitMessage(
		_textures(texSet, "GENERAL", "waitmessage"),
		m_theme.getFloat("GENERAL", "waitmessage_delay", 0.f),
		m_theme.getBool("GENERAL", "waitmessage_wiilight",	m_cfg.getBool("GENERAL", "waitmessage_wiilight", true))
	);
}

typedef struct map_entry
{
	char filename[8];
	u8 sha1[20];
} __attribute((packed)) map_entry_t;

void CMenu::_loadDefaultFont(bool korean)
{
	u32 size;
	bool retry = false;
	
	ISFS_Initialize();
	
	// Read content.map from ISFS
	u8 *content = ISFS_GetFile((u8 *) "/shared1/content.map", &size, 0);
	int items = size / sizeof(map_entry_t);
		
	gprintf("Open content.map, size %d, items %d\n", size, items);

retry:	
	bool kor_font = (korean && !retry) || (!korean && retry);
	map_entry_t *cm = (map_entry_t *) content;
	for (int i = 0; i < items; i++)
	{
		if (memcmp(cm[i].sha1, kor_font ? WIIFONT_HASH_KOR : WIIFONT_HASH, 20) == 0)
		{
			// Name found, load it and unpack it
			char u8_font_filename[22] = {0};
			strcpy(u8_font_filename, "/shared1/XXXXXXXX.app"); // Faster than sprintf
            memcpy(u8_font_filename+9, cm[i].filename, 8);
			
			u8 *u8_font_archive = ISFS_GetFile((u8 *) u8_font_filename, &size, 0);
			
			gprintf("Opened fontfile: %s: %d bytes\n", u8_font_filename, size);
			
			if (u8_font_archive != NULL)
			{
				const u8 *font_file = u8_get_file_by_index(u8_font_archive, 1, &size); // There is only one file in that app
				
				gprintf("Extracted font: %d\n", size);
				
				base_font = smartAnyAlloc(size);
				memcpy(base_font.get(), font_file, size);
				if(!!base_font.get())
					base_font_size = size;
			}
			SAFE_FREE(u8_font_archive);
			break;
		}
	}
	
	if (!retry)
	{
		retry = true;
		goto retry;
	}
	
	ISFS_Deinitialize();
	
	SAFE_FREE(content);
}

void CMenu::_cleanupDefaultFont()
{
	SMART_FREE(base_font);
	base_font_size = 0;
}