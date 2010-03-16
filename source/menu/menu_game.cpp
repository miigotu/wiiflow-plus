
#include "menu.hpp"
#include "loader/patchcode.h"
#include "loader/fs.h"
#include "loader/sys.h"
#include "loader/wdvd.h"
#include "loader/alt_ios.h"
#include "cheat.hpp"
#include <wiiuse/wpad.h>
#include <ogc/machine/processor.h>
#include <unistd.h>

#include "loader/wbfs.h"
#include "loader/usbstorage.h"
#include "loader/libwbfs/wiidisc.h"
#include "loader/frag.h"
#include "loader/fst.h"
#include "loader/wip.h"

using namespace std;

extern const u8 btngamecfg_png[];
extern const u8 btngamecfgs_png[];
extern const u8 stopkidon_png[];
extern const u8 stopkidons_png[];
extern const u8 stopkidoff_png[];
extern const u8 stopkidoffs_png[];
extern const u8 favoriteson_png[];
extern const u8 favoritesons_png[];
extern const u8 favoritesoff_png[];
extern const u8 favoritesoffs_png[];
extern const u8 delete_png[];
extern const u8 deletes_png[];

extern int mainIOS;

const CMenu::SOption CMenu::_languages[11] = {
	{ "lngdef", L"Default" },
	{ "lngjap", L"Japanese" },
	{ "lngeng", L"English" },
	{ "lngger", L"German" },
	{ "lngfre", L"French" },
	{ "lngspa", L"Spanish" },
	{ "lngita", L"Italian" },
	{ "lngdut", L"Dutch" },
	{ "lngsch", L"S. Chinese" },
	{ "lngtch", L"T. Chinese" },
	{ "lngkor", L"Korean" }
};

const CMenu::SOption CMenu::_videoModes[7] = {
	{ "viddef", L"Default" },
	{ "vidp50", L"PAL 50Hz" },
	{ "vidp60", L"PAL 60Hz" },
	{ "vidntsc", L"NTSC" },
	{ "vidpatch", L"Auto Patch" },
	{ "vidsys", L"System" },	
	{ "vidprog", L"Progressive" }
};

const CMenu::SOption CMenu::_vidModePatch[4] = {
	{ "vmpnone", L"None" },
	{ "vmpnormal", L"Normal" },
	{ "vmpmore", L"More" },
	{ "vmpall", L"All" }
};

const int CMenu::_ios[5] = {0, 249, 250, 222, 223};

static inline int loopNum(int i, int s)
{
	return i < 0 ? (s - (-i % s)) % s : i % s;
}

void CMenu::_hideGame(bool instant)
{
	m_btnMgr.hide(m_gameBtnPlay, instant);
	m_btnMgr.hide(m_gameBtnDelete, instant);
	m_btnMgr.hide(m_gameBtnSettings, instant);
	m_btnMgr.hide(m_gameBtnBack, instant);
	m_btnMgr.hide(m_gameBtnFavoriteOn, instant);
	m_btnMgr.hide(m_gameBtnFavoriteOff, instant);
	m_btnMgr.hide(m_gameBtnAdultOn, instant);
	m_btnMgr.hide(m_gameBtnAdultOff, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_gameLblUser); ++i)
		if (m_gameLblUser[i] != -1u)
			m_btnMgr.hide(m_gameLblUser[i], instant);
}

void CMenu::_showGame(void)
{
	_setBg(m_gameBg, m_gameBgLQ);
	m_btnMgr.show(m_gameBtnPlay);
	m_btnMgr.show(m_gameBtnBack);
	for (u32 i = 0; i < ARRAY_SIZE(m_gameLblUser); ++i)
		if (m_gameLblUser[i] != -1u)
			m_btnMgr.show(m_gameLblUser[i]);
}

static void setLanguage(int l)
{
	if (l > 0 && l <= 10)
		configbytes[0] = l - 1;
	else
		configbytes[0] = 0xCD;
}

