
#include "menu.hpp"
#include "svnrev.h"
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "http.h"
#include "pngu.h"

#include "loader/fs.h"
#include "loader/wdvd.h"
#include "loader/usbstorage.h"

#include <network.h>

#include "gecko.h"
#include <fstream>

#define TAG_GAME_ID		"gameid"
#define TAG_LOC			"loc"
#define TAG_REGION		"region"
#define TITLES_URL		"http://www.wiitdb.com/titles.txt?LANG=%s"
//Need to use wiiflow.ini to set these still
#define UPDATE_URL_VERSION	"http://update.wiiflow.org/txt/versions.txt"

using namespace std;

static const char FMT_BPIC6_URL[] = "http://wiitdb.com/wiitdb/artwork/coverfullHQ/{loc}/{gameid6}.png"\
"|http://wiitdb.com/wiitdb/artwork/coverfullHQ/EN/{gameid6}.png"\
"|http://wiitdb.com/wiitdb/artwork/coverfull/{loc}/{gameid6}.png"\
"|http://wiitdb.com/wiitdb/artwork/coverfull/EN/{gameid6}.png";
static const char FMT_BPIC4_URL[] = "http://wiitdb.com/wiitdb/artwork/coverfullHQ/{loc}/{gameid4}.png"\
"|http://wiitdb.com/wiitdb/artwork/coverfullHQ/EN/{gameid4}.png"\
"|http://wiitdb.com/wiitdb/artwork/coverfull/{loc}/{gameid4}.png"\
"|http://wiitdb.com/wiitdb/artwork/coverfull/EN/{gameid4}.png";
static const char FMT_PIC6_URL[] = "http://wiitdb.com/wiitdb/artwork/cover/{loc}/{gameid6}.png"\
"|http://wiitdb.com/wiitdb/artwork/cover/EN/{gameid6}.png";
static const char FMT_PIC4_URL[] = "http://wiitdb.com/wiitdb/artwork/cover/{loc}/{gameid4}.png"\
"|http://wiitdb.com/wiitdb/artwork/cover/EN/{gameid4}.png";

class LockMutex
{
	mutex_t &m_mutex;
public:
	LockMutex(mutex_t &m) : m_mutex(m) { LWP_MutexLock(m_mutex); }
	~LockMutex(void) { LWP_MutexUnlock(m_mutex); }
};

static string countryCode(const string &gameId)
{
	switch (gameId[3])
	{
		case 'E':
			return "US";
		case 'J':
			return "JA";
		case 'W':
			return "ZH";
		case 'K':
			return "KO";
		case 'R':
			return "RU";
		case 'P':
		case 'D':
		case 'F':
		case 'I':
		case 'S':
		case 'H':
		case 'X':
		case 'Y':
		case 'Z':
			switch (CONF_GetArea())
			{
				case CONF_AREA_BRA:
					return "PT";
				case CONF_AREA_AUS:
					return "AU";
			}
			switch (CONF_GetLanguage())
			{
				case CONF_LANG_ENGLISH:
					return "EN";
				case CONF_LANG_GERMAN:
					return "DE";
				case CONF_LANG_FRENCH:
					return "FR";
				case CONF_LANG_SPANISH:
					return "ES";
				case CONF_LANG_ITALIAN:
					return "IT";
				case CONF_LANG_DUTCH:
					return "NL";
			}
		case 'A':
			switch (CONF_GetArea())
			{
				case CONF_AREA_USA:
					return "US";
				case CONF_AREA_JPN:
					return "JA";
				case CONF_AREA_CHN:
				case CONF_AREA_HKG:
				case CONF_AREA_TWN:
					return "ZH";
				case CONF_AREA_KOR:
					return "KO";
				case CONF_AREA_BRA:
					return "PT";
				case CONF_AREA_AUS:
					return "AU";
			}
			switch (CONF_GetLanguage())
			{
				case CONF_LANG_ENGLISH:
					return "EN";
				case CONF_LANG_GERMAN:
					return "DE";
				case CONF_LANG_FRENCH:
					return "FR";
				case CONF_LANG_SPANISH:
					return "ES";
				case CONF_LANG_ITALIAN:
					return "IT";
				case CONF_LANG_DUTCH:
					return "NL";
			}
	}
	return "other";
}

