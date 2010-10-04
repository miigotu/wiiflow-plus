
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
	s32 voice;
	u8 loopFlag;
	u32 loopStart;
	u32 loopEnd;
	u8 volume;
	SSoundEffect(void) : data(), length(0), freq(48000), format(-1), voice(-1), loopFlag(0), loopStart(0), loopEnd(0) { }
	bool play(u8 vol, bool in_thread = false);
	static int playLoop(SSoundEffect *);
	void stop(void);
	void setVolume(u8 vol);
	u8 getVolume(void);
	bool fromWAVFile(const char *filename);
	bool fromWAV(const u8 *buffer, u32 size);
	bool fromBNS(const u8 *buffer, u32 size);
	bool fromAIFF(const u8 *buffer, u32 size);
};

void soundInit(void);
void soundDeinit(void);

#endif // !defined(__SOUND_HPP)
