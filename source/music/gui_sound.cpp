/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <unistd.h>
#include <string.h>
#include "SoundHandler.hpp"
#include "gui_sound.h"
#include "musicplayer.h"
#include "WavDecoder.hpp"
#include "gecko/gecko.h"
#include "loader/sys.h"

#define MAX_SND_VOICES      16

using namespace std;

static bool VoiceUsed[MAX_SND_VOICES] =
{
    true, false, false, false, false, false,
    false, false, false, false, false, false,
    false, false, false, false
};

static inline int GetFirstUnusedVoice()
{
    for(int i = 1; i < MAX_SND_VOICES; i++)
    {
        if(VoiceUsed[i] == false)
            return i;
    }

    return -1;
}

extern "C" void SoundCallback(s32 voice)
{
    SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
    if(!decoder)
        return;

    if(decoder->IsBufferReady())
    {
        if(ASND_AddVoice(voice, decoder->GetBuffer(), decoder->GetBufferSize()) == SND_OK)
        {
            decoder->LoadNext();
            SoundHandler::Instance()->ThreadSignal();
        }
    }
    else if(decoder->IsEOF())
    {
        ASND_StopVoice(voice);
        if(voice == 0)
            MusicPlayer::Instance()->SetPlaybackFinished(true); //see if next music must be played
    }
    else
    {
        SoundHandler::Instance()->ThreadSignal();
    }
}

GuiSound::GuiSound()
{
	gprintf("SND: Creating GuiSound instance\n");
	Init();
}

GuiSound::GuiSound(string filepath, int v)
{
	gprintf("SND: Creating GuiSound instance for file '%s' at voice %d\n", filepath.c_str(), voice);
	voice = v;
	Init();
	Load(filepath.c_str());
}

GuiSound::GuiSound(const u8 * snd, s32 len, bool isallocated, int v)
{
	gprintf("SND: Creating GuiSound instance for buffer with length %d\n", len);
	voice = v;
	Init();
	Load(snd, len, isallocated);
}

GuiSound::GuiSound(GuiSound *g)
{
	gprintf("SND: Creating GuiSound instance from other GuiSound object\n");
	Init();
	if (g == NULL) return;
	
	if (g->sound != NULL)
	{
		u8 * snd = (u8 *) malloc(g->length);
		memcpy(snd, g->sound, length);
		Load(snd, g->length, true);
	}
	else
	{
		Load(g->filepath.c_str());
	}
}

GuiSound::~GuiSound()
{
	gprintf("SND: Destructing GuiSound object\n");
	FreeMemory();
}

void GuiSound::Init()
{
	sound = NULL;
	length = 0;

	if (voice == -1)
    voice = GetFirstUnusedVoice();
    if(voice > 0)
        VoiceUsed[voice] = true;
	
	volume = 255;
	SoundEffectLength = 0;
	loop = false;
	allocated = false;
}

void GuiSound::FreeMemory()
{
	Stop();

	// Prevent reinitialization of SoundHandler since we're exiting
	if (!Sys_Exiting())
		SoundHandler::Instance()->RemoveDecoder(voice);

    if(allocated && sound != NULL)
    {
        free(sound);
        sound = NULL;
        allocated = false;
    }
	filepath = "";

    SoundEffectLength = 0;
}

bool GuiSound::Load(const char * filepath)
{
	gprintf("SND: Load file %s\n", filepath);
    FreeMemory();

	if(!filepath)
	{
		gprintf("SND: Loading failed, no path specified\n");
        return false;
	}

    u32 magic;
    FILE * f = fopen(filepath, "rb");
    if(!f)
	{
		gprintf("SND: Loading failed, cannot open file\n");
        return false;
	}

    fread(&magic, 1, 4, f);
    fclose(f);

	gprintf("SND: Setting voice to %d\n", this->voice);
    SoundHandler::Instance()->AddDecoder(voice, filepath);

    SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
    if(!decoder)
	{
		gprintf("SND: Loading failed, no decoder found\n");
		return false;
	}
	
    if(!decoder->IsBufferReady())
    {
        SoundHandler::Instance()->RemoveDecoder(voice);
		gprintf("SND: Loading failed, buffer not ready\n");
        return false;
    }

	this->filepath = filepath;
    SetLoop(loop);

	gprintf("SND: Load file done\n");
	return true;
}

