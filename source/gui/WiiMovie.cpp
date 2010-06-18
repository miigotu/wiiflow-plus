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
 * WiiMovie.cpp
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <unistd.h>
#include <asndlib.h>
#include <wiiuse/wpad.h>

#include "WiiMovie.hpp"
#include "gecko.h"

#define SND_BUFFERS     20

static u8 which = 0;
static vector<s16> soundbuffer[2];
static u16 sndsize[2] = {0, 0};
static u16 MaxSoundSize = 0;

WiiMovie::WiiMovie(const char * filepath)
{
    VideoFrameCount = 0;
    ExitRequested = false;
	fullScreen = false;
    Playing = false;
    volume = 128;

    string file(filepath);
    Video = openVideo(file);
    if(!Video)
    {
//        ShowError(tr("Unsupported format!"));
        ExitRequested = true;
		return;
    }

    SndChannels = (Video->getNumChannels() == 2) ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT;
    SndFrequence = Video->getFrequency();
    MaxSoundSize = Video->getMaxAudioSamples()*2;

    LWP_MutexInit(&mutex, true);
	LWP_CreateThread (&ReadThread, UpdateThread, this, NULL, 0, LWP_PRIO_HIGHEST);
}

WiiMovie::~WiiMovie()
{
    Playing = true;
    ExitRequested = true;

    LWP_ResumeThread(ReadThread);
    LWP_JoinThread(ReadThread, NULL);
    LWP_MutexDestroy(mutex);

    ASND_StopVoice(0);

    for(u8 i = 0; i < 2; i++)
    {
        soundbuffer[i].clear();
        sndsize[i] = 0;
    }
    which = 0;

    Frames.clear();

    if(Video)
       closeVideo(Video);
}

bool WiiMovie::Play(bool loop)
{
    if(!Video)
        return false;

    Playing = true;
    PlayTime.reset();
    LWP_ResumeThread(ReadThread);

	LWP_CreateThread(&PlayThread, PlayingThread, this, NULL, 0, 70);
//    InternalUpdate();
	
	Video->loop = loop;	
	
	return true;
}

void WiiMovie::Stop()
{
	LWP_JoinThread(PlayThread, NULL);
    ExitRequested = true;
}

void WiiMovie::SetVolume(int vol)
{
    volume = 255*vol/100;
    ASND_ChangeVolumeVoice(0, volume, volume);
}

void WiiMovie::SetScreenSize(int width, int height, int top, int left)
{
	screenwidth = width;
	screenheight = height;
	screenleft = left;
	screentop = top;
}

void WiiMovie::SetFullscreen()
{
    if(!Video)
        return;

    float newscale = 1000.0f;

    float vidwidth = (float) width * 1.0f;
    float vidheight = (float) height * 1.0f;
    int retries = 100;
	fullScreen = true;

    while(vidheight * newscale > screenheight || vidwidth * newscale > screenwidth)
    {
        if(vidheight * newscale > screenheight)
            newscale = screenheight/vidheight;
        if(vidwidth * newscale > screenwidth)
            newscale = screenwidth/vidwidth;

        retries--;
        if(retries == 0)
        {
            newscale = 1.0f;
            break;
        }
    }

    scaleX = scaleY = newscale;
}

void WiiMovie::SetFrameSize(int w, int h)
{
    if(!Video)
        return;

    scaleX = (float) w /(float) width;
    scaleY = (float) h /(float) height;
}

void WiiMovie::SetAspectRatio(float Aspect)
{
    if(!Video)
        return;

    float vidwidth = (float) height*scaleY*Aspect;

    scaleX = (float) width/vidwidth;
}

extern "C" void THPSoundCallback(int)
{
    if(soundbuffer[which].size() == 0 || sndsize[which] < MaxSoundSize*(SND_BUFFERS-1))
        return;

    if(ASND_AddVoice(0, (u8*) &soundbuffer[which][0], sndsize[which]) != SND_OK)
    {
        return;
    }

    which ^= 1;

    sndsize[which] = 0;
}

void * WiiMovie::UpdateThread(void *arg)
{
	while(!((WiiMovie *) arg)->ExitRequested)
	{
        ((WiiMovie *) arg)->InternalThreadUpdate();
	}
	return NULL;
}

void * WiiMovie::PlayingThread(void *arg)
{
	((WiiMovie *) arg)->InternalUpdate();
	return NULL;
}

void WiiMovie::InternalThreadUpdate()
{
    if(!Playing)
        LWP_SuspendThread(ReadThread);

    u32 FramesNeeded = (u32) (PlayTime.elapsed()*Video->getFps());

    while(VideoFrameCount < FramesNeeded)
    {
        LWP_MutexLock(mutex);
        Video->loadNextFrame();
        LWP_MutexUnlock(mutex);

        ++VideoFrameCount;

        if(Video->hasSound())
        {
            if(sndsize[which] > MaxSoundSize*(SND_BUFFERS-1))
                return;

            if(soundbuffer[which].size() == 0)
                soundbuffer[which].resize(Video->getMaxAudioSamples()*2*SND_BUFFERS);

            sndsize[which] += Video->getCurrentBuffer(&soundbuffer[which][sndsize[which]/2])*2*2;

            if(ASND_StatusVoice(0) == SND_UNUSED && sndsize[which] >= MaxSoundSize*(SND_BUFFERS-1))
            {
                ASND_StopVoice(0);
                ASND_SetVoice(0, SndChannels, SndFrequence, 0, (u8 *) &soundbuffer[which][0], sndsize[which], volume, volume, THPSoundCallback);
                which ^= 1;
            }
        }
    }

    usleep(100);
}

void WiiMovie::LoadNextFrame()
{
    if(!Video || !Playing)
    {
        usleep(100);
        return;
    }

    VideoFrame VideoF;
    LWP_MutexLock(mutex);
    Video->getCurrentFrame(VideoF);
    LWP_MutexUnlock(mutex);

    if(!VideoF.getData())
        return;

    if(width != VideoF.getWidth())
    {
        width = VideoF.getWidth();
        height = VideoF.getHeight();
		if (fullScreen) {
			SetFullscreen();
		} else {
			// Calculate new top and left
			screenleft = (screenwidth - width) / 2;
			screentop = (screenheight - height) / 2;
		}
    }

	STexture frame;
	if (frame.fromRAW(VideoF.getData(), VideoF.getWidth(), VideoF.getHeight()) == STexture::TE_OK)
		Frames.push_back(frame);
}

void WiiMovie::InternalUpdate()
{
    while(!ExitRequested)
    {
        LoadNextFrame();

        while(Frames.size() > 10 && !ExitRequested)
            usleep(100);
    }
}

bool WiiMovie::GetNextFrame(STexture *tex)
{
	if (!Video || Frames.size() == 0)
		return false;
	
	*tex = Frames.at(0);
	Frames.erase(Frames.begin());
	return true;
/*	
	bool retval = false;
	
	if (Frames.size() > 3)
	{
		*tex = Frames.at(3);
		retval = true;
	}
	
	if (Frames.size() > 4)
	{
		Frames.erase(Frames.begin());
	}
	return retval;
*/
}
