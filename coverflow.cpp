// Coverflow

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <new>
#include <zlib.h>

#include "coverflow.hpp"
#include "libpng/pngu/pngu.h"
#include "boxmesh.hpp"
#include "wstring.hpp"

using namespace std;

extern const u8 dvdskin_png[];
extern const u8 nopic_png[];
extern const u8 loading_png[];
extern const u8 flatnopic_png[];
extern const u8 flatloading_png[];
extern const u8 cffont_ttf[];
extern const u32 cffont_ttf_size;

class LockMutex
{
	mutex_t &m_mutex;
public:
	LockMutex(mutex_t &m) : m_mutex(m) { LWP_MutexLock(m_mutex); }
	~LockMutex(void) { LWP_MutexUnlock(m_mutex); }
};

static inline int loopNum(int i, int s)
{
	return i < 0 ? (s - (-i % s)) % s : i % s;
}

CCoverFlow::CCover::CCover(void) 
{
	index = 0;
	txtAngle = 0.f;
	txtTargetAngle = 0.f;
	txtColor = 0;
	txtTargetColor = 0;
	color = 0x00FFFFFF;
	targetColor = 0xFFFFFFFF;
	shadowColor = 0x00000000;
	targetShadowColor = 0x00000000;
	scale = Vector3D(1.f, 1.f, 1.f);
	targetScale = Vector3D(1.f, 1.f, 1.f);
}

CCoverFlow::CItem::CItem(const char *itemId, const wchar_t *itemTitle, const char *itemPic, const char *itemBoxPic) :
	id(itemId),
	title(itemTitle),
	picPath(itemPic),
	boxPicPath(itemBoxPic)
{
	state = CCoverFlow::STATE_Loading;
	boxTexture = false;
}

static inline wchar_t upperCaseWChar(wchar_t c)
{
	return c >= L'a' && c <= L'z' ? c & 0x00DF : c;
}

bool CCoverFlow::CItem::operator<(const CCoverFlow::CItem &i) const
{
	u32 s = min(title.size(), i.title.size());
	for (u32 k = 0; k < s; ++k)
		if (upperCaseWChar(i.title[k]) < upperCaseWChar(title[k]))
			return false;
		else if (upperCaseWChar(i.title[k]) > upperCaseWChar(title[k]))
			return true;
	return false;
}


CCoverFlow::CCoverFlow(void)
{
	m_loNormal.camera = Vector3D(0.f, 1.5f, 5.f);
	m_loNormal.cameraAim = Vector3D(0.f, 0.f, -1.f);
	m_loNormal.leftScale = Vector3D(1.f, 1.f, 1.f);
	m_loNormal.rightScale = Vector3D(1.f, 1.f, 1.f);
	m_loNormal.centerScale = Vector3D(1.f, 1.f, 1.f);
	m_loNormal.rowCenterScale = Vector3D(1.f, 1.f, 1.f);
	m_loNormal.leftPos = Vector3D(-1.6f, 0.f, 0.f);
	m_loNormal.rightPos = Vector3D(1.6f, 0.f, 0.f);
	m_loNormal.centerPos = Vector3D(0.f, 0.f, 1.f);
	m_loNormal.rowCenterPos = Vector3D(0.f, 0.f, 0.f);
	m_loNormal.leftAngle = Vector3D(0.f, 70.f, 0.f);
	m_loNormal.rightAngle = Vector3D(0.f, -70.f, 0.f);
	m_loNormal.centerAngle = Vector3D(0.f, 0.f, 0.f);
	m_loNormal.rowCenterAngle = Vector3D(0.f, 0.f, 0.f);
	m_loNormal.leftSpacer = Vector3D(-0.35f, 0.f, 0.f);
	m_loNormal.rightSpacer = Vector3D(0.35f, 0.f, 0.f);
	m_loNormal.leftDeltaAngle = Vector3D(0.f, 0.f, 0.f);
	m_loNormal.rightDeltaAngle = Vector3D(0.f, 0.f, 0.f);
	m_loNormal.txtLeftAngle = -55.f;
	m_loNormal.txtRightAngle = 55.f;
	m_loNormal.txtCenterAngle = 0.f;
	m_loNormal.txtLeftPos = Vector3D(-4.f, 0.f, 1.3f);
	m_loNormal.txtRightPos = Vector3D(4.f, 0.f, 1.3f);
	m_loNormal.txtCenterPos = Vector3D(0.f, 0.f, 2.6f);
	m_loNormal.txtSideWidth = 500.f;
	m_loNormal.txtCenterWidth = 500.f;
	m_loNormal.txtSideStyle = FTGX_ALIGN_BOTTOM | FTGX_JUSTIFY_CENTER;
	m_loNormal.txtCenterStyle = FTGX_ALIGN_BOTTOM | FTGX_JUSTIFY_CENTER;
	m_loNormal.endColor = CColor(0x3FFFFFFF);
	m_loNormal.begColor = CColor(0xCFFFFFFF);
	m_loNormal.mouseOffColor = CColor(0xFF00FF00);
	m_loNormal.shadowColorCenter = CColor(0x00000000);
	m_loNormal.shadowColorEnd = CColor(0x00000000);
	m_loNormal.shadowColorBeg = CColor(0x00000000);
	m_loNormal.shadowColorOff = CColor(0x00000000);
	m_loNormal.topSpacer = Vector3D(0.f, 2.f, 0.f);
	m_loNormal.bottomSpacer = Vector3D(0.f, -2.f, 0.f);
	m_loNormal.topDeltaAngle = Vector3D(0.f, 0.f, 0.f);
	m_loNormal.bottomDeltaAngle = Vector3D(0.f, 0.f, 0.f);
	m_loNormal.topAngle = Vector3D(0.f, 0.f, 0.f);
	m_loNormal.bottomAngle = Vector3D(0.f, 0.f, 0.f);
	// 
	m_loSelected.camera = Vector3D(0.f, 1.5f, 5.f);
	m_loSelected.cameraAim = Vector3D(0.f, 0.f, -1.f);
	m_loSelected.leftScale = Vector3D(1.f, 1.f, 1.f);
	m_loSelected.rightScale = Vector3D(1.f, 1.f, 1.f);
	m_loSelected.centerScale = Vector3D(1.f, 1.f, 1.f);
	m_loSelected.rowCenterScale = Vector3D(1.f, 1.f, 1.f);
	m_loSelected.leftPos = Vector3D(-4.6f, 2.f, 0.f);
	m_loSelected.rightPos = Vector3D(4.6f, 2.f, 0.f);
	m_loSelected.centerPos = Vector3D(-0.6f, 0.f, 2.6f);
	m_loSelected.rowCenterPos = Vector3D(0.f, 0.f, 0.f);
	m_loSelected.leftAngle = Vector3D(-45.f, 90.f, 0.f);
	m_loSelected.rightAngle = Vector3D(-45.f, -90.f, 0.f);
	m_loSelected.centerAngle = Vector3D(0.f, 380.f, 0.f);
	m_loSelected.rowCenterAngle = Vector3D(0.f, 0.f, 0.f);
	m_loSelected.leftSpacer = Vector3D(-0.35f, 0.f, 0.f);
	m_loSelected.rightSpacer = Vector3D(0.35f, 0.f, 0.f);
	m_loSelected.leftDeltaAngle = Vector3D(0.f, 0.f, 0.f);
	m_loSelected.rightDeltaAngle = Vector3D(0.f, 0.f, 0.f);
	m_loSelected.txtLeftAngle = -55.f;
	m_loSelected.txtRightAngle = 55.f;
	m_loSelected.txtCenterAngle = 0.f;
	m_loSelected.txtLeftPos = Vector3D(-4.35f, 0.f, 1.3f);
	m_loSelected.txtRightPos = Vector3D(4.35f, 0.f, 1.3f);
	m_loSelected.txtCenterPos = Vector3D(2.3f, 1.8f, 1.f);
	m_loSelected.txtSideWidth = 500.f;
	m_loSelected.txtCenterWidth =  310.f;
	m_loSelected.txtSideStyle = FTGX_ALIGN_BOTTOM | FTGX_JUSTIFY_CENTER;
	m_loSelected.txtCenterStyle = FTGX_ALIGN_TOP | FTGX_JUSTIFY_RIGHT;
	m_loSelected.endColor = CColor(0x3FFFFFFF);
	m_loSelected.begColor = CColor(0xCFFFFFFF);
	m_loSelected.mouseOffColor = CColor(0xFF00FF00);
	m_loSelected.shadowColorCenter = CColor(0x00000000);
	m_loSelected.shadowColorEnd = CColor(0x00000000);
	m_loSelected.shadowColorBeg = CColor(0x00000000);
	m_loSelected.shadowColorOff = CColor(0x00000000);
	m_loSelected.topSpacer = Vector3D(0.f, 2.f, 0.f);
	m_loSelected.bottomSpacer = Vector3D(0.f, -2.f, 0.f);
	m_loSelected.topDeltaAngle = Vector3D(0.f, 0.f, 0.f);
	m_loSelected.bottomDeltaAngle = Vector3D(0.f, 0.f, 0.f);
	m_loSelected.topAngle = Vector3D(0.f, 0.f, 0.f);
	m_loSelected.bottomAngle = Vector3D(0.f, 0.f, 0.f);
	// 
	m_mirrorAlpha = 0.2f;
	m_txtMirrorAlpha = 0.2f;
	m_delay = 0;
	m_minDelay = 5;
	m_jump = 0;
	m_mutex = 0;
	m_loadingPic = false;
	m_waitingToClear = false;
	m_moved = false;
	m_selected = false;
	m_tickCount = 0;
	m_hqCover = -1;
	m_blurRadius = 3;
	m_blurFactor = 1.f;
	// 
	m_mirrorBlur = false;
	m_effectTex.width = 96;
	m_effectTex.height = 72;
	m_effectTex.format = GX_TF_RGBA8;
	m_shadowScale = 1.f;
	m_shadowX = 0.f;
	m_shadowY = 0.f;
	m_flipCoverPos = Vector3D(0.f, 0.f, 0.f);
	m_flipCoverAngle = Vector3D(0.f, 180.f, 0.f);
	m_flipCoverScale = Vector3D(1.f, 1.f, 1.f);
	// Settings
	m_lodBias = -0.3f;
	m_aniso = GX_ANISO_1;
	m_edgeLOD = false;
	m_numBufCovers = 100;
	m_compressTextures = true;
	m_compressCache = false;
	m_deletePicsAfterCaching = false;
	m_box = true;
	m_rows = 1;
	m_columns = 11;
	m_range = m_rows * m_columns;
	m_mouse = -1;
	m_snd2 = false;
	m_soundVolume = 0xFF;
	// 
	LWP_MutexInit(&m_mutex, 0);
}

bool CCoverFlow::init(void)
{
	// Load font
	m_font.fromBuffer(cffont_ttf, cffont_ttf_size, 32, 32);
	m_fontColor = CColor(0xFFFFFFFF);
	// 
	if (CONF_GetAspectRatio() == CONF_ASPECT_16_9)
		guPerspective(m_projMtx, 45, 16.f / 9.f, .1f, 300.f);
	else
		guPerspective(m_projMtx, 45, 4.f / 3.f, .1f, 300.f);
	return true;
}

void CCoverFlow::simulateOtherScreenFormat(bool s)
{
	if ((CONF_GetAspectRatio() == CONF_ASPECT_16_9) != s)
		guPerspective(m_projMtx, 45, 16.f / 9.f, .1f, 300.f);
	else
		guPerspective(m_projMtx, 45, 4.f / 3.f, .1f, 300.f);
}

CCoverFlow::~CCoverFlow(void)
{
	clear();
	LWP_MutexDestroy(m_mutex);
}