void CMenu::_game(bool launch)
{
	s32 padsState;
	WPADData *wd;
	u32 btn;
	bool b;
	bool first = true;

	WPAD_Rumble(WPAD_CHAN_0, 0);
	if (!launch)
	{
		WPAD_ScanPads();
		_playGameSound();
		_showGame();
	}
	while (true)
	{
		string id(m_cf.getId());
		if (!first)
			WPAD_ScanPads();
		else
			first = false;
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
		if ((padsState & WPAD_BUTTON_1) != 0)
		{
			int cfVersion = 1 + loopNum(m_cfg.getInt(" GENERAL", "last_cf_mode", 1), m_numCFVersions);
			_loadCFLayout(cfVersion);
			m_cf.applySettings();
			m_cfg.setInt(" GENERAL", "last_cf_mode", cfVersion);
		}
		else if ((padsState & WPAD_BUTTON_2) != 0)
		{
			int cfVersion = 1 + loopNum(m_cfg.getInt(" GENERAL", "last_cf_mode", 1) - 2, m_numCFVersions);
			_loadCFLayout(cfVersion);
			m_cf.applySettings();
			m_cfg.setInt(" GENERAL", "last_cf_mode", cfVersion);
		}
		if (launch || (padsState & WPAD_BUTTON_A) != 0)
		{
			m_btnMgr.click();
			if (m_btnMgr.selected() == m_mainBtnQuit)
				break;
			else if (m_btnMgr.selected() == m_gameBtnDelete)
			{
				if (!WBFS_IsReadOnly())
				{
					_hideGame();
					_waitForGameSoundExtract();
					if (_wbfsOp(CMenu::WO_REMOVE_GAME))
						break;
					_showGame();
				}
				else
				{
					error(_t("wbfsop10", L"This filesystem is read-only. You cannot install games or remove them."));
				}
			}
			else if (m_btnMgr.selected() == m_gameBtnFavoriteOn || m_btnMgr.selected() == m_gameBtnFavoriteOff)
				m_cfg.setBool(id, "favorite", !m_cfg.getBool(id, "favorite", false));
			else if (m_btnMgr.selected() == m_gameBtnAdultOn || m_btnMgr.selected() == m_gameBtnAdultOff)
				m_cfg.setBool(id, "adult_only", !m_cfg.getBool(id, "adult_only", false));
			else if (m_btnMgr.selected() == m_gameBtnBack)
				break;
			else if (m_btnMgr.selected() == m_gameBtnSettings)
			{
				_hideGame();
				_waitForGameSoundExtract();
				_gameSettings();
				_showGame();
			}
			else if (launch || m_btnMgr.selected() == m_gameBtnPlay || (!wd->ir.valid && m_btnMgr.selected() == (u32)-1))
			{
				_hideGame();
				m_cf.clear();
				m_vid.waitMessage(m_waitMessage);
				_launchGame(id);
				launch = false;
				WPAD_SetVRes(0, m_vid.width() + m_cur.width(), m_vid.height() + m_cur.height());	// b/c IOS reload
				_showGame();
				_initCF();
				m_cf.select();
			}
			else if (m_cf.mouseOver(m_vid, m_cur.x(), m_cur.y()))
				m_cf.flip();
		}
		if ((padsState & (WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT | WPAD_BUTTON_MINUS | WPAD_BUTTON_PLUS)) != 0)
		{
			if ((padsState & WPAD_BUTTON_MINUS) != 0)
				m_cf.up();
			else if ((padsState & WPAD_BUTTON_LEFT) != 0)
				m_cf.left();
			else if ((padsState & WPAD_BUTTON_PLUS) != 0)
				m_cf.down();
			else // ((padsState & WPAD_BUTTON_RIGHT) != 0)
				m_cf.right();
			_showGame();
			_playGameSound();
		}
		// 
		if (wd->ir.valid)
		{
			b = m_cfg.getBool(id, "favorite", false);
			m_btnMgr.show(b ? m_gameBtnFavoriteOn : m_gameBtnFavoriteOff);
			m_btnMgr.hide(b ? m_gameBtnFavoriteOff : m_gameBtnFavoriteOn);

			if (!m_locked)
			{
				b = m_cfg.getBool(id, "adult_only", false);
				m_btnMgr.show(b ? m_gameBtnAdultOn : m_gameBtnAdultOff);
				m_btnMgr.hide(b ? m_gameBtnAdultOff : m_gameBtnAdultOn);
				m_btnMgr.show(m_gameBtnDelete);
				m_btnMgr.show(m_gameBtnSettings);
			}
		}
		else
		{
			m_btnMgr.hide(m_gameBtnFavoriteOn);
			m_btnMgr.hide(m_gameBtnFavoriteOff);
			m_btnMgr.hide(m_gameBtnAdultOn);
			m_btnMgr.hide(m_gameBtnAdultOff);
			m_btnMgr.hide(m_gameBtnDelete);
			m_btnMgr.hide(m_gameBtnSettings);
		}
		_mainLoopCommon(wd, true);
	}
	WPAD_Rumble(WPAD_CHAN_0, 0);
	_waitForGameSoundExtract();
	_hideGame();
}