static string makeURL(const string &format, const string &gameId, const string &country)
{
	string url;
	string::size_type i = 0;
	string::size_type j = 0;
	string k;

	if (format.empty())
		return string();
	url.reserve(format.size() + 42);
	while (i != string::npos)
	{
		j = format.find_first_of('{', i);
		if (j != i)
			url += format.substr(i, j == string::npos ? string::npos : j - i);
		i = j;
		if (i != string::npos)
		{
			++i;
			j = format.find_first_of('}', i);
			if (j == string::npos)
				break;
			if (j > i + 1)
			{
				k = format.substr(i, j - i);
				if (k.substr(0, k.size() - 1) == TAG_GAME_ID && k[k.size() - 1] >= '3' && k[k.size() - 1] <= '6')
					url += gameId.substr(0, (string::size_type)(k[k.size() - 1] - '0'));
				else if (k == TAG_LOC)
					url += country;
				else if (k == TAG_REGION)
					switch(gameId[3])
					{
					case 'E':
						url += "ntsc";
						break;
					case 'J':
						url += "ntscj";
						break;
					default:
						url += "pal";
					}
			}
			i = j + 1;
		}
	}
	return url;
}

void CMenu::_hideDownload(bool instant)
{
	m_btnMgr.hide(m_downloadLblTitle, instant);
	m_btnMgr.hide(m_downloadBtnCancel, instant);
	m_btnMgr.hide(m_downloadBtnAll, instant);
	m_btnMgr.hide(m_downloadBtnMissing, instant);
	m_btnMgr.hide(m_downloadBtnAllTitles, instant);
	m_btnMgr.hide(m_downloadBtnMissingTitles, instant);
	m_btnMgr.hide(m_downloadPBar, instant);
	m_btnMgr.hide(m_downloadLblMessage[0], 0, 0, -2.f, 0.f, instant);
	m_btnMgr.hide(m_downloadLblMessage[1], 0, 0, -2.f, 0.f, instant);
	m_btnMgr.hide(m_downloadLblCovers, instant);
	m_btnMgr.hide(m_downloadLblGameTitles, instant);
	m_btnMgr.hide(m_downloadLblWiiTDB, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_downloadLblUser); ++i)
		if (m_downloadLblUser[i] != -1u)
			m_btnMgr.hide(m_downloadLblUser[i], instant);
}

void CMenu::_showDownload(void)
{
	_setBg(m_downloadBg, m_downloadBg);
	m_btnMgr.show(m_downloadLblWiiTDB);
	m_btnMgr.show(m_downloadLblTitle);
	m_btnMgr.show(m_downloadBtnCancel);
	m_btnMgr.show(m_downloadBtnAll);
	m_btnMgr.show(m_downloadBtnMissing);
	m_btnMgr.show(m_downloadLblCovers);
	if (!m_locked)
	{
		m_btnMgr.show(m_downloadLblGameTitles);
		m_btnMgr.show(m_downloadBtnAllTitles);
		m_btnMgr.show(m_downloadBtnMissingTitles);
	}
	for (u32 i = 0; i < ARRAY_SIZE(m_downloadLblUser); ++i)
		if (m_downloadLblUser[i] != -1u)
			m_btnMgr.show(m_downloadLblUser[i]);
}

void CMenu::_setThrdMsg(const wstringEx &msg, float progress)
{
	if (m_thrdStop)
		return;
	if (msg != L"...")
		m_thrdMessage = msg;
	m_thrdMessageAdded = true;
	m_thrdProgress = progress;
}

bool CMenu::_downloadProgress(void *obj, int size, int position)
{
	CMenu *m = (CMenu *)obj;
	LWP_MutexLock(m->m_mutex);
	m->_setThrdMsg(L"...", m->m_thrdStep + m->m_thrdStepLen * ((float)position / (float)size));
	LWP_MutexUnlock(m->m_mutex);
	return !m->m_thrdStop;
}

int CMenu::_coverDownloaderAll(CMenu *m)
{
	if (!m->m_thrdWorking)
		return 0;
	m->_coverDownloader(false);
	m->m_thrdWorking = false;
	return 0;
}

int CMenu::_coverDownloaderMissing(CMenu *m)
{
	if (!m->m_thrdWorking)
		return 0;
	m->_coverDownloader(true);
	m->m_thrdWorking = false;
	return 0;
}

static bool checkPNGBuf(u8 *data)
{
	IMGCTX ctx;
	PNGUPROP imgProp;
	int ret;

	ctx = PNGU_SelectImageFromBuffer(data);
	if (ctx == NULL)
		return false;
	ret = PNGU_GetImageProperties(ctx, &imgProp);
	PNGU_ReleaseImageContext(ctx);
	return ret == PNGU_OK;
}