void CCoverFlow::setCachePath(const char *path, bool deleteSource, bool compress)
{
	m_cachePath = path;
	m_deletePicsAfterCaching = deleteSource;
	m_compressCache = compress;
}

void CCoverFlow::setTextureQuality(float lodBias, int aniso, bool edgeLOD)
{
	m_lodBias = min(max(-3.f, lodBias), 1.f);
	switch (aniso)
	{
		case 1:
			m_aniso = GX_ANISO_2;
			break;
		case 2:
			m_aniso = GX_ANISO_4;
			break;
		default:
			m_aniso = GX_ANISO_1;
	}
	m_edgeLOD = edgeLOD;
}

void CCoverFlow::setBoxMode(bool box)
{
	m_box = box;
}

void CCoverFlow::setBufferSize(u32 numCovers)
{
	if (!m_covers.empty())
		return;
	m_numBufCovers = min(max(3u, numCovers / 2u), 400u);
}

void CCoverFlow::setTextures(const string &loadingPic, const string &loadingPicFlat, const string &noCoverPic, const string &noCoverPicFlat)
{
	if (!m_covers.empty())
		return;
	m_pngLoadCover = loadingPic;
	m_pngLoadCoverFlat = loadingPicFlat;
	m_pngNoCover = noCoverPic;
	m_pngNoCoverFlat = noCoverPicFlat;
}

void CCoverFlow::setFont(SFont font, const CColor &color)
{
	if (!!font.data)
		m_font = font;
	m_fontColor = color;
	if (!m_covers.empty())
	{
		for (u32 i = 0; i < m_range; ++i)
			_loadCover(i, m_covers[i].index);
		_updateAllTargets();
	}
}

void CCoverFlow::_transposeCover(vector<CCoverFlow::CCover> &dst, u32 rows, u32 columns, int pos)
{
	int i = pos - (int)(rows * columns / 2);
	int j = rows >= 3 ? abs(i) - ((abs(i) + (int)rows / 2) / (int)rows) * 2 : abs(i);
	if (m_rows >= 3)
		j += ((j + ((int)m_rows - 2) / 2) / ((int)m_rows - 2)) * 2;
	int k = m_range / 2 + (i < 0 ? -j : j);
	if ((u32)k < m_range)
		dst[pos] = m_covers[k];
}

void CCoverFlow::setRange(u32 rows, u32 columns)
{
	u32 range;

	rows = rows < 3u ? 1u : min(rows | 1u, 9u) + 2u;
	columns = min(max(3u, columns | 1u), 21u) + 2u;
	range = rows * columns;
	if (rows == m_rows && columns == m_columns && range == m_range)
		return;
	if (!m_covers.empty())
	{
		stopPicLoader();
		vector<CCoverFlow::CCover> tmpCovers;
		tmpCovers.resize(range);
		if (rows >= 3)
			for (u32 x = 0; x < columns; ++x)
				for (u32 y = 1; y < rows - 1; ++y)
					_transposeCover(tmpCovers, rows, columns, x * rows + y);
		else
			for (u32 x = 0; x < range; ++x)
				_transposeCover(tmpCovers, rows, columns, x);
		swap(tmpCovers, m_covers);
		m_rows = rows;
		m_columns = columns;
		m_range = range;
		_loadAllCovers(m_covers[m_range / 2].index);
		_updateAllTargets();
		startPicLoader();
	}
	else
	{
		m_rows = rows;
		m_columns = columns;
		m_range = range;
	}
}

void CCoverFlow::setCameraPos(bool selected, const Vector3D &pos, const Vector3D &aim)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;
	lo.camera = pos;
	lo.cameraAim = aim;
}

void CCoverFlow::setCameraOsc(bool selected, const Vector3D &speed, const Vector3D &amp)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;
	lo.cameraOscSpeed = speed;
	lo.cameraOscAmp = amp;
}

void CCoverFlow::setCoverScale(bool selected, const Vector3D &left, const Vector3D &right, const Vector3D &center, const Vector3D &rowCenter)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;
	lo.leftScale = left;
	lo.rightScale = right;
	lo.centerScale = center;
	lo.rowCenterScale = rowCenter;
}

void CCoverFlow::setCoverPos(bool selected, const Vector3D &left, const Vector3D &right, const Vector3D &center, const Vector3D &rowCenter)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;
	lo.leftPos = left;
	lo.rightPos = right;
	lo.centerPos = center;
	lo.rowCenterPos = rowCenter;
}

void CCoverFlow::setCoverAngleOsc(bool selected, const Vector3D &speed, const Vector3D &amp)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;
	lo.coverOscASpeed = speed;
	lo.coverOscAAmp = amp;
}

void CCoverFlow::setCoverPosOsc(bool selected, const Vector3D &speed, const Vector3D &amp)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;
	lo.coverOscPSpeed = speed;
	lo.coverOscPAmp = amp;
}

void CCoverFlow::setSpacers(bool selected, const Vector3D &left, const Vector3D &right)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;
	lo.leftSpacer = left;
	lo.rightSpacer = right;
}

void CCoverFlow::setDeltaAngles(bool selected, const Vector3D &left, const Vector3D &right)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;
	lo.leftDeltaAngle = left;
	lo.rightDeltaAngle = right;
}

void CCoverFlow::setAngles(bool selected, const Vector3D &left, const Vector3D &right, const Vector3D &center, const Vector3D &rowCenter)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;
	lo.leftAngle = left;
	lo.rightAngle = right;
	lo.centerAngle = center;
	lo.rowCenterAngle = rowCenter;
}

void CCoverFlow::setTitleAngles(bool selected, float left, float right, float center)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;
	lo.txtLeftAngle = left;
	lo.txtRightAngle = right;
	lo.txtCenterAngle = center;
}

void CCoverFlow::setTitlePos(bool selected, const Vector3D &left, const Vector3D &right, const Vector3D &center)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;
	lo.txtLeftPos = left;
	lo.txtRightPos= right;
	lo.txtCenterPos= center;
}

void CCoverFlow::setTitleWidth(bool selected, float side, float center)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;
	lo.txtSideWidth = side;
	lo.txtCenterWidth= center;
}

void CCoverFlow::setTitleStyle(bool selected, u16 side, u16 center)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;
	lo.txtSideStyle = side;
	lo.txtCenterStyle = center;
}

void CCoverFlow::setColors(bool selected, const CColor &begColor, const CColor &endColor, const CColor &offColor)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;

	lo.begColor = begColor;
	lo.endColor = endColor;
	lo.mouseOffColor = offColor;
}

void CCoverFlow::setMirrorAlpha(float cover, float title)
{
	m_mirrorAlpha = cover;
	m_txtMirrorAlpha = title;
}

void CCoverFlow::setMirrorBlur(bool blur)
{
	m_mirrorBlur = blur;
}

void CCoverFlow::setShadowColors(bool selected, const CColor &centerColor, const CColor &begColor, const CColor &endColor, const CColor &offColor)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;

	lo.shadowColorCenter = centerColor;
	lo.shadowColorBeg = begColor;
	lo.shadowColorEnd = endColor;
	lo.shadowColorOff = offColor;
}

void CCoverFlow::setShadowPos(float scale, float x, float y)
{
	m_shadowScale = scale;
	m_shadowX = x;
	m_shadowY = y;
}

void CCoverFlow::setRowSpacers(bool selected, const Vector3D &top, const Vector3D &bottom)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;

	lo.topSpacer = top;
	lo.bottomSpacer = bottom;
}

void CCoverFlow::setRowDeltaAngles(bool selected, const Vector3D &top, const Vector3D &bottom)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;

	lo.topDeltaAngle = top;
	lo.bottomDeltaAngle = bottom;
}

void CCoverFlow::setRowAngles(bool selected, const Vector3D &top, const Vector3D &bottom)
{
	SLayout &lo = selected ? m_loSelected : m_loNormal;

	lo.topAngle = top;
	lo.bottomAngle = bottom;
}

void CCoverFlow::setCoverFlipping(const Vector3D &pos, const Vector3D &angle, const Vector3D &scale)
{
	m_flipCoverPos = pos;
	m_flipCoverAngle = angle;
	m_flipCoverScale = scale;
}

void CCoverFlow::setBlur(u32 blurResolution, u32 blurRadius, float blurFactor)
{
	static const struct { u32 x; u32 y; } blurRes[] = {
		{ 64, 48 }, { 96, 72 }, { 128, 96 }, { 192, 144 }
	};
	u32 i = min(max(0u, blurResolution), sizeof blurRes / sizeof blurRes[0] - 1u);
	m_effectTex.width = blurRes[i].x;
	m_effectTex.height = blurRes[i].y;
	m_effectTex.data.release();
	m_blurRadius = min(max(1u, blurRadius), 3u);
	m_blurFactor = min(max(1.f, blurFactor), 2.f);
}

void CCoverFlow::setSounds(const SSoundEffect &sound, const SSoundEffect &hoverSound, const SSoundEffect &selectSound, const SSoundEffect &cancelSound)
{
	m_sound1 = sound;
	m_sound2 = sound;
	m_hoverSound = hoverSound;
	m_selectSound = selectSound;
	m_cancelSound = cancelSound;
}

void CCoverFlow::setSoundVolume(u8 vol)
{
	m_soundVolume = vol;
}

void CCoverFlow::stopSound(void)
{
	m_sound1.stop();
	m_sound2.stop();
	m_hoverSound.stop();
}

void CCoverFlow::applySettings(void)
{
	LockMutex lock(m_mutex);
	if (m_covers.empty())
		return;
	_updateAllTargets();
}

void CCoverFlow::stopPicLoader(bool empty)
{
	m_waitingToClear = true;
	while (m_loadingPic)
		usleep(1000);
	if (empty)
		for (u32 i = 0; i < m_items.size(); ++i)
		{
			m_items[i].texture.data.release();
			m_items[i].state = CCoverFlow::STATE_Loading;
		}
}

void CCoverFlow::startPicLoader(void)
{
	lwp_t thread = 0;

	if (!m_loadingPic && !m_covers.empty())
	{
		m_moved = true;
		m_loadingPic = true;
		m_waitingToClear = false;
		LWP_CreateThread(&thread, (void *(*)(void *))CCoverFlow::_picLoader, (void *)this, 0, 8192, 40);
	}
}

void CCoverFlow::clear(void)
{
	stopPicLoader();
	m_covers.clear();
	m_items.clear();
	m_waitingToClear = false;
}

void CCoverFlow::reserve(u32 capacity)
{
	m_items.reserve(capacity);
}

void CCoverFlow::addItem(const char *id, const wchar_t *title, const char *picPath, const char *boxPicPath)
{
	if (!m_covers.empty())
		return;
	m_items.push_back(CCoverFlow::CItem(id, title, picPath, boxPicPath));
}

