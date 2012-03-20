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
#include "nand.hpp"
#include "cios.hpp"
#include "loader/playlog.h"

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
	m_base_font_size = 0;
	m_current_view = COVERFLOW_USB;
}

extern "C" { int makedir(char *newdir); }

static bool SeekPathOnParts(string &drive, const string &fmt, const string &folder, bool direction)
{
	struct stat dummy;

	if(direction)
	{
		for(int i = SD; i <= USB8; i++)
			if (DeviceHandler::Instance()->IsInserted(i) && DeviceHandler::Instance()->GetFSType(i) != PART_FS_WBFS
				&& (fmt.empty() || stat(sfmt(fmt.c_str(), DeviceName[i], folder.c_str()).c_str(), &dummy) == 0))
			{
				drive.clear();
				drive = DeviceName[i];
				break;
			}
	}
	else
	{
		for(int i = USB8; i >= SD; i--)
			if (DeviceHandler::Instance()->IsInserted(i) && DeviceHandler::Instance()->GetFSType(i) != PART_FS_WBFS
				&& (fmt.empty() || stat(sfmt(fmt.c_str(), DeviceName[i], folder.c_str()).c_str(), &dummy) == 0))
			{
				drive = DeviceName[i];
				break;
			}
	}
	return !drive.empty();
}

void CMenu::init(string/*  dolLocation */)
{
	string drive;
	string games;
	struct stat dummy;

	/* Clear Playlog */
	Playlog_Delete();

/* 	if(!dolLocation.empty())
	{
		int i = DeviceHandler::Instance()->PathToDriveType(dolLocation.c_str());
		if (DeviceHandler::Instance()->IsInserted(i) && DeviceHandler::Instance()->GetFSType(i) != PART_FS_WBFS
				&& (fmt.empty() || stat(dolLocation.c_str(), &dummy) == 0))
			drive = DeviceName[i];
	} */
	if(drive.empty() && !SeekPathOnParts(drive, string("%s:/%s/" CFG_FILENAME), string(APPDATA_DIR2), true))		// Look for /apps/wiiflow/wiiflow.ini
		if(!SeekPathOnParts(drive, string("%s:/%s/boot.dol"), string(APPDATA_DIR2), true))							// Look for /apps/wiiflow/boot.dol
			if(!SeekPathOnParts(drive, string("%s:/%s"), string(APPDATA_DIR2), true))								// Look for /apps/wiiflow
				if(!SeekPathOnParts(drive, string("%s:/%s"), string("apps"), true))									// Look for /apps
					if(!SeekPathOnParts(drive, string("%s/%s"), string(GAMES_DIR), true))							// Look for /wbfs
						if(!SeekPathOnParts(drive, string(""), string(""), true))									// Look for a writable partition

	if(!drive.empty())
		makedir((char *)sfmt("%s:/%s", drive.c_str(), APPDATA_DIR2).c_str()); //Make the apps dir, so saving wiiflow.ini does not fail.
		
	_loadDefaultFont(CONF_GetLanguage() == CONF_LANG_KOREAN);

	if(drive.empty()) // Should not happen
	{
		_buildMenus();
		error(L"No available partitions found!");
		m_exit = true;
		return;
	}

	m_appDir = sfmt("%s:/%s", drive.c_str(), APPDATA_DIR2);

#ifdef FILE_GECKO
	snprintf(gecko_logfile, sizeof(gecko_logfile), "%s/%s", m_appDir.c_str(), "gecko.txt");
	if(stat(gecko_logfile, &dummy) == 0)
		remove(gecko_logfile);
#endif /* FILE_GECKO */

	gprintf("Wiiflow boot.dol Location: %s\n", m_appDir.c_str());

	m_cfg.load(sfmt("%s/" CFG_FILENAME, m_appDir.c_str()).c_str());
	u8 dsi_timeout = m_cfg.getInt("DEBUG", "dsi_timeout");
	__exception_setreload(dsi_timeout > 5 ? dsi_timeout : 5);

	bool onSD = drive[0] == 's';
	
	drive.clear();

	int i = 0;
	if(!(m_cfg.getString2("GENERAL", "dir_data", m_dataDir)
		&& DeviceHandler::Instance()->IsInserted((i = DeviceHandler::Instance()->PathToDriveType(m_dataDir.c_str())))
		&& DeviceHandler::Instance()->GetFSType(i) != PART_FS_WBFS))
	{
		if(!SeekPathOnParts(drive, string("%s:/%s"), string(APPDATA_DIR), onSD))					// Look for /wiiflow
			if(!SeekPathOnParts(drive, string("%s/%s"), string(GAMES_DIR), onSD))					// Look for /wbfs
				if(!SeekPathOnParts(drive, string("%s:/%s/boot.dol"), string(APPDATA_DIR2), onSD))	// Look for /apps/wiiflow/boot.dol
					if(!SeekPathOnParts(drive, string("%s:/%s"), string(APPDATA_DIR2), onSD))		// Look for /apps/wiiflow
						if(!SeekPathOnParts(drive, string("%s:/%s"), string("apps"), onSD))			// Look for /apps
							if(!SeekPathOnParts(drive, string(""), string(""), onSD))				// Look for a writable partition

		if(drive.empty())
		{
			_buildMenus();
			error(L"No available storage devices!\nExitting.");
			m_exit = true;
			return;
		}

		m_dataDir = sfmt("%s:/%s", drive.c_str(), APPDATA_DIR);
		m_cfg.setString("GENERAL", "dir_data", m_dataDir.c_str());
	
	}

	gprintf("Data Directory: %s\n", m_dataDir.c_str());

	int emu_mode = EMU_DISABLED;
	m_cfg.getInt("NAND", "emulation", &emu_mode);
	Nand::Instance()->Init(m_cfg.getString("NAND", "path").c_str(),
		m_cfg.getInt("NAND", "partition", currentPartition),
		emu_mode == EMU_DISABLED
	);

	_load_installed_cioses();

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

	const char *domain = _domainFromView();
	const char *checkDir = m_current_view == COVERFLOW_HOMEBREW ? HOMEBREW_DIR : GAMES_DIR;

	currentPartition = m_cfg.getInt(domain, "partition", currentPartition);
	if(m_current_view != COVERFLOW_CHANNEL && (currentPartition > USB8 || !DeviceHandler::Instance()->IsInserted(currentPartition)
		|| (m_current_view == COVERFLOW_USB && DeviceHandler::Instance()->GetFSType(i) != PART_FS_WBFS && stat(sfmt(checkDir, DeviceName[i]).c_str(), &dummy) == -1)))
	{
		m_cfg.remove(domain, "partition");
		for(int i = SD; i <= USB8+1; i++) // Find a usb partition with the wbfs folder or wbfs file system, else leave it blank (defaults to 1 later)
		{
			if(i > USB8)
			{
				m_current_view = COVERFLOW_CHANNEL;
				break;
			}
			if (DeviceHandler::Instance()->IsInserted(i)
				&& ((m_current_view == COVERFLOW_USB && DeviceHandler::Instance()->GetFSType(i) == PART_FS_WBFS)
				|| stat(sfmt(checkDir, DeviceName[i]).c_str(), &dummy) == 0))
			{
				m_cfg.setInt(domain, "partition", i);
				currentPartition = i;
				break;
			}
		}
	}

	m_cf.init(m_base_font, m_base_font_size);

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

	m_gameList.Init(m_listCacheDir, m_settingsDir, m_loc.getString(m_curLanguage, "gametdb_code", "EN"));

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
	bool pShadowBlur = m_theme.getBool("GENERAL", "pointer_shadow_blur");

	for(int chan = WPAD_MAX_WIIMOTES-2; chan >= 0; chan--)
	{
		m_cursor[chan].init(sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString("GENERAL", sfmt("pointer%i", chan+1).c_str()).c_str()).c_str(),
			m_vid.wide(), pShadowColor, pShadowX, pShadowY, pShadowBlur, chan);
		WPAD_SetVRes(chan, m_vid.width() + m_cursor[chan].width(), m_vid.height() + m_cursor[chan].height());
	}

	m_btnMgr.init(m_vid);
	MusicPlayer::Instance()->Init(m_cfg, m_musicDir, sfmt("%s/music", m_themeDataDir.c_str()));

	_buildMenus();

	m_locked = m_cfg.getString("GENERAL", "parent_code").size() >= 4;
	m_btnMgr.setRumble(CONF_GetPadMotorMode() != 0);

	int exit_to = m_cfg.getInt("GENERAL", "exit_to");
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

	if (m_cfg.getBool("GENERAL", "favorites_on_startup"))
		m_favorites = m_cfg.getBool(domain, "favorites");
	m_category = m_cat.getInt(domain, "category");
	m_max_categories = m_cat.getInt(domain, "numcategories", 12);

	safe_vector<string> gamercards = stringToVector(m_cfg.getString("GAMERCARD", "gamercards"), "|");
	for (safe_vector<string>::iterator itr = gamercards.begin(); itr != gamercards.end(); itr++)
	{
		string defval = itr->compare("wiinertag") ? WIINNERTAG_URL : itr->compare("dutag") ? DUTAG_URL : "";
		register_card_provider(
			m_cfg.getString("GAMERCARD", sfmt("%s_url", (*itr).c_str()), defval).c_str(),
			m_cfg.getString("GAMERCARD", sfmt("%s_key", (*itr).c_str()), defval).c_str()
		);
	}
}