static bool checkPNGFile(const char *filename)
{
	FILE *file = NULL;
	long fileSize = 0;
	SmartBuf ptrPng;

	file = fopen(filename, "rb");
	if (file == NULL)
		return false;
	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	if (fileSize > 0)
	{
		ptrPng = smartCoverAlloc(fileSize);
		if (!!ptrPng)
			fread(ptrPng.get(), 1, fileSize, file);
	}
	fclose(file);
	return !ptrPng ? false : checkPNGBuf(ptrPng.get());
}

int CMenu::_initNetwork(char *ip)
{
	return if_config(ip, NULL, NULL, true);
}

int CMenu::_coverDownloader(bool missingOnly)
{
	u32 n;
	u32 nbSteps;
	u32 step;
	float listWeight = missingOnly ? 0.125f : 0.f;	// 1/8 of the progress bar for testing the PNGs we already have
	float dlWeight = 1.f - listWeight;
	int count = 0;
	int countFlat = 0;
	FILE *file = NULL;
	char ip[16];
	string url;
	string path;
	block png;
	vector<string> coverList;
	bool savePNG;
	bool success;
	vector<string> fmtURLFlat;
	vector<string> fmtURLBox;
	u32 bufferSize = 0x280000;	// Maximum download size 2 MB
	SmartBuf buffer;

	// Use the cover space as a temporary buffer
	buffer = smartCoverAlloc(bufferSize);
	if (!buffer)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(L"Not enough memory!", 1.f);
		LWP_MutexUnlock(m_mutex);
		m_thrdWorking = false;
		return 0;
	}
	savePNG = m_cfg.getBool(" GENERAL", "keep_png", true);

	if (m_current_view == COVERFLOW_CHANNEL) 
	{
		fmtURLBox = stringToVector(m_cfg.getString(" GENERAL", "url_full_covers_id4", FMT_BPIC4_URL), '|');
		fmtURLFlat = stringToVector(m_cfg.getString(" GENERAL", "url_flat_covers_id4", FMT_PIC4_URL), '|');
	}
	else
	{
		fmtURLBox = stringToVector(m_cfg.getString(" GENERAL", "url_full_covers", FMT_BPIC6_URL), '|');
		fmtURLFlat = stringToVector(m_cfg.getString(" GENERAL", "url_flat_covers", FMT_PIC6_URL), '|');
	}
	nbSteps = m_gameList.size();
	step = 0;
	if (m_coverDLGameId.empty())
	{
		coverList.reserve(m_gameList.size());
		for (u32 i = 0; i < m_gameList.size() && !m_thrdStop; ++i)
		{
			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg7", L"Listing covers to download..."), listWeight * (float)step / (float)nbSteps);
			LWP_MutexUnlock(m_mutex);
			++step;
			string id((const char *)m_gameList[i].id, sizeof m_gameList[i].id);
			path = sfmt("%s/%s.png", m_boxPicDir.c_str(), id.c_str());
			if (!missingOnly || (!m_cf.fullCoverCached(id.c_str()) && !checkPNGFile(path.c_str())))
				coverList.push_back(id);
		}
	}
	else
		coverList.push_back(m_coverDLGameId);
	n = coverList.size();
	if (n > 0 && !m_thrdStop)
	{
		step = 0;
		nbSteps = 1 + n * 2;
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg1", L"Initializing network..."), listWeight + dlWeight * (float)step / (float)nbSteps);
		LWP_MutexUnlock(m_mutex);
		if (!m_networkInit && _initNetwork(ip) < 0)
		{
			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg2", L"Network initialization failed!"), 1.f);
			LWP_MutexUnlock(m_mutex);
			m_thrdWorking = false;
			return 0;
		}
		m_thrdStepLen = dlWeight / (float)nbSteps;
		m_networkInit = true;
		for (u32 i = 0; i < coverList.size() && !m_thrdStop; ++i)
		{
			// Try to get the full cover
			success = false;
			for (u32 j = 0; !success && j < fmtURLBox.size() && !m_thrdStop; ++j)
			{
				url = makeURL(fmtURLBox[j], coverList[i], countryCode(coverList[i]));
				if (j == 0)
					++step;
				m_thrdStep = listWeight + dlWeight * (float)step / (float)nbSteps;
				LWP_MutexLock(m_mutex);
				_setThrdMsg(wfmt(_fmt("dlmsg3", L"Downloading from %s"), url.c_str()), m_thrdStep);
				LWP_MutexUnlock(m_mutex);
				png = downloadfile(buffer.get(), bufferSize, url.c_str(), CMenu::_downloadProgress, this);
				if (png.data != NULL)
				{
					if (savePNG)
					{
						path = sfmt("%s/%s.png", m_boxPicDir.c_str(), coverList[i].c_str());
						LWP_MutexLock(m_mutex);
						_setThrdMsg(wfmt(_fmt("dlmsg4", L"Saving %s"), path.c_str()), listWeight + dlWeight * (float)(step + 1) / (float)nbSteps);
						LWP_MutexUnlock(m_mutex);
						file = fopen(path.c_str(), "wb");
						if (file != NULL)
						{
							fwrite(png.data, png.size, 1, file);
							fclose(file);
						}
					}

					LWP_MutexLock(m_mutex);
					_setThrdMsg(wfmt(_fmt("dlmsg10", L"Making %s"), sfmt("%s.wfc", coverList[i].c_str()).c_str()), listWeight + dlWeight * (float)(step + 1) / (float)nbSteps);
					LWP_MutexUnlock(m_mutex);
					if (m_cf.preCacheCover(coverList[i].c_str(), png.data, true))
					{
						++count;
						success = true;
					}
				}
			}
			if (!success && !m_thrdStop)
			{
				path = sfmt("%s/%s.png", m_picDir.c_str(), coverList[i].c_str());
				if (!checkPNGFile(path.c_str()))
				{
					// Try to get the front cover
					if (m_thrdStop)
						break;
					for (u32 j = 0; !success && j < fmtURLFlat.size() && !m_thrdStop; ++j)
					{
						url = makeURL(fmtURLFlat[j], coverList[i], countryCode(coverList[i]));
						LWP_MutexLock(m_mutex);
						_setThrdMsg(wfmt(_fmt("dlmsg8", L"Full cover not found. Downloading from %s"), url.c_str()), listWeight + dlWeight * (float)step / (float)nbSteps);
						LWP_MutexUnlock(m_mutex);
						png = downloadfile(buffer.get(), bufferSize, url.c_str(), CMenu::_downloadProgress, this);
						if (png.data != NULL)
						{
							if (savePNG)
							{
								LWP_MutexLock(m_mutex);
								_setThrdMsg(wfmt(_fmt("dlmsg4", L"Saving %s"), path.c_str()), listWeight + dlWeight * (float)(step + 1) / (float)nbSteps);
								LWP_MutexUnlock(m_mutex);
								file = fopen(path.c_str(), "wb");
								if (file != NULL)
								{
									fwrite(png.data, png.size, 1, file);
									fclose(file);
								}
							}

							LWP_MutexLock(m_mutex);
							_setThrdMsg(wfmt(_fmt("dlmsg10", L"Making %s"), sfmt("%s.wfc", coverList[i].c_str()).c_str()), listWeight + dlWeight * (float)(step + 1) / (float)nbSteps);
							LWP_MutexUnlock(m_mutex);
							if (m_cf.preCacheCover(coverList[i].c_str(), png.data, false))
							{
								++countFlat;
								success = true;
							}
						}
					}
				}
			}
			++step;
		}
	}
	LWP_MutexLock(m_mutex);
	if (countFlat == 0)
		_setThrdMsg(wfmt(_fmt("dlmsg5", L"%i/%i files downloaded."), count, n), 1.f);
	else
		_setThrdMsg(wfmt(_fmt("dlmsg9", L"%i/%i files downloaded. %i are front covers only."), count + countFlat, n, countFlat), 1.f);
	LWP_MutexUnlock(m_mutex);
	m_thrdWorking = false;
	return 0;
}

