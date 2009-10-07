// Inspired by WiiPower's "video toy", but simpler

#include "videopatch.h"

#include <string.h>

#define ARRAY_SIZE(a)	(sizeof a / sizeof a[0])

extern GXRModeObj TVNtsc480Int;

GXRModeObj TVPal528Prog = 
{
    6,      		 // viDisplayMode
    640,             // fbWidth
    528,             // efbHeight
    528,             // xfbHeight
    (VI_MAX_WIDTH_PAL - 640)/2,         // viXOrigin
    (VI_MAX_HEIGHT_PAL - 528)/2,        // viYOrigin
    640,             // viWidth
    528,             // viHeight
    VI_XFBMODE_SF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_FALSE,        // aa

    // sample points arranged in increasing Y order
	{
		{6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
		{6,6},{6,6},{6,6},  // pix 1
		{6,6},{6,6},{6,6},  // pix 2
		{6,6},{6,6},{6,6}   // pix 3
	},
	
    // vertical filter[7], 1/64 units, 6 bits each
	{
		 0,         // line n-1
		 0,         // line n-1
		21,         // line n
		22,         // line n
		21,         // line n
		 0,         // line n+1
		 0          // line n+1
	}

};

GXRModeObj TVPal528ProgSoft = 
{
    6,      		 // viDisplayMode
    640,             // fbWidth
    528,             // efbHeight
    528,             // xfbHeight
    (VI_MAX_WIDTH_PAL - 640)/2,         // viXOrigin
    (VI_MAX_HEIGHT_PAL - 528)/2,        // viYOrigin
    640,             // viWidth
    528,             // viHeight
    VI_XFBMODE_SF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_FALSE,        // aa

    // sample points arranged in increasing Y order
	{
		{6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
		{6,6},{6,6},{6,6},  // pix 1
		{6,6},{6,6},{6,6},  // pix 2
		{6,6},{6,6},{6,6}   // pix 3
	},
	
    // vertical filter[7], 1/64 units, 6 bits each
	{
		 8,         // line n-1
		 8,         // line n-1
		10,         // line n
		12,         // line n
		10,         // line n
		 8,         // line n+1
		 8          // line n+1
	}

};

GXRModeObj TVPal528ProgUnknown = 
{
    6,      		 // viDisplayMode
    640,             // fbWidth
    264,             // efbHeight
    524,             // xfbHeight
    (VI_MAX_WIDTH_PAL - 640)/2,         // viXOrigin
    (VI_MAX_HEIGHT_PAL - 528)/2,        // viYOrigin
    640,             // viWidth
    524,             // viHeight
    VI_XFBMODE_SF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_TRUE,         // aa

    // sample points arranged in increasing Y order
	{
		{3,2},{9,6},{3,10},  // pix 0, 3 sample points, 1/12 units, 4 bits each
		{3,2},{9,6},{3,10},  // pix 1
		{9,2},{3,6},{9,10},  // pix 2
		{9,2},{3,6},{9,10}   // pix 3
	},
	
    // vertical filter[7], 1/64 units, 6 bits each
	{
		 4,         // line n-1
		 8,         // line n-1
		12,         // line n
		16,         // line n
		12,         // line n
		 8,         // line n+1
		 4          // line n+1
	}

};

GXRModeObj TVMpal480Prog =
{
    10,     		 // viDisplayMode
    640,             // fbWidth
    480,             // efbHeight
    480,             // xfbHeight
    (VI_MAX_WIDTH_NTSC - 640)/2,        // viXOrigin
    (VI_MAX_HEIGHT_NTSC - 480)/2,       // viYOrigin
    640,             // viWidth
    480,             // viHeight
    VI_XFBMODE_SF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_FALSE,        // aa

    // sample points arranged in increasing Y order
    {
		{6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
		{6,6},{6,6},{6,6},  // pix 1
		{6,6},{6,6},{6,6},  // pix 2
		{6,6},{6,6},{6,6}   // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
          0,         // line n-1
          0,         // line n-1
         21,         // line n
         22,         // line n
         21,         // line n
          0,         // line n+1
          0          // line n+1
    }
};

static const GXRModeObj *g_vidmodes[] = {
	&TVNtsc480Int,
	&TVNtsc480IntDf,
	&TVNtsc480Prog,

	&TVPal528Int, 
	&TVPal528IntDf,
	&TVPal528Prog,
	&TVPal528ProgSoft,
	&TVPal528ProgUnknown,

	&TVMpal480IntDf,
	&TVMpal480Prog,

	&TVEurgb60Hz480Int,
	&TVEurgb60Hz480IntDf,
	&TVEurgb60Hz480Prog
};

void applyVideoPatch(void *dst, u32 len, GXRModeObj *rmode, int level)
{
	u32 i;
	u32 *bufEnd = (u32 *)((u8 *)dst + (len - sizeof *rmode));
	u32 *p = (u32 *)dst;
	while (p <= bufEnd)
	{
		for (i = 0; i < ARRAY_SIZE(g_vidmodes); ++i)
			if (memcmp(p, g_vidmodes[i], sizeof *rmode) == 0)
			{
				// Video mode description found, replace it
				GXRModeObj *m = (GXRModeObj *)p;
				if (level == 2
					|| (((m->viTVMode & 3) == VI_PROGRESSIVE) == ((rmode->viTVMode & 3) == VI_PROGRESSIVE)
						&& (level == 1 || m->viHeight == rmode->viHeight)))
					memcpy(p, rmode, sizeof *rmode);
				p = (u32 *)(m + 1);
				break;
			}
		if (i == ARRAY_SIZE(g_vidmodes))
			p++;
	}
}