void CMenu::cleanup(bool ios_reload)
{
	m_cf.stopCoverLoader();

	CheckGameSoundThread(true);

	_stopSounds();

	if (!ios_reload)
		SMART_FREE(m_cameraSound);

	MusicPlayer::DestroyInstance();
	SoundHandler::DestroyInstance();
	soundDeinit();
	if (!ios_reload)
	{
		LWP_MutexDestroy(m_mutex);
		m_mutex = 0;
	}

	DeviceHandler::DestroyInstance();

	if (!ios_reload)
		_cleanupDefaultFont();

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

void CMenu::_loadCFCfg(SThemeData &theme)
{
	const char *domain = "_COVERFLOW";

	m_cf.setCachePath(m_cacheDir.c_str(), !m_cfg.getBool("GENERAL", "keep_png", true), m_cfg.getBool("GENERAL", "compress_cache"));
	m_cf.setBufferSize(m_cfg.getInt("GENERAL", "cover_buffer", 60));

	u32 flip_wav_size = 0, select_wav_size = 0, cancel_wav_size = 0;
	u8 *flip_wav = 0, *select_wav = 0, *cancel_wav = 0;

	m_cf.setSounds(
		_sound(theme.soundSet, domain, "flip_sound", flip_wav, flip_wav_size, string("default_flip_snd")),
		_sound(theme.soundSet, domain, "hover_sound", hover_wav, hover_wav_size, string("default_hover_snd")),
		_sound(theme.soundSet, domain, "select_sound", select_wav, select_wav_size, string("default_select_snd")),
		_sound(theme.soundSet, domain, "cancel_sound", cancel_wav, cancel_wav_size, string("default_cancel_snd"))
	);

	string texLoading = sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "loading_cover_box").c_str());
	string texNoCover = sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "missing_cover_box").c_str());
	string texLoadingFlat = sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "loading_cover_flat").c_str());
	string texNoCoverFlat = sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "missing_cover_flat").c_str());
	m_cf.setTextures(texLoading, texLoadingFlat, texNoCover, texNoCoverFlat);

	m_cf.setFont(_font(theme.fontSet, domain, TITLEFONT), m_theme.getColor(domain, "font_color", CColor(0xFFFFFFFF)));

	m_numCFVersions = min(max(2, m_theme.getInt(domain, "number_of_modes", 2)), 8);
	for (u32 i = 1; i <= m_numCFVersions; ++i)
		_loadCFLayout(i);

	_loadCFLayout((m_cfg.getInt(_domainFromView(), "last_cf_mode" , 1) + (int)m_numCFVersions) % (int)m_numCFVersions);
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
	string key169(key), key43(key);

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