// Draws a plane in the Z-Buffer only.
void CCoverFlow::_drawMirrorZ(void)
{
	GX_LoadPosMtxImm(m_viewMtx, GX_PNMTX0);
	// GX setup
	GX_SetNumChans(0);
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetNumTexGens(0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_DISABLE, GX_COLORNULL);
	GX_SetAlphaUpdate(GX_FALSE);
	GX_SetColorUpdate(GX_FALSE);
	GX_SetZMode(GX_ENABLE, GX_LEQUAL, GX_TRUE);
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(-10000.f, 0.f, -10000.f);
	GX_Position3f32(10000.f, 0.f, -10000.f);
	GX_Position3f32(10000.f, 0.f, 10000.f);
	GX_Position3f32(-10000.f, 0.f, 10000.f);
	GX_End();
	GX_SetAlphaUpdate(GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
}

void CCoverFlow::_effectBg(const STexture &tex)
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
	GX_SetAlphaUpdate(GX_TRUE);
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_FALSE);
	guMtxIdentity(modelViewMtx);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	GX_InitTexObj(&texObj, tex.data.get(), tex.width, tex.height, tex.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
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

void CCoverFlow::_effectBlur(CVideo &vid, bool vertical)
{
	int kSize = m_blurRadius * 2 + 1;
	GXTexObj texObj;
	Mtx mh1;
	Mtx mh2;
	Mtx mh3;
	Mtx mh4;
	Mtx mh5;
	Mtx mh6;
	Mtx modelViewMtx;
	float pixDist = m_blurFactor;
	float w = (float)m_effectTex.width;
	float h = (float)m_effectTex.height;
	float x = 0.f;
	float y = 0.f;
	CColor kGauss7(0xFF * 20 / 64, 0xFF * 15 / 64, 0xFF * 6 / 64, 0xFF * 1 / 64);
	CColor kGauss5(0xFF * 6 / 16, 0xFF * 4 / 16, 0xFF * 1 / 16, 0);
	CColor kGauss3(0xFF * 2 / 4, 0xFF * 1 / 4, 0, 0);
	CColor kBox(0xFF / kSize, 0xFF / kSize, 0xFF / kSize, 0xFF / kSize);
	Mtx44 projMtx;

	GX_SetScissor(0, 0, m_effectTex.width, m_effectTex.height);
	GX_SetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
	GX_InvVtxCache();
	GX_InvalidateTexAll();
	GX_InitTexObj(&texObj, m_effectTex.data.get(), m_effectTex.width, m_effectTex.height, m_effectTex.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);
	GX_SetNumTevStages(kSize);
	GX_SetNumTexGens(kSize);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	guMtxIdentity(mh1);
	guMtxIdentity(mh2);
	guMtxIdentity(mh3);
	guMtxIdentity(mh4);
	guMtxIdentity(mh5);
	guMtxIdentity(mh6);
	if (vertical)
	{
		guMtxRowCol(mh1, 0, 3) = pixDist * -1.f / w;
		guMtxRowCol(mh2, 0, 3) = pixDist * 1.f / w;
		guMtxRowCol(mh3, 0, 3) = pixDist * -2.f / w;
		guMtxRowCol(mh4, 0, 3) = pixDist * 2.f / w;
		guMtxRowCol(mh5, 0, 3) = pixDist * -3.f / w;
		guMtxRowCol(mh6, 0, 3) = pixDist * 3.f / w;
	}
	else
	{
		guMtxRowCol(mh1, 1, 3) = pixDist * -1.f / h;
		guMtxRowCol(mh2, 1, 3) = pixDist * 1.f / h;
		guMtxRowCol(mh3, 1, 3) = pixDist * -2.f / h;
		guMtxRowCol(mh4, 1, 3) = pixDist * 2.f / h;
		guMtxRowCol(mh5, 1, 3) = pixDist * -3.f / h;
		guMtxRowCol(mh6, 1, 3) = pixDist * 3.f / h;
	}
	GX_LoadTexMtxImm(mh1, GX_TEXMTX1, GX_MTX2x4);
	GX_LoadTexMtxImm(mh2, GX_TEXMTX2, GX_MTX2x4);
	GX_LoadTexMtxImm(mh3, GX_TEXMTX3, GX_MTX2x4);
	GX_LoadTexMtxImm(mh4, GX_TEXMTX4, GX_MTX2x4);
	GX_LoadTexMtxImm(mh3, GX_TEXMTX5, GX_MTX2x4);
	GX_LoadTexMtxImm(mh4, GX_TEXMTX6, GX_MTX2x4);
	GX_SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1);
	GX_SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX2);
	GX_SetTexCoordGen(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX3);
	GX_SetTexCoordGen(GX_TEXCOORD4, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX4);
	GX_SetTexCoordGen(GX_TEXCOORD5, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX5);
	GX_SetTexCoordGen(GX_TEXCOORD6, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX6);
	switch (m_blurRadius)
	{
		case 1:
			GX_SetTevKColor(GX_KCOLOR0, kGauss3);
			break;
		case 2:
			GX_SetTevKColor(GX_KCOLOR0, kGauss5);
			break;
		case 3:
			GX_SetTevKColor(GX_KCOLOR0, kGauss7);
			break;
		default:
			GX_SetTevKColor(GX_KCOLOR0, kBox);
	}
	GX_SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0_R);
	GX_SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K0_G);
	GX_SetTevKColorSel(GX_TEVSTAGE2, GX_TEV_KCSEL_K0_G);
	GX_SetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K0_B);
	GX_SetTevKColorSel(GX_TEVSTAGE4, GX_TEV_KCSEL_K0_B);
	GX_SetTevKColorSel(GX_TEVSTAGE5, GX_TEV_KCSEL_K0_A);
	GX_SetTevKColorSel(GX_TEVSTAGE6, GX_TEV_KCSEL_K0_A);
	GX_SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_R);
	GX_SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_G);
	GX_SetTevKAlphaSel(GX_TEVSTAGE2, GX_TEV_KASEL_K0_G);
	GX_SetTevKAlphaSel(GX_TEVSTAGE3, GX_TEV_KASEL_K0_B);
	GX_SetTevKAlphaSel(GX_TEVSTAGE4, GX_TEV_KASEL_K0_B);
	GX_SetTevKAlphaSel(GX_TEVSTAGE5, GX_TEV_KASEL_K0_A);
	GX_SetTevKAlphaSel(GX_TEVSTAGE6, GX_TEV_KASEL_K0_A);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
	GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
	GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
	GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	for (int i = 1; i < kSize; ++i)
	{
		GX_SetTevOrder(GX_TEVSTAGE0 + i, GX_TEXCOORD0 + i, GX_TEXMAP0, GX_COLORNULL);
		GX_SetTevColorIn(GX_TEVSTAGE0 + i, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
		GX_SetTevAlphaIn(GX_TEVSTAGE0 + i, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_APREV);
		GX_SetTevColorOp(GX_TEVSTAGE0 + i, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GX_SetTevAlphaOp(GX_TEVSTAGE0 + i, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	}
	GX_SetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_FALSE);
	GX_SetViewport(0, 0, 640.f, 480.f, 0.f, 1.f);
	guOrtho(projMtx, 0, 480.f + 0, 0, 640.f + 0, 0.f, 1000.0f);
	GX_LoadProjectionMtx(projMtx, GX_ORTHOGRAPHIC);
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetAlphaUpdate(GX_TRUE);
	GX_SetCullMode(GX_CULL_NONE);
	guMtxIdentity(modelViewMtx);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(x, y, 0.f);
	GX_TexCoord2f32(0.f, 0.f);
	GX_Position3f32(x + w, y, 0.f);
	GX_TexCoord2f32(1.f, 0.f);
	GX_Position3f32(x + w, y + h, 0.f);
	GX_TexCoord2f32(1.f, 1.f);
	GX_Position3f32(x, y + h, 0.f);
	GX_TexCoord2f32(0.f, 1.f);
	GX_End();
	vid.renderToTexture(m_effectTex, true);
}

bool CCoverFlow::_effectVisible(void)
{
	SLayout &lo = m_selected ? m_loSelected : m_loNormal;
	return (m_mirrorAlpha > 0.01f && m_mirrorBlur)
		|| lo.shadowColorCenter.a > 0 || lo.shadowColorBeg.a > 0
		|| lo.shadowColorEnd.a > 0 || lo.shadowColorOff.a > 0;
}

void CCoverFlow::makeEffectTexture(CVideo &vid, const STexture &bg)
{
	int aa = 8;

	if (!_effectVisible())
		return;
	GX_SetDither(GX_DISABLE);
	vid.setAA(aa, true, m_effectTex.width, m_effectTex.height);
	for (int i = 0; i < aa; ++i)
	{
		vid.prepareAAPass(i);
		vid.setup2DProjection(false, true);
		_effectBg(bg);
		if (m_mirrorBlur)
			_draw(CCoverFlow::CFDR_NORMAL, true, false);
		vid.shiftViewPort(m_shadowX, m_shadowY);
		_draw(CCoverFlow::CFDR_SHADOW, false, true);
		vid.renderAAPass(i);
	}
	GX_SetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
	GX_InvVtxCache();
	GX_InvalidateTexAll();
	vid.setup2DProjection(false, true);
	GX_SetViewport(0.f, 0.f, (float)m_effectTex.width, (float)m_effectTex.height, 0.f, 1.f);
	GX_SetScissor(0, 0, m_effectTex.width, m_effectTex.height);
	vid.drawAAScene();
	vid.renderToTexture(m_effectTex, true);
	_effectBlur(vid, false);
	_effectBlur(vid, true);
	GX_SetDither(GX_ENABLE);
}

void CCoverFlow::drawEffect(void)
{
	if (m_covers.empty())
		return;
	if (_effectVisible())
	{
		Mtx modelViewMtx;
		GXTexObj texObj;
		float w = 640.f;
		float h = 480.f;
		float x = 0.f;
		float y = 0.f;

		GX_SetNumTevStages(1);
		GX_SetNumChans(0);
		GX_ClearVtxDesc();
		GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
		GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
		GX_SetNumTexGens(1);
		GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
		GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
		GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
		GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
		GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
		GX_SetAlphaUpdate(GX_FALSE);
		GX_SetCullMode(GX_CULL_NONE);
		GX_SetZMode(GX_ENABLE, GX_LEQUAL, GX_FALSE);
		guMtxIdentity(modelViewMtx);
		GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
		GX_InitTexObj(&texObj, m_effectTex.data.get(), m_effectTex.width, m_effectTex.height, m_effectTex.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_LoadTexObj(&texObj, GX_TEXMAP0);
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32(x, y, -999.f);
		GX_TexCoord2f32(0.f, 0.f);
		GX_Position3f32(x + w, y, -999.f);
		GX_TexCoord2f32(1.f, 0.f);
		GX_Position3f32(x + w, y + h, -999.f);
		GX_TexCoord2f32(1.f, 1.f);
		GX_Position3f32(x, y + h, -999.f);
		GX_TexCoord2f32(0.f, 1.f);
		GX_End();
	}
	if (!m_mirrorBlur)
		_draw(CCoverFlow::CFDR_NORMAL, true, true);
}

void CCoverFlow::draw(void)
{
	_draw();
}

void CCoverFlow::drawText(bool withRectangle)
{
	LockMutex lock(m_mutex);
	Vector3D up(0.f, 1.f, 0.f);
	Vector3D dir(m_cameraAim);
	Vector3D pos(m_cameraPos);

	if (m_covers.empty())
		return;
	if (m_fontColor.a == 0)
		return;
	pos += _cameraMoves();
	// Camera
	GX_LoadProjectionMtx(m_projMtx, GX_PERSPECTIVE);
	guLookAt(m_viewMtx, &pos, &up, &dir);
	// Text
	GX_SetNumChans(1);
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetZMode(GX_DISABLE, GX_LEQUAL, GX_FALSE);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	for (u32 i = 0; i < m_range; ++i)
	{
		_drawTitle(loopNum((i & 1) != 0 ? m_range / 2 - (i + 1) / 2 : m_range / 2 + i / 2, m_range), true, withRectangle);
		_drawTitle(loopNum((i & 1) != 0 ? m_range / 2 - (i + 1) / 2 : m_range / 2 + i / 2, m_range), false, withRectangle);
	}
}

inline int innerToOuter(int i, int range)
{
	return loopNum((i & 1) != 0 ? range / 2 - (i + 1) / 2 : range / 2 + i / 2, range);
}

void CCoverFlow::_draw(DrawMode dm, bool mirror, bool blend)
{
	LockMutex lock(m_mutex);
	Vector3D up(0.f, 1.f, 0.f);
	Vector3D dir(m_cameraAim);
	Vector3D pos(m_cameraPos);

	if (mirror && m_mirrorAlpha <= 0.f)
		return;
	if (m_covers.empty())
		return;
	pos += _cameraMoves();
	// GX setup
	GX_ClearVtxDesc();
	GX_SetNumTevStages(1);
	if (dm == CCoverFlow::CFDR_NORMAL)
	{
		GX_SetNumChans(0);
		GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
		GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
		GX_SetNumTexGens(1);
		GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
		GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO);
		GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
		GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
	}
	else
	{
		GX_SetNumChans(1);
		GX_SetNumTexGens(0);
		GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
		GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
		GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
		GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLORNULL);
	}
	GX_SetTevKColor(GX_KCOLOR0, CColor(0xFF, 0xFF, 0xFF, 0xFF));
	GX_SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
	GX_SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
	if (blend)
		GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	else
		GX_SetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_TRUE);
	// Camera
	GX_LoadProjectionMtx(m_projMtx, GX_PERSPECTIVE);
	guLookAt(m_viewMtx, &pos, &up, &dir);
	// Covers
	GX_SetCullMode(m_box ? (mirror ? GX_CULL_BACK : GX_CULL_FRONT) : GX_CULL_NONE);
	if (dm == CCoverFlow::CFDR_SHADOW)
	{
		// Shadow
		GX_SetZMode(GX_DISABLE, GX_LEQUAL, GX_FALSE);
		for (int x = (int)m_columns - 3; x >= 0; --x)
		{
			int xx = innerToOuter(x, m_columns);
			if (m_rows >= 3)
				for (int y = (int)m_rows - 3; y >= 0; --y)
					_drawCover(xx * m_rows + innerToOuter(y, m_rows), mirror, dm);
			else
				_drawCover(xx, mirror, dm);
		}
	}
	else
	{
		// Normal
		GX_SetZMode(GX_ENABLE, GX_LEQUAL, GX_TRUE);
		for (int x = 0; x < (int)m_columns - 2; ++x)
		{
			int xx = innerToOuter(x, m_columns);
			if (m_rows >= 3)
				for (int y = 0; y < (int)m_rows - 2; ++y)
					_drawCover(xx * m_rows + innerToOuter(y, m_rows), mirror, dm);
			else
				_drawCover(xx, mirror, dm);
		}
		// Vanishing covers
		GX_SetZMode(GX_ENABLE, GX_LEQUAL, GX_FALSE);
		for (u32 y = 0; y < m_rows; ++y)
		{
			_drawCover(y, mirror, dm);
			_drawCover((m_columns - 1) * m_rows + y, mirror, dm);
		}
		if (m_rows >= 3)
			for (u32 x = 1; x < m_columns - 1; ++x)
			{
				_drawCover(x * m_rows, mirror, dm);
				_drawCover(x * m_rows + m_rows - 1, mirror, dm);
			}
	}
}

