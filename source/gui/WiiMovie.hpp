#ifndef WII_MOVIE_H_
#define WII_MOVIE_H_

#include "gcvid.h"
#include "Timer.h"
#include "texture.hpp"

using namespace std;

class WiiMovie
{
    public:
        WiiMovie(const char * filepath);
        ~WiiMovie();
        bool Play(bool loop = false);
        void Stop();
        void SetVolume(int vol);
		void SetScreenSize(int width, int height, int top, int left);
        void SetFullscreen();
        void SetFrameSize(int w, int h);
        void SetAspectRatio(float Aspect);
		bool GetNextFrame(STexture *tex);
    protected:
        void InternalUpdate();
		static void * UpdateThread(void *arg);
		static void * PlayingThread(void *arg);
        void InternalThreadUpdate();
        void LoadNextFrame();

		lwp_t ReadThread;
		lwp_t PlayThread;
		mutex_t mutex;

        VideoFile * Video;
        Timer PlayTime;
        u32 VideoFrameCount;
        vector<STexture> Frames;
		bool Playing;
		bool ExitRequested;
		bool fullScreen;
		int SndChannels;
		int SndFrequence;
		int volume;
		
		int screentop;
		int screenleft;
		int screenwidth;
		int screenheight;
		float scaleX;
		float scaleY;
		int width;
		int height;
};

#endif