const char *CMenu::_cfDomain(bool selected)
{
	if(m_cfg.getBool(_domainFromView(), "seperate_views", m_cfg.getBool(_domainFromView(), "smallbox", m_current_view == COVERFLOW_HOMEBREW)))
	{
		switch(m_current_view)
		{
			case COVERFLOW_CHANNEL:
				return selected ? "_CHANFLOW_%i_S" : "_CHANFLOW_%i";
			case COVERFLOW_HOMEBREW:
				return selected ? "_BREWFLOW_%i_S" : "_BREWFLOW_%i";
			default:
				return selected ? "_COVERFLOW_%i_S" : "_COVERFLOW_%i";
		}
	}
	else return selected ? "_COVERFLOW_%i_S" : "_COVERFLOW_%i";
}

void CMenu::_loadCFLayout(int version, bool forceAA, bool otherScrnFmt)
{
	version = 1 + ((version + (int)m_numCFVersions) % (int)m_numCFVersions);

	bool smallbox = m_cfg.getBool(_domainFromView(), "smallbox", m_current_view == COVERFLOW_HOMEBREW);
	string domain(sfmt(_cfDomain(), version).c_str());
	string domainSel(sfmt(_cfDomain(true), version).c_str());

	int max_fsaa = m_theme.getInt(domain, "max_fsaa", 3);
	_setAA(forceAA ? max_fsaa : min(max_fsaa, m_cfg.getInt("GENERAL", "max_fsaa", 3)));

	m_cf.setTextureQuality(m_theme.getFloat(domain, "tex_lod_bias", -3.f),
		m_theme.getInt(domain, "tex_aniso", 2),
		m_theme.getBool(domain, "tex_edge_lod", true));

	m_cf.setRange(_getCFInt(domain, "rows", smallbox ? 5 : 1, otherScrnFmt), _getCFInt(domain, "columns", 9, otherScrnFmt));

	m_cf.setCameraPos(false,
		_getCFV3D(domain, "camera_pos", Vector3D(0.f, 1.5f, 5.f), otherScrnFmt),
		_getCFV3D(domain, "camera_aim", Vector3D(0.f, 0.f, -1.f), otherScrnFmt));

	m_cf.setCameraPos(true,
		_getCFV3D(domainSel, "camera_pos", Vector3D(0.f, 1.5f, 5.f), otherScrnFmt),
		_getCFV3D(domainSel, "camera_aim", Vector3D(0.f, 0.f, -1.f), otherScrnFmt));

	m_cf.setCameraOsc(false,
		_getCFV3D(domain, "camera_osc_speed", Vector3D(2.f, 1.1f, 1.3f), otherScrnFmt),
		_getCFV3D(domain, "camera_osc_amp", Vector3D(0.1f, 0.2f, 0.1f), otherScrnFmt));

	m_cf.setCameraOsc(true,
		_getCFV3D(domainSel, "camera_osc_speed", Vector3D(), otherScrnFmt),
		_getCFV3D(domainSel, "camera_osc_amp", Vector3D(), otherScrnFmt));

	float def_cvr_posX = smallbox ? 1.f : 1.6f;
	float def_cvr_posY = smallbox ? -0.6f : 0.f;
	m_cf.setCoverPos(false,
		_getCFV3D(domain, "left_pos", Vector3D(-def_cvr_posX, def_cvr_posY, 0.f), otherScrnFmt),
		_getCFV3D(domain, "right_pos", Vector3D(def_cvr_posX, def_cvr_posY, 0.f), otherScrnFmt),
		_getCFV3D(domain, "center_pos", Vector3D(0.f, def_cvr_posY, 1.f), otherScrnFmt),
		_getCFV3D(domain, "row_center_pos", Vector3D(0.f, def_cvr_posY, 0.f), otherScrnFmt));

	def_cvr_posX = smallbox ? 1.f : 4.6f;
	float def_cvr_posX1 = smallbox ? 0.f : -0.6f;
	m_cf.setCoverPos(true,
		_getCFV3D(domainSel, "left_pos", Vector3D(-def_cvr_posX, def_cvr_posY, 0.f), otherScrnFmt),
		_getCFV3D(domainSel, "right_pos", Vector3D(def_cvr_posX, def_cvr_posY, 0.f), otherScrnFmt),
		_getCFV3D(domainSel, "center_pos", Vector3D(def_cvr_posX1, 0.f, 2.6f), otherScrnFmt),
		_getCFV3D(domainSel, "row_center_pos", Vector3D(0.f, def_cvr_posY, 0.f), otherScrnFmt));

	m_cf.setCoverAngleOsc(false,
		m_theme.getVector3D(domain, "cover_osc_speed", Vector3D(2.f, 2.f, 0.f)),
		m_theme.getVector3D(domain, "cover_osc_amp", Vector3D(5.f, 10.f, 0.f)));

	m_cf.setCoverAngleOsc(true,
		m_theme.getVector3D(domainSel, "cover_osc_speed", Vector3D(2.1f, 2.1f, 0.f)),
		m_theme.getVector3D(domainSel, "cover_osc_amp", Vector3D(2.f, 5.f, 0.f)));

	m_cf.setCoverPosOsc(false,
		m_theme.getVector3D(domain, "cover_pos_osc_speed"),
		m_theme.getVector3D(domain, "cover_pos_osc_amp"));

	m_cf.setCoverPosOsc(true,
		m_theme.getVector3D(domainSel, "cover_pos_osc_speed"),
		m_theme.getVector3D(domainSel, "cover_pos_osc_amp"));

	float spacerX = smallbox ? 	1.f : 0.35f;
	m_cf.setSpacers(false,
		m_theme.getVector3D(domain, "left_spacer", Vector3D(-spacerX, 0.f, 0.f)),
		m_theme.getVector3D(domain, "right_spacer", Vector3D(spacerX, 0.f, 0.f)));

	m_cf.setSpacers(true,
		m_theme.getVector3D(domainSel, "left_spacer", Vector3D(-spacerX, 0.f, 0.f)),
		m_theme.getVector3D(domainSel, "right_spacer", Vector3D(spacerX, 0.f, 0.f)));

	m_cf.setDeltaAngles(false,
		m_theme.getVector3D(domain, "left_delta_angle"),
		m_theme.getVector3D(domain, "right_delta_angle"));

	m_cf.setDeltaAngles(true,
		m_theme.getVector3D(domainSel, "left_delta_angle"),
		m_theme.getVector3D(domainSel, "right_delta_angle"));

	float angleY = smallbox ? 0.f : 70.f;
	m_cf.setAngles(false,
		m_theme.getVector3D(domain, "left_angle", Vector3D(0.f, angleY, 0.f)),
		m_theme.getVector3D(domain, "right_angle", Vector3D(0.f, -angleY, 0.f)),
		m_theme.getVector3D(domain, "center_angle"),
		m_theme.getVector3D(domain, "row_center_angle"));

	angleY = smallbox ? 0.f : 90.f;
	float angleY1 = smallbox ? 0.f : 380.f;
	float angleX = smallbox ? 0.f : -45.f;
	m_cf.setAngles(true,
		m_theme.getVector3D(domainSel, "left_angle", Vector3D(angleX, angleY, 0.f)),
		m_theme.getVector3D(domainSel, "right_angle", Vector3D(angleX, -angleY, 0.f)),
		m_theme.getVector3D(domainSel, "center_angle", Vector3D(0.f, angleY1, 0.f)),
		m_theme.getVector3D(domainSel, "row_center_angle"));

	angleX = smallbox ? 0.f : 55.f;
	m_cf.setTitleAngles(false,
		_getCFFloat(domain, "text_left_angle", -angleX, otherScrnFmt),
		_getCFFloat(domain, "text_right_angle", angleX, otherScrnFmt),
		_getCFFloat(domain, "text_center_angle", 0.f, otherScrnFmt));

	m_cf.setTitleAngles(true,
		_getCFFloat(domainSel, "text_left_angle", -angleX, otherScrnFmt),
		_getCFFloat(domainSel, "text_right_angle", angleX, otherScrnFmt),
		_getCFFloat(domainSel, "text_center_angle", 0.f, otherScrnFmt));

	m_cf.setTitlePos(false,
		_getCFV3D(domain, "text_left_pos", Vector3D(-4.f, 0.f, 1.3f), otherScrnFmt),
		_getCFV3D(domain, "text_right_pos", Vector3D(4.f, 0.f, 1.3f), otherScrnFmt),
		_getCFV3D(domain, "text_center_pos", Vector3D(0.f, 0.f, 2.6f), otherScrnFmt));

	m_cf.setTitlePos(true,
		_getCFV3D(domainSel, "text_left_pos", Vector3D(-4.f, 0.f, 1.3f), otherScrnFmt),
		_getCFV3D(domainSel, "text_right_pos", Vector3D(4.f, 0.f, 1.3f), otherScrnFmt),
		_getCFV3D(domainSel, "text_center_pos", Vector3D(.6f, 1.6f, 1.6f), otherScrnFmt));

	m_cf.setTitleWidth(false,
		_getCFFloat(domain, "text_side_wrap_width", 600.f, otherScrnFmt),
		_getCFFloat(domain, "text_center_wrap_width", 600.f, otherScrnFmt));

	m_cf.setTitleWidth(true,
		_getCFFloat(domainSel, "text_side_wrap_width", 600.f, otherScrnFmt),
		_getCFFloat(domainSel, "text_center_wrap_width", 380.f, otherScrnFmt));

	m_cf.setTitleStyle(false,
		_textStyle(domain.c_str(), "text_side_style", FTGX_ALIGN_MIDDLE | FTGX_JUSTIFY_CENTER),
		_textStyle(domain.c_str(), "text_center_style", FTGX_ALIGN_BOTTOM | FTGX_JUSTIFY_CENTER));

	m_cf.setTitleStyle(true,
		_textStyle(domainSel.c_str(), "text_side_style", FTGX_ALIGN_MIDDLE | FTGX_JUSTIFY_CENTER),
		_textStyle(domainSel.c_str(), "text_center_style", FTGX_ALIGN_TOP | FTGX_JUSTIFY_LEFT));

	m_cf.setColors(false,
		m_theme.getColor(domain, "color_beg", 0xCFFFFFFF),
		m_theme.getColor(domain, "color_end", 0x3FFFFFFF),
		m_theme.getColor(domain, "color_off", 0x7FFFFFFF));

	m_cf.setColors(true,
		m_theme.getColor(domainSel, "color_beg", 0x7FFFFFFF),
		m_theme.getColor(domainSel, "color_end", 0x1FFFFFFF),
		m_theme.getColor(domain, "color_off", 0x7FFFFFFF));

	m_cf.setMirrorAlpha(m_theme.getFloat(domain, "mirror_alpha", 0.25f), m_theme.getFloat(domain, "title_mirror_alpha", 0.2f));

	m_cf.setMirrorBlur(m_theme.getBool(domain, "mirror_blur", true));

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
		m_theme.getFloat(domain, "shadow_x"),
		m_theme.getFloat(domain, "shadow_y"));

	float spacerY = smallbox ? 0.60f : 2.f;
	m_cf.setRowSpacers(false,
		m_theme.getVector3D(domain, "top_spacer", Vector3D(0.f, spacerY, 0.f)),
		m_theme.getVector3D(domain, "bottom_spacer", Vector3D(0.f, -spacerY, 0.f)));

	m_cf.setRowSpacers(true,
		m_theme.getVector3D(domainSel, "top_spacer", Vector3D(0.f, spacerY, 0.f)),
		m_theme.getVector3D(domainSel, "bottom_spacer", Vector3D(0.f, -spacerY, 0.f)));

	m_cf.setRowDeltaAngles(false,
		m_theme.getVector3D(domain, "top_delta_angle"),
		m_theme.getVector3D(domain, "bottom_delta_angle"));

	m_cf.setRowDeltaAngles(true,
		m_theme.getVector3D(domainSel, "top_delta_angle"),
		m_theme.getVector3D(domainSel, "bottom_delta_angle"));

	m_cf.setRowAngles(false,
		m_theme.getVector3D(domain, "top_angle"),
		m_theme.getVector3D(domain, "bottom_angle"));

	m_cf.setRowAngles(true,
		m_theme.getVector3D(domainSel, "top_angle"),
		m_theme.getVector3D(domainSel, "bottom_angle"));

	Vector3D def_cvr_scale = smallbox ? Vector3D(0.667f, 0.25f, 1.f) : Vector3D(1.f, 1.f, 1.f);
	m_cf.setCoverScale(false,
		m_theme.getVector3D(domain, "left_scale", def_cvr_scale),
		m_theme.getVector3D(domain, "right_scale", def_cvr_scale),
		m_theme.getVector3D(domain, "center_scale", def_cvr_scale),
		m_theme.getVector3D(domain, "row_center_scale", def_cvr_scale));

	m_cf.setCoverScale(true,
		m_theme.getVector3D(domainSel, "left_scale", def_cvr_scale),
		m_theme.getVector3D(domainSel, "right_scale", def_cvr_scale),
		m_theme.getVector3D(domainSel, "center_scale", def_cvr_scale),
		m_theme.getVector3D(domainSel, "row_center_scale", def_cvr_scale));

	float flipX = smallbox ? 359.f : 180.f;
	m_cf.setCoverFlipping(
		_getCFV3D(domainSel, "flip_pos", Vector3D(), otherScrnFmt),
		_getCFV3D(domainSel, "flip_angle", Vector3D(0.f, flipX, 0.f), otherScrnFmt),
		_getCFV3D(domainSel, "flip_scale", def_cvr_scale, otherScrnFmt));

	m_cf.setBlur(
		m_theme.getInt(domain, "blur_resolution", 1),
		m_theme.getInt(domain, "blur_radius", 2),
		m_theme.getFloat(domain, "blur_factor", 1.f));
}