void CMenu::_download(string gameId)
{
	lwp_t thread = LWP_THREAD_NULL;
	int msg = 0;
	wstringEx prevMsg;

	SetupInput();
	_showDownload();
	m_btnMgr.setText(m_downloadBtnCancel, _t("dl1", L"Cancel"));
	m_thrdStop = false;
	m_thrdMessageAdded = false;
	m_coverDLGameId = gameId;
	while (true)
	{
		_mainLoopCommon(false, m_thrdWorking);
		if ((BTN_HOME_PRESSED || BTN_B_PRESSED) && !m_thrdWorking)
			break;
		else if (BTN_UP_PRESSED)
			m_btnMgr.up();
		else if (BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if ((BTN_A_PRESSED || !gameId.empty()) && !(m_thrdWorking && m_thrdStop))
		{
			m_btnMgr.click();
			if ((m_btnMgr.selected() == m_downloadBtnAll || m_btnMgr.selected() == m_downloadBtnMissing || !gameId.empty()) && !m_thrdWorking)
			{
				bool dlAll = m_btnMgr.selected() == m_downloadBtnAll;
				m_btnMgr.show(m_downloadPBar);
				m_btnMgr.setProgress(m_downloadPBar, 0.f);
				m_btnMgr.hide(m_downloadBtnAll);
				m_btnMgr.hide(m_downloadBtnMissing);
				m_btnMgr.hide(m_downloadBtnAllTitles);
				m_btnMgr.hide(m_downloadBtnMissingTitles);
				m_btnMgr.hide(m_downloadLblCovers);
				m_btnMgr.hide(m_downloadLblGameTitles);
				m_thrdStop = false;
				m_thrdWorking = true;
				gameId.clear();
				if (dlAll)
					LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_coverDownloaderAll, (void *)this, 0, 8192, 40);
				else
					LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_coverDownloaderMissing, (void *)this, 0, 8192, 40);
			}
			else if ((m_btnMgr.selected() == m_downloadBtnAllTitles || m_btnMgr.selected() == m_downloadBtnMissingTitles) && !m_thrdWorking)
			{
				bool dlAll = m_btnMgr.selected() == m_downloadBtnAllTitles;
				m_btnMgr.show(m_downloadPBar);
				m_btnMgr.setProgress(m_downloadPBar, 0.f);
				m_btnMgr.hide(m_downloadBtnAll);
				m_btnMgr.hide(m_downloadBtnMissing);
				m_btnMgr.hide(m_downloadBtnAllTitles);
				m_btnMgr.hide(m_downloadBtnMissingTitles);
				m_btnMgr.hide(m_downloadLblCovers);
				m_btnMgr.hide(m_downloadLblGameTitles);
				m_thrdStop = false;
				m_thrdWorking = true;
				if (dlAll)
					LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_titleDownloaderAll, (void *)this, 0, 8192, 40);
				else
					LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_titleDownloaderMissing, (void *)this, 0, 8192, 40);
			}
			else if (m_btnMgr.selected() == m_downloadBtnCancel)
			{
				LockMutex lock(m_mutex);
				m_thrdStop = true;
				m_thrdMessageAdded = true;
				m_thrdMessage = _t("dlmsg6", L"Canceling...");
			}
		}
		if (Sys_Exiting())
		{
			LockMutex lock(m_mutex);
			m_thrdStop = true;
			m_thrdMessageAdded = true;
			m_thrdMessage = _t("dlmsg6", L"Canceling...");
			m_thrdWorking = false;
		}
		// 
		if (m_thrdMessageAdded)
		{
			LockMutex lock(m_mutex);
			m_thrdMessageAdded = false;
			m_btnMgr.setProgress(m_downloadPBar, m_thrdProgress);
			if (m_thrdProgress == 1.f)
				m_btnMgr.setText(m_downloadBtnCancel, _t("dl2", L"Back"));
			if (prevMsg != m_thrdMessage)
			{
				prevMsg = m_thrdMessage;
				m_btnMgr.setText(m_downloadLblMessage[msg], m_thrdMessage, false);
				m_btnMgr.hide(m_downloadLblMessage[msg], 0, 0, -1.f, -1.f, true);
				m_btnMgr.show(m_downloadLblMessage[msg]);
				msg ^= 1;
				m_btnMgr.hide(m_downloadLblMessage[msg], 0, 0, -1.f, -1.f);
			}
		}
		if (m_thrdStop && !m_thrdWorking)
			break;
	}
	if (thread != LWP_THREAD_NULL)
	{
		LWP_JoinThread(thread, NULL);
		thread = LWP_THREAD_NULL;
	}
	_hideDownload();
}

