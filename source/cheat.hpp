
#ifndef __CHEAT_HPP
#define __CHEAT_HPP

#include "smartptr.hpp"

void loadCheatFile(SmartBuf &buffer, u32 &size, const char *gameId);
void loadCheat(const u8 *buffer, u32 size);

#endif	// !defined(__CHEAT_HPP)