Vector3D CCoverFlow::_cameraMoves(void)
{
	SLayout &lo = m_selected ? m_loSelected : m_loNormal;
	float tick = (float)m_tickCount * 0.005f;
	return Vector3D(cos(tick * lo.cameraOscSpeed.x) * lo.cameraOscAmp.x,
		cos(tick * lo.cameraOscSpeed.y) * lo.cameraOscAmp.y,
		cos(tick * lo.cameraOscSpeed.z) * lo.cameraOscAmp.z);
}

Vector3D CCoverFlow::_coverMovesA(void)
{
	SLayout &lo = m_selected ? m_loSelected : m_loNormal;
	float tick = (float)m_tickCount * 0.005f;
	return Vector3D(cos(tick * lo.coverOscASpeed.x) * lo.coverOscAAmp.x,
		sin(tick * lo.coverOscASpeed.y) * lo.coverOscAAmp.y,
		cos(tick * lo.coverOscASpeed.z) * lo.coverOscAAmp.z);
}

Vector3D CCoverFlow::_coverMovesP(void)
{
	SLayout &lo = m_selected ? m_loSelected : m_loNormal;
	float tick = (float)m_tickCount * 0.005f;
	return Vector3D(cos(tick * lo.coverOscPSpeed.x) * lo.coverOscPAmp.x,
		sin(tick * lo.coverOscPSpeed.y) * lo.coverOscPAmp.y,
		cos(tick * lo.coverOscPSpeed.z) * lo.coverOscPAmp.z);
}

void CCoverFlow::_drawTitle(int i, bool mirror, bool rectangle)
{
	Mtx modelMtx;
	Mtx modelViewMtx;
	Vector3D rotAxis(0.f, 1.f, 0.f);
	CColor color(m_fontColor);

	if (m_covers[i].txtColor == 0)
		return;
	color.a = mirror ? (u8)((float)m_covers[i].txtColor * m_txtMirrorAlpha) : m_covers[i].txtColor;
	if (rectangle && !mirror)
	{
		// GX setup
		GX_SetNumTevStages(1);
		GX_SetNumChans(1);
		GX_ClearVtxDesc();
		GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
		GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
		GX_SetNumTexGens(0);
		GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
		GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
		GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
		GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
		// 
		Mtx rotMtx;
		guMtxIdentity(modelMtx);
		guMtxScaleApply(modelMtx, modelMtx, 0.005f, 0.005f, 0.005f);
		guMtxRotDeg(rotMtx, 'Y', m_covers[i].txtAngle);
		guMtxConcat(rotMtx, modelMtx, modelMtx);
		guMtxTransApply(modelMtx, modelMtx, m_covers[i].txtPos.x, mirror ? -m_covers[i].txtPos.y : m_covers[i].txtPos.y, m_covers[i].txtPos.z);
		guMtxConcat(m_viewMtx, modelMtx, modelViewMtx);
		GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
		const SLayout &lo = m_selected ? m_loSelected : m_loNormal;
		u16 s = (u32)i == m_range / 2 ? lo.txtCenterStyle : lo.txtSideStyle;
		float x;
		float y;
		float w = (u32)i == m_range / 2 ? lo.txtCenterWidth : lo.txtSideWidth;
		float h = m_font.lineSpacing;
		if ((s & FTGX_JUSTIFY_CENTER) != 0)
			x = -w * 0.5f;
		else if ((s & FTGX_JUSTIFY_RIGHT) != 0)
			x = -w;
		else
			x = 0.f;
		if ((s & FTGX_ALIGN_MIDDLE) != 0)
			y = -h * 0.5f;
		else if ((s & FTGX_ALIGN_TOP) != 0)
			y = -h;
		else
			y = 0.f;
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32(x, y, 0.f);
		GX_Color4u8(color.r, color.g, color.b, color.a / 2);
		GX_Position3f32(x + w, y, 0.f);
		GX_Color4u8(color.r, color.g, color.b, color.a / 2);
		GX_Position3f32(x + w, y + h, 0.f);
		GX_Color4u8(color.r, color.g, color.b, color.a / 2);
		GX_Position3f32(x, y + h, 0.f);
		GX_Color4u8(color.r, color.g, color.b, color.a / 2);
		GX_End();
	}
	m_covers[i].title.setColor(color);
	m_font.font->reset();
	m_font.font->setXScale(0.005f);
	m_font.font->setYScale(mirror ? 0.005f : -0.005f);
	GX_ClearVtxDesc();
	GX_SetNumTevStages(1);
	GX_SetNumChans(1);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetNumTexGens(1);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	// 
	guMtxIdentity(modelMtx);
	guMtxRotAxisDeg(modelMtx, &rotAxis, m_covers[i].txtAngle);
	guMtxTransApply(modelMtx, modelMtx, m_covers[i].txtPos.x, mirror ? -m_covers[i].txtPos.y : m_covers[i].txtPos.y, m_covers[i].txtPos.z);
	guMtxConcat(m_viewMtx, modelMtx, modelViewMtx);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	m_covers[i].title.draw();
}

void CCoverFlow::_drawCover(int i, bool mirror, CCoverFlow::DrawMode dm)
{
	Mtx modelMtx;
	Mtx rotMtx;
	Mtx modelViewMtx;
	Vector3D osc;
	Vector3D oscP;

	if (m_covers[i].color.a == 0 || (dm == CCoverFlow::CFDR_SHADOW && m_covers[i].shadowColor.a == 0))
		return;
	if (dm == CCoverFlow::CFDR_STENCIL && (i > 0xFF || _invisibleCover(i / m_rows, i % m_rows)))
		return;
	if ((u32)i == m_range / 2)
	{
		osc = _coverMovesA();
		oscP = _coverMovesP();
	}
	// 
	guMtxIdentity(modelMtx);
	guMtxScaleApply(modelMtx, modelMtx, m_covers[i].scale.x, m_covers[i].scale.y, m_covers[i].scale.z);
	if (dm == CCoverFlow::CFDR_SHADOW)
		guMtxScaleApply(modelMtx, modelMtx, 1.f + (m_shadowScale - 1.f) * g_boxSize.y / g_boxSize.x, m_shadowScale, m_shadowScale);
	guMtxRotDeg(rotMtx, 'Z', m_covers[i].angle.z + osc.z);
	guMtxConcat(rotMtx, modelMtx, modelMtx);
	guMtxRotDeg(rotMtx, 'X', m_covers[i].angle.x + osc.x);
	guMtxConcat(rotMtx, modelMtx, modelMtx);
	guMtxRotDeg(rotMtx, 'Y', m_covers[i].angle.y + osc.y);
	guMtxConcat(rotMtx, modelMtx, modelMtx);
	guMtxTransApply(modelMtx, modelMtx, m_covers[i].pos.x + oscP.x, m_covers[i].pos.y + oscP.y + g_coverYCenter, m_covers[i].pos.z + oscP.z);
	if (mirror)
		guMtxScaleApply(modelMtx, modelMtx, 1.f, -1.f, 1.f);
	guMtxConcat(m_viewMtx, modelMtx, modelViewMtx);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	// Cover
	if (m_box)
		_drawCoverBox(i, mirror, dm);
	else
		_drawCoverFlat(i, mirror, dm);
}

STexture &CCoverFlow::_coverTexture(int i)
{
	if (!m_items[i].texture.data)
		return m_items[i].state == CCoverFlow::STATE_Loading ? m_loadingTexture : m_noCoverTexture;
	return m_items[i].texture;
}

