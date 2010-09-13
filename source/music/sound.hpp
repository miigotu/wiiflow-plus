
#ifndef __SOUND_HPP
#define __SOUND_HPP

#include <gccore.h>
#include "smartptr.hpp"

struct SSoundEffect
{
	SmartBuf data;
	u32 length;
	u32 freq;
	u8 format;
	int voice;
	u8 loopFlag;
	u32 loopStart;
	SSoundEffect(void) : data(), length(0), freq(48000), format(-1), voice(-1), loopFlag(0), loopStart(0) { }
	bool play(u8 volume);
	void stop(void);
	bool fromWAVFile(const char *filename);
	bool fromWAV(const u8 *buffer, u32 size);
	bool fromBNS(const u8 *buffer, u32 size);
	bool fromAIFF(const u8 *buffer, u32 size);
};

void soundInit(void);
void soundDeinit(void);

#endif // !defined(__SOUND_HPP)