static SmartBuf extractDOL(const char *dolName, u32 &size, const char *gameId)
{
	SmartBuf dolFile;
	Disc_SetWBFS(0, NULL);
	wbfs_disc_t *disc = WBFS_OpenDisc((u8 *)gameId);
	wiidisc_t *wdisc = wd_open_disc((int (*)(void *, u32, u32, void *))wbfs_disc_read, disc);
	dolFile = SmartBuf(wd_extract_file(wdisc, &size, ALL_PARTITIONS, dolName, ALLOC_COVER), SmartBuf::SRCALL_COVER);
	wd_close_disc(wdisc);
	WBFS_CloseDisc(disc);
	return dolFile;
}

void CMenu::_launchGame(const string &id)
{
	bool vipatch = m_cfg.testOptBool(id, "vipatch", m_cfg.getBool(" GENERAL", "vipatch", false));
	bool cheat = m_cfg.testOptBool(id, "cheat", m_cfg.getBool(" GENERAL", "cheat", false));
	bool countryPatch = m_cfg.testOptBool(id, "country_patch", m_cfg.getBool(" GENERAL", "country_patch", false));
	bool err002Fix = m_cfg.testOptBool(id, "error_002_fix", m_cfg.getBool(" GENERAL", "error_002_fix", true));
	u8 videoMode = (u8)min((u32)m_cfg.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_videoModes) - 1u);
	string altdol = m_cfg.getString(id, "dol");
	int language = min((u32)m_cfg.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
	int iosNum = CMenu::_ios[min((u32)m_cfg.getInt(id, "ios", 0), ARRAY_SIZE(CMenu::_ios) - 1u)];
	if (iosNum == 0)
		iosNum = mainIOS;
	bool mload = iosNum == 222 || iosNum == 223;
	int minIOSRev = mload ? IOS_222_MIN_REV : IOS_249_MIN_REV;
	bool blockIOSReload = m_cfg.getBool(id, "block_ios_reload", false);
	u8 patchVidMode = min((u32)m_cfg.getInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
	SmartBuf cheatFile;
	u32 cheatSize = 0;
	SmartBuf dolFile;
	u32 dolSize = 0;
	bool iosLoaded = false;

	_waitForGameSoundExtract();
	if (videoMode == 0)
		videoMode = (u8)min((u32)m_cfg.getInt(" GENERAL", "video_mode", 0), ARRAY_SIZE(CMenu::_videoModes) - 1);
	if (language == 0)
		language = min((u32)m_cfg.getInt(" GENERAL", "game_language", 0), ARRAY_SIZE(CMenu::_languages) - 1);
	if (strcasecmp(altdol.c_str(), "main.dol") == 0)
		altdol.clear();
	m_cfg.setString(" GENERAL", "current_game", id);
	m_cfg.save();
	setLanguage(language);
	_stopSounds(); // fix: code dump with IOS 222/223 when music is playing
	if (iosNum != mainIOS || !_networkFix())
	{
		if (!loadIOS(iosNum, true))
		{
			error(sfmt("Couldn't load IOS %i", iosNum));
			return;
		}
		iosLoaded = true;
	}
	COVER_clear();
	if (IOS_GetRevision() < minIOSRev)
	{
		error(sfmt("IOS %i rev %i or higher is required.\nPlease install the latest version.", iosNum, minIOSRev));
		Sys_LoadMenu();
	}
	if (mload && blockIOSReload)
		disableIOSReload();
	if (!altdol.empty())
		dolFile = extractDOL(altdol.c_str(), dolSize, id.c_str());
	if (cheat)
		loadCheatFile(cheatFile, cheatSize, m_cheatDir.c_str(), id.c_str());
	
	load_bca_code((u8 *) m_bcaDir.c_str(), (u8 *) id.c_str());
	load_wip_patches((u8 *) m_wipDir.c_str(), (u8 *) id.c_str());
	ocarina_load_code((u8 *) id.c_str(), cheatFile.get(), cheatSize);

	if (get_frag_list((u8 *)id.c_str()) < 0)
	{
		if (iosLoaded)
			Sys_LoadMenu();
		return;
	}

	if (set_frag_list((u8 *)id.c_str()) < 0)
	{
		if (iosLoaded)
			Sys_LoadMenu();
		return;
	}

	if (Disc_SetUSB((u8 *)id.c_str()) < 0)
	{
		error(L"Disc_SetWBFS failed");
		if (iosLoaded)
			Sys_LoadMenu();
		return;
	}
	
	WBFS_Close();
	
	if (Disc_Open() < 0)
	{
		error(L"Disc_Open failed");
		if (iosLoaded) 	
			Sys_LoadMenu();
		return;
	}
	Fat_Unmount();
	cleanup();
	USBStorage_Deinit();
	if (Disc_WiiBoot(videoMode, cheatFile.get(), cheatSize, vipatch, countryPatch, err002Fix, dolFile.get(), dolSize, patchVidMode) < 0)
	{
		Sys_LoadMenu();
	}
}

bool CMenu::_networkFix(void)
{
    if (!m_networkInit)
		return true;
	s32 kd_fd;
	s32 ret = -1;
	STACK_ALIGN(u8, kd_buf, 32, 32);

	kd_fd = IOS_Open("/dev/net/kd/request", 0);
	if (kd_fd >= 0)
	{
		ret = IOS_Ioctl(kd_fd, 7, NULL, 0, kd_buf, 32);
		IOS_Close(kd_fd);
	}
	return ret >= 0;
}

void CMenu::_initGameMenu(CMenu::SThemeData &theme)
{
	CColor fontColor(0xD0BFDFFF);
	STexture texFavOn;
	STexture texFavOnSel;
	STexture texFavOff;
	STexture texFavOffSel;
	STexture texAdultOn;
	STexture texAdultOnSel;
	STexture texAdultOff;
	STexture texAdultOffSel;
	STexture texDelete;
	STexture texDeleteSel;
	STexture texSettings;
	STexture texSettingsSel;
	STexture bgLQ;

	texFavOn.fromPNG(favoriteson_png);
	texFavOnSel.fromPNG(favoritesons_png);
	texFavOff.fromPNG(favoritesoff_png);
	texFavOffSel.fromPNG(favoritesoffs_png);
	texAdultOn.fromPNG(stopkidon_png);
	texAdultOnSel.fromPNG(stopkidons_png);
	texAdultOff.fromPNG(stopkidoff_png);
	texAdultOffSel.fromPNG(stopkidoffs_png);
	texDelete.fromPNG(delete_png);
	texDeleteSel.fromPNG(deletes_png);
	texSettings.fromPNG(btngamecfg_png);
	texSettingsSel.fromPNG(btngamecfgs_png);
	_addUserLabels(theme, m_gameLblUser, ARRAY_SIZE(m_gameLblUser), "GAME");
	m_gameBg = _texture(theme.texSet, "GAME/BG", "texture", theme.bg);
	if (m_theme.loaded() && STexture::TE_OK == bgLQ.fromPNGFile(sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString("GAME/BG", "texture").c_str()).c_str(), GX_TF_CMPR, ALLOC_MEM2, 64, 64))
		m_gameBgLQ = bgLQ;
	m_gameBtnPlay = _addButton(theme, "GAME/PLAY_BTN", theme.btnFont, L"", 420, 354, 200, 56, fontColor);
	m_gameBtnBack = _addButton(theme, "GAME/BACK_BTN", theme.btnFont, L"", 420, 410, 200, 56, fontColor);
	m_gameBtnFavoriteOn = _addPicButton(theme, "GAME/FAVORITE_ON", texFavOn, texFavOnSel, 460, 170, 48, 48);
	m_gameBtnFavoriteOff = _addPicButton(theme, "GAME/FAVORITE_OFF", texFavOff, texFavOffSel, 460, 170, 48, 48);
	m_gameBtnAdultOn = _addPicButton(theme, "GAME/ADULTONLY_ON", texAdultOn, texAdultOnSel, 532, 170, 48, 48);
	m_gameBtnAdultOff = _addPicButton(theme, "GAME/ADULTONLY_OFF", texAdultOff, texAdultOffSel, 532, 170, 48, 48);
	m_gameBtnSettings = _addPicButton(theme, "GAME/SETTINGS_BTN", texSettings, texSettingsSel, 460, 242, 48, 48);
	m_gameBtnDelete = _addPicButton(theme, "GAME/DELETE_BTN", texDelete, texDeleteSel, 532, 242, 48, 48);
	// 
	_setHideAnim(m_gameBtnPlay, "GAME/PLAY_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameBtnBack, "GAME/BACK_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameBtnFavoriteOn, "GAME/FAVORITE_ON", 0, 0, -1.5f, -1.5f);
	_setHideAnim(m_gameBtnFavoriteOff, "GAME/FAVORITE_OFF", 0, 0, -1.5f, -1.5f);
	_setHideAnim(m_gameBtnAdultOn, "GAME/ADULTONLY_ON", 0, 0, -1.5f, -1.5f);
	_setHideAnim(m_gameBtnAdultOff, "GAME/ADULTONLY_OFF", 0, 0, -1.5f, -1.5f);
	_setHideAnim(m_gameBtnSettings, "GAME/SETTINGS_BTN", 0, 0, -1.5f, -1.5f);
	_setHideAnim(m_gameBtnDelete, "GAME/DELETE_BTN", 0, 0, -1.5f, -1.5f);
	_hideGame(true);
	_textGame();
}