bool GuiSound::Load(const u8 * snd, s32 len, bool isallocated)
{
	gprintf("SND: Load buffer of length %d\n", len);
    FreeMemory();
	this->voice = voice;

    if(!snd)
	{
		gprintf("SND: Loading failed, empty buffer\n");
        return false;
	}

    if(!isallocated && *((u32 *) snd) == 'RIFF')
    {
		gprintf("SND: Loading sound effect\n");
        return LoadSoundEffect(snd, len);
    }

    if(*((u32 *) snd) == 'IMD5')
    {
		gprintf("SND: Uncompressing banner sound\n");
        UncompressSoundbin(snd, len, isallocated);
    }
    else
    {
        sound = (u8 *) snd;
        length = len;
        allocated = isallocated;
    }
	
	gprintf("SND: Setting voice to %d\n", this->voice);
	ghexdump(sound, 4);
    SoundHandler::Instance()->AddDecoder(this->voice, sound, length);

    SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
    if(!decoder)
	{
		gprintf("SND: Loading failed, decoder not found\n");
        return false;
	}

    if(!decoder->IsBufferReady())
    {
		gprintf("SND: Loading failed, buffer not ready\n");
        SoundHandler::Instance()->RemoveDecoder(voice);
        return false;
    }

    SetLoop(loop);

	gprintf("SND: Loading done\n");
	return true;
}

bool GuiSound::LoadSoundEffect(const u8 * snd, s32 len)
{
	FreeMemory();

    WavDecoder decoder(snd, len);
    decoder.Rewind();

    u32 done = 0;
    sound = (u8 *) malloc(4096);
    memset(sound, 0, 4096);

    while(1)
    {
        u8 * tmpsnd = (u8 *) realloc(sound, done+4096);
        if(!tmpsnd)
        {
            free(sound);
            sound = NULL;
            return false;
        }

        sound = tmpsnd;

        int read = decoder.Read(sound+done, 4096, done);
        if(read <= 0)
            break;

        done += read;
    }

    sound = (u8 *) realloc(sound, done);
    SoundEffectLength = done;
    allocated = true;

    return true;
}

void GuiSound::Play(int vol, bool restart)
{
	gprintf("SND: Playing soundeffect\n");
    if(SoundEffectLength > 0)
    {
		gprintf("SND: Setting new sound effect\n");

        ASND_StopVoice(voice);
        ASND_SetVoice(voice, VOICE_STEREO_16BIT, 32000, 0, sound, SoundEffectLength, vol, vol, NULL);
		gprintf("SND: Sound is playing\n");
        return;
    }

    if(IsPlaying() && !restart)
	{
		gprintf("SND: Playing failed: sound is already playing\n");
		return;
	}

	if(voice < 0 || voice >= 16)
	{
		gprintf("SND: Playing failed: invalid voice specified (%d)\n", voice);
		return;
	}
	
    SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
    if(!decoder)
	{
		gprintf("SND: Playing failed: no decoder found\n");
        return;
	}

	ASND_StopVoice(voice);
    if(decoder->IsEOF())
    {
        decoder->ClearBuffer();
        decoder->Rewind();
        decoder->Decode();
    }

    u8 * curbuffer = decoder->GetBuffer();
    int bufsize = decoder->GetBufferSize();
    decoder->LoadNext();
    SoundHandler::Instance()->ThreadSignal();

    ASND_SetVoice(voice, decoder->GetFormat(), decoder->GetSampleRate(), 0, curbuffer, bufsize, vol, vol, SoundCallback);
	gprintf("SND: Sound is playing\n");
}