void CCoverFlow::_drawCoverFlat(int i, bool mirror, CCoverFlow::DrawMode dm)
{
	GXTexObj texObj;
	STexture &tex = _coverTexture(m_covers[i].index);
	bool boxTex = m_items[m_covers[i].index].boxTexture && !!m_items[m_covers[i].index].texture.data;

	switch (dm)
	{
		case CCoverFlow::CFDR_NORMAL:
		{
			CColor color(m_covers[i].color);
			if (mirror)
				color.a = (u8)((float)color.a * m_mirrorAlpha);
			GX_SetTevKColor(GX_KCOLOR0, color);
			break;
		}
		case CCoverFlow::CFDR_STENCIL:
			GX_SetTevKColor(GX_KCOLOR0, CColor(i + 1, 0xFF, 0xFF, 0xFF));
			break;
		case CCoverFlow::CFDR_SHADOW:
		{
			CColor color(m_covers[i].shadowColor);
			if (mirror)
				color.a = (u8)((float)color.a * m_mirrorAlpha);
			GX_SetTevKColor(GX_KCOLOR0, color);
			break;
		}
	}
	if (dm == CCoverFlow::CFDR_NORMAL)
	{
		GX_InitTexObj(&texObj, tex.data.get(), tex.width, tex.height, tex.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		if (tex.maxLOD > 0)
			GX_InitTexObjLOD(&texObj, GX_LIN_MIP_LIN, GX_LINEAR, 0.f, (float)tex.maxLOD, mirror ? 1.f : m_lodBias, GX_FALSE, m_edgeLOD ? GX_TRUE : GX_FALSE, m_aniso);
		GX_LoadTexObj(&texObj, GX_TEXMAP0);
	}
	GX_Begin(GX_QUADS, GX_VTXFMT0, g_flatCoverMeshSize);
	for (u32 j = 0; j < g_flatCoverMeshSize; ++j)
	{
		GX_Position3f32(g_flatCoverMesh[j].pos.x, g_flatCoverMesh[j].pos.y, g_flatCoverMesh[j].pos.z);
		if (dm == CCoverFlow::CFDR_NORMAL)
		{
			if (boxTex)
				GX_TexCoord2f32(g_flatCoverBoxTex[j].x, g_flatCoverBoxTex[j].y);
			else
				GX_TexCoord2f32(g_flatCoverMesh[j].texCoord.x, g_flatCoverMesh[j].texCoord.y);
		}
	}
	GX_End();
}

void CCoverFlow::_drawCoverBox(int i, bool mirror, CCoverFlow::DrawMode dm)
{
	GXTexObj texObj;
	STexture &tex = _coverTexture(m_covers[i].index);
	CColor color;
	bool flatTex = !m_items[m_covers[i].index].boxTexture && !!m_items[m_covers[i].index].texture.data;

	switch (dm)
	{
		case CCoverFlow::CFDR_NORMAL:
			color = m_covers[i].color;
			if (mirror)
				color.a = (u8)((float)color.a * m_mirrorAlpha);
			GX_SetTevKColor(GX_KCOLOR0, color);
			break;
		case CCoverFlow::CFDR_STENCIL:
			GX_SetTevKColor(GX_KCOLOR0, CColor(i + 1, 0xFF, 0xFF, 0xFF));
			break;
		case CCoverFlow::CFDR_SHADOW:
			color = m_covers[i].shadowColor;
			if (mirror)
				color.a = (u8)((float)color.a * m_mirrorAlpha);
			GX_SetTevKColor(GX_KCOLOR0, color);
			break;
	}
	if (dm == CCoverFlow::CFDR_NORMAL)
	{
		GX_InitTexObj(&texObj, m_dvdSkin.data.get(), m_dvdSkin.width, m_dvdSkin.height, m_dvdSkin.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_LoadTexObj(&texObj, GX_TEXMAP0);
	}
	GX_Begin(GX_QUADS, GX_VTXFMT0, g_boxMeshQSize);
	for (u32 j = 0; j < g_boxMeshQSize; ++j)
	{
		GX_Position3f32(g_boxMeshQ[j].pos.x, g_boxMeshQ[j].pos.y, g_boxMeshQ[j].pos.z);
		if (dm == CCoverFlow::CFDR_NORMAL)
			GX_TexCoord2f32(g_boxMeshQ[j].texCoord.x, g_boxMeshQ[j].texCoord.y);
	}
	GX_End();
	GX_Begin(GX_TRIANGLES, GX_VTXFMT0, g_boxMeshTSize);
	for (u32 j = 0; j < g_boxMeshTSize; ++j)
	{
		GX_Position3f32(g_boxMeshT[j].pos.x, g_boxMeshT[j].pos.y, g_boxMeshT[j].pos.z);
		if (dm == CCoverFlow::CFDR_NORMAL)
			GX_TexCoord2f32(g_boxMeshT[j].texCoord.x, g_boxMeshT[j].texCoord.y);
	}
	GX_End();
	if (dm == CCoverFlow::CFDR_NORMAL)
	{
		STexture *myTex = &tex;
		if (flatTex)
			myTex = &m_noCoverTexture;
		GX_InitTexObj(&texObj, myTex->data.get(), myTex->width, myTex->height, myTex->format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		if (myTex->maxLOD > 0)
			GX_InitTexObjLOD(&texObj, GX_LIN_MIP_LIN, GX_LINEAR, 0.f, (float)myTex->maxLOD, mirror ? 1.f : m_lodBias, GX_FALSE, m_edgeLOD ? GX_TRUE : GX_FALSE, m_aniso);
		GX_LoadTexObj(&texObj, GX_TEXMAP0);
	}
	GX_Begin(GX_QUADS, GX_VTXFMT0, g_boxBackCoverMeshSize);
	for (u32 j = 0; j < g_boxBackCoverMeshSize; ++j)
	{
		GX_Position3f32(g_boxBackCoverMesh[j].pos.x, g_boxBackCoverMesh[j].pos.y, g_boxBackCoverMesh[j].pos.z);
		if (dm == CCoverFlow::CFDR_NORMAL)
			GX_TexCoord2f32(g_boxBackCoverMesh[j].texCoord.x, g_boxBackCoverMesh[j].texCoord.y);
	}
	GX_End();
	if (dm == CCoverFlow::CFDR_NORMAL && flatTex)
	{
		GX_InitTexObj(&texObj, tex.data.get(), tex.width, tex.height, tex.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		if (tex.maxLOD > 0)
			GX_InitTexObjLOD(&texObj, GX_LIN_MIP_LIN, GX_LINEAR, 0.f, (float)tex.maxLOD, mirror ? 1.f : m_lodBias, GX_FALSE, m_edgeLOD ? GX_TRUE : GX_FALSE, m_aniso);
		GX_LoadTexObj(&texObj, GX_TEXMAP0);
	}
	GX_Begin(GX_QUADS, GX_VTXFMT0, g_boxCoverMeshSize);
	for (u32 j = 0; j < g_boxCoverMeshSize; ++j)
	{
		GX_Position3f32(g_boxCoverMesh[j].pos.x, g_boxCoverMesh[j].pos.y, g_boxCoverMesh[j].pos.z);
		if (dm == CCoverFlow::CFDR_NORMAL)
		{
			if (flatTex)
				GX_TexCoord2f32(g_boxCoverFlatTex[j].x, g_boxCoverFlatTex[j].y);
			else
				GX_TexCoord2f32(g_boxCoverMesh[j].texCoord.x, g_boxCoverMesh[j].texCoord.y);
		}
	}
	GX_End();
}

void CCoverFlow::_loadCover(int i, int item)
{
	m_covers[i].index = item;
	m_covers[i].title.setText(m_font, m_items[item].title);
}

string CCoverFlow::getId(void) const
{
	if (m_covers.empty() || m_items.empty())
		return "";
	return m_items[loopNum(m_covers[m_range / 2].index + m_jump, m_items.size())].id;
}

string CCoverFlow::getNextId(void) const
{
	if (m_covers.empty() || m_items.empty())
		return "";
	return m_items[loopNum(m_covers[m_range / 2].index + m_jump + 1, m_items.size())].id;
}

string CCoverFlow::getTitle(void) const
{
	if (m_covers.empty())
		return "";
	return m_items[m_covers[m_range / 2].index].title.toUTF8();
}

bool CCoverFlow::select(void)
{
	int j;
	int curPos;
	LockMutex lock(m_mutex);

	if (m_covers.empty() || m_jump != 0)
		return false;
	if (m_selected)
		return true;
	curPos = (int)m_range / 2;
	if ((u32)m_mouse < m_range && m_mouse != curPos)
	{
		if (m_rows >= 3)
			j = ((m_mouse / m_rows - 1) * (m_rows - 2) + m_mouse % m_rows - 1)
				- ((curPos / m_rows - 1) * (m_rows - 2) + curPos % m_rows - 1);
		else
			j = m_mouse - (int)(m_range / 2);
		_setJump(j);
	}
	m_cameraPos += _cameraMoves();
	m_covers[m_range / 2].angle += _coverMovesA();
	m_covers[m_range / 2].pos += _coverMovesP();
	m_selected = true;
	m_covers[m_range / 2].angle -= _coverMovesA();
	m_covers[m_range / 2].pos -= _coverMovesP();
	m_cameraPos -= _cameraMoves();
	_updateAllTargets();
	m_selectSound.play(m_soundVolume);
	return true;
}

void CCoverFlow::cancel(void)
{
	LockMutex lock(m_mutex);
	if (m_covers.empty())
		return;
	_unselect();
	_updateAllTargets();
	m_cancelSound.play(m_soundVolume);
}

void CCoverFlow::_updateAllTargets(bool instant)
{
	m_targetCameraPos = m_selected ? m_loSelected.camera : m_loNormal.camera;
	m_targetCameraAim = m_selected ? m_loSelected.cameraAim : m_loNormal.cameraAim;
	for (u32 i = 0; i < m_range; ++i)
		_updateTarget(i, instant);
}

void CCoverFlow::_updateTarget(int i, bool instant)
{
	SLayout &lo = m_selected ? m_loSelected : m_loNormal;
	int hcenter = m_columns / 2;
	int vcenter = m_rows / 2;
	int x = i / m_rows;
	int y = i % m_rows;
	CCover &cvr = m_covers[i];

	// Left covers
	if (x < hcenter)
	{
		Vector3D deltaAngle(lo.leftDeltaAngle * (float)(hcenter - 1 - x));
		cvr.targetAngle = lo.leftAngle + deltaAngle;
		cvr.targetPos = lo.leftPos + lo.leftSpacer.rotateY(deltaAngle.y * 0.5f) * (float)(hcenter - 1 - x);
		cvr.targetScale = lo.leftScale;
		if (_invisibleCover(x, y))
		{
			cvr.targetColor = CColor(lo.endColor.r, lo.endColor.g, lo.endColor.b, 0);
			cvr.targetShadowColor = CColor(lo.shadowColorEnd.r, lo.shadowColorEnd.g, lo.shadowColorEnd.b, 0);
		}
		else
		{
			u8 a = (x - 1) * 0xFF / max(1, hcenter - 2);
			cvr.targetColor = CColor::interpolate(lo.endColor, lo.begColor, a);
			cvr.targetShadowColor = CColor::interpolate(lo.shadowColorEnd, lo.shadowColorBeg, a);
		}
		cvr.txtTargetAngle = lo.txtLeftAngle;
		cvr.txtTargetPos = lo.txtLeftPos;
		cvr.txtTargetColor = 0;
		cvr.title.setFrame(lo.txtSideWidth, lo.txtSideStyle, false, instant);
	}
	// Right covers
	else if (x > hcenter)
	{
		Vector3D deltaAngle(lo.rightDeltaAngle * (float)(x - hcenter - 1));
		cvr.targetAngle = lo.rightAngle + deltaAngle;
		cvr.targetPos = lo.rightPos + lo.rightSpacer.rotateY(deltaAngle.y * 0.5f) * (float)(x - hcenter - 1);
		cvr.targetScale = lo.rightScale;
		if (_invisibleCover(x, y))
		{
			cvr.targetColor = CColor(lo.endColor.r, lo.endColor.g, lo.endColor.b, 0);
			cvr.targetShadowColor = CColor(lo.shadowColorEnd.r, lo.shadowColorEnd.g, lo.shadowColorEnd.b, 0);
		}
		else
		{
			u8 a = (m_columns - x - 2) * 0xFF / max(1, hcenter - 2);
			cvr.targetColor = CColor::interpolate(lo.endColor, lo.begColor, a);
			cvr.targetShadowColor = CColor::interpolate(lo.shadowColorEnd, lo.shadowColorBeg, a);
		}
		cvr.txtTargetAngle = lo.txtRightAngle;
		cvr.txtTargetPos = lo.txtRightPos;
		cvr.txtTargetColor = 0;
		cvr.title.setFrame(lo.txtSideWidth, lo.txtSideStyle, false, instant);
	}
	// New center cover
	else if (y == vcenter)
	{
		cvr.targetColor = 0xFFFFFFFF;
		cvr.targetShadowColor = lo.shadowColorCenter;
		cvr.targetAngle = lo.centerAngle;
		cvr.targetPos = lo.centerPos;
		cvr.targetScale = lo.centerScale;
		cvr.txtTargetColor = 0xFF;
		cvr.txtTargetAngle = lo.txtCenterAngle;
		cvr.txtTargetPos = lo.txtCenterPos;
		cvr.title.setFrame(lo.txtCenterWidth, lo.txtCenterStyle, false, instant);
	}
	else // Center of a row
	{
		cvr.targetColor = lo.begColor;
		cvr.targetShadowColor = lo.shadowColorBeg;
		cvr.targetAngle = lo.rowCenterAngle;
		cvr.targetPos = lo.rowCenterPos;
		cvr.targetScale = lo.rowCenterScale;
		cvr.txtTargetColor = 0;
		if (y < vcenter)
		{
			cvr.txtTargetAngle = lo.txtLeftAngle;
			cvr.txtTargetPos = lo.txtLeftPos;
		}
		else // (y > vcenter)
		{
			cvr.txtTargetAngle = lo.txtRightAngle;
			cvr.txtTargetPos = lo.txtRightPos;
		}
		cvr.title.setFrame(lo.txtSideWidth, lo.txtSideStyle, false, instant);
	}
	// Top row
	if (y < vcenter)
	{
		Vector3D deltaAngle(lo.topDeltaAngle * (float)(vcenter - y - 1));
		cvr.targetAngle += lo.topAngle + deltaAngle;
		cvr.targetPos += lo.topSpacer.rotateX(deltaAngle.x * 0.5f) * (float)(vcenter - y);
		if (_invisibleCover(x, y))
		{
			cvr.targetColor = CColor(lo.endColor.r, lo.endColor.g, lo.endColor.b, 0);
			cvr.targetShadowColor = CColor(lo.shadowColorEnd.r, lo.shadowColorEnd.g, lo.shadowColorEnd.b, 0);
		}
		else
		{
			u8 a = (y - 1) * 0xFF / max(1, vcenter - 2);
			CColor c1(CColor::interpolate(lo.endColor, lo.begColor, a));
			cvr.targetColor = CColor::interpolate(c1, cvr.targetColor, 0x7F);
			CColor c2(CColor::interpolate(lo.shadowColorEnd, lo.shadowColorBeg, a));
			cvr.targetShadowColor = CColor::interpolate(c2, cvr.targetShadowColor, 0x7F);
		}
	}
	// Bottom row
	else if (y > vcenter)
	{
		Vector3D deltaAngle(lo.bottomDeltaAngle * (float)(y - vcenter - 1));
		cvr.targetAngle += lo.bottomAngle + deltaAngle;
		cvr.targetPos += lo.bottomSpacer.rotateX(deltaAngle.x * 0.5f) * (float)(y - vcenter);
		if (_invisibleCover(x, y))
		{
			cvr.targetColor = CColor(lo.endColor.r, lo.endColor.g, lo.endColor.b, 0);
			cvr.targetShadowColor = CColor(lo.shadowColorEnd.r, lo.shadowColorEnd.g, lo.shadowColorEnd.b, 0);
		}
		else
		{
			u8 a = (m_rows - y - 2) * 0xFF / max(1, vcenter - 2);
			CColor c1(CColor::interpolate(lo.endColor, lo.begColor, a));
			cvr.targetColor = CColor::interpolate(c1, cvr.targetColor, 0x7F);
			CColor c2(CColor::interpolate(lo.shadowColorEnd, lo.shadowColorBeg, a));
			cvr.targetShadowColor = CColor::interpolate(c2, cvr.targetShadowColor, 0x7F);
		}
	}
	else if (vcenter > 0 && !_invisibleCover(x, y) && x != hcenter)
	{
		cvr.targetColor = CColor::interpolate(lo.begColor, cvr.targetColor, 0x7F);
		cvr.targetShadowColor = CColor::interpolate(lo.shadowColorBeg, cvr.targetShadowColor, 0x7F);
	}
	// Mouse selection
	if (!m_selected && (u32)m_mouse < m_range)
	{
		if (i == m_mouse)
		{
			cvr.targetColor = 0xFFFFFFFF;
			cvr.targetShadowColor = lo.shadowColorCenter;
			cvr.txtTargetAngle = lo.txtCenterAngle;
			cvr.txtTargetPos = lo.txtCenterPos;
			cvr.txtTargetColor = 0xFF;
			cvr.title.setFrame(lo.txtCenterWidth, lo.txtCenterStyle, false, instant);
		}
		else
		{
			cvr.targetColor = lo.mouseOffColor;
			cvr.targetShadowColor = lo.shadowColorOff;
			cvr.txtTargetAngle = m_mouse > i ? lo.txtLeftAngle : lo.txtRightAngle;
			cvr.txtTargetPos = m_mouse > i ? lo.txtLeftPos : lo.txtRightPos;
			cvr.txtTargetColor = 0;
			cvr.title.setFrame(lo.txtSideWidth, lo.txtSideStyle, false, instant);
		}
		if (_invisibleCover(x, y))
		{
			cvr.targetColor.a = 0;
			cvr.targetShadowColor.a = 0;
		}
	}
	// 
	if (instant)
		_instantTarget(i);
}

void CCoverFlow::_instantTarget(int i)
{
	CCover &cvr = m_covers[i];

	cvr.angle = cvr.targetAngle;
	cvr.pos = cvr.targetPos;
	cvr.scale = cvr.targetScale;
	cvr.color = cvr.targetColor;
	cvr.shadowColor = cvr.targetShadowColor;
	cvr.txtAngle = cvr.txtTargetAngle;
	cvr.txtPos = cvr.txtTargetPos;
	cvr.txtColor = cvr.txtTargetColor;
}

bool CCoverFlow::start(const char *id)
{
	if (m_items.empty())
		return true;
	// Load resident textures
	if (STexture::TE_OK != m_dvdSkin.fromPNG(dvdskin_png))
		return false;
	if (m_box)
	{
		if (m_pngLoadCover.empty() || STexture::TE_OK != m_loadingTexture.fromPNGFile(m_pngLoadCover.c_str(), GX_TF_CMPR, ALLOC_COVER, 32, 512))
			if (STexture::TE_OK != m_loadingTexture.fromPNG(loading_png, GX_TF_CMPR, ALLOC_COVER, 32, 512))
				return false;
		if (m_pngNoCover.empty() || STexture::TE_OK != m_noCoverTexture.fromPNGFile(m_pngNoCover.c_str(), GX_TF_CMPR, ALLOC_COVER, 32, 512))
			if (STexture::TE_OK != m_noCoverTexture.fromPNG(nopic_png, GX_TF_CMPR, ALLOC_COVER, 32, 512))
				return false;
	}
	else
	{
		if (m_pngLoadCoverFlat.empty() || STexture::TE_OK != m_loadingTexture.fromPNGFile(m_pngLoadCoverFlat.c_str(), GX_TF_CMPR, ALLOC_COVER, 32, 512))
			if (STexture::TE_OK != m_loadingTexture.fromPNG(flatloading_png, GX_TF_CMPR, ALLOC_COVER, 32, 512))
				return false;
		if (m_pngNoCoverFlat.empty() || STexture::TE_OK != m_noCoverTexture.fromPNGFile(m_pngNoCoverFlat.c_str(), GX_TF_CMPR, ALLOC_COVER, 32, 512))
			if (STexture::TE_OK != m_noCoverTexture.fromPNG(flatnopic_png, GX_TF_CMPR, ALLOC_COVER, 32, 512))
				return false;
	}
	// Sort items
	sort(m_items.begin(), m_items.end());
	m_covers.clear();
	m_covers.resize(m_range);
	m_jump = 0;
	m_selected = false;
	if (id == 0 || !findId(id, true))
		_loadAllCovers(0);
	_updateAllTargets();
	startPicLoader();
	return true;
}

void CCoverFlow::up(void)
{
	LockMutex lock(m_mutex);
	if (m_covers.empty())
		return;
	if (m_jump != 0)
		return;
	_left(m_minDelay, 1);
}

void CCoverFlow::down(void)
{
	LockMutex lock(m_mutex);
	if (m_covers.empty())
		return;
	if (m_jump != 0)
		return;
	_right(m_minDelay, 1);
}

void CCoverFlow::left(void)
{
	LockMutex lock(m_mutex);
	if (m_covers.empty())
		return;
	if (m_jump != 0)
		return;
	_left(m_minDelay, m_rows >= 3 ? m_rows - 2 : 1);
}

void CCoverFlow::right(void)
{
	LockMutex lock(m_mutex);
	if (m_covers.empty())
		return;
	if (m_jump != 0)
		return;
	_right(m_minDelay, m_rows >= 3 ? m_rows - 2 : 1);
}

void CCoverFlow::_playSound(void)
{
	if (m_soundVolume > 0)
	{
		m_snd2 = !m_snd2;
		if (m_snd2)
			m_sound1.play(m_soundVolume);
		else
			m_sound2.play(m_soundVolume);
	}
}

void CCoverFlow::_left(int repeatDelay, u32 step)
{
	int prev;
	int arrStep;

	if (m_delay > 0)
		return;
	m_moved = true;
	m_delay = repeatDelay;
	m_covers[m_range / 2].angle += _coverMovesA();
	m_covers[m_range / 2].pos += _coverMovesP();
	if (m_rows >= 3)
	{
		prev = m_covers[m_range / 2].index;
		// Shift the array to the right
		arrStep = step % (m_rows - 2) + (step / (m_rows - 2)) * m_rows;
		for (int i = (int)m_range - 1; i >= arrStep; --i)
			m_covers[i] = m_covers[i - arrStep];
		_loadAllCovers(loopNum((int)prev - step, m_items.size()));
		_updateAllTargets();
		if (arrStep % m_rows != 0)
			for (u32 i = 0; i < m_columns; ++i)
				_instantTarget(i * m_rows);
		if (arrStep / m_rows != 0)
			for (u32 i = 0; i < m_rows; ++i)
				_instantTarget(i);
	}
	else
	{
		prev = m_covers[0].index;
		// Shift the array to the right
		for (int i = (int)m_range - 1; i > 0; --i)
			m_covers[i] = m_covers[i - 1];
		_loadCover(0, loopNum(prev - step, m_items.size()));
		_updateAllTargets();
		_instantTarget(0);
	}
	_playSound();
	m_covers[m_range / 2].angle -= _coverMovesA();
	m_covers[m_range / 2].pos -= _coverMovesP();
}

void CCoverFlow::_right(int repeatDelay, u32 step)
{
	int prev;
	u32 arrStep;

	if (m_delay > 0)
		return;
	m_moved = true;
	m_delay = repeatDelay;
	m_covers[m_range / 2].angle += _coverMovesA();
	m_covers[m_range / 2].pos += _coverMovesP();
	if (m_rows >= 3)
	{
		prev = m_covers[m_range / 2].index;
		// Shift the array to the left
		arrStep = step % (m_rows - 2) + (step / (m_rows - 2)) * m_rows;
		for (u32 i = 0; i < m_range - arrStep; ++i)
			m_covers[i] = m_covers[i + arrStep];
		_loadAllCovers(loopNum(prev + step, m_items.size()));
		_updateAllTargets();
		if (arrStep % m_rows != 0)
			for (u32 i = 0; i < m_columns; ++i)
				_instantTarget(i * m_rows + m_rows - 1);
		if (arrStep / m_rows != 0)
			for (u32 i = 0; i < m_rows; ++i)
				_instantTarget(i + (m_columns - 1) * m_rows);
	}
	else
	{
		prev = m_covers[m_range - 1].index;
		// Shift the array to the left
		for (u32 i = 0; i < m_range - 1; ++i)
			m_covers[i] = m_covers[i + 1];
		_loadCover(m_range - 1, loopNum(prev + step, m_items.size()));
		_updateAllTargets();
		_instantTarget(m_range - 1);
	}
	_playSound();
	m_covers[m_range / 2].angle -= _coverMovesA();
	m_covers[m_range / 2].pos -= _coverMovesP();
}

u32 CCoverFlow::_currentPos(void) const
{
	if (m_covers.empty())
		return 0;
	return m_covers[m_range / 2].index;
}

void CCoverFlow::mouse(CVideo &vid, int x, int y)
{
	if (m_covers.empty())
		return;
	int m = m_mouse;
	if (x < 0 || y < 0)
		m_mouse = -1;
	else
	{
		vid.prepareStencil();
		_draw(CCoverFlow::CFDR_STENCIL, false, false);
		vid.renderStencil();
		m_mouse = vid.stencilVal(x, y) - 1;
	}
	if (m != m_mouse)
	{
		if ((u32)m_mouse < m_range)
			m_hoverSound.play(m_soundVolume);
		_updateAllTargets();
	}
}

bool CCoverFlow::mouseOver(CVideo &vid, int x, int y)
{
	if (m_covers.empty())
		return false;
	vid.prepareStencil();
	_draw(CCoverFlow::CFDR_STENCIL, false, false);
	vid.renderStencil();
	vid.prepareStencil();
	_draw(CCoverFlow::CFDR_STENCIL, false, false);
	vid.renderStencil();
	return vid.stencilVal(x, y) == (int)m_range / 2 + 1;
}

bool CCoverFlow::findId(const char *id, bool instant)
{
	LockMutex lock(m_mutex);
	string strId(id);
	u32 i;
	int j;
	u32 curPos = _currentPos();

	if (m_items.empty() || (instant && m_covers.empty()))
		return false;
	// 
	for (i = 0; i < m_items.size(); ++i)
		if (m_items[i].id == strId)
			break;
	if (i >= m_items.size())
		return false;
	m_jump = 0;
	if (instant)
	{
		_loadAllCovers(i);
		_updateAllTargets();
	}
	else
	{
		j = (int)i - (int)curPos;
		if (abs(j) <= (int)m_items.size() / 2)
			_setJump(j);
		else
			_setJump(j < 0 ? j + (int)m_items.size() : j - (int)m_items.size());
	}
	return true;
}

void CCoverFlow::pageUp(void)
{
	int j;
	int n;

	if (m_covers.empty())
		return;
	if (m_rows >= 3)
	{
		j = m_jump - max(1, (int)m_columns - 2);
		if (m_jump != 0)
		{
			n = ((int)m_items.size() - 1) / (m_rows - 2) + 1;
			j = j < 0 ? -(-j % n) : j % n;
		}
		j *= (int)m_rows - 2;
	}
	else
	{
		j = m_jump - max(1, (int)m_range / 2);
		if (m_jump != 0)
		{
			n = (int)m_items.size();
			j = j < 0 ? -(-j % n) : j % n;
		}
	}
	_setJump(j);
}

void CCoverFlow::pageDown(void)
{
	int j;
	int n;

	if (m_covers.empty())
		return;
	if (m_rows >= 3)
	{
		j = m_jump + max(1, (int)m_columns - 2);
		if (m_jump != 0)
		{
			n = ((int)m_items.size() - 1) / (m_rows - 2) + 1;
			j = j < 0 ? -(-j % n) : j % n;
		}
		j *= (int)m_rows - 2;
	}
	else
	{
		j = m_jump + max(1, (int)m_range / 2);
		if (m_jump != 0)
		{
			n = (int)m_items.size();
			j = j < 0 ? -(-j % n) : j % n;
		}
	}
	_setJump(j);
}

void CCoverFlow::flip(bool force, bool f)
{
	LockMutex lock(m_mutex);
	if (m_covers.empty() || !m_selected)
		return;
	CCoverFlow::CCover &cvr = m_covers[m_range / 2];
	if (!force)
		f = m_loSelected.centerAngle == cvr.targetAngle && m_loSelected.centerPos == cvr.targetPos && m_loSelected.centerScale == cvr.targetScale;
	if (f)
	{
		cvr.targetPos = m_loSelected.centerPos + m_flipCoverPos;
		cvr.targetAngle = m_loSelected.centerAngle + m_flipCoverAngle;
		cvr.targetScale = m_loSelected.centerScale * m_flipCoverScale;
		cvr.txtTargetColor = m_flipCoverPos == Vector3D(0.f, 0.f, 0.f) ? 0xFF : 0;
	}
	else
	{
		cvr.targetPos = m_loSelected.centerPos;
		cvr.targetAngle = m_loSelected.centerAngle;
		cvr.targetScale = m_loSelected.centerScale;
		cvr.txtTargetColor = 0xFF;
	}
}

bool CCoverFlow::_invisibleCover(u32 x, u32 y)
{
	return (m_rows > 1 && (y == 0 || y == m_rows - 1)) || x == 0 || x == m_columns - 1;
}

void CCoverFlow::_loadAllCovers(int i)
{
	int r = (int)m_rows;
	int c = (int)m_columns;
	int s = (int)m_items.size();

	if (m_rows >= 3)
		for (int j = 0; j < r; ++j)
			for (int k = 0; k < c; ++k)
				_loadCover(j + k * r, loopNum(i + (j - r / 2) + (k - c / 2) * (r - 2), s));
	else
		for (int k = 0; k < (int)m_range; ++k)
			_loadCover(k, loopNum(i + k - (int)m_range / 2, s));
}

void CCoverFlow::_setJump(int j)
{
	_loadAllCovers(_currentPos());
	m_jump = j;
	if (m_jump == 0)
		_updateAllTargets();
}

void CCoverFlow::_completeJump(void)
{
	if (m_rows >= 3)
		_loadAllCovers((int)m_covers[m_range / 2].index + m_jump);
	else
	{
		if (m_jump < 0)
			_loadAllCovers((int)m_covers[0].index + m_jump + (int)m_range / 2);
		else
			_loadAllCovers((int)m_covers[m_range - 1].index + m_jump - ((int)m_range - 1) / 2);
	}
}

wchar_t CCoverFlow::nextLetter(void)
{
	LockMutex lock(m_mutex);
	u32 curPos;
	u32 n = m_items.size();
	u32 i;
	wchar_t c;

	if (m_covers.empty())
		return '\0';
	_completeJump();
	curPos = _currentPos();
	c = upperCaseWChar(m_items[curPos].title.c_str()[0]);
	for (i = 1; i < n; ++i)
		if (upperCaseWChar(m_items[loopNum(curPos + i, n)].title.c_str()[0]) != c)
			break;
	if (i < n)
	{
		_setJump(i);
		c = upperCaseWChar(m_items[loopNum(curPos + i, n)].title.c_str()[0]);
	}
	_updateAllTargets();
	return c;
}

wchar_t CCoverFlow::prevLetter(void)
{
	LockMutex lock(m_mutex);
	u32 curPos;
	u32 n = m_items.size();
	u32 i;
	wchar_t c;

	if (m_covers.empty())
		return '\0';
	_completeJump();
	curPos = _currentPos();
	c = upperCaseWChar(m_items[curPos].title.c_str()[0]);
	for (i = 1; i < n; ++i)
		if (upperCaseWChar(m_items[loopNum(curPos - i, n)].title.c_str()[0]) != c)
			break;
	if (i < n)
	{
		_setJump(-i);
		c = upperCaseWChar(m_items[loopNum(curPos - i, n)].title.c_str()[0]);
	}
	_updateAllTargets();
	return c;
}

void CCoverFlow::_coverTick(int i)
{
	float speed = m_selected ? 0.07f : 0.1f;
	Vector3D posDist(m_covers[i].targetPos - m_covers[i].pos);
	int colorDist;

	if (posDist.sqNorm() < 0.5f)
		speed *= 0.5f;
	m_covers[i].angle += (m_covers[i].targetAngle - m_covers[i].angle) * speed;
	m_covers[i].pos += posDist * speed;
	m_covers[i].scale += (m_covers[i].targetScale - m_covers[i].scale) * speed;
	if (m_covers[i].color != m_covers[i].targetColor)
	{
		CColor c(m_covers[i].color);
		m_covers[i].color = CColor::interpolate(c, m_covers[i].targetColor, 0x20);
		if (m_covers[i].color == c)	// If the interpolation doesn't do anything because of numerical approximation, force the target color
			m_covers[i].color = m_covers[i].targetColor;
	}
	if (m_covers[i].shadowColor != m_covers[i].targetShadowColor)
	{
		CColor c(m_covers[i].shadowColor);
		m_covers[i].shadowColor = CColor::interpolate(c, m_covers[i].targetShadowColor, 0x20);
		if (m_covers[i].shadowColor == c)	// If the interpolation doesn't do anything because of numerical approximation, force the target color
			m_covers[i].shadowColor = m_covers[i].targetShadowColor;
	}
	m_covers[i].txtAngle += (m_covers[i].txtTargetAngle - m_covers[i].txtAngle) * speed;
	m_covers[i].txtPos += (m_covers[i].txtTargetPos - m_covers[i].txtPos) * speed;
	colorDist = (int)m_covers[i].txtTargetColor - (int)m_covers[i].txtColor;
	m_covers[i].txtColor += abs(colorDist) >= 8 ? (u8)(colorDist / 8) : (u8)colorDist;
	m_covers[i].title.tick();
}

void CCoverFlow::_unselect(void)
{
	if (m_selected)
	{
		m_cameraPos += _cameraMoves();
		m_covers[m_range / 2].angle += _coverMovesA();
		m_covers[m_range / 2].pos += _coverMovesP();
		m_selected = false;
		m_covers[m_range / 2].angle -= _coverMovesA();
		m_covers[m_range / 2].pos -= _coverMovesP();
		m_cameraPos -= _cameraMoves();
		m_targetCameraPos = m_loNormal.camera;
		m_targetCameraAim = m_loNormal.cameraAim;
	}
}

void CCoverFlow::_jump(void)
{
	int step;
	int delay = 2;

	if (m_rows >= 3)
		step = (u32)abs(m_jump) <= (m_rows - 2) / 2 ? 1 : m_rows - 2;
	else
		// Cheat and skip covers we wouldn't see anyway
		// This means the jump shouldn't be modified before it's done completely
		step = (u32)abs(m_jump) > m_range + 1 ? abs(m_jump) - (m_range + 1) : 1;
	if (m_jump < 0)
	{
		_left(delay, step);
		m_jump += step;
	}
	else if (m_jump > 0)
	{
		_right(delay, step);
		m_jump -= step;
	}
}

void CCoverFlow::tick(void)
{
	LockMutex lock(m_mutex);
	if (m_covers.empty())
		return;
	++m_tickCount;
	if (m_delay > 0)
		--m_delay;
	else
		_jump();
	for (u32 i = 0; i < m_range; ++i)
		_coverTick(i);
	m_cameraPos += (m_targetCameraPos - m_cameraPos) * 0.2f;
	m_cameraAim += (m_targetCameraAim - m_cameraAim) * 0.2f;
}

struct SWFCHeader
{
	u8 newFmt : 1;	// Was 0 in beta
	u8 full : 1;
	u8 cmpr : 1;
	u8 zipped : 1;
	u8 backCover : 1;
	u16 width;
	u16 height;
	u8 maxLOD;
	u16 backWidth;
	u16 backHeight;
	u8 backMaxLOD;
public:
	u32 getWidth(void) const { return width * 4; }
	u32 getHeight(void) const { return height * 4; }
	u32 getBackWidth(void) const { return backWidth * 4; }
	u32 getBackHeight(void) const { return backHeight * 4; }
	SWFCHeader(void)
	{
		memset(this, 0, sizeof *this);
	}
	SWFCHeader(const STexture &tex, bool f, bool z, const STexture &backTex = STexture())
	{
		newFmt = 1;
		full = f ? 1 : 0;
		cmpr = tex.format == GX_TF_CMPR ? 1 : 0;
		zipped = z ? 1 : 0;
		width = tex.width / 4;
		height = tex.height / 4;
		maxLOD = tex.maxLOD;
		backCover = !!backTex.data ? 1 : 0;
		backWidth = backTex.width / 4;
		backHeight = backTex.height / 4;
		backMaxLOD = backTex.maxLOD;
	}
};

bool CCoverFlow::preCacheCover(const char *id, const u8 *png, bool full)
{
	STexture tex;
	u8 textureFmt = m_compressTextures ? GX_TF_CMPR : GX_TF_RGB565;
	u32 bufSize = 0;
	FILE *file;
	SmartBuf zBuffer;
	uLongf zBufferSize;

	if (m_cachePath.empty())
		return false;
	if (STexture::TE_OK != tex.fromPNG(png, textureFmt, ALLOC_COVER, 32))
		return false;
	bufSize = fixGX_GetTexBufferSize(tex.width, tex.height, tex.format, tex.maxLOD > 0 ? GX_TRUE : GX_FALSE, tex.maxLOD);
	if (m_compressCache)
	{
		zBufferSize = bufSize + bufSize / 100 + 12;
		zBuffer = smartMem2Alloc(zBufferSize);
		if (!zBuffer)
			return false;
		if (compress(zBuffer.get(), &zBufferSize, tex.data.get(), bufSize) != Z_OK)
			return false;
	}
	file = fopen(sfmt("%s/%s.wfc", m_cachePath.c_str(), id).c_str(), "wb");
	if (file == 0)
		return false;
	SWFCHeader header(tex, full, m_compressCache);
	fwrite(&header, 1, sizeof header, file);
	if (m_compressCache)
		fwrite(zBuffer.get(), 1, zBufferSize, file);
	else
		fwrite(tex.data.get(), 1, bufSize, file);
	fclose(file);
	return true;
}

bool CCoverFlow::fullCoverCached(const char *id)
{
	SWFCHeader header;
	FILE *file;
	bool found = false;

	file = fopen(sfmt("%s/%s.wfc", m_cachePath.c_str(), id).c_str(), "rb");
	if (file != 0)
	{
		found = fread(&header, 1, sizeof header, file) == sizeof header
			&& header.full != 0 && m_compressTextures == (header.cmpr != 0)
			&& header.getWidth() >= 8 && header.getHeight() >= 8
			&& header.getWidth() <= 1024 && header.getHeight() <= 1024;
		fclose(file);
	}
	return found;
}

bool CCoverFlow::_loadCoverTexPNG(u32 i, bool box, bool hq)
{
	u8 textureFmt = m_compressTextures ? GX_TF_CMPR : GX_TF_RGB565;
	STexture tex;
	u32 bufSize;
	FILE *file = 0;
	SmartBuf zBuffer;
	uLongf zBufferSize;

	if (m_waitingToClear)
		return false;
	const char *path = box ? m_items[i].boxPicPath.c_str() : m_items[i].picPath.c_str();
	if (STexture::TE_OK != tex.fromPNGFile(path, textureFmt, ALLOC_COVER, 32))
		return false;
	if (m_waitingToClear)
		return false;
	LWP_MutexLock(m_mutex);
	m_items[i].texture = tex;
	m_items[i].boxTexture = box;
	m_items[i].state = CCoverFlow::STATE_Ready;
	LWP_MutexUnlock(m_mutex);
	// Save the texture to the cache folder for the next time
	if (!m_cachePath.empty())
	{
		bufSize = fixGX_GetTexBufferSize(tex.width, tex.height, tex.format, tex.maxLOD > 0 ? GX_TRUE : GX_FALSE, tex.maxLOD);
		zBufferSize = m_compressCache ? bufSize + bufSize / 100 + 12 : bufSize;
		zBuffer = m_compressCache ? smartMalloc(zBufferSize) : tex.data;
		if (!!zBuffer && (!m_compressCache || compress(zBuffer.get(), &zBufferSize, tex.data.get(), bufSize) == Z_OK))
		{
			file = fopen(sfmt("%s/%s.wfc", m_cachePath.c_str(), m_items[i].id.c_str()).c_str(), "wb");
			if (file != 0)
			{
				SWFCHeader header(tex, box, m_compressCache);
				fwrite(&header, 1, sizeof header, file);
				fwrite(zBuffer.get(), 1, zBufferSize, file);
				fclose(file);
				if (m_deletePicsAfterCaching)
					remove(path);
			}
		}
	}
	if (!hq)
		_dropHQLOD(i);
	return true;
}

bool CCoverFlow::_calcTexLQLOD(STexture &tex)
{
	bool done = false;

	while (tex.width > 512 && tex.height > 512 && tex.maxLOD > 0)
	{
		--tex.maxLOD;
		tex.width >>= 1;
		tex.height >>= 1;
		done = true;
	}
	return done;
}

void CCoverFlow::_dropHQLOD(int i)
{
	LockMutex lock(m_mutex);
	if ((u32)i >= m_items.size())
		return;
	STexture &prevTex = m_items[i].texture;
	STexture newTex;
	u32 prevTexLen;
	u32 newTexLen;

	newTex.maxLOD = prevTex.maxLOD;
	newTex.width = prevTex.width;
	newTex.height = prevTex.height;
	newTex.format = prevTex.format;
	if (!CCoverFlow::_calcTexLQLOD(newTex))
		return;
	prevTexLen = fixGX_GetTexBufferSize(prevTex.width, prevTex.height, prevTex.format, prevTex.maxLOD > 0 ? GX_TRUE : GX_FALSE, prevTex.maxLOD);
	newTexLen = fixGX_GetTexBufferSize(newTex.width, newTex.height, newTex.format, newTex.maxLOD > 0 ? GX_TRUE : GX_FALSE, newTex.maxLOD);
	newTex.data = smartCoverAlloc(newTexLen);
	if (!newTex.data)
		return;
	memcpy(newTex.data.get(), prevTex.data.get() + (prevTexLen - newTexLen), newTexLen);
	DCFlushRange(newTex.data.get(), newTexLen);
	prevTex = newTex;
}

CCoverFlow::CLRet CCoverFlow::_loadCoverTex(u32 i, bool box, bool hq)
{
	SmartBuf ptrTex;
	bool success = false;
	FILE *file = 0;
	u32 fileSize;
	STexture tex;
	SWFCHeader header;
	u32 bufSize;
	u32 texLen;
	SmartBuf zBuffer;
	bool allocFailed = false;

	if (m_waitingToClear)
		return CL_ERROR;
	// Try to find the texture in the cache
	if (!m_cachePath.empty())
	{
		file = fopen(sfmt("%s/%s.wfc", m_cachePath.c_str(), m_items[i].id.c_str()).c_str(), "rb");
		if (file != 0)
		{
			fseek(file, 0, SEEK_END);
			fileSize = ftell(file);
			if (fileSize > sizeof header)
			{
				fseek(file, 0, SEEK_SET);
				fread(&header, 1, sizeof header, file);
				// Try to find a matching cache file, otherwise try the PNG file, otherwise try again with the cache file with less constraints
				if (header.newFmt != 0 && (((!box || header.full != 0) && (header.cmpr != 0) == m_compressTextures) || (!_loadCoverTexPNG(i, box, hq))))
				{
					tex.format = header.cmpr != 0 ? GX_TF_CMPR : GX_TF_RGB565;
					tex.width = header.getWidth();
					tex.height = header.getHeight();
					tex.maxLOD = header.maxLOD;
					bufSize = fixGX_GetTexBufferSize(tex.width, tex.height, tex.format, tex.maxLOD > 0 ? GX_TRUE : GX_FALSE, tex.maxLOD);
					if (!hq)
						CCoverFlow::_calcTexLQLOD(tex);
					texLen = fixGX_GetTexBufferSize(tex.width, tex.height, tex.format, tex.maxLOD > 0 ? GX_TRUE : GX_FALSE, tex.maxLOD);
					tex.data = smartCoverAlloc(texLen);
					ptrTex = (header.zipped != 0) ? smartMem2Alloc(bufSize) : tex.data;
					if (!ptrTex || !tex.data)
						allocFailed = true;
					else
					{
						zBuffer = (header.zipped != 0) ? smartMem2Alloc(fileSize - sizeof header) : ptrTex;
						if (!!zBuffer && ((header.zipped != 0) || fileSize - sizeof header == bufSize))
						{
							if (header.zipped == 0)
							{
								fseek(file, fileSize - sizeof header - texLen, SEEK_CUR);
								fread(tex.data.get(), 1, texLen, file);
							}
							else
								fread(zBuffer.get(), 1, fileSize - sizeof header, file);
							uLongf size = bufSize;
							if (header.zipped == 0 || (uncompress(ptrTex.get(), &size, zBuffer.get(), fileSize - sizeof header) == Z_OK && size == bufSize))
							{
								if (header.zipped != 0)
									memcpy(tex.data.get(), ptrTex.get() + bufSize - texLen, texLen);
								LockMutex lock(m_mutex);
								m_items[i].texture = tex;
								DCFlushRange(tex.data.get(), texLen);
								m_items[i].state = CCoverFlow::STATE_Ready;
								m_items[i].boxTexture = header.full != 0;
								success = true;
							}
						}
					}
				}
			}
			//
			fclose(file);
			if (success)
				return CCoverFlow::CL_OK;
		}
	}
	if (allocFailed)
		return CCoverFlow::CL_NOMEM;
	// If not found, load the PNG
	return _loadCoverTexPNG(i, box, hq) ? CCoverFlow::CL_OK : CCoverFlow::CL_ERROR;
}

int CCoverFlow::_picLoader(CCoverFlow *cf)
{
	u32 numItems;
	u32 firstItem;
	u32 lastVisible;
	u32 i;
	bool box = cf->m_box;
	CCoverFlow::CLRet ret;
	u32 bufferSize = cf->m_range + cf->m_numBufCovers * 2;
	int newHQCover = -1;

	if (!cf->m_loadingPic)
		return 0;
	while (!cf->m_waitingToClear)
	{
		ret = CCoverFlow::CL_OK;
		cf->m_moved = false;
		numItems = cf->m_items.size();
		firstItem = cf->m_covers[cf->m_range / 2].index;
		lastVisible = bufferSize - 1;
		newHQCover = firstItem;
		if ((u32)cf->m_hqCover < numItems && newHQCover != cf->m_hqCover)
		{
			cf->_dropHQLOD(cf->m_hqCover);
			cf->m_hqCover = -1;
		}
		for (u32 j = numItems - 1; j > lastVisible; --j)
		{
			i = loopNum((j & 1) != 0 ? firstItem - (j + 1) / 2 : firstItem + j / 2, numItems);
			LWP_MutexLock(cf->m_mutex);
			cf->m_items[i].texture.data.release();
			cf->m_items[i].state = CCoverFlow::STATE_Loading;
			LWP_MutexUnlock(cf->m_mutex);
		}
		for (u32 j = 0; j <= lastVisible; ++j)
		{
			if (cf->m_waitingToClear || cf->m_moved || ret == CCoverFlow::CL_NOMEM)
				break;
			i = loopNum((j & 1) != 0 ? firstItem - (j + 1) / 2 : firstItem + j / 2, numItems);
			if (cf->m_items[i].state != CCoverFlow::STATE_Loading && (i != (u32)newHQCover || newHQCover == cf->m_hqCover))
				continue;
			cf->m_hqCover = newHQCover;
			ret = cf->_loadCoverTex(i, box, i == (u32)newHQCover);
			if (ret == CCoverFlow::CL_ERROR)
			{
				ret = cf->_loadCoverTex(i, !box, i == (u32)newHQCover);
				if (ret == CCoverFlow::CL_ERROR && !cf->m_waitingToClear)
					cf->m_items[i].state = CCoverFlow::STATE_NoCover;
			}
		}
		if (ret == CCoverFlow::CL_NOMEM && bufferSize > 3)
			bufferSize -= 2;
	}
	cf->m_loadingPic = false;
	return 0;
}
