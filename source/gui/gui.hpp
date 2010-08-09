
// Buttons

#ifndef __GUI_HPP
#define __GUI_HPP

#include "video.hpp"
#include "FreeTypeGX.h"
#include "wstringEx.hpp"
#include "smartptr.hpp"
#include "text.hpp"
#include "sound.hpp"

#include <vector>

struct SButtonTextureSet
{
	STexture left;
	STexture right;
	STexture center;
	STexture leftSel;
	STexture rightSel;
	STexture centerSel;
};

class CButtonsMgr
{
public:
	bool init(void);
	void setRumble(bool enabled) { m_rumbleEnabled = enabled; }
	void reserve(u32 capacity) { m_elts.reserve(capacity); }
	u32 addButton(SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color,
		const SButtonTextureSet &texSet, const SSoundEffect &clickSound = _noSound, const SSoundEffect &hoverSound = _noSound);
	u32 addLabel(SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, u16 style, const STexture &bg = _noTexture);
	u32 addPicButton(const u8 *pngNormal, const u8 *pngSelected, int x, int y, u32 width, u32 height,
		const SSoundEffect &clickSound = _noSound, const SSoundEffect &hoverSound = _noSound);
	u32 addPicButton(STexture &texNormal, STexture &texSelected, int x, int y, u32 width, u32 height,
		const SSoundEffect &clickSound = _noSound, const SSoundEffect &hoverSound = _noSound);
	u32 addProgressBar(int x, int y, u32 width, u32 height, SButtonTextureSet &texSet);
	void setText(u32 id, const wstringEx &text, bool unwrap = false);
	void setText(u32 id, const wstringEx &text, u32 startline, bool unwrap = false);
	void setTexture(u32 id ,STexture &bg);
	void setTexture(u32 id, STexture &bg, int width, int height);
	void setProgress(u32 id, float f, bool instant = false);
	void hide(u32 id, int dx, int dy, float scaleX, float scaleY, bool instant = false);
	void hide(u32 id, bool instant = false);
	void show(u32 id);
	void mouse(int wmote, int x, int y);
	void up(void);
	void down(void);
	void draw(void);
	void tick(void);
	void click(u32 id = (u32)-1);
	u32 selected(void) const { return m_selected; }
	void deselect(void){ m_selected = false; }
	void stopSounds(void);
	void setSoundVolume(int vol);
private:
	struct SHideParam
	{
		int dx;
		int dy;
		float scaleX;
		float scaleY;
	public:
		SHideParam(void) : dx(0), dy(0), scaleX(1.f), scaleY(1.f) { }
	};
	enum EltType {
		GUIELT_BUTTON,
		GUIELT_LABEL,
		GUIELT_PROGRESS
	};
	struct SElement
	{
		SHideParam hideParam;
		EltType t;
		bool visible;
		int x;
		int y;
		int w;
		int h;
		Vector3D pos;
		Vector3D targetPos;
		u8 alpha;
		u8 targetAlpha;
		float scaleX;
		float scaleY;
		float targetScaleX;
		float targetScaleY;
	public:
		virtual ~SElement(void) { }
		virtual void tick(void);
	protected:
		SElement(void) { }
	};
	struct SButton : public SElement
	{
		SFont font;
		SButtonTextureSet tex;
		wstringEx text;
		CColor textColor;
		float click;
		SSoundEffect clickSound;
		SSoundEffect hoverSound;
	public:
		SButton(void) { t = GUIELT_BUTTON; }
		virtual void tick(void);
	};
	struct SLabel : public SElement
	{
		SFont font;
		CText text;
		CColor textColor;
		u16 textStyle;
		STexture texBg;
	public:
		SLabel(void) { t = GUIELT_LABEL; }
		virtual void tick(void);
	};
	struct SProgressBar : public SElement
	{
		SButtonTextureSet tex;
		float val;
		float targetVal;
	public:
		SProgressBar(void) { t = GUIELT_PROGRESS; }
		virtual void tick(void);
	};
private:
	std::vector<SmartPtr<SElement> > m_elts;
	u32 m_selected;
	bool m_rumbleEnabled;
	int m_rumble;
	SSoundEffect m_sndHover;
	SSoundEffect m_sndClick;
	u8 m_soundVolume;
private:
	void _drawBtn(const SButton &b, bool selected, bool click);
	void _drawLbl(SLabel &b);
	void _drawPBar(const SProgressBar &b);
	static STexture _noTexture;
	static SSoundEffect _noSound;
};

#endif // !defined(__GUI_HPP)