void CMenu::_textGame(void)
{
	m_btnMgr.setText(m_gameBtnPlay, _t("gm1", L"Play"));
	m_btnMgr.setText(m_gameBtnBack, _t("gm2", L"Back"));
}

// 

static void _extractBnr(SmartBuf &bnr, u32 &size, const string &gameId)
{
	bnr.release();
	Disc_SetWBFS(0, NULL);
	wbfs_disc_t *disc = WBFS_OpenDisc((u8 *)gameId.c_str());
	if (disc != NULL)
	{
		wiidisc_t *wdisc = wd_open_disc((int (*)(void *, u32, u32, void *))wbfs_disc_read, disc);
		if (wdisc != NULL)
		{
			bnr = SmartBuf(wd_extract_file(wdisc, &size, ALL_PARTITIONS, "opening.bnr", ALLOC_MEM2), SmartBuf::SRCALL_MEM2);
			wd_close_disc(wdisc);
		}
		WBFS_CloseDisc(disc);
	}
}

struct IMD5Header
{
	u32 fcc;
	u32 filesize;
	u8 zeroes[8];
	u8 crypto[16];
} __attribute__((packed));

struct IMETHeader
{
	u8 zeroes[64];
	u32 fcc;
	u8 unk[8];
	u32 iconSize;
	u32 bannerSize;
	u32 soundSize;
	u32 flag1;
	u8 names[7][84];
	u8 zeroes_2[0x348];
	u8 crypto[16];
} __attribute__((packed));