void CMenu::_buildMenus(void)
{
	SThemeData theme;

	if(!m_base_font.get()) _loadDefaultFont(CONF_GetLanguage() == CONF_LANG_KOREAN);

	_font(theme.fontSet, "GENERAL", BUTTONFONT);
	theme.btnFontColor = m_theme.getColor("GENERAL", "button_font_color", 0xD0BFDFFF);

	_font(theme.fontSet, "GENERAL", LABELFONT);
	theme.lblFontColor = m_theme.getColor("GENERAL", "label_font_color", 0xD0BFDFFF);

	_font(theme.fontSet, "GENERAL", TITLEFONT);
	theme.titleFontColor = m_theme.getColor("GENERAL", "title_font_color", 0xD0BFDFFF);

	_font(theme.fontSet, "GENERAL", TEXTFONT);
	theme.txtFontColor = m_theme.getColor("GENERAL", "text_font_color", 0xFFFFFFFF);

	theme.clickSound	= _sound(theme.soundSet, "GENERAL", "click_sound", click_wav, click_wav_size, string("default_click_snd"));
	theme.hoverSound	= _sound(theme.soundSet, "GENERAL", "hover_sound", hover_wav, hover_wav_size, string("default_hover_snd"));
	theme.cameraSound	= _sound(theme.soundSet, "GENERAL", "camera_sound", camera_wav, camera_wav_size, string("default_camera_snd"));
	m_cameraSound = theme.cameraSound;

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

	_loadCFCfg(theme);
}

