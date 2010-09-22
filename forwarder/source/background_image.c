#include <malloc.h>
#include "pngu.h"
#include "video.h"
#include "filelist.h"

static int imagewidth = 0;
static int imageheight = 0;

u8 * GetImageData()
{
    PNGUPROP imgProp;
    IMGCTX ctx;
	u8 * data = NULL;
	int ret;
//Leave this for language specific splash screens later.
/*     switch (CONF_GetLanguage()) {
        case CONF_LANG_FRENCH:
            ctx = PNGU_SelectImageFromBuffer(bk169fr_png);
            break;
        case CONF_LANG_JAPANESE:
            ctx = PNGU_SelectImageFromBuffer(bk169jp_png);
            break;
        case CONF_LANG_SPANISH:
            ctx = PNGU_SelectImageFromBuffer(bk169sp_png);
            break;
        case CONF_LANG_ITALIAN:
            ctx = PNGU_SelectImageFromBuffer(bk169it_png);
            break;
        case CONF_LANG_DUTCH:
            ctx = PNGU_SelectImageFromBuffer(bk169du_png);
            break;
        case CONF_LANG_GERMAN:
            ctx = PNGU_SelectImageFromBuffer(bk169ge_png);
            break;
        default:
            ctx = PNGU_SelectImageFromBuffer(bk169en_png);
            break;
    } */
	if(CONF_GetAspectRatio() == CONF_ASPECT_16_9)
		ctx = PNGU_SelectImageFromBuffer(background_wide_png);
	else
		ctx = PNGU_SelectImageFromBuffer(background_png);

	if (!ctx)
		return NULL;

	ret = PNGU_GetImageProperties(ctx, &imgProp);
	if (ret != PNGU_OK)
		return NULL;

    imagewidth = imgProp.imgWidth;
    imageheight = imgProp.imgHeight;

    int len = ((((imgProp.imgWidth+3)>>2)*((imgProp.imgHeight+3)>>2)*32*2) + 31) & ~31;

    data = (u8 *)memalign (32, len);

    ret = PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, data, 255);

	DCFlushRange(data, len);

	PNGU_ReleaseImageContext(ctx);

    return data;
}

void Background_Show(float x, float y, float z, u8 * data, float angle, float scaleX, float scaleY, u8 alpha)
{
/*     //16:9 to 4:3 correction if needed
    if(CONF_GetAspectRatio() != CONF_ASPECT_16_9)
    {
        scaleX *= 640.0f/720.0f;
        x += (imagewidth*scaleX - imagewidth)/2.0f;
    } */

	/* Draw image */
	Menu_DrawImg(x, y, z, imagewidth, imageheight, data, angle, scaleX, scaleY, alpha);
    Menu_Render();
}

void fadein(u8 * imgdata)
{
	int i;

	/* fadein of image */
	for(i = 0; i < 255; i = i+10)
	{
		if(i>255) i = 255;
		Background_Show(0, 0, 0, imgdata, 0, 1, 1, i);
	}
}

void fadeout(u8 * imgdata)
{
	int i;

	/* fadeoout of image */
	for(i = 255; i > 1; i = i-7)
	{
		if(i < 0) i = 0;
		Background_Show(0, 0, 0, imgdata, 0, 1, 1, i);
	}
}