struct U8Header
{
	u32 fcc;
	u32 rootNodeOffset;
	u32 headerSize;
	u32 dataOffset;
	u8 zeroes[16];
} __attribute__((packed));

struct U8Entry
{
	struct
	{
		u32 fileType : 8;
		u32 nameOffset : 24;
	};
	u32 fileOffset;
	union
	{
		u32 fileLength;
		u32 numEntries;
	};
} __attribute__((packed));

struct LZ77Info
{
	u16 length : 4;
	u16 offset : 12;
} __attribute__((packed));

static char *u8Filename(const U8Entry *fst, int i)
{
	return (char *)(fst + fst[0].numEntries) + fst[i].nameOffset;
}

inline u32 le32(u32 i)
{
	return ((i & 0xFF) << 24) | ((i & 0xFF00) << 8) | ((i & 0xFF0000) >> 8) | ((i & 0xFF000000) >> 24);
}

SmartBuf uncompressLZ77(u32 &size, const u8 *inputBuf, u32 inputLength)
{
	SmartBuf buffer;
	if (inputLength <= 0x8 || *(const u32 *)inputBuf != 'LZ77' || inputBuf[4] != 0x10)
		return buffer;
	u32 uncSize = le32(((const u32 *)inputBuf)[1] << 8);
	const u8 *inBuf = inputBuf + 8;
	const u8 *inBufEnd = inputBuf + inputLength;
	buffer = smartMem2Alloc(uncSize);
	if (!buffer)
		return buffer;
	u8 *bufCur = buffer.get();
	u8 *bufEnd = buffer.get() + uncSize;
	while (bufCur < bufEnd && inBuf < inBufEnd)
	{
		u8 flags = *inBuf;
		++inBuf;
		for (int i = 0; i < 8 && bufCur < bufEnd && inBuf < inBufEnd; ++i)
		{
			if ((flags & 0x80) != 0)
			{
				const LZ77Info &info = *(const LZ77Info *)inBuf;
				inBuf += sizeof (LZ77Info);
				int length = info.length + 3;
				if (bufCur - info.offset - 1 < buffer.get() || bufCur + length > bufEnd)
					return buffer;
				memcpy(bufCur, bufCur - info.offset - 1, length);
				bufCur += length;
			}
			else
			{
				*bufCur = *inBuf;
				++inBuf;
				++bufCur;
			}
			flags <<= 1;
		}
	}
	size = uncSize;
	return buffer;
}