typedef struct
{
	string ext;
	u32 min;
	u32 max;
	u32 def;
	u32 res;
} FontHolder;

SFont CMenu::_font(CMenu::FontSet &fontSet, const char *domain, const char *key, u32 fontSize)
{
	FontHolder fonts[3] = {{ "_size", 6u, 300u, fontSize, 0 }, { "_line_height", 6u, 300u, fontSize + 4, 0 }, { "_weight", 1u, 32u, FONT_BOLD, 0 }};

	bool themeloaded = m_theme.loaded();
	bool general = strncmp(domain, "GENERAL", 7) == 0;
	string filename;
	if(themeloaded)
	{
		filename = m_theme.getString(domain, key);
		if(!general && filename.empty())
			filename = m_theme.getString("GENERAL", key);
	}

	bool useDefault = filename.empty();
	if(useDefault) filename = key;

	for(u32 i = 0; i < 3; i++)
	{
		if(themeloaded)
		{
			string value = key;
			value += fonts[i].ext;
			if(!general)
				fonts[i].res = (u32)m_theme.getInt(domain, value);
			if(fonts[i].res <= 0)
				fonts[i].res = (u32)m_theme.getInt("GENERAL", value);

			fonts[i].res = min(max(fonts[i].min, fonts[i].res <= 0 ? fonts[i].def : fonts[i].res), fonts[i].max);
		}
		else fonts[i].res = fonts[i].def;
	}

	CMenu::FontSet::iterator i = fontSet.find(CMenu::FontDesc(upperCase(filename.c_str()), fonts[0].res));
	if (i != fontSet.end()) return i->second;

	SFont retFont;
 	if (!useDefault && retFont.fromFile(sfmt("%s/%s", m_themeDataDir.c_str(), filename.c_str()).c_str(), fonts[0].res, fonts[1].res, fonts[2].res, 1))
		return fontSet[CMenu::FontDesc(upperCase(filename.c_str()), fonts[0].res)] = retFont;

	if(!m_base_font) _loadDefaultFont(CONF_GetLanguage() == CONF_LANG_KOREAN);
	if(retFont.fromBuffer(m_base_font, m_base_font_size, fonts[0].res, fonts[1].res, fonts[2].res, 1))
		return fontSet[CMenu::FontDesc(upperCase(filename.c_str()), fonts[0].res)] = retFont;

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
					continue;
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
	if(filename.empty()) filename = key;
	texSet[filename] = def;
	return def;
}