void GuiSound::Play()
{
	Play(volume);
}

void GuiSound::Stop()
{
	if (!IsPlaying()) return;

	gprintf("SND: Stop sound, voice %d\n", voice);
	if(voice < 0 || voice >= 16)
		return;

	gprintf("SND: Stop ASND voice\n");
	ASND_StopVoice(voice);

	gprintf("SND: Find decoder...");
    SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
    if(!decoder)
	{
		gprintf("failed\n");
        return;
	}
	gprintf("done\nSND: Clear buffer\n");

    decoder->ClearBuffer();
    gprintf("SND: Rewind\n");
	Rewind();
	
	gprintf("SND: Signalling thread\n");
    SoundHandler::Instance()->ThreadSignal();
}

void GuiSound::Pause()
{
	if(voice < 0 || voice >= 16)
		return;

    ASND_StopVoice(voice);
}

void GuiSound::Resume()
{
    Play();
}

bool GuiSound::IsPlaying()
{
	if(voice < 0 || voice >= 16)
		return false;

    int result = ASND_StatusVoice(voice);

	if(result == SND_WORKING || result == SND_WAITING)
		return true;

	return false;
}

int GuiSound::GetVolume()
{
	return volume;
}

void GuiSound::SetVolume(int vol)
{
	if(voice < 0 || voice >= 16)
		return;

	if(vol < 0)
		return;

    volume = 255*(vol/100.0);
    ASND_ChangeVolumeVoice(voice, volume, volume);
}

void GuiSound::SetLoop(u8 l)
{
	loop = l;

    SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
    if(!decoder)
        return;

    decoder->SetLoop(l == 1);
}

void GuiSound::Rewind()
{
    SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
    if(!decoder)
        return;

    decoder->Rewind();
}

struct _LZ77Info
{
        u16 length : 4;
        u16 offset : 12;
} __attribute__((packed));

typedef struct _LZ77Info LZ77Info;

u8 * uncompressLZ77(const u8 *inBuf, u32 inLength, u32 * size)
{
	u8 *buffer = NULL;
	if (inLength <= 0x8 || *((const u32 *)inBuf) != 0x4C5A3737 /*"LZ77"*/ || inBuf[4] != 0x10)
		return NULL;

	u32 uncSize = le32(((const u32 *)inBuf)[1] << 8);

	const u8 *inBufEnd = inBuf + inLength;
	inBuf += 8;

	buffer = (u8 *) malloc(uncSize);

	if (!buffer)
		return buffer;

	u8 *bufCur = buffer;
	u8 *bufEnd = buffer + uncSize;

	while (bufCur < bufEnd && inBuf < inBufEnd)
	{
		u8 flags = *inBuf;
		++inBuf;
		int i = 0;
		for (i = 0; i < 8 && bufCur < bufEnd && inBuf < inBufEnd; ++i)
		{
			if ((flags & 0x80) != 0)
			{
				const LZ77Info  * info = (const LZ77Info *)inBuf;
				inBuf += sizeof (LZ77Info);
				int length = info->length + 3;
				if (bufCur - info->offset - 1 < buffer || bufCur + length > bufEnd)
					return buffer;
				memcpy(bufCur, bufCur - info->offset - 1, length);
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

	*size = uncSize;

	return buffer;
}

void GuiSound::UncompressSoundbin(const u8 * snd, int len, bool isallocated)
{
    const u8 * file = snd+32;
    if(*((u32 *) file) == 'LZ77')
    {
        u32 size = 0;
        sound = uncompressLZ77(file, len-32, &size);
        length = size;
    }
    else
    {
        length = len-32;
        sound = (u8 *) malloc(length);
        memcpy(sound, file, length);
    }

    if(isallocated)
        free((u8 *) snd);

    allocated = true;
}

void soundInit(void)
{
        ASND_Init();
        ASND_Pause(0);
}

void soundDeinit(void)
{
        ASND_Pause(1);
        ASND_End();
}