void CMenu::_initDownloadMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_downloadLblUser, ARRAY_SIZE(m_downloadLblUser), "DOWNLOAD");
	m_downloadBg = _texture(theme.texSet, "DOWNLOAD/BG", "texture", theme.bg);
	m_downloadLblTitle = _addLabel(theme, "DOWNLOAD/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_downloadPBar = _addProgressBar(theme, "DOWNLOAD/PROGRESS_BAR", 40, 200, 560, 20);
	m_downloadBtnCancel = _addButton(theme, "DOWNLOAD/CANCEL_BTN", theme.btnFont, L"", 420, 410, 200, 56, theme.btnFontColor);
	m_downloadLblCovers = _addLabel(theme, "DOWNLOAD/COVERS", theme.btnFont, L"", 40, 150, 320, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadBtnAll = _addButton(theme, "DOWNLOAD/ALL_BTN", theme.btnFont, L"", 370, 150, 230, 56, theme.btnFontColor);
	m_downloadBtnMissing = _addButton(theme, "DOWNLOAD/MISSING_BTN", theme.btnFont, L"", 370, 210, 230, 56, theme.btnFontColor);
	m_downloadLblGameTitles = _addLabel(theme, "DOWNLOAD/GAME_TITLES", theme.btnFont, L"", 40, 270, 320, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadBtnAllTitles = _addButton(theme, "DOWNLOAD/ALL_TITLES_BTN", theme.btnFont, L"", 370, 270, 230, 56, theme.btnFontColor);
	m_downloadBtnMissingTitles = _addButton(theme, "DOWNLOAD/MISSING_TITLES_BTN", theme.btnFont, L"", 370, 330, 230, 56, theme.btnFontColor);
	m_downloadLblWiiTDB = _addLabel(theme, "DOWNLOAD/WIITDB", theme.btnFont, L"", 40, 400, 370, 60, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadLblMessage[0] = _addLabel(theme, "DOWNLOAD/MESSAGE1", theme.lblFont, L"", 40, 228, 560, 100, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_downloadLblMessage[1] = _addLabel(theme, "DOWNLOAD/MESSAGE2", theme.lblFont, L"", 40, 228, 560, 100, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	// 
	_setHideAnim(m_downloadLblTitle, "DOWNLOAD/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadPBar, "DOWNLOAD/PROGRESS_BAR", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadLblCovers, "DOWNLOAD/COVERS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnCancel, "DOWNLOAD/CANCEL_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnAll, "DOWNLOAD/ALL_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnMissing, "DOWNLOAD/MISSING_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadLblGameTitles, "DOWNLOAD/GAME_TITLES", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnAllTitles, "DOWNLOAD/ALL_TITLES_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnMissingTitles, "DOWNLOAD/MISSING_TITLES_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadLblWiiTDB, "DOWNLOAD/WIITDB", 0, 0, -2.f, 0.f);
	_hideDownload(true);
	_textDownload();
}

void CMenu::_textDownload(void)
{
	m_btnMgr.setText(m_downloadBtnCancel, _t("dl1", L"Cancel"));
	m_btnMgr.setText(m_downloadBtnAll, _t("dl3", L"All"));
	m_btnMgr.setText(m_downloadBtnMissing, _t("dl4", L"Missing"));
	m_btnMgr.setText(m_downloadLblTitle, _t("dl5", L"Download"));
	m_btnMgr.setText(m_downloadBtnAllTitles, _t("dl6", L"All"));
	m_btnMgr.setText(m_downloadBtnMissingTitles, _t("dl7", L"Missing"));
	m_btnMgr.setText(m_downloadLblCovers, _t("dl8", L"Covers"));
	m_btnMgr.setText(m_downloadLblGameTitles, _t("dl9", L"Game titles"));
	m_btnMgr.setText(m_downloadLblWiiTDB, _t("dl10", L"Please donate\nto WiiTDB.com"));
}

s8 CMenu::_versionTxtDownloaderInit(CMenu *m) //Handler to download versions txt file
{
	if (!m->m_thrdWorking)
		return 0;
	return m->_versionTxtDownloader();
}

s8 CMenu::_versionTxtDownloader() // code to download new version txt file
{
	block vtxt;
	char ip[16];
	wstringEx ws;
	SmartBuf buffer;
	FILE *file = NULL;
	u32 bufferSize = 1 * 0x001000;	// Maximum download size 4kb
	buffer = smartCoverAlloc(bufferSize);
	if (!buffer)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(L"Not enough memory", 1.f);
		LWP_MutexUnlock(m_mutex);
		m_thrdWorking = false;
		return 0;
	}
	
	// langCode = m_loc.getString(m_curLanguage, "wiitdb_code", "EN");
	
	LWP_MutexLock(m_mutex);
	_setThrdMsg(_t("dlmsg1", L"Initializing network..."), 0.f);
	LWP_MutexUnlock(m_mutex);
	
	if (!m_networkInit && _initNetwork(ip) < 0)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg2", L"Network initialization failed!"), 1.f);
		LWP_MutexUnlock(m_mutex);
	}
	else
	{
		m_networkInit = true;

		// DLoad txt file
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg11", L"Downloading..."), 0.2f);
		LWP_MutexUnlock(m_mutex);

		m_thrdStep = 0.2f;
		m_thrdStepLen = 0.9f - 0.2f;
		gprintf("TXT update URL: %s\n\n", m_cfg.getString(" GENERAL", "updatetxturl", UPDATE_URL_VERSION).c_str());
		vtxt = downloadfile(buffer.get(), bufferSize, m_cfg.getString(" GENERAL", "updatetxturl", UPDATE_URL_VERSION).c_str(),CMenu::_downloadProgress, this);
		if (vtxt.data == 0 || vtxt.size < 19)
		{
			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg20", L"No version information found."), 1.f); // TODO: Check for 16
			LWP_MutexUnlock(m_mutex);
		}
		else
		{
			// txt download finished, now save file
			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg13", L"Saving..."), 0.9f);
			LWP_MutexUnlock(m_mutex);			
			
			file = fopen(m_ver.c_str(), "wb");
			sleep(1);
			if (file != NULL)
			{
				fwrite(vtxt.data, 1, vtxt.size, file);
				fclose(file);
				// version file valid, check for version with SVN_REV
				int svnrev = atoi(SVN_REV);
				gprintf("Installed Version: %d\n", svnrev);
				m_version.load(m_ver.c_str());
				int rev = m_version.getInt("GENERAL", "latestversion", 0);
				gprintf("Latest available Version: %d\n", rev);
				if (svnrev < rev)
				{
					// new version available
					LWP_MutexLock(m_mutex);
					_setThrdMsg(_t("dlmsg19", L"New update available!"), 1.f);
					LWP_MutexUnlock(m_mutex);
				}
				else
				{
					// no new version available
					LWP_MutexLock(m_mutex);
					_setThrdMsg(_t("dlmsg17", L"No new updates found."), 1.f);
					LWP_MutexUnlock(m_mutex);
				}
			}
			else
			{
				LWP_MutexLock(m_mutex);
				_setThrdMsg(_t("dlmsg15", L"Saving failed!"), 1.f);
				LWP_MutexUnlock(m_mutex);
			}

		}
	}
	m_thrdWorking = false;
	return 0;
}

s8 CMenu::_versionDownloaderInit(CMenu *m) //Handler to download new dol
{
	if (!m->m_thrdWorking)
		return 0;
	return m->_versionDownloader();
}

s8 CMenu::_versionDownloader() // code to download new dol
{
	block newdol;
	char ip[16];
	wstringEx ws;
	SmartBuf buffer;
	FILE *file = NULL;
	u32 bufferSize = 1 * 0x400000;	// Maximum download size 4 MB
	
	// check for existing dol
    ifstream filestr;
	gprintf("DOL Path: %s\n", m_dol.c_str());
    filestr.open(m_dol.c_str());

    if (filestr.fail())
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg18", L"boot.dol not found at default path!"), 1.f);
		LWP_MutexUnlock(m_mutex);
		m_thrdWorking = false;

        return 0;
	}
    filestr.close();

	buffer = smartAnyAlloc(bufferSize);
	if (!buffer)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(L"Not enough memory!", 1.f);
		LWP_MutexUnlock(m_mutex);
		m_thrdWorking = false;
		return 0;
	}
	
	// langCode = m_loc.getString(m_curLanguage, "wiitdb_code", "EN");
	
	LWP_MutexLock(m_mutex);
	_setThrdMsg(_t("dlmsg1", L"Initializing network..."), 0.f);
	LWP_MutexUnlock(m_mutex);
	
	if (!m_networkInit && _initNetwork(ip) < 0)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg2", L"Network initialization failed!"), 1.f);
		LWP_MutexUnlock(m_mutex);
	}
	else
	{
		// Load actual file
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg11", L"Downloading..."), 0.2f);
		LWP_MutexUnlock(m_mutex);

		m_thrdStep = 0.2f;
		m_thrdStepLen = 0.9f - 0.2f;
		gprintf("Update URL: %s\n", m_update_url);
		newdol = downloadfile(buffer.get(), bufferSize, m_update_url, CMenu::_downloadProgress, this);
		if (newdol.data == 0 || newdol.size < 4000)
		{
			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg12", L"Download failed!"), 1.f);
			LWP_MutexUnlock(m_mutex);
		}
		else
		{
			// download finished, backup boot.dol and write new file.
			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg13", L"Saving..."), 0.9f);
			LWP_MutexUnlock(m_mutex);			
			
			char dol_backup[33];
			strcpy(dol_backup, m_dol.c_str());
			strcat(dol_backup, ".backup");
			remove(dol_backup);
			rename(m_dol.c_str(), dol_backup);
			file = fopen(m_dol.c_str(), "wb");
			sleep(1);
			if (file != NULL)
			{
				fwrite(newdol.data, 1, newdol.size, file);
				fclose(file);
				LWP_MutexLock(m_mutex);
				_setThrdMsg(_t("dlmsg20", L"WiiFlow will now exit to allow the update to take effect."), 1.f);
				LWP_MutexUnlock(m_mutex);
				sleep(3);
				exit(0);
			}
			else
			{
				rename(dol_backup, m_dol.c_str());
				LWP_MutexLock(m_mutex);
				_setThrdMsg(_t("dlmsg15", L"Saving failed!"), 1.f);
				LWP_MutexUnlock(m_mutex);
			}
		}
	}
	m_thrdWorking = false;
	return 0;
}

int CMenu::_titleDownloaderAll(CMenu *m)
{
	if (!m->m_thrdWorking)
		return 0;
	return m->_titleDownloader(false);
}

int CMenu::_titleDownloaderMissing(CMenu *m)
{
	if (!m->m_thrdWorking)
		return 0;
	return m->_titleDownloader(true);
}

int CMenu::_titleDownloader(bool missingOnly)
{
	block txt;
	Config titles;
	char ip[16];
	string langCode;
	wstringEx ws;
	bool ok = false;
	SmartBuf buffer;
	const char *p;
	const char *txtEnd;
	u32 len;
	u32 bufferSize = 1 * 0x080000;	// Maximum download size 512kb

	buffer = smartAnyAlloc(bufferSize);
	if (!buffer)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(L"Not enough memory", 1.f);
		LWP_MutexUnlock(m_mutex);
		return 0;
	}
	titles.load(sfmt("%s/titles.ini", m_appDir.c_str()).c_str());
	langCode = m_loc.getString(m_curLanguage, "wiitdb_code", "EN");
	LWP_MutexLock(m_mutex);
	_setThrdMsg(_t("dlmsg1", L"Initializing network..."), 0.f);
	LWP_MutexUnlock(m_mutex);
	if (!m_networkInit && _initNetwork(ip) < 0)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg2", L"Network initialization failed!"), 1.f);
		LWP_MutexUnlock(m_mutex);
	}
	else
	{
		m_networkInit = true;
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg11", L"Downloading..."), 0.1f);
		LWP_MutexUnlock(m_mutex);
		m_thrdStep = 0.1f;
		m_thrdStepLen = 0.9f - 0.1f;
		txt = downloadfile(buffer.get(), bufferSize, sfmt(TITLES_URL, langCode.c_str()).c_str(), CMenu::_downloadProgress, this);
		if (txt.data == 0 || txt.size < 42)
		{
			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg12", L"Download failed!"), 1.f);
			LWP_MutexUnlock(m_mutex);
		}
		else
		{
			txt.data[min(txt.size, bufferSize - 1)] = 0;
			p = (const char *)txt.data;
			txtEnd = (const char *)txt.data + txt.size;
			while (p < txtEnd)
			{
				len = strcspn((char *)p, "\n");
				if (len > 7)
				{
					string l((char *)p, len);
					u32 i = l.find_first_of('=');
					u32 j = l.find_first_of(' ');
					u32 n = min(i, j);
					if (i != string::npos && n >= 4 && (n == 6 || n == 4))
					{
						u32 m = l.find_first_not_of(' ', i + 1);
						if (m != string::npos)
						{
							ok = true;
							wstringEx w;
							w.fromUTF8(l.substr(m, l[l.size() - 1] == '\r' ? l.size() - 1 - m : l.size() - m).c_str());
							if (missingOnly)
								titles.getWString("TITLES", l.substr(0, n), w);
							else
								titles.setWString("TITLES", l.substr(0, n), w);
						}
					}
				}
				p += len + 1;
			}
			if (!ok)
			{
				LWP_MutexLock(m_mutex);
				_setThrdMsg(_t("dlmsg16", L"Couldn't read file!"), 1.f);
				LWP_MutexUnlock(m_mutex);
			}
		}
	}
	if (ok)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg13", L"Saving..."), 0.9f);
		LWP_MutexUnlock(m_mutex);
		titles.save();
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg14", L"Done."), 1.f);
		LWP_MutexUnlock(m_mutex);
	}
	txt.data = NULL;
	buffer.release();
	m_thrdWorking = false;
	return 0;
}


int CMenu::_update(CMenu *m)
{
	if (!m->m_thrdWorking)
		return 0;
	return m->_updateDownloader();
}

int CMenu::_updateDownloader()
{
	return 0;
}