void CMenu::_loadGameSound(const std::string &id)
{
	SmartBuf bnr;
	const u8 *soundBin;
	const u8 *bnrArc;
	u32 bnrSize;
	u32 sndType;
	const U8Entry *fst;
	u32 i;
	const u8 *soundChunk;
	u32 soundChunkSize;
	SmartBuf uncompressed;

	_extractBnr(bnr, bnrSize, id);
	if (!bnr)
		return;
	const IMETHeader &imetHdr = *(IMETHeader *)bnr.get();
	if (imetHdr.fcc != 'IMET')
		return;
	bnrArc = (const u8 *)(&imetHdr + 1);
	const U8Header &bnrArcHdr = *(U8Header *)bnrArc;
	fst = (const U8Entry *)(bnrArc + bnrArcHdr.rootNodeOffset);
	for (i = 1; i < fst[0].numEntries; ++i)
		if (fst[i].fileType == 0 && strcasecmp(u8Filename(fst, i), "sound.bin") == 0)
			break;
	if (i >= fst[0].numEntries)
		return;
	soundBin = bnrArc + fst[i].fileOffset;
	if (((IMD5Header *)soundBin)->fcc != 'IMD5')
		return;
	soundChunk = soundBin + sizeof (IMD5Header);
	sndType = *(u32 *)soundChunk;
	soundChunkSize = fst[i].fileLength - sizeof (IMD5Header);
	if (sndType == 'LZ77')
	{
		u32 uncSize;
		uncompressed = uncompressLZ77(uncSize, soundChunk, soundChunkSize);
		if (!uncompressed)
			return;
		soundChunk = uncompressed.get();
		soundChunkSize = uncSize;
		sndType = *(u32 *)soundChunk;
	}
	switch (sndType)
	{
		case 'RIFF':
			LWP_MutexLock(m_gameSndMutex);
			m_gameSoundTmp.fromWAV(soundChunk, soundChunkSize);
			LWP_MutexUnlock(m_gameSndMutex);
			break;
		case 'BNS ':
			LWP_MutexLock(m_gameSndMutex);
			m_gameSoundTmp.fromBNS(soundChunk, soundChunkSize);
			LWP_MutexUnlock(m_gameSndMutex);
			break;
		case 'FORM':
			LWP_MutexLock(m_gameSndMutex);
			m_gameSoundTmp.fromAIFF(soundChunk, soundChunkSize);
			LWP_MutexUnlock(m_gameSndMutex);
			break;
	}
}

int CMenu::_loadGameSoundThrd(CMenu *m)
{
	string prevId;
	string id;

	LWP_MutexLock(m->m_gameSndMutex);
	id = m->m_gameSoundId;
	m->m_gameSoundId.clear();
	LWP_MutexUnlock(m->m_gameSndMutex);
	while (id != prevId && !id.empty())
	{
		prevId = id;
		m->_loadGameSound(id);
		LWP_MutexLock(m->m_gameSndMutex);
		id = m->m_gameSoundId;
		m->m_gameSoundId.clear();
		LWP_MutexUnlock(m->m_gameSndMutex);
	}
	m->m_gameSoundThread = 0;
	return 0;
}

void CMenu::_playGameSound(void)
{
	if (m_bnrSndVol == 0)
		return;
	LWP_MutexLock(m_gameSndMutex);
	m_gameSoundId = m_cf.getId();
	LWP_MutexUnlock(m_gameSndMutex);
	m_cf.stopPicLoader();
	if (m_gameSoundThread == 0)
		LWP_CreateThread(&m_gameSoundThread, (void *(*)(void *))CMenu::_loadGameSoundThrd, (void *)this, 0, 8 * 1024, 40);
}

void CMenu::_waitForGameSoundExtract(void)
{
	for (int i = 0; i < 30 && m_gameSoundThread != 0; ++i)	// 3 s
		usleep(100000);
	if (m_gameSoundThread != 0)
	{
		error(L"Error while reading a game disc");
		Sys_Reboot();
	}
}