SmartGuiSound CMenu::_sound(CMenu::SoundSet &soundSet, const char *domain, const char *key, const u8 * snd, u32 len, string name)
{
	string filename;
	if (m_theme.loaded())
		filename = m_theme.getString(domain, key);

	bool defVal = filename.empty();

	if(!defVal)
		filename = sfmt("%s/%s", m_themeDataDir.c_str(), filename.c_str());

	CMenu::SoundSet::iterator i = soundSet.find(upperCase(defVal ? name.c_str() : filename.c_str()));
	if (i != soundSet.end()) return i->second;

	if(!defVal)
		return soundSet[upperCase(filename.c_str())] = SmartGuiSound(new GuiSound(filename));

	return soundSet[upperCase(name.c_str())] = SmartGuiSound(new GuiSound(snd, len, name));
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

u32 CMenu::_addButton(CMenu::SThemeData &theme, const char *domain, int x, int y, u32 width, u32 height, const wstringEx &text)
{
	SButtonTextureSet btnTexSet;
	CColor c(theme.btnFontColor);

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

	SFont font = _font(theme.fontSet, domain, BUTTONFONT);

	SmartGuiSound clickSound = _sound(theme.soundSet, domain, "click_sound", theme.clickSound->GetSound(), theme.clickSound->GetLength(), theme.clickSound->GetName());
	SmartGuiSound hoverSound = _sound(theme.soundSet, domain, "hover_sound", theme.hoverSound->GetSound(), theme.hoverSound->GetLength(), theme.hoverSound->GetName());

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
	SmartGuiSound clickSound = _sound(theme.soundSet, domain, "click_sound", theme.clickSound->GetSound(), theme.clickSound->GetLength(), theme.clickSound->GetName());
	SmartGuiSound hoverSound = _sound(theme.soundSet, domain, "hover_sound", theme.hoverSound->GetSound(), theme.hoverSound->GetLength(), theme.hoverSound->GetName());

	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
		x = m_vid.width() - x - width;
	if (btnPos & FTGX_ALIGN_BOTTOM)
		y = m_vid.height() - y - height;

	return m_btnMgr.addPicButton(tex1, tex2, x, y, width, height, clickSound, hoverSound);
}

u32 CMenu::_addLabel(CMenu::SThemeData &theme, const char *domain, int x, int y, u32 width, u32 height, u16 style, CMenu_Alias type, const wstringEx &text)
{
	CColor c;

	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	SFont font;
	switch(type)
	{
		case ADD_TEXT:
			 font = _font(theme.fontSet, domain, TEXTFONT);
			c = CColor(theme.txtFontColor);
			break;
		case ADD_TITLE:
			font = _font(theme.fontSet, domain, TITLEFONT);
			c = CColor(theme.titleFontColor);
			break;
		default:
			font = _font(theme.fontSet, domain, LABELFONT);
			c = CColor(theme.lblFontColor);
			break;
	}
	c = m_theme.getColor(domain, "color", c);
	style = _textStyle(domain, "style", style);

	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
		x = m_vid.width() - x - width;
	if (btnPos & FTGX_ALIGN_BOTTOM)
		y = m_vid.height() - y - height;

	return m_btnMgr.addLabel(font, text, x, y, width, height, c, style);
}

u32 CMenu::_addTitle(CMenu::SThemeData &theme, const char *domain, int x, int y, u32 width, u32 height, u16 style, const wstringEx &text)
{
	return _addLabel(theme, domain, x, y, width, height, style, ADD_TITLE, text);
}

u32 CMenu::_addText(CMenu::SThemeData &theme, const char *domain, int x, int y, u32 width, u32 height, u16 style, const wstringEx &text)
{
	return _addLabel(theme, domain, x, y, width, height, style, ADD_TEXT, text);
}

u32 CMenu::_addLabel(CMenu::SThemeData &theme, const char *domain, int x, int y, u32 width, u32 height, u16 style, STexture &bg, const wstringEx &text)
{
	CColor c(theme.btnFontColor);

	c = m_theme.getColor(domain, "color", c);
	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	SFont font = _font(theme.fontSet, domain, BUTTONFONT);
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
			ids[i] = _addLabel(theme, dom.c_str(), 40, 200, 64, 64, 0, emptyTex);
			_setHideAnim(ids[i], dom.c_str(), -50, 0, 0.f, 0.f);
		}
		else
			ids[i] = -1u;
	}
}

void CMenu::_initCF(void)
{
	Config m_dump;
	const char *domain = _domainFromView();

	m_cf.clear();
	m_cf.reserve(m_gameList.size());

 	m_gamelistdump = m_cfg.getBool(domain, "dump_list");
	if(m_gamelistdump) m_dump.load(sfmt("%s/titlesdump.ini", m_settingsDir.c_str()).c_str());

	m_gcfg1.load(sfmt("%s/gameconfig1.ini", m_settingsDir.c_str()).c_str());
	for (u32 i = 0; i < m_gameList.size(); ++i)
	{
		u64 chantitle = m_gameList[i].hdr.chantitle;
		if (m_current_view == COVERFLOW_CHANNEL && chantitle == HBC_108)
			strncpy((char *) m_gameList[i].hdr.id, "JODI", 6);

		string id = string((const char *)m_gameList[i].hdr.id, m_current_view == COVERFLOW_CHANNEL || m_current_view == COVERFLOW_HOMEBREW ?  4 : 6);

		if ((!m_favorites || m_gcfg1.getBool("FAVORITES", id)) && (!m_locked || !m_gcfg1.getBool("ADULTONLY", id)) && !m_gcfg1.getBool("HIDDEN", id))
		{
			if (m_category != 0)
			{
				const char *categories = m_cat.getString("CATEGORIES", id).c_str();
				if (strlen(categories) != 12 || categories[m_category] == '0')
					continue;
			}

			int playcount = m_gcfg1.getInt("PLAYCOUNT", id);
			unsigned int lastPlayed = m_gcfg1.getUInt("LASTPLAYED", id);

			if(m_gamelistdump)
				m_dump.setWString(domain, id, m_gameList[i].title);

			if (m_current_view != COVERFLOW_HOMEBREW) // Fixme
				m_cf.addItem(&m_gameList[i], sfmt("%s/%s.png", m_picDir.c_str(), id.c_str()).c_str(), sfmt("%s/%s.png", m_boxPicDir.c_str(), id.c_str()).c_str(), playcount, lastPlayed);
			else
			{
				string s = sfmt("%s", m_gameList[i].path);
				m_cf.addItem(&m_gameList[i], sfmt("%s/icon.png", s.substr(0, s.find_last_of("/")).c_str()).c_str(), sfmt("%s/%s.png", m_boxPicDir.c_str(), id.c_str()).c_str(), playcount, lastPlayed);
			}
		}
	}
	m_gcfg1.unload();
 	if (m_gamelistdump)
	{
		m_dump.save(true);
		m_cfg.remove(domain, "dump_list");
	}
 	m_cf.setBoxMode(m_cfg.getBool("GENERAL", "box_mode", true));
	m_cf.setCompression(m_cfg.getBool("GENERAL", "allow_texture_compression", true));
	m_cf.setSorting((Sorting)m_cfg.getInt(domain, "sort"));
	if (m_curGameId.empty() || !m_cf.findId(m_curGameId.c_str(), true))
		m_cf.findId(m_cfg.getString(domain, "current_item").c_str(), true);
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

	if (withCF && m_gameSelected && m_gamesound_changed && m_gameSoundHdr == NULL && !m_gameSound.IsPlaying() && MusicPlayer::Instance()->GetVolume() == 0)
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
	m_btnMgr.setText(m_mem2FreeSize, wfmt(L"Mem2 Free:%u, Mem1 Free:%u", MEM2_freesize(), SYS_GetArena1Size()), true);
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

void CMenu::_loadList(void)
{
	m_cf.clear();
	m_gameList.clear();

	gprintf("Loading items of ");

	if(m_cfg.getBool(_domainFromView(), "update_cache")) m_gameList.Update(m_current_view);
	switch(m_current_view)
	{
		case COVERFLOW_CHANNEL:
			gprintf("channel view from ");
			_loadChannelList();
			break;
		case COVERFLOW_HOMEBREW:
			gprintf("homebrew view from ");
			_loadHomebrewList();
			break;
		default:
			gprintf("usb view from ");
			_loadGameList();
			break;
	}
	m_cfg.remove(_domainFromView(), "update_cache");
}

void CMenu::_loadGameList(void)
{
	currentPartition = m_cfg.getInt("GAMES", "partition", currentPartition);
	if(!DeviceHandler::Instance()->IsInserted(currentPartition))
		return;

	gprintf("%s\n", DeviceName[currentPartition]);
	DeviceHandler::Instance()->Open_WBFS(currentPartition);
	m_gameList.Load(sfmt(GAMES_DIR, DeviceName[currentPartition]), ".wbfs|.iso");
}

void CMenu::_loadHomebrewList()
{
	currentPartition = m_cfg.getInt("HOMEBREW", "partition", DeviceHandler::Instance()->PathToDriveType(m_appDir.c_str()));
	if(!DeviceHandler::Instance()->IsInserted(currentPartition))
		return;

	gprintf("%s\n", DeviceName[currentPartition]);
	DeviceHandler::Instance()->Open_WBFS(currentPartition);

	m_gameList.Load(sfmt(HOMEBREW_DIR, DeviceName[currentPartition]), ".dol|.elf");
}

void CMenu::_loadChannelList(void)
{
	currentPartition = m_cfg.getInt("NAND", "partition", currentPartition);
	static u8 lastPartition = currentPartition;

	int emu_mode = EMU_DISABLED;
	m_cfg.getInt("NAND", "emulation", &emu_mode);
	if(emu_mode > EMU_FULL)
	{
		emu_mode = EMU_DISABLED;
		m_cfg.remove("NAND", "emulation");
	}
	static int last_emu_mode = emu_mode;

	if(emu_mode != 0 && !DeviceHandler::Instance()->IsInserted(currentPartition))
		return;

	static bool first = true, failed = false;

	bool changed = lastPartition != currentPartition || last_emu_mode != emu_mode || first || failed;

	gprintf("%s, which is %s\n", emu_mode ? DeviceName[currentPartition] : "NAND", changed ? "refreshing." : "cached.");

	string path = m_cfg.getString("NAND", "path");

	if(first && emu_mode)
	{
		char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);

		u32 sysconf_size, meez_size;

		sprintf(filepath, "/shared2/sys/SYSCONF");
		u8 *sysconf = ISFS_GetFile((u8 *) &filepath, &sysconf_size, -1);

		if(sysconf != NULL && sysconf_size > 0)
		{
			bzero(filepath, ISFS_MAXPATH);
			sprintf(filepath, "%s:%sshared2/sys/SYSCONF", DeviceName[currentPartition], path.c_str());
			FILE *file = fopen(filepath, "wb");
			if(file)
			{
				fwrite(sysconf, 1, sysconf_size, file);
				gprintf("Written SYSCONF to: %s\n", filepath);
				fclose(file);
			}
			else gprintf("Openning %s failed returning %i\n", filepath, file);
			SAFE_FREE(sysconf);
		}
		bzero(filepath, ISFS_MAXPATH);
		sprintf(filepath, "/shared2/menu/FaceLib/RFL_DB.dat");
		u8 *meez = ISFS_GetFile((u8 *) &filepath, &meez_size, -1);

		if(meez != NULL && meez_size > 0)
		{
			bzero(filepath, ISFS_MAXPATH);
			sprintf(filepath, "%s:%sshared2/menu/FaceLib/RFL_DB.dat", DeviceName[currentPartition], path.c_str());
			FILE *file = fopen(filepath, "wb");
			if(file)
			{
				fwrite(meez, 1, meez_size, file);
				gprintf("Written Mii's to: %s\n", filepath);
				fclose(file);
			}
			else gprintf("Openning %s failed returning %i\n", filepath, file);
			SAFE_FREE(meez);
		}
		first = false;
	}

	if(changed)
	{
		Nand::Instance()->Disable_Emu();
		if(!DeviceHandler::Instance()->IsInserted(lastPartition))
			DeviceHandler::Instance()->Mount(lastPartition);

		DeviceHandler::Instance()->UnMount(currentPartition);

		Nand::Instance()->Init(path.c_str(), currentPartition, emu_mode == EMU_DISABLED);
		if(Nand::Instance()->Enable_Emu() < 0)
		{
			Nand::Instance()->Disable_Emu();
			failed = true;
		}
		else failed = false;
	}

	if(!DeviceHandler::Instance()->IsInserted(currentPartition))
		DeviceHandler::Instance()->Mount(currentPartition);

	string nandpath = sfmt("%s:%s", DeviceName[currentPartition], path.empty() ? "/" : path.c_str());
	gprintf("nandpath = %s\n", nandpath.c_str());

	if(!failed) m_gameList.LoadChannels(emu_mode ? nandpath : "", 0);

	lastPartition = currentPartition;
	last_emu_mode = emu_mode;
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
	gprintf("Loading cIOS map\n");

	_installed_cios[0] = 1;
	u8 base = 0;

	for (u8 slot = 100; slot < 254; slot++)
		if(cIOSInfo::D2X(slot, &base))
		{
			gprintf("Found base %u in slot %u\n", base, slot);
			_installed_cios[slot] = base;
		}
}

void CMenu::_hideWaitMessage(bool force)
{
	m_vid.hideWaitMessage(force);
}

void CMenu::_showWaitMessage()
{
	TexSet texSet;
	m_vid.waitMessage(
		_textures(texSet, "GENERAL", "waitmessage"),
		m_theme.getFloat("GENERAL", "waitmessage_delay"),
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

	// Read content.map from ISFS
	u8 *content = ISFS_GetFile((u8 *) "/shared1/content.map", &size, 0);
	int items = size / sizeof(map_entry_t);

	//gprintf("Open content.map, size %d, items %d\n", size, items);

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

			//gprintf("Opened fontfile: %s: %d bytes\n", u8_font_filename, size);

			if (u8_font_archive != NULL)
			{
				const u8 *font_file = u8_get_file_by_index(u8_font_archive, 1, &size); // There is only one file in that app

				//gprintf("Extracted font: %d\n", size);

				m_base_font = smartMem2Alloc(size);
				memcpy(m_base_font.get(), font_file, size);
				if(!!m_base_font)
					m_base_font_size = size;
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

	SAFE_FREE(content);
}

void CMenu::_cleanupDefaultFont()
{
	SMART_FREE(m_base_font);
	m_base_font_size = 0;
}

const char *CMenu::_domainFromView()
{
	switch(m_current_view)
	{
		case COVERFLOW_CHANNEL:
			return "NAND";
		case COVERFLOW_HOMEBREW:
			return "HOMEBREW";
		default:
			return "GAMES";
	}
	return "NULL";
}

void CMenu::UpdateCache(u32 view)
{
	if(view == COVERFLOW_MAX)
	{
		UpdateCache(COVERFLOW_USB);
		UpdateCache(COVERFLOW_HOMEBREW);
		UpdateCache(COVERFLOW_CHANNEL);
		return;
	}

	const char *domain;
	switch(view)
	{
		case COVERFLOW_CHANNEL:
			domain = "NAND";
			break;
		case COVERFLOW_HOMEBREW:
			domain = "HOMEBREW";
			break;
		default:
			domain = "GAMES";
			break;
	}

	m_cfg.setBool(domain, "update_cache", true);
}